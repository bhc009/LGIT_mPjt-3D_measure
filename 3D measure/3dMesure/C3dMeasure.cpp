// #pragma once

#include "StdAfx.h"
#include "C3dMeasure.h"

#include <iostream>
#include <fstream>
using namespace std;


UINT threadProcCheckEndMeasure(LPVOID lParam )
{
	C3dMeasure *pMain = (C3dMeasure*)lParam;

	while( TRUE )
	{
		WaitForSingleObject( pMain->m_hEventCheckEndMeasure, INFINITE );

		BOOL bMeasuring;
		VKResult result = (VKResult)VK_IsMeasuring(&bMeasuring);
		switch(result)
		{
		case VKResult_OK:
			while( bMeasuring )
			{
				VK_IsMeasuring(&bMeasuring);
			}
			pMain->measureEnd();
			break;

		case VKResult_NotAccepted:
			AfxMessageBox(_T("Invalid status."));
			break;

		case VKResult_InvalidArgument:
			AfxMessageBox(_T("Invalid parameter."));
			break;

		default:
			AfxMessageBox(_T("Acquisition failed."));
			break;
		}

		ResetEvent(pMain->m_hEventCheckEndMeasure);
	}


	return 0;
}


C3dMeasure::C3dMeasure(void)
{
	m_bStateConnection = FALSE;
	m_bStateMeasure = FALSE;

	m_hWndParent = 0;
	m_uiMessage = 0;

	m_pHeightMap = NULL;
	m_pHeight_origin = NULL;
	m_pLaserMap = NULL;
	m_pDicMap = NULL;
	m_pLaserImage = NULL;
	m_pCddImage = NULL;

	m_nLastMeasurementResultDataWidth = 0;
	m_nLastMeasurementResultDataHeight = 0;

	m_strPathHeightMap	= PATH_HEIGHT_MAP; 
	m_strPathLaserMap	= PATH_LASER_MAP; 
	m_strPathLaserImage	= PATH_LASER_IMAGE; 
	m_strPathDicImage	= PATH_DIC_IMAGE; 
	m_strPathCcdImage	= PATH_CCD_IMAGE; 
	m_strPathThickness	= PATH_DATA_THICKNESS; 
	m_strPathVkImage	= PATH_VK_IMAGE; 


	m_hEventCheckEndMeasure = CreateEvent( NULL, TRUE, FALSE, NULL );
	m_hThreadCheckEndMeasure = AfxBeginThread( threadProcCheckEndMeasure, (LPVOID)this );

	MakeFolder();
}

C3dMeasure::~C3dMeasure(void)
{
	if( m_pHeightMap )
	{
		delete[] m_pHeightMap;
	}
	m_pHeightMap= NULL;

	if( m_pHeight_origin )
	{
		delete[] m_pHeight_origin;
	}
	m_pHeight_origin= NULL;

	if( m_pLaserMap )
	{
		delete[] m_pLaserMap;
	}
	m_pLaserMap= NULL;

	if( m_pDicMap )
	{
		delete[] m_pDicMap;
	}
	m_pDicMap= NULL;

	if( m_pLaserImage )
	{
		delete[] m_pLaserImage;
	}
	m_pLaserImage= NULL;

	if( m_pCddImage )
	{
		delete[] m_pCddImage;
	}
	m_pCddImage= NULL;

	if( m_hThreadCheckEndMeasure )
	{
		delete m_hThreadCheckEndMeasure;
	}
	m_hThreadCheckEndMeasure = NULL;
}


