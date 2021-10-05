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
#include "../stdafx.h"
#include <afxmt.h>
#include "GapSensorIf.h"
#include "djThreadObj.h"

//
class CMeConfocalSensor : public CGapSensorIf
{
protected:
	//
	int m_nSensorType;
	CString m_strIp;
	CString m_strInterface;

	//
	DWORD m_hInstance;
	bool m_bConnected;
	bool m_bSensing;
	bool m_bUpdated;

	//
	float m_fSamplingRate; //hz
	int m_nSamplingNum; //single ��忡���� ���

	double * m_pDataBuf;
	int m_nDataBufLen;
	int m_nDataNum;

	double m_fGapMin, m_fGapMax, m_fGapAvg, m_fGapPeak2Peak;
	double m_fGapMaxError, m_fGapMinError, m_fGapAvgError;
	bool m_bGapOutOfRange;

	double m_fOutOfRangeVal;
	double m_fValRangeMax, m_fValRangeMin;
	double m_fValTolerance;

	//
	CdjThreadObj * m_pThread;
	bool m_bCoutinousSensing;
	bool m_bStopSensing;
	
	//
	CCriticalSection m_Lock;

public:
	CMeConfocalSensor();
	CMeConfocalSensor(const CMeConfocalSensor &);
	virtual ~CMeConfocalSensor();

	CMeConfocalSensor & operator=(const CMeConfocalSensor &);

public:
	//
	virtual bool Connect(const CString strConInfo/*type, ip, interface, sample rate(hz), range min, range max, val. for out of range, tolerance*/);
	virtual bool Disconnect();
	virtual bool IsConnected();

	//
	virtual bool StartContinousSensing();
	virtual double sensing(int nTime);	// LBH
	virtual bool StartSingleSensing(int nSamplingNum);
	virtual bool StopSensing();
	virtual bool IsGapSensing();
	virtual bool IsContinousSensing();

	virtual bool SetSamplingRate(float fHz);
	virtual float/*Hz*/ GetSamplingRate();

	virtual float GetGapRangeMin();
	virtual float GetGapRangeMax();
	virtual float GetGapTolerance();

	//
	virtual bool IsGapUpdated();

	virtual float GetGap();
	virtual float GetGapAvg();
	virtual float GetGapMax();
	virtual float GetGapMin();
	virtual float GetGapPeakToPeak();
	virtual float GetGapMaxError();
	virtual float GetGapMinError();
	virtual float GetGapAvgError();
	
	virtual bool IsGapOutOfRange();

	virtual bool IsGapInPos(float fGap, float fTolerance/*�̸�, < 0.0 �̸� �⺻�� ����*/);
	virtual bool IsGapInPos(float fCurGap, float fTgtGap, float fTolerance/*�̸�, < 0.0 �̸� �⺻�� ����*/);

	virtual bool IsGapLower(float fGap, float fRefGap);
	virtual bool IsGapHigher(float fGap, float fRefGap);
	virtual bool IsGapSame(float fGap, float fRefGap);

	//
	virtual int /*data ����*/ CaptureGapData(double * pBuf, int nBufSize);

protected:
	static DWORD OnSensingThreadFuncWrapper(LPVOID pUserObj);
	DWORD OnSensingThreadFunc();

protected:
	bool AnalyzeGap(double * pBuf, int nDataNum);
};