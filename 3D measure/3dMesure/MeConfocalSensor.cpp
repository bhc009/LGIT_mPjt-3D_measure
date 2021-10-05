/*
[20170222-�ֵ���]
�� ����ġ ��ȿ�� ����

[20170127-�ֵ���]
�� gap range ��ȸ �Լ� �߰�

[20170116-�ֵ���
�� ������ ����ġ(float�� overflow ��)�� ���� ���͸� ����

[20161213-�ֵ���]
�� GAP ��� ��� ���� ����
- ����Ÿ ����(N) ��� (N-1)�� ����

[20161207-�ֵ���]
*/

//
#pragma once

//
#include "stdafx.h"
#include "MeConfocalSensor.h"
#include "MeDaqLib.h"
#include "djString.h"

//
#define DEF_BUFFER_LEN		1000

//
CMeConfocalSensor::CMeConfocalSensor()
{
	//
	m_nSensorType = NO_SENSOR;

	//
	m_hInstance = NULL;
	m_bConnected = false;
	m_bSensing = false;
	m_bUpdated = false;

	//
	m_fSamplingRate = 0.0f;
	m_nSamplingNum = 0;

	m_pDataBuf = NULL;
	m_nDataBufLen = 0;
	m_nDataNum = 0;

	m_fGapMin = 0.0;
	m_fGapMax = 0.0;
	m_fGapAvg = 0.0;
	m_fGapPeak2Peak = 0.0;
	m_fGapMaxError = 0.0;
	m_fGapMinError = 0.0;
	m_fGapAvgError = 0.0;

	m_bGapOutOfRange = true;

	m_fOutOfRangeVal = -1.0;
	m_fValRangeMax = 3.0;
	m_fValRangeMin = -1.0;
	m_fValTolerance = 0.001;

	//
	m_pThread = NULL;
	m_bCoutinousSensing = false;
	m_bStopSensing = true;
}

CMeConfocalSensor::~CMeConfocalSensor()
{
	//
	Disconnect();

	//������ ����Ÿ�� ���������� ����
	if (m_pDataBuf != NULL)
	{
		delete[] m_pDataBuf;
		m_pDataBuf = NULL;

		m_nDataBufLen = 0;
		m_nDataNum = 0;
	}
}

//
bool CMeConfocalSensor::Connect(const CString strConInfo/*type, ip, interface, sample rate(hz), range min, range max, val. for out of range*/)

{
	//
	if (m_bConnected == true)
		return false;


	ERR_CODE nRet = ERR_NOERROR;



	//
	m_hInstance = ::CreateSensorInstByName(_T("DT6220"));

	if( !m_hInstance )
	{
		if( m_hInstance )
		{
			::CloseSensor(m_hInstance);
			::ReleaseSensorInstance(m_hInstance);
		}

		m_bConnected = false;

		return false;
	}



	//
	nRet = ::SetParameterInt( m_hInstance, _T("IP_EnableLogging"), 1);

	nRet = ::OpenSensorTCPIP(m_hInstance,strConInfo);

	if( !m_hInstance )
	{
		if( m_hInstance )
		{
			::CloseSensor(m_hInstance);
			::ReleaseSensorInstance(m_hInstance);
		}

		m_bConnected = false;

		return false;
	}



	m_bConnected = true;
	m_bSensing = false;
	m_bUpdated = false;

	//������ ����Ͽ� ���� �Ҵ�
	if (m_pDataBuf == NULL)
	{
		m_nDataBufLen = DEF_BUFFER_LEN;
		m_pDataBuf = new double[m_nDataBufLen];
		m_nDataNum = 0;
	}

	//������ ����Ͽ� ������ ����
	if (m_pThread == NULL)
	{
		m_pThread = new CdjThreadObj;
		m_pThread->Initialize(OnSensingThreadFuncWrapper, NULL, NULL, this, 30/*30hz*/);
	}


	//
	return true;
}