void __stdcall OnHardwareEventCallback(VKHardwareEventId eventId, void* pvUserParam)
{
	VKHardwareEventId vkHardwareEventId = static_cast<VKHardwareEventId>(eventId);

	switch(vkHardwareEventId)
	{
	case PmtProtectionDetected:
		AfxMessageBox(_T("Disconnect failed."));
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 연결
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::connect()
{
	BOOL bOk = TRUE;
	VKResult result = (VKResult)VK_Initialize();


	switch(result)
	{
	case VKResult_OK:
		result = (VKResult)VK_SetHardwareEventCallback(OnHardwareEventCallback, this);

		if(result != VKResult_OK)
		{
			setState( _3DM_CONNECT_, FALSE );
// 			AfxMessageBox(_T("Failed to register."));
			bOk = FALSE;
		}else{
// 			AfxMessageBox(_T("Connect success!"));
			setState( _3DM_CONNECT_, TRUE );
			initializeLensPos();
		}
		break;

	case VKResult_AlreadyInitialized:
		setState( _3DM_CONNECT_, TRUE );
// 		AfxMessageBox(_T("Already initialized."));
		bOk = FALSE;
		break;

	case VKResult_NotAccepted:
		setState( _3DM_CONNECT_, FALSE );
// 		AfxMessageBox(_T("Invalid status."));
		bOk = FALSE;
		break;

	default:
		setState( _3DM_CONNECT_, FALSE );
// 		AfxMessageBox(_T("Failed to connect."));
		bOk = FALSE;
		break;
	}


	return bOk;
}


//////////////////////////////////////////////////////////////////////////
//
// 연결
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::disconnect()
{
	BOOL bResult = TRUE;
	VKResult result = (VKResult)VK_Uninitialize();

	switch(result)
	{
	case VKResult_OK:
		setState( _3DM_CONNECT_, FALSE );
		AfxMessageBox(_T("Disconnect success!"));
		break;

	case VKResult_NotInitialized:
		bResult = FALSE;
		break;

	case VKResult_NotAccepted:
		bResult = FALSE;
		AfxMessageBox(_T("Invalid status."));
		break;

	default:
		bResult = FALSE;
		AfxMessageBox(_T("Failed to end process"));
		break;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// 연결
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::isMeasuring()
{
	BOOL bResult = FALSE;

	BOOL bMeasuring;
	VKResult result = (VKResult)VK_IsMeasuring(&bMeasuring);

	switch(result)
	{
	case VKResult_OK:
		bResult = bMeasuring;
		break;
	default:
		break;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::initializeLensPos()
{
	long lPosition;

	VKResult result_lensPos = (VKResult)VK_GetUpperPosition(&lPosition);
	if(result_lensPos == VKResult_OK)
	{
// 		m_spinUpperPos.SetPos32(lPosition);
	}

	result_lensPos = (VKResult)VK_GetLowerPosition(&lPosition);
	if(result_lensPos == VKResult_OK)
	{
// 		m_spinLowerPos.SetPos32(lPosition);
	}

	result_lensPos = (VKResult)VK_GetDistance(&lPosition);
	if(result_lensPos == VKResult_OK)
	{
// 		m_spinLensDist.SetPos32(lPosition);
	}


// 	GetCurrentLensPos();


	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//	렌즈 위치를 얻는다.
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::GetCurrentLensPos(int *pLensPos)
{	
	long lPosition;
	VKResult result_lensPos = (VKResult)VK_GetLensPosition(&lPosition);
	if(result_lensPos == VKResult_OK)
	{
// 		CString strLensPos;
// 		strLensPos.Format(_T("%7d"), lPosition);
// 		m_dLensPosition.SetWindowText(strLensPos);

		*pLensPos = (int)lPosition;
	} else {
		*pLensPos = -1;
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::setState( int type, BOOL bValue )
{
	BOOL bResult = TRUE;

	switch( type )
	{
	case _3DM_CONNECT_:
		m_bStateConnection = bValue;
		break;

	case _3DM_MEASURE_:
		m_bStateMeasure = bValue;
		break;

	default:
		bResult = FALSE;
	}


	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::setViewType( int iType )
{
	BOOL bResultFlag = FALSE;

	ViewType viewType = (ViewType)iType;	// 0:camera, 1:Laser
	VKResult result = (VKResult)VK_SetViewType(viewType);


	switch(result)
	{
	case VKResult_OK:
		bResultFlag = TRUE;
		break;

	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		break;

	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		break;

	default:
		AfxMessageBox(_T("Failed to set."));
		break;
	}


	return bResultFlag;
}


//////////////////////////////////////////////////////////////////////////
//
// Auto focus 실행
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::autofocusImage()
{
	BOOL bResult = FALSE;

	if( !m_bStateConnection || m_bStateMeasure )
	{
		return FALSE;
	}

	VKResult result = (VKResult)VK_DoCameraAutofocus();
	switch(result)
	{
	case VKResult_OK:
		bResult = TRUE;
		break;
	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		break;
	case VKResult_AutoProcessCancel:
		AfxMessageBox(_T("Autofocus is canceled."));
		break;
	case VKResult_AutoProcessChangedRevolver:
		AfxMessageBox(_T("Revolver changed"));
		break;
	case VKResult_LaserFailureAbort:
		AfxMessageBox(_T("Laser output disorder detected."));
		break;
	case VKResult_AutoFocusFail:
		AfxMessageBox(_T("Autofocus failed."));
		break;
	default:
		AfxMessageBox(_T("Autofocus failed."));
		break;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Auto focus 실행
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::autofocusLaser()
{
	BOOL bResult = FALSE;

	if( !m_bStateConnection || m_bStateMeasure )
	{
		return FALSE;
	}

	VKResult result = (VKResult)VK_DoAutofocus();

	switch(result)
	{
	case VKResult_OK:
		bResult = TRUE;
		break;
	case VKResult_NotAccepted:
		//AfxMessageBox(_T("Invalid status."));
		break;
	case VKResult_AutoProcessCancel:
		//AfxMessageBox(_T("Autofocus is canceled."));
		break;
	case VKResult_AutoProcessChangedRevolver:
		//AfxMessageBox(_T("Revolver changed"));
		break;
	case VKResult_LaserFailureAbort:
		//AfxMessageBox(_T("Laser output disorder detected."));
		break;
	case VKResult_AutoFocusFail:
		//AfxMessageBox(_T("Autofocus failed."));
		break;
	default:
		//AfxMessageBox(_T("Autofocus failed."));
		break;
	}

	return bResult;
}


BOOL C3dMeasure::moveUp(long nanoStep)
{
	BOOL bResult = FALSE;

	if( !m_bStateConnection || m_bStateMeasure )
	{
		return FALSE;
	}

	VKResult result = (VKResult)VK_MoveLens(-nanoStep);

	switch(result)
	{
	case VKResult_OK:
		bResult = TRUE;
		break;
	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		break;
	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		break;
	case VKResult_ReachedLimit:
		AfxMessageBox(_T("Cannot move due to the limit."));
		break;
	default:
		AfxMessageBox(_T("Failed to set."));
		break;
	}

	return bResult;
}


BOOL C3dMeasure::moveDown(long nanoStep)
{
	BOOL bResult = FALSE;

	if( !m_bStateConnection || m_bStateMeasure )
	{
		return FALSE;
	}

	VKResult result = (VKResult)VK_MoveLens(nanoStep);

	switch(result)
	{
	case VKResult_OK:
		bResult = TRUE;
		break;
	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		break;
	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		break;
	case VKResult_ReachedLimit:
		AfxMessageBox(_T("Cannot move due to the limit."));
		break;
	default:
		AfxMessageBox(_T("Failed to set."));
		break;
	}

	return bResult;
}

BOOL C3dMeasure::move(long nanoStep)
{
	BOOL bResult = FALSE;

	if( !m_bStateConnection || m_bStateMeasure )
	{
		return FALSE;
	}


	// 현재 위치
	long curPos;
	if( !getCurPos(&curPos) )
	{
		return false;
	}

	// 이동량 계산
	VKResult result = (VKResult)VK_MoveLens(nanoStep - curPos );

	switch(result)
	{
	case VKResult_OK:
		bResult = TRUE;
		break;
	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		break;
	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		break;
	case VKResult_ReachedLimit:
		AfxMessageBox(_T("Cannot move due to the limit."));
		break;
	default:
		AfxMessageBox(_T("Failed to set."));
		break;
	}



	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// 초기화
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::measure()
{
	BOOL bResult = FALSE;


	MeasurementQuality quality;
	VK_GetMeasurementQuality(&quality);

	VKResult result = (VKResult)VK_StartMeasurement();

	switch(result)
	{
	case VKResult_OK:	
		SetMeasurementResultDataSize(quality);
		m_dHeightBasePosition = 0;

		setState( _3DM_MEASURE_, TRUE );

		bResult = TRUE;

		break;

	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		break;

	case VKResult_ShortageDistance:
		AfxMessageBox(_T("Too short distance."));
		break;

	case VKResult_ExcessDistance:
		AfxMessageBox(_T("Too long distance."));
		break;

	case VKResult_NotSupportFaceFilmThickness:
		AfxMessageBox(_T("Transparent (film) is not supported."));
		break;

	case VKResult_ExcessPitch:
		AfxMessageBox(_T("Too large pitch"));
		break;

	default:
		AfxMessageBox(_T("Failed to start measurement."));
		break;
	}


	// 측정 완료 확인용 스레드 시작
	if( bResult )
	{
		SetEvent(m_hEventCheckEndMeasure);
	}


	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// 측정 중단
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::measureStop()
{
	BOOL bResult = TRUE;

	VKResult result = (VKResult)VK_StopMeasurement();

	switch(result)
	{
	case VKResult_OK:
		setState( _3DM_MEASURE_, FALSE );
		break;

	case VKResult_NotAccepted:
		bResult = FALSE;
		AfxMessageBox(_T("Invalid status."));
		break;

	default:
		bResult = FALSE;
		AfxMessageBox(_T("Cancel failed."));
		break;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// 초기화
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::measureEnd()
{
	setState( _3DM_MEASURE_, FALSE );

	SendMessage( m_hWndParent, m_uiMessage, 0, (LPARAM)this );

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// 초기화
//
//////////////////////////////////////////////////////////////////////////
// BOOL C3dMeasure::measureStop()
// {
// 	VKResult result = (VKResult)VK_StopMeasurement();
// 
// 	switch(result)
// 	{
// 	case VKResult_OK:
// 		setState( _3DM_MEASURE_, FALSE );
// 		AfxMessageBox(_T("Cancel OK"));
// 		break;
// 
// 	case VKResult_NotAccepted:
// 		AfxMessageBox(_T("Invalid status."));
// 		break;
// 
// 	default:
// 		AfxMessageBox(_T("Cancel failed."));
// 		break;
// 	}
// 
// 	return TRUE;
// }


//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::read3dData()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	long lWidth, lHeight;
	VKResult result = (VKResult) GetDataSize(&lWidth, &lHeight);

	if( result != VKResult_OK )
	{
		AfxMessageBox(_T("Measured data not found exception!"));	
		return FALSE;
	}


	//////////////////////////////////////////////////////////////////////////
	// Read laser data
	//////////////////////////////////////////////////////////////////////////
	//long* pData = new long[lWidth * lHeight];
	if(m_pLaserMap!=NULL)
	{
		delete[] m_pLaserMap;
		m_pLaserMap = NULL;
	}
	m_pLaserMap = new unsigned short[lWidth * lHeight];

	result = (VKResult)VK_GetLightData2(m_pLaserMap);

	if(result != VKResult_OK)
	{
		AfxMessageBox(_T("Laser data not found exception!"));	
		return FALSE;
	}


	//////////////////////////////////////////////////////////////////////////
	// Read laser image
	//////////////////////////////////////////////////////////////////////////
	if( m_pLaserImage )
	{
		delete[] m_pLaserImage;
	}
	m_pLaserImage = new BYTE[lWidth * lHeight];

	int arrayIndex = 0;
	BYTE *pLaser = m_pLaserImage;
	for( int iPixelIndex=0 ; iPixelIndex < lWidth*lHeight; iPixelIndex++ )
	{
		BYTE luminance = (BYTE)(m_pLaserMap[iPixelIndex]>>8);		
		*pLaser++ = luminance;
	}


	//////////////////////////////////////////////////////////////////////////
	// Read height data
	//////////////////////////////////////////////////////////////////////////
	if(m_pHeightMap!=NULL)
	{
		delete[] m_pHeightMap;
		m_pHeightMap = NULL;
	}
	if(m_pHeight_origin!=NULL)
	{
		delete[] m_pHeight_origin;
		m_pHeight_origin = NULL;
	}
	m_pHeightMap = new double[lWidth * lHeight];
	m_pHeight_origin = new long[lWidth * lHeight];

	// 기준 높이
	if( GetHeightBasePosition( m_dHeightBasePosition ) == false ) 
	{
		AfxMessageBox(_T("Base position of height is not found exception!"));	
		return FALSE;
	}

	result = (VKResult)VK_GetHeightDataEx2(m_pHeight_origin, 1, HeightDataType_Unit100Picometer);

	if(result == VKResult_OK)
	{
		getHeightData(m_pHeight_origin, m_pHeightMap, lWidth, lHeight, m_dHeightBasePosition);
	} else {
		AfxMessageBox(_T("VK_GetHeightDataEx2 : error"));	
		return FALSE;
	}


	//////////////////////////////////////////////////////////////////////////
	// Read DIC
	//////////////////////////////////////////////////////////////////////////
	if(m_pDicMap!=NULL)
	{
		delete[] m_pDicMap;
		m_pDicMap = NULL;
	}
	m_pDicMap = new BYTE[3*lWidth * lHeight];

	result = (VKResult)VK_GetDifferentiationData(m_pDicMap);

	if(result != VKResult_OK)
	{
		AfxMessageBox(_T("DIC data not found exception!"));	
		return FALSE;
	}


	//////////////////////////////////////////////////////////////////////////
	// Read Cdd Image
	//////////////////////////////////////////////////////////////////////////
	if(m_pCddImage!=NULL)
	{
		delete[] m_pCddImage;
		m_pCddImage = NULL;
	}
	m_pCddImage = new BYTE[3*lWidth * lHeight];

	result = (VKResult)VK_GetCcdData(m_pCddImage);

	if(result != VKResult_OK)
	{
		AfxMessageBox(_T("CCD image not found exception!"));	
		return FALSE;
	}


	return TRUE;
}

BOOL C3dMeasure::GetHeightBasePosition( double &height_base_position )
{
	MeasurementParameter3 parameter;
	ZeroMemory(&parameter, sizeof(parameter));
	parameter.nSize = sizeof(parameter);

	VKResult result = (VKResult)VK_GetMeasurementResultParameter2(&parameter);

	if( result != VKResult_OK ) 
	{
		return FALSE;
	}

	height_base_position = parameter.dZeroHeight;

	return TRUE;
}


BOOL C3dMeasure::SetMeasurementResultDataSize(MeasurementQuality measurementQuality)
{
	m_nLastMeasurementResultDataWidth = 1024;
	m_nLastMeasurementResultDataHeight = 768;
	switch (measurementQuality)
	{
	case HighSpeed:
	case HighResolution:
	case StandardHighSpeed:
		break;
	case HighDensity:
		m_nLastMeasurementResultDataWidth *= 2;
		m_nLastMeasurementResultDataHeight *= 2;
		break;
	case Part1Of12Accuracy:
	case Part1Of12SuperHighSpeed:
		m_nLastMeasurementResultDataHeight = 64;
		break;
	default:
		break;
	}

	return TRUE;
}


BOOL C3dMeasure::saveHeightMap()
{
	long lWidth, lHeight;
	VKResult result = (VKResult) GetDataSize(&lWidth, &lHeight);

	if( result != VKResult_OK )
	{
		AfxMessageBox(_T("Measured data not found exception!"));		
		return FALSE;
	}

	if( !SaveHeightDataToCsvFile(m_pHeightMap, lWidth, lHeight) )
	{
		return FALSE;
	}

	return TRUE;
}


BOOL C3dMeasure::loadHeightMap()
{
	if( m_pHeightMap )
	{
		delete[] m_pHeightMap;
	}
	m_pHeightMap = new double[ _3D_WIDTH_*_3D_HEIGHT_ ];


	ifstream is(PATH_HEIGHT_MAP);
	char s[100];

	double *pData = m_pHeightMap;
	memset(pData, 0, sizeof(double));

	for( int y=0 ; y<_3D_HEIGHT_ ; y++ )
	{
		for( int x=0 ; x<_3D_WIDTH_-1 ; x++ )
		{
			is.getline(s, sizeof(s), ',');
			pData[ x + y*_3D_WIDTH_ ] = _tstof(s);
		}
		is.getline(s, sizeof(s));
		pData[ _3D_WIDTH_-1 + y*_3D_WIDTH_ ] = _tstof(s);
	}

// 	while (is.good()) 
// 	{
// 		is.getline(s, sizeof(s), ',');
// 
// 		if (is.bad()) 
// 		{
// 			break;
// 		}
// 
// 		*pData++ = _tstof(s);
// 	}

// 	//파일 생성
// 	CStdioFile file;
// 	if(!file.Open(PATH_HEIGHT_MAP, CFile::modeRead))
// 	{
// 		return FALSE;
// 	}
// 
// 
// 	CString strBufferLine;
// 	double *pData = m_pHeightMap;
// 	CString strTemp;
// 	for( int y=0 ; y<_3D_HEIGHT_ ; y++ )
// 	{
// 		file.ReadString(strBufferLine);
// 		for( int x=0 ; x<_3D_WIDTH_ ; x++ )
// 		{
// 			AfxExtractSubString(strTemp, strBufferLine, x, ',');
// 
// 			*pData = _tstof(strTemp);
// 			pData++;
// 		}
// 	}


	return TRUE;
}


BOOL C3dMeasure::loadHeightMap(CString strFilePath)
{
	if( m_pHeightMap )
	{
		delete[] m_pHeightMap;
	}
	m_pHeightMap = new double[ _3D_WIDTH_*_3D_HEIGHT_ ];


	ifstream is(strFilePath);
	char s[100];

	double *pData = m_pHeightMap;
	memset(pData, 0, sizeof(double));

	for( int y=0 ; y<_3D_HEIGHT_ ; y++ )
	{
		for( int x=0 ; x<_3D_WIDTH_-1 ; x++ )
		{
			is.getline(s, sizeof(s), ',');
			pData[ x + y*_3D_WIDTH_ ] = _tstof(s);
		}
		is.getline(s, sizeof(s));
		pData[ _3D_WIDTH_-1 + y*_3D_WIDTH_ ] = _tstof(s);
	}

	return TRUE;
}


BOOL C3dMeasure::saveLaserMap()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	long lWidth, lHeight;

	VKResult result = (VKResult) GetDataSize(&lWidth, &lHeight);

	if( result != VKResult_OK )
	{
		AfxMessageBox(_T("Measured data not found exception!"));		
		return FALSE;
	}else if( m_pLaserMap == NULL)
	{
		AfxMessageBox(_T("Light Intensity Data Not Found!!"));		
		return FALSE;
	}


	//csv저장
	unsigned short* pData = m_pLaserMap;

	CStdioFile file;
	CString strPath;
	strPath.Format(PATH_LASER_MAP);
	if(file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) == FALSE)
	{
		return FALSE; //파일 생성 실패
	}

	CString strTemp;
	for( long y = 0; y < lHeight; ++y )
	{
		for( long x = 0; x < lWidth; ++x )
		{
			if( x == lWidth - 1 )
			{
				strTemp.Format("%d\n", *pData);
				file.WriteString(strTemp);
			}else
			{
				strTemp.Format("%d,", *pData);
				file.WriteString(strTemp);
			}
			++pData;
		}
	}
	file.Close();



	///
	pData = m_pLaserMap;
	strPath.Format(m_strPathLaserMap);
	if(file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) == FALSE)
	{
		return false; //파일 생성 실패
	}

	for( long y = 0; y < lHeight; ++y )
	{
		for( long x = 0; x < lWidth; ++x )
		{
			if( x == lWidth - 1 )
			{
				strTemp.Format("%d\n", *pData);
				file.WriteString(strTemp);
			}else
			{
				strTemp.Format("%d,", *pData);
				file.WriteString(strTemp);
			}
			++pData;
		}
	}
	file.Close();
	
	
	return TRUE;
}


BOOL C3dMeasure::loadLaserMap()
{
	//
	//
	//
	if( m_pLaserMap )
	{
		delete[] m_pLaserMap;
	}
	m_pLaserMap = new unsigned short[ _3D_WIDTH_*_3D_HEIGHT_ ];


	ifstream is(PATH_LASER_MAP);
	char s[100];

	unsigned short *pData = m_pLaserMap;
	memset(pData, _3D_WIDTH_*_3D_HEIGHT_, sizeof(unsigned short));
	for( int y=0 ; y<_3D_HEIGHT_ ; y++ )
	{
		for( int x=0 ; x<_3D_WIDTH_-1 ; x++ )
		{
			is.getline(s, sizeof(s), ',');
			pData[ x + y*_3D_WIDTH_ ] = _tstol(s);
		}
		is.getline(s, sizeof(s));
		pData[ _3D_WIDTH_-1 + y*_3D_WIDTH_ ] = _tstol(s);
	}


	// Make laser image
	if( m_pLaserImage )
	{
		delete[] m_pLaserImage;
	}
	m_pLaserImage = new BYTE[ _3D_WIDTH_*_3D_HEIGHT_ ];

	BYTE *pImage = m_pLaserImage;
	for( int iPixelIndex=0 ; iPixelIndex < _3D_WIDTH_*_3D_HEIGHT_; iPixelIndex++ )
	{
		BYTE luminance = (BYTE)(m_pLaserMap[iPixelIndex]>>8);		
		*pImage++ = luminance;
	}


	return TRUE;
}

BOOL C3dMeasure::loadLaserMap(CString strFilePath)
{
	//
	//
	//
	if( m_pLaserMap )
	{
		delete[] m_pLaserMap;
	}
	m_pLaserMap = new unsigned short[ _3D_WIDTH_*_3D_HEIGHT_ ];


	ifstream is(strFilePath);
	char s[100];

	unsigned short *pData = m_pLaserMap;
	memset(pData, _3D_WIDTH_*_3D_HEIGHT_, sizeof(unsigned short));
	for( int y=0 ; y<_3D_HEIGHT_ ; y++ )
	{
		for( int x=0 ; x<_3D_WIDTH_-1 ; x++ )
		{
			is.getline(s, sizeof(s), ',');
			pData[ x + y*_3D_WIDTH_ ] = _tstol(s);
		}
		is.getline(s, sizeof(s));
		pData[ _3D_WIDTH_-1 + y*_3D_WIDTH_ ] = _tstol(s);
	}


	// Make laser image
	if( m_pLaserImage )
	{
		delete[] m_pLaserImage;
	}
	m_pLaserImage = new BYTE[ _3D_WIDTH_*_3D_HEIGHT_ ];

	BYTE *pImage = m_pLaserImage;
	for( int iPixelIndex=0 ; iPixelIndex < _3D_WIDTH_*_3D_HEIGHT_; iPixelIndex++ )
	{
		BYTE luminance = (BYTE)(m_pLaserMap[iPixelIndex]>>8);		
		*pImage++ = luminance;
	}


	return TRUE;
}



BOOL C3dMeasure::saveLaserImage()
{
	VKResult result = VKResult_UnknownError;

	long lWidth=_3D_WIDTH_, lHeight=_3D_HEIGHT_;
	

// 	BYTE *pData = new BYTE[lWidth * lHeight];


	if( m_pLaserMap==NULL )
	{
		AfxMessageBox("No laser data...");
		return FALSE;
	}

	CBhImage bhImage;
	bhImage.Create( lWidth, lHeight, 8, 0 );


	int arrayIndex = 0;
	BYTE *pImage = bhImage.GetImage();
	for( int iPixelIndex=0 ; iPixelIndex < lWidth*lHeight; iPixelIndex++ )
	{
		BYTE luminance = (BYTE)(m_pLaserMap[iPixelIndex]>>8);		
		*pImage++ = luminance;
	}

	bhImage.Invalidate();
	bhImage.Save(PATH_LASER_IMAGE);
	bhImage.Save(m_strPathLaserImage);


	return TRUE;
}

BOOL C3dMeasure::saveDicImage()
{
	CBhImage bhImage;
	bhImage.SetColorImage(m_pDicMap, _3D_WIDTH_, _3D_HEIGHT_);	
	bhImage.Save(PATH_DIC_IMAGE);
	bhImage.Save(m_strPathDicImage);


	return TRUE;
}


BOOL C3dMeasure::loadDicImage()
{
	CBhImage bhImage;
	if( !bhImage.Open(PATH_DIC_IMAGE) )
	{
		return FALSE;
	}


	if( m_pDicMap )
	{
		delete[] m_pDicMap;
	}

	m_pDicMap = new BYTE[3*bhImage.GetWidth()*bhImage.GetHeight()];

	memcpy( m_pDicMap, bhImage.GetImage(), 3*bhImage.GetWidth()*bhImage.GetHeight());

	return TRUE;
}


BOOL C3dMeasure::loadDicImage(CString strFilePath)
{
	CBhImage bhImage;
	if( !bhImage.Open(strFilePath) )
	{
		return FALSE;
	}


	if( m_pDicMap )
	{
		delete[] m_pDicMap;
	}

	m_pDicMap = new BYTE[3*bhImage.GetWidth()*bhImage.GetHeight()];

	memcpy( m_pDicMap, bhImage.GetImage(), 3*bhImage.GetWidth()*bhImage.GetHeight());

	return TRUE;
}


BOOL C3dMeasure::saveCcdImage()
{
	CBhImage bhImage;
	bhImage.SetColorImage(m_pCddImage, _3D_WIDTH_, _3D_HEIGHT_);	
	bhImage.Save(PATH_CCD_IMAGE);
	bhImage.Save(m_strPathCcdImage);


	return TRUE;
}


BOOL C3dMeasure::loadCcdImage()
{
	CBhImage bhImage;
	if( !bhImage.Open(PATH_CCD_IMAGE) )
	{
		return FALSE;
	}


	if( m_pCddImage )
	{
		delete[] m_pCddImage;
	}

	m_pCddImage = new BYTE[3*bhImage.GetWidth()*bhImage.GetHeight()];

	memcpy( m_pCddImage, bhImage.GetImage(), 3*bhImage.GetWidth()*bhImage.GetHeight());

	return TRUE;
}


BOOL C3dMeasure::loadCcdImage(CString strFilePath)
{
	CBhImage bhImage;
	if( !bhImage.Open(strFilePath) )
	{
		return FALSE;
	}


	if( m_pCddImage )
	{
		delete[] m_pCddImage;
	}

	m_pCddImage = new BYTE[3*bhImage.GetWidth()*bhImage.GetHeight()];

	memcpy( m_pCddImage, bhImage.GetImage(), 3*bhImage.GetWidth()*bhImage.GetHeight());

	return TRUE;
}


BOOL C3dMeasure::saveVkImage()
{
	VKResult result;

	result = (VKResult)VK_SaveMeasurementResult(PATH_VK_IMAGE);
	switch(result)
	{
	case VKResult_OK:
		break;
	case VKResult_NotAccepted:
// 		AfxMessageBox(_T("Invalid status."));
		return FALSE;
	case VKResult_InvalidArgument:
// 		AfxMessageBox(_T("Invalid parameter."));
		return FALSE;
	default:
// 		AfxMessageBox(_T("Failed to save."));
		return FALSE;
	}


	result = (VKResult)VK_SaveMeasurementResult(m_strPathVkImage);
	switch(result)
	{
	case VKResult_OK:
		break;
	case VKResult_NotAccepted:
		// 		AfxMessageBox(_T("Invalid status."));
		return FALSE;
	case VKResult_InvalidArgument:
		// 		AfxMessageBox(_T("Invalid parameter."));
		return FALSE;
	default:
		// 		AfxMessageBox(_T("Failed to save."));
		return FALSE;
	}


	return TRUE;
}


BOOL C3dMeasure::setParam( HWND hWnd, UINT uiMsg)
{
	m_hWndParent = hWnd;

	m_uiMessage = uiMsg;

	return TRUE;
}


BOOL C3dMeasure::SaveHeightDataToCsvFile(double* pData, long width, long height )
{
	if( pData==NULL )
	{
		OutputDebugString("SaveHeightDataToCsvFile error : no data");

		return FALSE;
	}


	// Dialog 사용 시 에러 발생함... 디버깅 필요
// 	TCHAR BASED_CODE szFilter[] = _T("CSV file (*.csv)|*.csv|");
// 	CFileDialog dlg(FALSE, _T("csv"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
// 	if (dlg.DoModal() != IDOK)
// 	{
// 		return FALSE;
// 	}


	CStdioFile file;
	CString strPath;
	strPath.Format(PATH_HEIGHT_MAP);
	if(file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) == FALSE)
	{
		return FALSE; //파일 생성 실패
	}

	double* pDataTemp = pData;
	
	CString strTemp;

	for( long y = 0; y < height; ++y )
	{
		for( long x = 0; x < width; ++x )
		{
			if( x == width - 1 )
			{
				strTemp.Format("%f\n", *pDataTemp);
				file.WriteString(strTemp);
			}else
			{
				strTemp.Format("%f,", *pDataTemp);
				file.WriteString(strTemp);
			}
			++pDataTemp;
		}
	}
	file.Close();


	////////////////////

	pDataTemp = pData;
	strPath.Format(m_strPathHeightMap);

	if(file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) == FALSE)
	{
		return false; //파일 생성 실패
	}

	for( long y = 0; y < height; ++y )
	{
		for( long x = 0; x < width; ++x )
		{
			if( x == width - 1 )
			{
				strTemp.Format("%f\n", *pDataTemp);
				file.WriteString(strTemp);
			}else
			{
				strTemp.Format("%f,", *pDataTemp);
				file.WriteString(strTemp);
			}
			++pDataTemp;
		}
	}
	file.Close();

	return TRUE;
}

VKResult C3dMeasure::GetDataSize(long* plWidth, long* plHeight)
{
	ASSERT(plWidth != NULL);
	ASSERT(plHeight != NULL);

	if( m_nLastMeasurementResultDataWidth <= 0 ||
		m_nLastMeasurementResultDataHeight <= 0)
	{
		return VKResult_UnknownError;
	}

	*plWidth = m_nLastMeasurementResultDataWidth;
	*plHeight = m_nLastMeasurementResultDataHeight;

	return VKResult_OK;
}


//////////////////////////////////////////////////////////////////////////
//
// 3D 센서로 부터 높이 데이터를 가져온다.
//
//////////////////////////////////////////////////////////////////////////
BOOL C3dMeasure::getHeightData(long* pSrc, double* pDst, long width, long height, double height_base_position )
{
	for( long y = 0; y < height; ++y )
	{
		for( long x = 0; x < width; ++x )
		{
			double height = height_base_position - *pSrc * 0.0001; 
			*pDst = height;
			++pSrc;
			++pDst;
		}
	}

	return TRUE;
}


bool C3dMeasure::MakeFolder()
{
	CString tmp = PATH_SAVE;

	CString tok;
	while(0 <= tmp.Find("\\"))
	{
		tok += tmp.Left(tmp.Find("\\")+1);
		tmp = tmp.Right(tmp.GetLength() - (tmp.Find("\\")+1));

		CreateDirectory(tok, NULL);
	}

	CreateDirectory(PATH_SAVE, NULL);

	return TRUE;
}


double* C3dMeasure::getHeightMap( int *pWidth, int *pHeight )
{
	*pWidth = _3D_WIDTH_;
	*pHeight = _3D_HEIGHT_;

	return m_pHeightMap;
}


unsigned short* C3dMeasure::getLaserMap( int *pWidth, int *pHeight )
{
	*pWidth = _3D_WIDTH_;
	*pHeight = _3D_HEIGHT_;

	return m_pLaserMap;
}


BYTE* C3dMeasure::getDicImage( int *pWidth, int *pHeight )
{
	*pWidth = _3D_WIDTH_;
	*pHeight = _3D_HEIGHT_;

	return m_pDicMap;
}


BYTE* C3dMeasure::getLaserImage( int *pWidth, int *pHeight )
{
	*pWidth = _3D_WIDTH_;
	*pHeight = _3D_HEIGHT_;

	return m_pLaserImage;
}


BYTE *C3dMeasure::getCcdImage( int *pWidth, int *pHeight )
{
	*pWidth = _3D_WIDTH_;
	*pHeight = _3D_HEIGHT_;

	return m_pCddImage;
}


bool C3dMeasure::getCurPos(long *pos)
{
	bool bResultOK = false;

	long lLensPos;
	VKResult result = (VKResult)VK_GetLensPosition(&lLensPos);
	switch(result)
	{
	case VKResult_OK:
		bResultOK = true;
		*pos = lLensPos;
		break;

	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		return bResultOK;

	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		return bResultOK;

	default:
		AfxMessageBox(_T("Failed to get."));
		return bResultOK;
	}


	return bResultOK;

}


bool C3dMeasure::getZoom(int *pZoom)
{
	bool bResultOK = false;

	ZoomIndex zoom;
	VKResult result = (VKResult)VK_GetZoom(&zoom);

	switch(result)
	{
	case VKResult_OK:
		{
			switch(zoom)
			{
			case Zoom_10x:
				*pZoom = 10;
				break;
			case Zoom_20x:
				*pZoom = 20;
				break;
			case Zoom_30x:
				*pZoom = 30;
				break;
			case Zoom_50x:
				*pZoom = 50;
				break;
			case Zoom_80x:
				*pZoom = 80;
				break;
			case Obsoleted_Zoom_40x:
			case Obsoleted_Zoom_60x:
			default:
				*pZoom = -1;
				break;
			}

			return true;
		}

	case VKResult_NotAccepted:
	case VKResult_NotInitialized:
	case VKResult_ConnectionLost:  
	case VKResult_InvalidArgument:
	case VKResult_UnknownError:
	default:
		*pZoom = -1;
		return false;
	}

	return true;
}


bool C3dMeasure::getUpperPos(long *pos)
{
	bool bResultOK = false;

	long lUpperPosition;
	VKResult result = (VKResult)VK_GetUpperPosition(&lUpperPosition);

	switch(result)
	{
	case VKResult_OK:
		bResultOK = true;
		*pos = lUpperPosition;
		break;

	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		return bResultOK;

	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		return bResultOK;

	default:
		AfxMessageBox(_T("Failed to get."));
		return bResultOK;
	}

	return bResultOK;
}


bool C3dMeasure::setUpperPos(long dPos)
{
	bool bResultOK = false;

	VKResult result = (VKResult)VK_SetUpperPosition(dPos);

	switch(result)
	{
	case VKResult_OK:
		bResultOK = true;
		break;

	case VKResult_NotAccepted:
// 		AfxMessageBox(_T("Invalid status."));
		break;

	case VKResult_InvalidArgument:
// 		AfxMessageBox(_T("Invalid parameter."));
		break;

	default:
// 		AfxMessageBox(_T("Failed to set."));
		break;
	}

	return bResultOK;
}


bool C3dMeasure::getLowerPos(long *pos)
{
	bool bResultOK = false;

	long lLowerPosition;
	VKResult result = (VKResult)VK_GetLowerPosition(&lLowerPosition);
	switch(result)
	{
	case VKResult_OK:
		*pos = lLowerPosition;
		bResultOK = true;
		break;

	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		return bResultOK;

	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		return bResultOK;

	default:
		AfxMessageBox(_T("Failed to get."));
		return bResultOK;
	}

	return bResultOK;
}


bool C3dMeasure::setLowerPos(long dPos)
{
	bool bResultOK = false;

	VKResult result = (VKResult)VK_SetLowerPosition(dPos);

	switch(result)
	{
	case VKResult_OK:
		bResultOK = true;
		break;

	case VKResult_NotAccepted:
// 		AfxMessageBox(_T("Invalid status."));
		break;

	case VKResult_InvalidArgument:
// 		AfxMessageBox(_T("Invalid parameter."));
		break;

	default:
// 		AfxMessageBox(_T("Failed to set."));
		break;
	}

	return bResultOK;
}


bool C3dMeasure::getDistance(long *dist)
{
	bool bResultOK = false;

	long lDistance;
	VKResult result = (VKResult)VK_GetDistance(&lDistance);
	switch(result)
	{
	case VKResult_OK:
		bResultOK = true;
		*dist = lDistance;
		break;

	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		return bResultOK;

	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		return bResultOK;

	default:
		AfxMessageBox(_T("Failed to get."));
		return bResultOK;
	}

	return bResultOK;
}


bool C3dMeasure::setDistance(long dDistance)
{
	bool bResultOK = false;

	VKResult result = (VKResult)VK_SetDistance(dDistance);
	switch(result)
	{
	case VKResult_OK:
		break;
	case VKResult_NotAccepted:
// 		AfxMessageBox(_T("Invalid status."));
		break;
	case VKResult_InvalidArgument:
// 		AfxMessageBox(_T("Invalid parameter."));
		break;
	default:
// 		AfxMessageBox(_T("Failed to set."));
		break;
	}

	return bResultOK;
}


bool C3dMeasure::setMeasureRangeOffset( long lLowerOffset, long lUpperOffset )
{
	long curPos;
	if( !getCurPos(&curPos) )
	{
		return false;
	}

	if( !setLowerPos(curPos + lLowerOffset) )
	{
		return false;
	}

	if( !setUpperPos(curPos - lUpperOffset) )
	{
		return false;
	}

	return true;
}
