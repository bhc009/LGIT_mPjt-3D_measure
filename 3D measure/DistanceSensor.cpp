// DistanceSensor.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "3D measure.h"
#include "DistanceSensor.h"


// CDistanceSensor

IMPLEMENT_DYNAMIC(CDistanceSensor, CWnd)

CDistanceSensor::CDistanceSensor()
{
	m_Comm = NULL;

	m_strPortNum = "COM1";

	m_iMode = DS_MODE_CALIBRATION;
	m_iPos = 0;

	m_cntValid = 1;

	m_dValue = 0.0;

	m_pData = new char[BUFFER_LENGTH];

	for( int i=0 ; i<MAX_DISTANCE_COUNT ; i++ )
	{
		m_calibrationDistance[i] = 0;
		m_measureDistance[i] = 0;
		m_thickness[i] = 25;
	}

	m_bConnection = FALSE;

	clear();
}

CDistanceSensor::~CDistanceSensor()
{
	if(m_Comm)	//컴포트가 존재하면
	{
		m_Comm->Close();

		delete[] m_Comm;
		m_Comm = NULL;
	}

	if( m_pData )
	{
		delete[] m_pData;
		m_pData = NULL;
	}
}


BEGIN_MESSAGE_MAP(CDistanceSensor, CWnd)
	ON_MESSAGE(WM_RECEIVE_THICKNESS_SENSOR, OnReceive)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////////