bool CMeConfocalSensor::Disconnect()
{
	//
	StopSensing();

	//������ ����
	if (m_pThread != NULL)
	{
		if (m_pThread->IsInitialized())
		{
			m_pThread->Finalize(TRUE);
		}

		delete m_pThread;
		m_pThread = NULL;
	}

	//��� ����
	if (m_bConnected)
	{
		::CloseSensor(m_hInstance);
		m_bConnected = false;
	}

	//�ν��Ͻ� ����
	if (m_hInstance != NULL)
	{
		::ReleaseSensorInstance(m_hInstance);
		m_hInstance = NULL;
	}

	//���۴� ���� ����ġ ������ ���� ���ܵ�. �Ҹ��ڿ��� ������

	//
	return true;
}

inline bool CMeConfocalSensor::IsConnected()
{
	return m_bConnected;
}

//
bool CMeConfocalSensor::StartContinousSensing()
{
	//
	CSingleLock lock(&m_Lock, TRUE);

	//
	if (!m_bConnected)
		return false;

	if (m_bSensing)
		return false;

	//���� �Ҵ�
	if (m_pDataBuf != NULL && m_nDataBufLen < DEF_BUFFER_LEN)
	{
		delete[] m_pDataBuf;
		m_pDataBuf = NULL;

		m_nDataBufLen = 0;
		m_nDataNum = 0;
	}

	if (m_pDataBuf == NULL)
	{
		m_nDataBufLen = DEF_BUFFER_LEN;
		m_nDataNum = 0;

		m_pDataBuf = new double[m_nDataBufLen];
	}

	//
	m_nDataNum = 0;
	m_bSensing = true;
	m_bCoutinousSensing = true;
	m_bStopSensing = false;

	//
	return true;
}

//
double CMeConfocalSensor::sensing(int nTime)
{
	bool bDone = false;
	ERR_CODE err;
	int ca = 0;
	int avail = 0;
	int rawData[9999];
	double scaledData[9999];
	int gbs = 0;


	if( m_bConnected )
	{
		avail = 0;

		err = DataAvail(m_hInstance, &avail);

		if( avail<3 )
		{
			return 0;
		}

		err = TransferData( m_hInstance, rawData, scaledData, 9999, &gbs );

		ExecSCmd(m_hInstance, "Clear_Buffers");
	}

	return scaledData[2];

/*
	// ���� ������ ����
	{
		ca = 0;

		err = DataAvail(m_hInstance, &ca);

		if( ca > 0 )
		{
			int *rawData = new int [ca];
			double *scaleData = new double[ca];
			int ebs = ca;
			int gbs = 0;
			err = TransferData( m_hInstance, rawData, scaleData, ebs, &gbs );

			CString str;
			str.Format("���� ������[����] = %d\n", ca );
			OutputDebugString(str);

			delete[] rawData;
			delete[] scaleData;
		}
	}


	int cnt=0;
	double *pAvgData = new double[nTime];

	while(cnt<nTime)
	{
		ca = 0;

		err = DataAvail(m_hInstance, &ca);

		if( ca > 100 )
		{
			int *rawData = new int [ca];
			double *scaleData = new double[ca];
			int ebs = ca;
			int gbs = 0;
			err = TransferData( m_hInstance, rawData, scaleData, ebs, &gbs );

			CString str;
			str.Format("���� ������ = %d\n", ca );
			OutputDebugString(str);

			double dSum=0;
			for( int i=0 ; i<gbs ; i++ )
			{
				dSum += scaleData[i];
			}
			pAvgData[cnt] = dSum/gbs;

			delete[] rawData;
			delete[] scaleData;

			cnt++;
		}

		Sleep(100);
	}


	double dSum = 0;
	for( int i=0 ; i<cnt ; i++ )
	{
		dSum += pAvgData[i];
	}

	delete[] pAvgData;


	//
	return dSum/cnt;*/
}



bool CMeConfocalSensor::StartSingleSensing(int nSamplingNum)
{
	//
	CSingleLock lock(&m_Lock, TRUE);

	//
	if (!m_bConnected)
		return false;

	if (m_bSensing)
		return false;

	//���� �Ҵ�
	if (m_pDataBuf != NULL && m_nDataBufLen < nSamplingNum)
	{
		delete[] m_pDataBuf;
		m_pDataBuf = NULL;

		m_nDataBufLen = 0;
		m_nDataNum = 0;
	}

	if (m_pDataBuf == NULL)
	{
		m_nDataBufLen = max(nSamplingNum, DEF_BUFFER_LEN);
		m_nDataNum = 0;

		m_pDataBuf = new double[m_nDataBufLen];
	}

	//
	m_nSamplingNum = nSamplingNum;
	m_nDataNum = 0;
	m_bSensing = true;
	m_bCoutinousSensing = false;
	m_bStopSensing = false;

	//
	return true;
}

