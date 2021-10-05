#include "StdAfx.h"
#include "LensChanger.h"


CLensChanger::CLensChanger(void)
{
	m_bConnection = false;

	m_nPortNo = 3;
	m_dwBaudrate = 115200;
	m_iSlaveNo = 5;
	m_bServoOn = false;

	m_iCurPos = LENS_CHANGE_POS_DEFAULT;
	m_bInitPos = false;
	m_bRuning = false;

	HANDLE hEventCheckEndMeasure = CreateEvent( NULL, TRUE, FALSE, NULL );
}


CLensChanger::~CLensChanger(void)
{
}


bool CLensChanger::m_setPortNum(int i)
{
	m_nPortNo = i;

	return true;
}


bool CLensChanger::connect()
{
	bool bResult = false;

	if( FAS_Connect(m_nPortNo, m_dwBaudrate) )
	{
		m_bConnection = true;

		for (int i=0; i<MAX_SLAVE_NUMS; i++)
		{
			if (FAS_IsSlaveExist(m_nPortNo, i))
			{
				m_iSlaveNo = i;
			}
		}

		//
		getAxisStatus();

		//
		bResult = true;

	} else {
		m_iCurPos = LENS_CHANGE_POS_DEFAULT;	// 에러 > 렌즈위치 불명
	}

	return bResult;
}


bool CLensChanger::disconnect()
{

	FAS_Close(m_nPortNo);

	m_bConnection = false;

	return true;
}


bool CLensChanger::IsConnected()
{
	return m_bConnection;
}


bool CLensChanger::IsServoOn()
{
	return m_bServoOn;
}


bool CLensChanger::IsRunning()
{
	return m_bRuning;
}


bool CLensChanger::servoOn()
{
	if (m_bConnection)
	{
		FAS_ServoEnable(m_nPortNo, m_iSlaveNo, TRUE);

		Sleep(2000);

		getAxisStatus();

		return true;
	}

	return false;
}


bool CLensChanger::servoOff()
{
	if (m_bConnection)
	{
		FAS_ServoEnable(m_nPortNo, m_iSlaveNo, FALSE);

		Sleep(2000);

		getAxisStatus();

		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Home 위치로 렌즈를 움직인다.
//
//////////////////////////////////////////////////////////////////////////
bool CLensChanger::goHome()
{
	if (m_bConnection)
	{
		// Move to origin
		if(	FAS_MoveOriginSingleAxis(m_nPortNo, m_iSlaveNo) == FMM_OK )
		{
			m_iCurPos = 0;
		}

		// Move to table #0
// 		if( FAS_PosTableRunItem(m_nPortNo, m_iSlaveNo, 0) == FMM_OK )
// 		{
// 			m_iCurPos = 0;
// 		}

		// 구동 완료 체크
		m_bRuning = true;
		while( m_bRuning )
		{
			getAxisStatus();

			Sleep(100);
		}

		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Table에 지정된 위치로 렌즈를 움직인다.
//
//////////////////////////////////////////////////////////////////////////
bool CLensChanger::goTablePos(int iTargetPosition)
{
	bool bResult = false;

	if (m_bConnection==false || iTargetPosition>4 )
	{
		m_iCurPos = LENS_CHANGE_POS_DEFAULT;	// 에러 > 렌즈위치 불명

		return bResult;
	}


	// 초기화가 되어있지 않으면 origin으로 이동한다.
	if( m_bInitPos==false )
	{
		goHome();
		m_bInitPos = true;
	} else {
		if( iTargetPosition<m_iCurPos && iTargetPosition!=0 )
		{
			goHome();
		}
	}


	// 구동 완료 확인
	m_bRuning = true;
	while( m_bRuning )
	{
		getAxisStatus();

		Sleep(100);
	}


	// 목표위치로 이동한다.
	if( m_iCurPos !=iTargetPosition )
	{
		if( FAS_PosTableRunItem(m_nPortNo, m_iSlaveNo, iTargetPosition) == FMM_OK )
		{
			m_iCurPos = iTargetPosition;

			bResult = true;
		} else {
			bResult = false;

			m_iCurPos = LENS_CHANGE_POS_DEFAULT;	// 에러 > 렌즈위치 불명
		}

		// 구동 완료 확인
		m_bRuning = true;
		while( m_bRuning )
		{
			getAxisStatus();

			Sleep(100);
		}
	} else {
		bResult = true;
	}

	return bResult;
}


bool CLensChanger::go10x()
{
	return goTablePos(0);
}


bool CLensChanger::go20x()
{
	return goTablePos(1);
}


bool CLensChanger::go50x()
{
	return goTablePos(2);
}


bool CLensChanger::go150x()
{
	return goTablePos(3);
}


bool CLensChanger::goNone()
{
	return goTablePos(4);
}


bool CLensChanger::stop()
{
	if (m_bConnection)
	{
		if( FAS_MoveStop(m_nPortNo, m_iSlaveNo) == FMM_OK )
		{
			return true;
		} else {
			m_iCurPos = LENS_CHANGE_POS_DEFAULT;	// 에러 > 렌즈위치 불명

			return false;
		}
	}

	return false;
}


bool CLensChanger::getAxisStatus()
{
	bool bResult = false;

	if (m_bConnection)
	{
		EZISERVO_AXISSTATUS axisStatus;

		if( FAS_GetAxisStatus( m_nPortNo, m_iSlaveNo, &(axisStatus.dwValue) ) == FMM_OK )
		{
			if( axisStatus.FFLAG_SERVOON == 0 )
			{
				m_bServoOn = false;
			} else {
				m_bServoOn = true;
			}

			if( axisStatus.FFLAG_MOTIONING==0 )
			{
				m_bRuning = false;
			} else {
				m_bRuning = true;
			}

			bResult = true;
		}
	}

	return bResult;
}


int CLensChanger::getCurrentLens()
{
	switch(m_iCurPos)
	{
	case 0:
		return 10;
	case 1:
		return 20;
	case 2:
		return 50;
	case 3:
		return 150;
		
	default:
		return 0;
	}
}
