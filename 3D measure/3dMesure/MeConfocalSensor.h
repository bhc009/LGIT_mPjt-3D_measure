/*
[20170222-최동진]
ㅁ 측정치 유효성 관리

[20170127-최동진]
ㅁ gap range 조회 함수 추가

[20170116-최동진
ㅁ 비정상 측정치(float형 overflow 값)에 대한 필터링 적용

[20161213-최동진]
ㅁ GAP 평균 계산 오류 정정
   - 데이타 개수(N) 대신 (N-1)로 나눔

[20161207-최동진]
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
	int m_nSamplingNum; //single 모드에서만 사용

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

	virtual bool IsGapInPos(float fGap, float fTolerance/*미만, < 0.0 이며 기본값 적용*/);
	virtual bool IsGapInPos(float fCurGap, float fTgtGap, float fTolerance/*미만, < 0.0 이며 기본값 적용*/);

	virtual bool IsGapLower(float fGap, float fRefGap);
	virtual bool IsGapHigher(float fGap, float fRefGap);
	virtual bool IsGapSame(float fGap, float fRefGap);

	//
	virtual int /*data 개수*/ CaptureGapData(double * pBuf, int nBufSize);

protected:
	static DWORD OnSensingThreadFuncWrapper(LPVOID pUserObj);
	DWORD OnSensingThreadFunc();

protected:
	bool AnalyzeGap(double * pBuf, int nDataNum);
};