bool CMeConfocalSensor::StopSensing()
{
	//
	CSingleLock lock(&m_Lock, TRUE);

	//
	if (m_bCoutinousSensing)
	{
		m_bStopSensing = true;

		return true;
	}
	else
	{
		return false;
	}
}

inline bool CMeConfocalSensor::IsGapSensing()
{
	return m_bSensing;
}

inline bool CMeConfocalSensor::IsContinousSensing()
{
	return m_bCoutinousSensing;
}

inline bool CMeConfocalSensor::SetSamplingRate(float fHz)
{
	if (fHz > 0.0f)
	{
		m_fSamplingRate = fHz;

		return true;
	}
	else
	{
		return false;
	}
}

inline float/*Hz*/ CMeConfocalSensor::GetSamplingRate()
{
	return m_fSamplingRate;
}

inline float CMeConfocalSensor::GetGapRangeMin()
{
	return m_fValRangeMin;
}

inline float CMeConfocalSensor::GetGapRangeMax()
{
	return m_fValRangeMax;
}

inline float CMeConfocalSensor::GetGapTolerance()
{
	return m_fValTolerance;
}

//
inline bool CMeConfocalSensor::IsGapUpdated()
{
	return m_bUpdated;;
}

inline float CMeConfocalSensor::GetGap()
{
	return m_fGapAvg;
}

inline float CMeConfocalSensor::GetGapAvg()
{
	return m_fGapAvg;
}

inline float CMeConfocalSensor::GetGapMax()
{
	return m_fGapMax;
}

inline float CMeConfocalSensor::GetGapMin()
{
	return m_fGapMin;
}

inline float CMeConfocalSensor::GetGapPeakToPeak()
{
	return m_fGapPeak2Peak;
}

inline float CMeConfocalSensor::GetGapMaxError()
{
	return m_fGapMaxError;
}

inline float CMeConfocalSensor::GetGapMinError()
{
	return m_fGapMinError;
}

inline float CMeConfocalSensor::GetGapAvgError()
{
	return m_fGapAvgError;
}

inline bool CMeConfocalSensor::IsGapOutOfRange()
{
	return m_bGapOutOfRange;
}

