#pragma once

// #include "3D measureDlg.h"

// class CMy3DmeasureDlg;
class CBhThread
{
public:
	CBhThread(void);
	~CBhThread(void);

	HANDLE m_hEventMeasure;
	HANDLE m_hEventAutofocus;
	HANDLE m_hEventCalibration;
	HANDLE m_hEventChangeLens;
};