//
// 3D 측정 완료 시
//
//////////////////////////////////////////////////////////////////////////
LRESULT CDistanceSensor::OnReceive(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here
	int length = wParam;

	if( read(length) )	// Frame 완성 시	
	{
		if( m_iMode==DS_MODE_CALIBRATION )
		{
			setCalibrationkDistance(m_iPos, m_dValue);
		} else {
			setMeasureDistance(m_iPos, m_dValue);
		}

		calcThickness();

		::SendMessage(GetParent()->GetSafeHwnd(),WM_RECEIVE_THICKNESS_SENSOR,0,0);

	}


	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Open port
//
//////////////////////////////////////////////////////////////////////////
bool CDistanceSensor::open()
{
// 	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( m_Comm )
	{
		delete[] m_Comm;
		m_Comm = NULL;
	}


// 	CString str = "COM8";
// 	m_Comm= new CPYH_Comm("\\\\.\\"+str,"9600","None","8 Bit","1 Bit");	// initial Comm port
	m_Comm= new CPYH_Comm(	"\\\\.\\"+m_strPortNum,
							"9600",
							"None",
							"8 Bit",
							"1 Bit");	// initial Comm port

	if(	m_Comm->Create(GetSafeHwnd()) != 0)	//통신포트를 열고 윈도우의 핸들을 넘긴다.
	{
		m_Comm->Clear();

		m_Comm->m_msgReceive = WM_RECEIVE_THICKNESS_SENSOR;
		m_Comm->m_msgClose = WM_CLOSE_THICKNESS_SENSOR;

		m_bConnection = TRUE;
	}
	else
	{
		if( m_Comm )
		{
			delete[] m_Comm;
			m_Comm = NULL;
		}

		m_bConnection = FALSE;

		return false;
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Close port
//
//////////////////////////////////////////////////////////////////////////
bool CDistanceSensor::close()
{
	if(m_Comm)	//컴포트가 존재하면
	{
		m_Comm->Close();

		delete[] m_Comm;
		m_Comm = NULL;

		m_bConnection = FALSE;

	} else {
		return false;
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Send command
//
//////////////////////////////////////////////////////////////////////////
double CDistanceSensor::measure()
{
	char cStr[12];

	cStr[0] = 'S';
	cStr[1] = 'R';
	cStr[2] = ',';

	// ID number
	cStr[3] = '0';
	cStr[4] = '0';
	cStr[5] = ',';

	// Data number
	cStr[6] = '0';
	cStr[7] = '0';
	cStr[8] = '2';

	// CR + LF
	cStr[9] = 0x0D;
	cStr[10] = 0x0A;
	cStr[11] = 0x00;

	m_Comm->Send(cStr,11);

	return 0;
}



//////////////////////////////////////////////////////////////////////////
//
// Send command
//
//////////////////////////////////////////////////////////////////////////
bool CDistanceSensor::measureAll()
{
	if( !m_Comm )
	{	
		return false;
	}

	char cStr[12];

	cStr[0] = 'M';
	cStr[1] = '0';
	// CR + LF
	cStr[2] = 0x0D;
	cStr[3] = 0x0A;
	cStr[4] = 0x00;

	return m_Comm->Send(cStr,4);
}


//////////////////////////////////////////////////////////////////////////
//
// Frame 완성 시 true return
//
//////////////////////////////////////////////////////////////////////////
bool CDistanceSensor::read(int len)
{
	// 데이터 읽기
	m_Comm->Receive(m_pData+m_nData,len);

	m_nData += len;


	// Frame 완성여부 체크
	if( m_nData>1 )
	{
		// check CR + LF
		if( m_pData[m_nData-2]==0x0D &&  m_pData[m_nData-1]==0x0A )
		{
			m_dValue = decode();

			clear();	// clear

			return true;
		}
	}


	return false;
}


void CDistanceSensor::clear()
{
	for( int i=0 ; i<BUFFER_LENGTH ; i++ )
	{
		m_pData[i] = 0x00;
	}

	m_nData = 0;
}


void CDistanceSensor::setState( int iMode, int iPos )
{
	m_iMode = iMode;
	m_iPos = iPos;
}


void CDistanceSensor::SetPortNum( int iNum )
{
	m_strPortNum.Format("COM%d", iNum );
}


double CDistanceSensor::getDistance()
{
	return m_dValue;
}


BOOL CDistanceSensor::setCalibrationkDistance(int iPos, double dValue)
{
	if( iPos<0 || iPos>=MAX_DISTANCE_COUNT )
	{
		return FALSE;
	}

	m_calibrationDistance[iPos] = dValue;

	return TRUE;
}


BOOL CDistanceSensor::setMeasureDistance(int iPos, double dValue)
{
	if( iPos<0 || iPos>=MAX_DISTANCE_COUNT )
	{
		return FALSE;
	}


	m_measureDistance[iPos] = dValue;


	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// 두께 값 계산
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDistanceSensor::calcThickness()
{
	// Calibration 시와 mask 있을 때 측정 값 차이
	for( int i=0 ; i<MAX_DISTANCE_COUNT ; i++ )
	{
		m_distanceDifference[i] = m_measureDistance[i] - m_calibrationDistance[i];
	}

	for( int i=0 ; i<MAX_DISTANCE_COUNT/2 ; i++ )
	{
		m_thickness[ 2*i ] = m_distanceDifference[2*i + 1] - m_distanceDifference[2*i];	// 두께 값 계산
		m_thickness[ 2*i + 1 ] = m_thickness[ 2*i ];	// 두께 값 복사( 의미 X )
	}


	for( int i=0 ; i<MAX_DISTANCE_COUNT ; i++ )
	{
		m_thickness[i] = m_calibrationDistance[i] - m_measureDistance[i];
	}

	return TRUE;
}


double CDistanceSensor::GetThickness( int iPos )
{
	// 20190802 for LGD
	return m_thickness[iPos];



	if( iPos<0 || iPos>=MAX_DISTANCE_COUNT )
	{
		return 0;
	}

	if( m_iMode==DS_MODE_CALIBRATION )
	{
		return 0;
	}

	return m_thickness[iPos];
}


//////////////////////////////////////////////////////////////////////////
//
// 두께 값 획득
//
//	!! 일단 4point 측정했다고 치자...
//
//////////////////////////////////////////////////////////////////////////
double CDistanceSensor::GetThickness( )
{
	// 20190802 for LGD
	return m_thickness[0];


	if( m_iMode==DS_MODE_CALIBRATION )
	{
		return 0;
	}


	double dThickness = 0;

	for( int i=0 ; i<4 ; i++ )
	{
		dThickness += m_thickness[i];
	}
	dThickness = dThickness/4;


	return dThickness;
}


double CDistanceSensor::decode()
{
	if( m_nData<12 )
	{
		return 0.0;
	}

	double a = convert( m_pData[3] );	// 부호
	double b = 100000*convert( m_pData[4] );
	double c = 10000*convert( m_pData[5] );
	double d = 1000*convert( m_pData[6] );
	double e = 1000*convert( m_pData[7] );	// '.'
	double f = 100*convert( m_pData[8] );
	double g = 10*convert( m_pData[9] );
	double h = convert( m_pData[10] );
	double i = 0.1*convert( m_pData[11] );

	return a * ( b + c + d + f + g + h + i );
}


int CDistanceSensor::convert( char c )
{
	switch( c )
	{
	case '0':
		return 0;

	case '1':
		return 1;

	case '2':
		return 2;

	case '3':
		return 3;

	case '4':
		return 4;

	case '5':
		return 5;

	case '6':
		return 6;

	case '7':
		return 7;

	case '8':
		return 8;

	case '9':
		return 9;

	case '+':
		return 1;

	case '-':
		return -1;
	
	default:
		return 0;
	}

		return 0;
}
