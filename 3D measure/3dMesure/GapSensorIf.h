/*
[20170127-�ֵ���]
�� gap range ��ȸ �Լ� �߰�

[20161207-�ֵ���]
*/

//
#pragma once


#include "../stdafx.h"

//
class CGapSensorIf
{
public:
	virtual ~CGapSensorIf()
	{

	}

public:
	//
	virtual bool Connect(const CString strConInfo) = 0;
	virtual bool Disconnect() = 0;
	virtual bool IsConnected() = 0;

	//
	virtual bool StartContinousSensing() = 0; //���� ��û�� ������ ��� ���ø�
	virtual bool StartSingleSensing(int nSamplingNum) = 0; //���� ���� ���ø� �� ���� ����
	virtual bool StopSensing() = 0;
	virtual bool IsGapSensing() = 0;
	virtual bool IsContinousSensing() = 0;

	virtual bool SetSamplingRate(float fHz) = 0;
	virtual float/*Hz*/ GetSamplingRate() = 0;

	virtual float GetGapRangeMin() = 0;
	virtual float GetGapRangeMax() = 0;
	virtual float GetGapTolerance() = 0;

	//
	virtual bool IsGapUpdated() = 0;

	virtual float GetGap() = 0;
	virtual float GetGapAvg() = 0;
	virtual float GetGapMax() = 0;
	virtual float GetGapMin() = 0;
	virtual float GetGapPeakToPeak() = 0;
	virtual float GetGapMaxError() = 0;
	virtual float GetGapMinError() = 0;
	virtual float GetGapAvgError() = 0;

	virtual bool IsGapOutOfRange() = 0;

	virtual bool IsGapInPos(float fGap, float fTolerance/*�̸�*/) = 0; //���� gap�� fGap�� �������� Ȯ��
	virtual bool IsGapInPos(float fCurGap, float fTgtGap, float fTolerance/*�̸�*/) = 0; //���� gap�� ��ǥ gap�� �������� Ȯ��

	virtual bool IsGapLower(float fGap, float fRefGap) = 0;
	virtual bool IsGapHigher(float fGap, float fRefGap) = 0;
	virtual bool IsGapSame(float fGap, float fRefGap) = 0;

	//
	virtual int /*data ����*/ CaptureGapData(double * pBuf, int nBufSize) = 0;
};