bool CMeConfocalSensor::IsGapInPos(float fGap, float fTolerance/*�̸�*/)
{
	if (!m_bGapOutOfRange)
	{
		float fCurGap = GetGap();
		float fDiff = fabs(fCurGap - fGap);
		float fTol = (fTolerance >= 0.0f) ? fTolerance : m_fValTolerance;

		if (fDiff < fTol)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

bool CMeConfocalSensor::IsGapInPos(float fCurGap, float fTgtGap, float fTolerance/*�̸�*/)
{
	float fDiff = fabs(fTgtGap - fCurGap);
	float fTol = (fTolerance >= 0.0f) ? fTolerance : m_fValTolerance;

	if (fDiff < fTol)
		return true;
	else
		return false;
}

bool CMeConfocalSensor::IsGapLower(float fGap, float fRefGap)
{
	if (!IsGapSame(fGap, fRefGap))
	{
		if (fGap < fRefGap)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

bool CMeConfocalSensor::IsGapHigher(float fGap, float fRefGap)
{
	if (!IsGapSame(fGap, fRefGap))
	{
		if (fGap > fRefGap)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

bool CMeConfocalSensor::IsGapSame(float fGap, float fRefGap)
{
	float fDiff = (float)fabs(fGap - fRefGap);
	float fDiffTol = 0.001f;

	if (fDiff < fDiffTol)
		return true;
	else
		return false;
}


int /*data ����*/ CMeConfocalSensor::CaptureGapData(double * pBuf, int nBufSize)
{
	//
	CSingleLock lock(&m_Lock, TRUE);

	//
	if (pBuf == NULL || nBufSize <= 0)
		return 0;

	//
	int iCaptureStartPos = (m_nDataNum >= nBufSize) ? (m_nDataNum - nBufSize) : 0;
	int nCaptureDataNum = m_nDataNum - iCaptureStartPos;

	::memcpy(pBuf, &m_pDataBuf[iCaptureStartPos], nCaptureDataNum);

	//
	return nCaptureDataNum;
}

DWORD CMeConfocalSensor::OnSensingThreadFuncWrapper(LPVOID pUserObj)
{
	CMeConfocalSensor * pThis = (CMeConfocalSensor *)pUserObj;

	return pThis->OnSensingThreadFunc();
}

DWORD CMeConfocalSensor::OnSensingThreadFunc()
{
	//
	CSingleLock lock(&m_Lock, TRUE);

	//
	if (m_bConnected == false)
		return 0;

	if (m_bStopSensing == true)
	{
		m_bSensing = false;
		m_bCoutinousSensing = false;

		return 0;
	}

	if (m_bCoutinousSensing == true) //���� ����
	{
		int nTransferedDataNum = 0;
		ERR_CODE nRet = ::TransferData(m_hInstance, NULL, &m_pDataBuf[m_nDataNum], m_nDataBufLen - m_nDataNum, &nTransferedDataNum);

		if (nRet == ERR_NOERROR)
		{
			if (nTransferedDataNum > 0)
			{
				//
				m_bUpdated = true;

				//
				m_nDataNum += nTransferedDataNum;
				
				//
				int nRollingThr = (int)((float)m_nDataBufLen * 0.7f);
				int nKeepThr = (int)((float)m_nDataBufLen * 0.3f);

				if (m_nDataNum > nRollingThr)
				{
					int iKeepStartPos = m_nDataNum - nKeepThr;
					int nCopyNum = m_nDataNum - iKeepStartPos;

					::memcpy(m_pDataBuf, &m_pDataBuf[iKeepStartPos], sizeof(double) * nCopyNum);
					m_nDataNum = nCopyNum;
				}

				//
				float fTimeThr = 0.1f; //0.1��, ���� ��忡���� �� ������ ���� ������...
				int nMinSampleNum = 10;  //���� sampling rate�� ���� ����
				int nMaxSampleNum = 100; //���� sampling rate�� ���� ����
				int nAnalyzeThr = max(nMinSampleNum, min(nMaxSampleNum, (int)(m_fSamplingRate * fTimeThr)));

				if (m_nDataNum >= nAnalyzeThr)
				{
					//
					AnalyzeGap(&m_pDataBuf[m_nDataNum - nAnalyzeThr], nAnalyzeThr);
				}
			}
		}
		else
		{
			//���� ��ġ ����
			//���������� ����Ÿ�� ��ȸ���� �ʴ� ��찡 ����. ������ �ֱ�� ����ȵ�..
		}

		//
		//TRACE(_T("[GAP]nRet:%d nRcvData:%d nTotalData:%d Avg:%.3f Min:%.3f Max:%.3f P2P:%.3f Avg.Err:%.3f Max.Err:%.3f Gap.Min.Err:%.3f\n"), nRet, nTransferedDataNum, m_nDataNum, m_fGapAvg, m_fGapMax, m_fGapMin, m_fGapPeak2Peak, m_fGapAvgError, m_fGapMaxError, m_fGapMinError);
	}
	else //if(m_bCoutinousSensing == false)  //1ȸ ����
	{
		int nTransferedDataNum = 0;
		int *pA  = new int [1000];
		//ERR_CODE nRet = ::TransferData(m_hInstance, NULL, &(m_pDataBuf[m_nDataNum]), m_nSamplingNum - m_nDataNum, &nTransferedDataNum);
		ERR_CODE nRet = ::TransferData(m_hInstance, pA, &(m_pDataBuf[m_nDataNum]), m_nSamplingNum - m_nDataNum, &nTransferedDataNum);

		delete[] pA;;

		if (nRet == ERR_NOERROR)
		{
			if (nTransferedDataNum > 0)
			{
				//
				m_bUpdated = true;

				//
				m_nDataNum += nTransferedDataNum;

				if (m_nDataNum >= m_nSamplingNum)
				{
					//
					AnalyzeGap(m_pDataBuf, m_nDataNum);

					//
					m_bStopSensing = true;
					m_bSensing = false;
				}
			}

			//
			//TRACE(_T("[GAP]Time:%d nRet:%d nRcvData:%d nTotalData:%d Avg:%.3f Min:%.3f Max:%.3f P2P:%.3f Avg.Err:%.3f Max.Err:%.3f Gap.Min.Err:%.3f\n"), ::GetTickCount(), nRet, nTransferedDataNum, m_nDataNum, m_fGapAvg, m_fGapMax, m_fGapMin, m_fGapPeak2Peak, m_fGapAvgError, m_fGapMaxError, m_fGapMinError);
		}
		else
		{
			//���� ��ġ ����
			//���������� ����Ÿ�� ��ȸ���� �ʴ� ��찡 ����. ������ �ֱ�� ����ȵ�..
		}
	}

	//
	return 0;
}

bool CMeConfocalSensor::AnalyzeGap(double * pBuf, int nDataNum)
{
	/*
	���, �ִ�, �ּ� ���
	*/

	//
	if (m_pDataBuf == NULL || nDataNum <= 0)
		return false;

	//
	double fSum = 0.0;
	double fAvg = 0.0;
	double fMax = pBuf[0];
	double fMin = pBuf[0];
	double fPeak2Peak = 0.0;
	float fValidDataRatioThr = 0.5f;
	int nValidDataNumThr = max(1, nDataNum * fValidDataRatioThr);
	int nValidDataNum = 0;
	int nOutOfRangeDataNum = 0;
	bool bValidData = false;

	for (int i = 0; i < nDataNum; i++)
	{
		double fVal = pBuf[i];

		if (m_fValRangeMin <= fVal && fVal <= m_fValRangeMax) //�����Ⱚ ���͸� ����. ��Ȥ ��ô ū ��(float�� ���� �ʰ�)�� ������.
		{
			fSum += fVal;
			fMax = max(fMax, fVal);
			fMin = min(fMin, fVal);

			nValidDataNum++;
		}
	}

	nOutOfRangeDataNum = nDataNum - nValidDataNum;
	
	if (nValidDataNum >= nValidDataNumThr)
		bValidData = true;

	if (!bValidData)
	{
		m_fGapAvg = m_fOutOfRangeVal;
		m_fGapMax = m_fOutOfRangeVal;
		m_fGapMin = m_fOutOfRangeVal;
		m_fGapPeak2Peak = m_fOutOfRangeVal;
		m_fGapAvgError = m_fOutOfRangeVal;
		m_fGapMaxError = m_fOutOfRangeVal;
		m_fGapMinError = m_fOutOfRangeVal;

		m_bGapOutOfRange = true;

		return true;
	}

	//
	fAvg = fSum / (double)nValidDataNum;
	fPeak2Peak = fMax - fMin;

	//
	double fMaxErrorSum = 0.0;
	double fMinErrorSum = 0.0;
	double fAvgErrorSum = 0.0;
	double fMaxError = 0.0;
	double fMinError = 0.0;
	double fAvgError = 0.0;

	for (int i = 0; i < nDataNum; i++)
	{
		double fVal = pBuf[i];

		if (m_fValRangeMin <= fVal && fVal <= m_fValRangeMax) //�����Ⱚ ���͸� ����. ��Ȥ ��ô ū ��(float�� ���� �ʰ�)�� ������.
		{
			fMaxErrorSum += fMax - fVal;
			fMinErrorSum += fVal - fMin;
			fAvgErrorSum += (fVal >= fAvg) ? (fVal - fAvg) : (fAvg - fVal);
		}
	}

	fMaxError = fMaxErrorSum / (double)nValidDataNum;
	fMinError = fMinErrorSum / (double)nValidDataNum;
	fAvgError = fAvgErrorSum / (double)nValidDataNum;

	//
	m_fGapAvg = fAvg;
	m_fGapMax = fMax;
	m_fGapMin = fMin;
	m_fGapPeak2Peak = fPeak2Peak;
	m_fGapAvgError = fAvgError;
	m_fGapMaxError = fMaxError;
	m_fGapMinError = fMinError;

	m_bGapOutOfRange = false;

	//
	return true;
}