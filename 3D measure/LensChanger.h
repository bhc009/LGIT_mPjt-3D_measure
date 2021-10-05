#pragma once

#include "revolver/FAS_EziMOTIONPlusR.h"

#define LENS_CHANGE_POS_DEFAULT	-1

class CLensChanger
{
public:
	CLensChanger(void);
	~CLensChanger(void);

	bool m_bConnection;
	bool m_bServoOn;
	bool m_bRuning;

	BYTE m_nPortNo;
	BYTE m_iSlaveNo;
	DWORD m_dwBaudrate;

	BYTE m_iCurPos;
	bool m_bInitPos;

	bool m_setPortNum(int i);

	bool connect();
	bool disconnect();

	bool IsConnected();
	bool IsServoOn();
	bool IsRunning();

	bool servoOn();
	bool servoOff();

	bool goHome();
	bool goTablePos(int iPos);
	bool go10x();
	bool go20x();
	bool go50x();
	bool go150x();
	bool goNone();

	bool stop();

	bool getAxisStatus();

	int getCurrentLens();
};

