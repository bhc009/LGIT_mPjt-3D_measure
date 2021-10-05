
// 3D measureDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "3D measure.h"
#include "3D measureDlg.h"
#include "afxdialogex.h"
#include "DialogRecipe.h"

#include <iostream>
#include <fstream>


#include <list>
using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 측정 thread
UINT threadProcMeasure(LPVOID lParam )
{
	CMy3DmeasureDlg *pMain = (CMy3DmeasureDlg*)lParam;

	while( TRUE )
	{
		WaitForSingleObject( pMain->m_thread.m_hEventMeasure, INFINITE );

		pMain->sequenceMeasure();

		pMain->changePgmState(STATE_NONE);

		//
		ResetEvent(pMain->m_thread.m_hEventMeasure);
	}

	return 0;
}

// Auto focus thread
UINT threadProcAutofocus(LPVOID lParam )
{
	CMy3DmeasureDlg *pMain = (CMy3DmeasureDlg*)lParam;

	while( TRUE )
	{
		WaitForSingleObject( pMain->m_thread.m_hEventAutofocus, INFINITE );


		pMain->sequenceCheckMaskMovement();

		pMain->changePgmState(STATE_NONE);

		//
		ResetEvent(pMain->m_thread.m_hEventAutofocus);
	}

	return 0;
}


// Calibration thread
UINT threadProcCalibration(LPVOID lParam )
{
	CMy3DmeasureDlg *pMain = (CMy3DmeasureDlg*)lParam;

	while( TRUE )
	{
		WaitForSingleObject( pMain->m_thread.m_hEventCalibration, INFINITE );

		pMain->sequenceCalibration();

		pMain->changePgmState(STATE_NONE);

		//
		ResetEvent(pMain->m_thread.m_hEventCalibration);
	}

	return 0;
}


// Calibration thread
UINT threadProcChangeLens( LPVOID lParam )
{
	CMy3DmeasureDlg *pMain = (CMy3DmeasureDlg*)lParam;

	while( TRUE )
	{
		WaitForSingleObject( pMain->m_thread.m_hEventChangeLens, INFINITE );

		//
		pMain->sequenceCharngeLens();

		//
		ResetEvent(pMain->m_thread.m_hEventChangeLens);
	}

	return 0;
}


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMy3DmeasureDlg 대화 상자




CMy3DmeasureDlg::CMy3DmeasureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMy3DmeasureDlg::IDD, pParent)
	, m_dCalibrationThickness(30.0)
	, m_nIteration(1)
	, m_soketType(0)
	, m_socketIp(_T("127.0.0.1"))
	, m_socketPort(7010)
	, m_iModeMeasureMethod(MEASURE_METHOD_LARGE_HOLE)
	, m_dCurPos(0)
	, m_dUpperPos(0)
	, m_dLowerPos(0)
	, m_d3dRangeDistance(0)
	, m_3dMeaserRangeLowerOffset(30)
	, m_3dMeaserRangeUpperOffset(10)
	, m_dHolePitchX(467.0)
	, m_dHolePitchY(467.0)
	, m_dHoleWidth(210.0)
	, m_dHoleHeight(210.0)
	, m_operationMode(0)
	, m_ctrlDisplayProcessingImage(FALSE)
	, m_distanceSensorIP(_T(""))
	, m_bUseDistanceSensor(FALSE)
	, m_offet(0)
	, m_targetThickness(25)
	, m_bUseThicknessSensor(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_hThreadMeasure = AfxBeginThread( threadProcMeasure, (LPVOID)this );
	m_hThreadAutofocus = AfxBeginThread( threadProcAutofocus, (LPVOID)this );
	m_hThreadCalibration = AfxBeginThread( threadProcCalibration, (LPVOID)this );
	m_hThreadChangeLens = AfxBeginThread( threadProcChangeLens, (LPVOID)this );

	m_bStop = false;

	m_state = STATE_NONE;

	m_nCnt = 0;
	m_nCntSaveImage = 0;
	m_nCntCal = 0;

	m_strRecipe.Format("Default recipe");
	m_strQRCode.Format("Default QR code");

	QueryPerformanceFrequency(&m_globalCounterFreq);
	QueryPerformanceFrequency(&m_localCounterFreq);

	m_bStateCheck = FALSE;

	m_pLogger = Logger::getInstance();


	m_iLensMag_src = _LENS_CMD_FROM_USER_;
	m_iLensMag_taget = 10;
}

void CMy3DmeasureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MESSAGE, m_ctrlMessage);
	DDX_Text(pDX, IDC_EDIT_CALIBRATION_THICK, m_dCalibrationThickness);
	DDV_MinMaxDouble(pDX, m_dCalibrationThickness, 0.0, 999.9);
	DDX_Text(pDX, IDC_EDIT_ITERATION, m_nIteration);
	DDV_MinMaxUInt(pDX, m_nIteration, 1, 999);
	DDX_Radio(pDX, IDC_RADIO_SERVER_SOCKET, m_soketType);
	DDV_MinMaxInt(pDX, m_soketType, 0, 1);
	DDX_Text(pDX, IDC_EDIT_SOCKET_IP, m_socketIp);
	DDX_Text(pDX, IDC_EDIT_SOCKET_PORT, m_socketPort);
	DDX_Radio(pDX, IDC_RADIO_MEASURE_SLOPEANGLE2, m_iModeMeasureMethod);
	DDX_Control(pDX, IDC_COMBO1, m_ctrlRevolverPortNum);
	DDX_Text(pDX, IDC_EDIT_3D_POSITION_CUR, m_dCurPos);
	DDX_Text(pDX, IDC_EDIT_3D_POSITION_UPPER, m_dUpperPos);
	DDX_Text(pDX, IDC_EDIT_3D_POSITION_LOWER, m_dLowerPos);
	DDX_Text(pDX, IDC_EDIT_3D_DISTANCE, m_d3dRangeDistance);
	DDX_Text(pDX, IDC_EDIT_3D_LOWER_DISTANCE, m_3dMeaserRangeLowerOffset);
	DDV_MinMaxDouble(pDX, m_3dMeaserRangeLowerOffset, 0, 999);
	DDX_Text(pDX, IDC_EDIT_3D_UPPER_DISTANCE, m_3dMeaserRangeUpperOffset);
	DDV_MinMaxDouble(pDX, m_3dMeaserRangeUpperOffset, 0, 999);
	DDX_Control(pDX, IDC_COMBO2, m_ctrl3dViewType);
	DDX_Control(pDX, IDC_COMBO3, m_ctrlHoleShape);
	DDX_Text(pDX, IDC_EDIT3, m_dHolePitchX);
	DDX_Text(pDX, IDC_EDIT4, m_dHolePitchY);
	DDX_Text(pDX, IDC_EDIT5, m_dHoleWidth);
	DDX_Text(pDX, IDC_EDIT6, m_dHoleHeight);
	DDX_Radio(pDX, IDC_RADIO_MODE_AUTO, m_operationMode);
	DDV_MinMaxInt(pDX, m_operationMode, 0, 1);
	DDX_Check(pDX, IDC_CHECK_DISPLAY_PROCESSING_IMAGE, m_ctrlDisplayProcessingImage);
	DDX_Control(pDX, IDC_COMBO4, m_ctrlHoleInfoArrange);
	DDX_Text(pDX, IDC_DISTANCE_SENSOR_IP, m_distanceSensorIP);
	DDX_Check(pDX, IDC_CHECK_USE_DISTANCE, m_bUseDistanceSensor);
	DDX_Text(pDX, IDC_EIDT_MEASURE_OFFSET, m_offet);
	DDX_Text(pDX, IDC_EDIT_TARGET_THICKNESS, m_targetThickness);
	DDX_Check(pDX, IDC_CHECK_THICKNESS_SENSOR_USE, m_bUseThicknessSensor);
	DDX_Control(pDX, IDC_COMBO_COM_PORT_THICK_SENSOR, m_comboComPort_thickSensor);
}

BEGIN_MESSAGE_MAP(CMy3DmeasureDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE( WM_END_MEASURE, OnReceive )
	ON_MESSAGE( WM_UPDATEDATA_FALSE, OnUpdateData )

	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CMy3DmeasureDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CMy3DmeasureDlg::OnBnClickedButtonDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_MEASURE, &CMy3DmeasureDlg::OnBnClickedButtonMeasure)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_HEIGHT, &CMy3DmeasureDlg::OnBnClickedButtonSaveHeight)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_HEIGHT, &CMy3DmeasureDlg::OnBnClickedButtonLoadHeight)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_LIGHT, &CMy3DmeasureDlg::OnBnClickedButtonSaveLaser)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_LIGHT, &CMy3DmeasureDlg::OnBnClickedButtonLoadLaser)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_DCT, &CMy3DmeasureDlg::OnBnClickedButtonSaveDct)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_DCT, &CMy3DmeasureDlg::OnBnClickedButtonLoadDct)
	ON_BN_CLICKED(IDC_BUTTON_READ_DATA, &CMy3DmeasureDlg::OnBnClickedButtonReadData)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_LIGHT_IMAGE, &CMy3DmeasureDlg::OnBnClickedButtonSaveLightImage)
	ON_BN_CLICKED(IDC_RADIO_HEIGHT, &CMy3DmeasureDlg::OnBnClickedRadioHeight)
	ON_BN_CLICKED(IDC_RADIO_LASER, &CMy3DmeasureDlg::OnBnClickedRadioLaser)
	ON_BN_CLICKED(IDC_RADIO_DIC, &CMy3DmeasureDlg::OnBnClickedRadioDic)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CMy3DmeasureDlg::OnBnClickedButtonTest)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT_DIST_SENSOR, &CMy3DmeasureDlg::OnBnClickedButtonConnectDistSensor)
	ON_BN_CLICKED(IDC_BUTTON_MEASURE_DIST_CONTINUOUSLY, &CMy3DmeasureDlg::OnBnClickedButtonMeasureDist)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT_DIST_SENSOR, &CMy3DmeasureDlg::OnBnClickedButtonDisconnectDistSensor)
	ON_BN_CLICKED(IDC_BUTTON_STOP_MEASURE_DIST, &CMy3DmeasureDlg::OnBnClickedButtonStopMeasureDist)
	ON_BN_CLICKED(IDC_BUTTON_MEASURE_DIST_ONECE, &CMy3DmeasureDlg::OnBnClickedButtonMeasureDistOnece)
	ON_BN_CLICKED(IDC_BUTTON_CALIBRATION, &CMy3DmeasureDlg::OnBnClickedButtonCalibration)
	ON_EN_CHANGE(IDC_EDIT_CALIBRATION_THICK, &CMy3DmeasureDlg::OnEnChangeEditCalibrationThick)
	ON_BN_CLICKED(IDC_RADIO_THICKNESS_METHOD_DIRECT, &CMy3DmeasureDlg::OnBnClickedRadioThicknessMethodDirect)
	ON_BN_CLICKED(IDC_RADIO_THICKNESS_METHOD_INDIRECT, &CMy3DmeasureDlg::OnBnClickedRadioThicknessMethodIndirect)
	ON_BN_CLICKED(IDC_BUTTON_SERVO_ON, &CMy3DmeasureDlg::OnBnClickedButtonServoOn)
	ON_BN_CLICKED(IDC_BUTTON_LENS_10, &CMy3DmeasureDlg::OnBnClickedButtonLens10)
	ON_BN_CLICKED(IDC_BUTTON_LENS_20, &CMy3DmeasureDlg::OnBnClickedButtonLens20)
	ON_BN_CLICKED(IDC_BUTTON_LENS_50, &CMy3DmeasureDlg::OnBnClickedButtonLens50)
	ON_BN_CLICKED(IDC_BUTTON_LENS_150, &CMy3DmeasureDlg::OnBnClickedButtonLens150)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT_REVOLVER, &CMy3DmeasureDlg::OnBnClickedButtonConnectRevolver)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT_REVOLVER, &CMy3DmeasureDlg::OnBnClickedButtonDisconnectRevolver)
	ON_BN_CLICKED(IDC_BUTTON_SERVO_OFF, &CMy3DmeasureDlg::OnBnClickedButtonServoOff)
	ON_BN_CLICKED(IDC_BUTTON_GO_HOME, &CMy3DmeasureDlg::OnBnClickedButtonGoHome)
	ON_BN_CLICKED(IDC_BUTTON_3D_LASER_AUTOFOFUS, &CMy3DmeasureDlg::OnBnClickedButton3dLaserAutofofus)
	ON_BN_CLICKED(IDC_BUTTON_3D_IMAGE_AUTOFOFUS, &CMy3DmeasureDlg::OnBnClickedButton3dImageAutofofus)
	ON_BN_CLICKED(IDC_BUTTON_3D_UP, &CMy3DmeasureDlg::OnBnClickedButton3dUp)
	ON_BN_CLICKED(IDC_BUTTON_3D_DOWN, &CMy3DmeasureDlg::OnBnClickedButton3dDown)
	ON_EN_CHANGE(IDC_EDIT_ITERATION, &CMy3DmeasureDlg::OnEnChangeEditIteration)
	ON_BN_CLICKED(IDC_BUTTON_CALCULATE, &CMy3DmeasureDlg::OnBnClickedButtonCalculate)
	ON_BN_CLICKED(IDC_BUTTON_STOP_CALCULATION, &CMy3DmeasureDlg::OnBnClickedButtonStopCaculation)
	ON_BN_CLICKED(IDC_BUTTON_SOCKET_CREATE, &CMy3DmeasureDlg::OnBnClickedButtonSocketCreate)
	ON_BN_CLICKED(IDC_RADIO_SERVER_SOCKET, &CMy3DmeasureDlg::OnBnClickedRadioServerSocket)
	ON_BN_CLICKED(IDC_RADIO_CLIENT_SOCKET, &CMy3DmeasureDlg::OnBnClickedRadioClientSocket)
	ON_BN_CLICKED(IDC_BUTTON_SOCKET_CONNECT, &CMy3DmeasureDlg::OnBnClickedButtonSocketConnect)
	ON_BN_CLICKED(IDC_BUTTON_SOCKET_DISCONNECT, &CMy3DmeasureDlg::OnBnClickedButtonSocketDisconnect)
	ON_WM_DESTROY()
	ON_MESSAGE( WM_ACCEPT_SOCKET, OnAcceptSocket )
	ON_MESSAGE( WM_RECEIVE_DATA, OnReceiveSocket)
	ON_MESSAGE( WM_RECEIVE_THICKNESS_SENSOR, OnReceiveThichness)

// 	ON_BN_CLICKED(IDC_RADIO_MEASURE_SLOPEANGLE, &CMy3DmeasureDlg::OnBnClickedRadioMeasureSlopeangle)
	ON_BN_CLICKED(IDC_RADIO_MEASURE_SLOPEANGLE2, &CMy3DmeasureDlg::OnBnClickedRadioMeasureStepHeight)
	ON_BN_CLICKED(IDC_RADIO_MEASURE_SLOPEANGLE3, &CMy3DmeasureDlg::OnBnClickedRadioMeasureSlopeangleNStepHeight)
	ON_BN_CLICKED(IDC_RADIO_MEASURE_THICKNESS, &CMy3DmeasureDlg::OnBnClickedRadioMeasureThickness)
	ON_BN_CLICKED(IDC_BUTTON_3D_SET_UPPER_POS, &CMy3DmeasureDlg::OnBnClickedButton3dSetUpperPos)
	ON_BN_CLICKED(IDC_BUTTON_3D_SET_LOWER_POS, &CMy3DmeasureDlg::OnBnClickedButton3dSetLowerPos)
	ON_BN_CLICKED(IDC_BUTTON_3D_SET_DISTANCE, &CMy3DmeasureDlg::OnBnClickedButton3dSetDistance)
	ON_BN_CLICKED(IDC_RADIO_MODE_AUTO, &CMy3DmeasureDlg::OnBnClickedRadioModeAuto)
	ON_BN_CLICKED(IDC_RADIO_MODE_MANUAL, &CMy3DmeasureDlg::OnBnClickedRadioModeManual)
	ON_EN_CHANGE(IDC_EDIT_3D_LOWER_DISTANCE, &CMy3DmeasureDlg::OnEnChangeEdit3dLowerDistance)
	ON_EN_CHANGE(IDC_EDIT_3D_UPPER_DISTANCE, &CMy3DmeasureDlg::OnEnChangeEdit3dUpperDistance)
	ON_BN_CLICKED(IDC_BUTTON_3D_AUTO_RANGE, &CMy3DmeasureDlg::OnBnClickedButton3dAutoRange)
	ON_BN_CLICKED(IDC_BUTTON_3D_SELECT_VIEW_TYPE, &CMy3DmeasureDlg::OnBnClickedButton3dSelectViewType)
	ON_EN_CHANGE(IDC_EDIT6, &CMy3DmeasureDlg::OnEnChangeEdit6)
	ON_EN_CHANGE(IDC_EDIT3, &CMy3DmeasureDlg::OnEnChangeEdit3)
	ON_EN_CHANGE(IDC_EDIT4, &CMy3DmeasureDlg::OnEnChangeEdit4)
	ON_EN_CHANGE(IDC_EDIT5, &CMy3DmeasureDlg::OnEnChangeEdit5)
	ON_BN_CLICKED(IDC_BUTTON_SET_HOLE_SHAPE_INFO, &CMy3DmeasureDlg::OnBnClickedButtonSetHoleShapeInfo)
	ON_BN_CLICKED(IDC_BUTTON_AUTO_FOCUS, &CMy3DmeasureDlg::OnBnClickedButtonAutoFocus)
	ON_BN_CLICKED(IDC_CHECK_DISPLAY_PROCESSING_IMAGE, &CMy3DmeasureDlg::OnBnClickedCheckDisplayProcessingImage)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CCD, &CMy3DmeasureDlg::OnBnClickedButtonSaveCcd)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_CCD, &CMy3DmeasureDlg::OnBnClickedButtonLoadCcd)
	ON_BN_CLICKED(IDC_BUTTON_TEST_MASK_MOVEMENT, &CMy3DmeasureDlg::OnBnClickedButtonTestMaskMovement)
	ON_BN_CLICKED(IDC_BUTTON_TEST2, &CMy3DmeasureDlg::OnBnClickedButtonTest2)
	ON_BN_CLICKED(IDC_BUTTON2, &CMy3DmeasureDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON_LENS_NONE, &CMy3DmeasureDlg::OnBnClickedButtonLensNone)
	ON_BN_CLICKED(IDC_CHECK_USE_DISTANCE, &CMy3DmeasureDlg::OnBnClickedCheckUseDistance)
	ON_BN_CLICKED(IDC_BUTTON_THICKSENSOR_OPEN, &CMy3DmeasureDlg::OnBnClickedButtonThicksensorOpen)
	ON_BN_CLICKED(IDC_BUTTON_THICKSENSOR_READ, &CMy3DmeasureDlg::OnBnClickedButtonThicksensorRead)
	ON_EN_CHANGE(IDC_EIDT_MEASURE_OFFSET, &CMy3DmeasureDlg::OnEnChangeEidtMeasureOffset)
	ON_BN_CLICKED(IDC_CHECK_THICKNESS_SENSOR_USE, &CMy3DmeasureDlg::OnBnClickedCheckThicknessSensorUse)
	ON_EN_CHANGE(IDC_EDIT_TARGET_THICKNESS, &CMy3DmeasureDlg::OnEnChangeEditTargetThickness)
	ON_BN_CLICKED(IDC_BUTTON_INITIALIZE, &CMy3DmeasureDlg::OnBnClickedButtonInitialize)
	ON_BN_CLICKED(IDC_BUTTON_RECIPE, &CMy3DmeasureDlg::OnBnClickedButtonRecipe)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_VK, &CMy3DmeasureDlg::OnBnClickedButtonSaveVk)
END_MESSAGE_MAP()


// CMy3DmeasureDlg 메시지 처리기

BOOL CMy3DmeasureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_3dMeasure.setParam( this->GetSafeHwnd(), WM_END_MEASURE );

	SetTimer( 1, 200, NULL );


	// 프로그램 모드(자동 또는 수동)
	m_operationMode = 0;


	// 거리센서와 IF port 설정
	m_ctrlRevolverPortNum.SetCurSel(9);	// COM3 설정
	m_distanceSensorIP = "169.254.168.150";
	m_distanceSensorIP = "192.6.94.3";	// E6


	// 3D 카메라 뷰 카메라 선택
	m_ctrl3dViewType.SetCurSel(0);


	// 검사대상 정보 설정
	m_ctrlHoleShape.SetCurSel(HOLE_SHAPE_DIAMOND);
	m_ctrlHoleInfoArrange.SetCurSel(DIAMOND_ARRANGEMENT);


	// 접촉센서
	m_comboComPort_thickSensor.SetCurSel(19);


	//////////////////////////////////////////////////////////////////////////
	// 
	makeLogFolder();

	// 설정 파일 로딩
	loadInit();

	// Dialog item 갱신
	UpdateData(FALSE);

	// site 정보 갱신
	setSite();
	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// 제품 정보 입력
// 	PARAM_MASK_INFO param;
// 	param.iPitchX	= (int)m_dHolePitchX;
// 	param.iPitchY	= (int)m_dHolePitchY;
// 	param.iWidth	= (int)m_dHoleWidth;
// 	param.iHeight	= (int)m_dHoleHeight;
// 	param.shape = (HOLE_SHAPE)m_ctrlHoleShape.GetCurSel();
// 	param.arrangement = (HOLE_ARRANGEMENT)m_ctrlHoleInfoArrange.GetCurSel();
// 	param.dThickness = m_targetThickness;
	m_stMaskInfo.iPitchX	= (int)m_dHolePitchX;
	m_stMaskInfo.iPitchY	= (int)m_dHolePitchY;
	m_stMaskInfo.iWidth		= (int)m_dHoleWidth;
	m_stMaskInfo.iHeight	= (int)m_dHoleHeight;
	m_stMaskInfo.shape		= (HOLE_SHAPE)m_ctrlHoleShape.GetCurSel();
	m_stMaskInfo.arrangement = (HOLE_ARRANGEMENT)m_ctrlHoleInfoArrange.GetCurSel();
	m_stMaskInfo.dThickness = m_targetThickness;
	m_stMaskInfo.dStepHeightOffset = m_offet;

	m_inspector.setParamImage(m_stMaskInfo);
	//
	//////////////////////////////////////////////////////////////////////////


	// 접촉센서에 대한 윈도우 생성
	m_thicknessSensor.Create(NULL,NULL,WS_CHILD, CRect(0,0,0,0), this, NULL );


	m_stc3dConnection.SubclassDlgItem(IDC_STATIC_3D_CONNECT, this);
	m_stc3dConnection.SetTextColor(RGB(0,0,0));
	m_stc3dConnection.SetFontSize(8);

	m_stc3dMeasure.SubclassDlgItem(IDC_STATIC_3D_MEASURE, this);
	m_stc3dMeasure.SetTextColor(RGB(0,0,0));
	m_stc3dMeasure.SetFontSize(8);


	m_stcRevoverConnection.SubclassDlgItem(IDC_STATIC_REVOLVER_CONNECT, this);
	m_stcRevoverConnection.SetTextColor(RGB(0,0,0));
	m_stcRevoverConnection.SetFontSize(8);

	m_stcRevoverServo.SubclassDlgItem(IDC_STATIC_REVOLVER_SERVO, this);
	m_stcRevoverServo.SetTextColor(RGB(0,0,0));
	m_stcRevoverServo.SetFontSize(8);

	m_stcRevoverRun.SubclassDlgItem(IDC_STATIC_REVOLVER_STATE, this);
	m_stcRevoverRun.SetTextColor(RGB(0,0,0));
	m_stcRevoverRun.SetFontSize(8);

	m_stcProgram.SubclassDlgItem(IDC_STATIC_PROGRAM_STATE, this);
	m_stcProgram.SetTextColor(RGB(0,0,0));
	m_stcProgram.SetFontSize(8);

	m_stcSocketConnection.SubclassDlgItem(IDC_STATIC_SOCKET_CONNECTION, this);
	m_stcSocketConnection.SetTextColor(RGB(0,0,0));
	m_stcSocketConnection.SetFontSize(8);

	m_stcContactSensorConnection.SubclassDlgItem(IDC_STATIC_CONTACT_SENSOR_CONNECTION, this);
	m_stcContactSensorConnection.SetTextColor(RGB(0,0,0));
	m_stcContactSensorConnection.SetFontSize(8);

	m_stcStateCheck.SubclassDlgItem(IDC_STATIC_SOCKET_CONNECTION_STATE_CHECK, this);
	m_stcStateCheck.SetTextColor(RGB(0,0,0));
	m_stcStateCheck.SetFontSize(8);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMy3DmeasureDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMy3DmeasureDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMy3DmeasureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMy3DmeasureDlg::OnBnClickedButtonConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("3D 광학계 연결 > ");

	if( m_3dMeasure.connect() )
	{
		update3dPosition();
	} else {
		dispay_message("3D 광학계 연결 > 실패");
	}


	dispay_message("3D 광학계 연결 > 완료");
		
}


void CMy3DmeasureDlg::OnBnClickedButtonDisconnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_3dMeasure.disconnect();
}


void CMy3DmeasureDlg::OnBnClickedButtonMeasure()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("3D 측정 > ");

 	if( m_state==STATE_NONE )
	{
		if( m_3dMeasure.measure() )
		{
// 			dispay_message("3D 측정 : ...");
		} else {
			changePgmState(STATE_NONE);

			dispay_message("3D 측정 > 실패");
		}
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveHeight()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("높이 데이터 저장 > ");

	m_3dMeasure.saveHeightMap();

	dispay_message("높이 데이터 저장 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadHeight()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("높이 데이터 읽기 >");

	m_3dMeasure.loadHeightMap();

	dispay_message("높이 데이터 읽기 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveLaser()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("레이저 데이터 저장 > ");

	m_3dMeasure.saveLaserMap();

	dispay_message("레이저 데이터 저장 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadLaser()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("레이저 데이터 읽기 > ");

	m_3dMeasure.loadLaserMap();

	dispay_message("레이저 데이터 읽기 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveDct()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("Save DIC > ");

	m_3dMeasure.saveDicImage();

	dispay_message("Save DIC > OK");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadDct()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("DIC 파일읽기 > ");

	if( m_3dMeasure.loadDicImage() )
	{
		dispay_message("DIC 파일읽기 > 완료");

	} else {
		dispay_message("DIC 파일읽기 > 실패");

	}
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveCcd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("Save CCD > ");

	if( !m_3dMeasure.saveCcdImage() )
	{
		dispay_message("Save CCD > 실패");

		return;
	}

	dispay_message("Save CCD > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadCcd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("Load CCD image > ");

	if( !m_3dMeasure.loadCcdImage() )
	{
		dispay_message("Load CCD image > 실패");

		return;
	}

	dispay_message("Load CCD image > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonReadData()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("3D 데이터 불러오기 > ");

	if( !m_3dMeasure.read3dData() )
	{
		dispay_message("3D 데이터 불러오기 > 실패");
	}

	dispay_message("3D 데이터 불러오기 > 완료");
}


//////////////////////////////////////////////////////////////////////////
//
// 시간 counter 시작
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::start_global_counter()
{
	QueryPerformanceCounter(&m_globalCounterStart);
}



//////////////////////////////////////////////////////////////////////////
//
// 시간 counter 완료
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::end_global_counter(CString strMessage)
{
	QueryPerformanceCounter(&m_globalCounterEnd);

	CString str;
	str.Format(" = %f sec", (double)(m_globalCounterEnd.QuadPart - m_globalCounterStart.QuadPart) / (double)m_globalCounterFreq.QuadPart );

	dispay_message(strMessage+str);
}


//////////////////////////////////////////////////////////////////////////
//
// 시간 counter 시작
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::start_local_counter()
{
	QueryPerformanceCounter(&m_localCounterStart);
}



//////////////////////////////////////////////////////////////////////////
//
// 시간 counter 완료
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::end_local_counter(CString strMessage)
{
	QueryPerformanceCounter(&m_localCounterEnd);

	CString str;
	str.Format(" = %f sec", (double)(m_localCounterEnd.QuadPart - m_localCounterStart.QuadPart) / (double)m_localCounterFreq.QuadPart );

	dispay_message(strMessage+str);
}


//////////////////////////////////////////////////////////////////////////
//
// Message를 화면에 표시한다.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::dispay_message(CString strMsg)
{
	// dislplay message
	m_ctrlMessage.InsertString(0, strMsg );


	// Debug view message
	OutputDebugString(strMsg+"\n");


	// Log file
	// 시간 얻기
	CTime today = CTime::GetCurrentTime();


	// 파일명 생성
	CString strName;
	strName.Format( "log%d_%d_%d_%d.log", 
					today.GetYear(),
					today.GetMonth(),
					today.GetDay(),
					today.GetHour());

	CString strPath;
	strPath = PATH_LOG;

	std::string strFileName = CT2CA(strPath + strName);


	// 파일명 설정
	m_pLogger->setFileName(strFileName);	// 파일명 설정


	// Log 쓰기
	m_pLogger->info(strMsg);
}


//////////////////////////////////////////////////////////////////////////
//
//	프로그램 동작상태 설정
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::changePgmState( PGM_STATE statusIn )
{
	if( m_state==STATE_NONE || statusIn==STATE_NONE )
	{
		m_state = statusIn;

		switch( m_state )
		{
		case STATE_NONE:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "대기중");
			m_stcProgram.SetBkColor(RGB(255,255,255));
			break;

		case STATE_CALIBRATION:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "Calibation...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_AUTOFOCUS:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "Auto focus...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_MEASURE:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "측정중...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_RUNING:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "동작중...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_LENS_CHANGE:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "배율변경중...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		default:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "???");
			m_stcProgram.SetBkColor(RGB(255,0,0));
		}

		return true;
	}

	return false;
}



bool CMy3DmeasureDlg::IsPgmStateFree()
{
	if( m_state==STATE_NONE )
	{
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
//
//	프로그램 동작상태 설정
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::loadInit( )
{
	dispay_message("ini 파일 읽기 > 시작");

	ifstream is("d:\\3d measure.ini");

	char s[100];

	while( !is.eof() )
	{
// 		operation_mode,1
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_operationMode = (int)_tstof(s);


// 		soket_type,1
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_soketType = (int)_tstof(s);

// 		soket_IP,127.0.0.1
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_socketIp = s;

// 		soket_port,7010
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_socketPort = (UINT)(_tstof(s));

// 		dist_IP,169.254.168.150
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_distanceSensorIP = s;

// 		revolver_PORT,10
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_ctrlRevolverPortNum.SetCurSel((int)_tstof(s)-1);

// 		FMM_pitch_x,227.0
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_dHolePitchX = _tstof(s);

// 		FMM_pitch_y,227.0
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_dHolePitchY = _tstof(s);

// 		FMM_width,105.0
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_dHoleWidth = _tstof(s);

// 		FMM_height,130.0
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_dHoleHeight = _tstof(s);

// 		FMM_shape,1
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_ctrlHoleShape.SetCurSel((int)_tstof(s));

// 		FMM_arrage,0
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_ctrlHoleInfoArrange.SetCurSel((int)_tstof(s));

		// 		FMM_thickness,20
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		m_targetThickness = _tstof(s);


		// 		SITE,0	0:LGD, 1:LGIT
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		int i = _tstoi(s);
		switch(i)
		{
		case 0:
			m_stSite = SITE_LGD;
			break;
		case 1:
			m_stSite = SITE_LGIT;
			break;
		default:
			AfxMessageBox("loadInit( )에서 site 정보가 맞지 않음");
		}

		// 	두께 센서 사용 여부
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		i = _tstoi(s);
		if( i==0 )
		{
			m_bUseThicknessSensor = FALSE;
		} else {
			m_bUseThicknessSensor = TRUE;
		}

		// 두께 센서 포트
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		i = _tstoi(s);

		int port = max( 0, min( 19, i - 1 ) );

		m_comboComPort_thickSensor.SetCurSel(i-1);

	}


	dispay_message("ini 파일 읽기 > 완료");
}


//////////////////////////////////////////////////////////////////////////
//
//	Auto focus 실시
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::autoFocus( UINT mag )
{
	// Live view 로 화면 전환
	if( !m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() ) )
	{
		dispay_message(" autoFocus : view 전환 실패");

		return false;
	}


	//
	switch( mag )
	{
		case 10:
			// 10배 렌즈로 변환
			if( !m_lensChanger.go10x() )
			{
				dispay_message(" autoFocus : 실패>10배 렌즈 이동");

				return false;
			}


			// Laser AF 실시
 			//if( !m_3dMeasure.autofocusLaser() )
			{
				// image AF 실시
				if( !m_3dMeasure.autofocusImage() )
				{
					dispay_message(" autoFocus : 실패>10배 렌즈 AF");

					return false;
				}
			}
			break;

		case 20:
			// 20배 렌즈로 변경
			if( !m_lensChanger.go20x() )
			{
				dispay_message(" autoFocus : 실패>20배 렌즈 이동");

				return false;
			}

			// 레이저 AF 실시
			if( !m_3dMeasure.autofocusLaser() )
			{
				// image AF 실시
				if( !m_3dMeasure.autofocusImage() )
				{
					dispay_message(" autoFocus : 실패>20배 렌즈 AF");

					return false;
				}
			}
			break;

		case 50:
			// 50배 렌즈로 변경
			if( !m_lensChanger.go50x() )
			{
				dispay_message(" autoFocus : 실패>50배 렌즈 이동");
				return false;
			}

			// 레이저 AF 실시
			//if( !m_3dMeasure.autofocusLaser() )
			{
				// image AF 실시
				if( !m_3dMeasure.autofocusImage() )
				{
					dispay_message(" autoFocus : 실패>50배 렌즈 AF");
					return false;
				}
			}
			break;

		case 150:
			// 50배 렌즈로 변경
			if( !m_lensChanger.go150x() )
			{
				dispay_message(" autoFocus : 실패>150배 렌즈 이동");
				return false;
			}

			// 레이저 AF 실시
			if( !m_3dMeasure.autofocusLaser() )
			{
				dispay_message(" autoFocus : 실패>150배 렌즈 AF");
				return false;
			}
			break;

		default:
			dispay_message(" autoFocus : 실패>배율 이상");
			return false;

	}


	//
	setMeasureRange( true, 0, 0 );


	return true;
}



bool CMy3DmeasureDlg::changeLens( UINT mag )
{
	switch( mag )
	{
	case 10:
		// 10배 렌즈로 변환
		if( !m_lensChanger.go10x() )
		{
			dispay_message(" changeLens : 실패>10배 렌즈 이동");

			return false;
		}
		break;

	case 20:
		// 20배 렌즈로 변경
		if( !m_lensChanger.go20x() )
		{
			dispay_message(" changeLens : 실패>20배 렌즈 이동");

			return false;
		}
		break;

	case 50:
		// 50배 렌즈로 변경
		if( !m_lensChanger.go50x() )
		{
			dispay_message(" changeLens : 실패>50배 렌즈 이동");
			return false;
		}
		break;

	case 150:
		// 50배 렌즈로 변경
		if( !m_lensChanger.go150x() )
		{
			dispay_message(" changeLens : 실패>150배 렌즈 이동");
			return false;
		}
		break;

	default:
		dispay_message(" changeLens : 실패>배율 이상");
		return false;

	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	3D 광학계 측정 범위 설정
//
//	- bAuto 가 ture일 경우 자동으로 설정( 다이얼로그 값 )
//	- dLowOffset, dHighOffset [단위 : um]
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::setMeasureRange( bool bAuto, double dLowOffset, double dHighOffset )
{
	long lowOffset = (long)(dLowOffset*1000);
	long highOffset = (long)(dHighOffset*1000);


	if( bAuto )
	{
		lowOffset = (long)(m_3dMeaserRangeLowerOffset*1000);
		highOffset = (long)(m_3dMeaserRangeUpperOffset*1000);
	}



	// 측정 범위 설정
	if( !m_3dMeasure.setMeasureRangeOffset( lowOffset, highOffset) )
	{
		dispay_message(" setMeasureRange : 실패...");
		return false;
	}


	// 화면 갱신
	update3dPosition();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	3D 카메라 위치 이동
//
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMy3DmeasureDlg::moveLens( double dMicro )
{
	return m_3dMeasure.move( (long)(dMicro*1000) );
}



//////////////////////////////////////////////////////////////////////////
//
//	3D 측정 실시
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::measure3d( double *dDistance )
{
	// 1. 측정 시작
	dispay_message(" measure3d > ");

	if( !m_3dMeasure.measure() )
	{
		dispay_message(" measure3d > 실패 : 측정시작");

		return false;
	}


	// 2. 측정 완료까지 대기 & 거리센서 데이터 획득
	double fDistanceSum = 0.0;
	UINT nData = 0;
	while( m_3dMeasure.isMeasuring() )
	{
		/* 
		// 3D 측정중 거리센서 획득
		m_distanceSensor.StopSensing();
		m_distanceSensor.StartSingleSensing(500);

		Sleep(100);	// 바로 읽으면 데이터 획득전에 값을 읽어오는 듯 하다.

		fDistanceSum += m_distanceSensor.GetGapAvg()*1000;
		nData++;

		CString strTemp;
		strTemp.Format("거리센서 측정 : %.3f\n", m_distanceSensor.GetGapAvg()*1000);
		dispay_message(strTemp);

		Sleep(50);
		*/
	}
//	*dDistance = fDistanceSum/nData;

	*dDistance = 0;


	// 3. Read 3D data
	if( !m_3dMeasure.read3dData() )
	{
		dispay_message(" measure3d > 실패 : 3D 데이터 획득");
		
		return false;
	}

	dispay_message(" measure3d > 완료");

	return true;
}


void CMy3DmeasureDlg::measureSlopeAngle()
{
	dispay_message("slope angle 측정 시작");

	dispay_message("slope angle 측정 완료");

}


bool CMy3DmeasureDlg::measure3d()
{
	dispay_message("3D 측정 >");


	//
	if( !m_3dMeasure.measure() )
	{
		dispay_message("3D 측정 > 실패 > scan 시작");

		return false;
	}


	// 측정 완료까지 대기
	while( m_3dMeasure.isMeasuring() )
	{
	}


	//
	if( !m_3dMeasure.read3dData() )
	{
		dispay_message("3D 측정 > 실패 > 데이터 획득");

		return false;
	}


	dispay_message("3D 측정 > 완료");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	step height를 직접 측정한다.
//
//
//
void CMy3DmeasureDlg::measureSmallHoleRegion()
{
	dispay_message("▲▲▲ 소공경 검사 >");

	// 결과 저장 폴더 생성
	makeSaveFolder();

	bool bSuccess = true;


	//////////////////////////////////////////////////////////////////////////
	// Auto focus
	//////////////////////////////////////////////////////////////////////////
	if( m_operationMode==AUTO_MODE )
	{
			// Live view 로 화면 전환
		if( !m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() ) )
		{
			dispay_message(" autoFocus : view 전환 실패");
	
			bSuccess = false;
		}

// 		m_3dMeasure.move(6200*1000);

		// 10배 렌즈로 변경
		if( bSuccess )
		{
			if( !changeLens(10) )
			{
				dispay_message(" 소공경 검사 > 실패 > changeLens 10");
				bSuccess = false;
			}
		}

		// 10배 렌즈 AF
		if( bSuccess )
		{
			if( !autoFocus(10) )
			{
				dispay_message(" 소공경 검사 > 실패 > auto focus 10");
				bSuccess = false;
			}
		}

		// 50배 렌즈로 변경
		if( bSuccess )
		{
			if( !changeLens(50) )
			{
				dispay_message(" 소공경 검사 > 실패 > changeLens 50");
				bSuccess = false;
			}
		}

		if( !autoFocus(50) )
		{
			dispay_message(" 소공경 검사 > 실패 > auto focus 50");

			bSuccess = false;
		}


		// 측정 range를 설정
		// Mask 두께를 고려하여 30 um로 설정
// 		if( !setMeasureRange( false, 30, 30 ) )
		if( !setMeasureRange( false, m_stRecip3D.dScanRangeDown, m_stRecip3D.dScanRangeUp ) )
		{
			dispay_message(" sequenceCheckMaskMovement : 실패");
			bSuccess = false;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// 반복 측정
	//////////////////////////////////////////////////////////////////////////
	list<double> listStepHeight;
	list<double> listMaskWidth;

	if( bSuccess )
	{
		for( UINT i=0 ; i<m_nIteration ; i++ )
		{
			// 예외 체크
			if( m_bStop )
			{
				m_bStop = false;
				bSuccess = false;

				i = m_nIteration;

				dispay_message(" 소공경 검사 > 중지");
			
				break;
			}


			//////////////////////////////////////////////////////////////////////////
			// 3D 측정
			//
			double dDistance=0;
			measure3d(&dDistance);
			//
			//////////////////////////////////////////////////////////////////////////



			//////////////////////////////////////////////////////////////////////////
			// 측정 데이터 저장
			//
			start_local_counter();

			m_3dMeasure.m_strPathHeightMap.Format(m_strSaveFilePath+"\\Height map %d.csv", i );
			m_3dMeasure.m_strPathLaserMap.Format(m_strSaveFilePath+"\\Laser map %d.csv", i );
			m_3dMeasure.m_strPathLaserImage.Format(m_strSaveFilePath+"\\Laser image %d.bmp", i );
			m_3dMeasure.m_strPathDicImage.Format(m_strSaveFilePath+"\\DIC image %d.bmp", i );
			m_3dMeasure.m_strPathCcdImage.Format(m_strSaveFilePath+"\\CCD image %d.bmp", i );
			m_3dMeasure.m_strPathVkImage.Format(m_strSaveFilePath+"\\Vk image %d", i );
			m_3dMeasure.m_strPathThickness.Format(m_strSaveFilePath+"\\Thickness %d.csv", i );

			m_3dMeasure.saveHeightMap();
			m_3dMeasure.saveLaserMap();
			m_3dMeasure.saveLaserImage();
			m_3dMeasure.saveDicImage();
			m_3dMeasure.saveCcdImage();
			m_3dMeasure.saveVkImage();

			end_local_counter("소공경 검사 > 데이터 저장");
			//
			//////////////////////////////////////////////////////////////////////////

			

			//////////////////////////////////////////////////////////////////////////
			//
			//
			start_local_counter();

			// step height 측정
			int widthHeight, heightHeight;
			double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);

			int widthLaser, heightLaser;
			BYTE *pLaserImage = m_3dMeasure.getLaserImage(&widthLaser, &heightLaser);

			int widthDic, heightDic;
			BYTE *pDicImage = m_3dMeasure.getDicImage( &widthDic, &heightDic );

			double dStepHeight=0;

			if( !m_inspector.measureStepHeightAtSmallHoleRegion(	pHeightMap, 
																pLaserImage, 
																pDicImage, 
																widthHeight, heightHeight, 
																&listStepHeight ) )
			{
				dispay_message(" 소공경 검사 > 실패 > 측정");
			}

			end_local_counter("소공경 검사 > 영상처리");
			//
			//////////////////////////////////////////////////////////////////////////

		}
	}


	// Save file
	if( bSuccess )
	{
		CStdioFile file;
		CString strPath;
		strPath.Format(PATH_DATA_STEP_HEIGHT);
		strPath.Format(m_strSaveFilePath+"\\Step height.csv");

		if( file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
		{
			CString strTemp;
			list<double>::iterator iteStepHeight = listStepHeight.begin();

			strTemp.Format("Step height,Mask width,Hole width\n");	// step height
			file.WriteString(strTemp);

			for( UINT i=0 ; i<listStepHeight.size() ; i++ )
			{
				strTemp.Format("%f,", *iteStepHeight);	// step height
				file.WriteString(strTemp);

				if( i%8==7 )
				{
					strTemp.Format("\n");	// hole width
					file.WriteString(strTemp);
				}

				iteStepHeight++;
			}

			file.Close();
		} else 
		{
			dispay_message(" 소공경 검사 > 실패 > 결과 저장");
		}
	}


	//
	if( m_operationMode==AUTO_MODE )
	{
		// 렌즈 변경
		if( !changeLens(10) )
		{
			dispay_message(" 소공경 검사 > 실패 > 10배 렌즈로 변경");
		}

		// 결과 데이터 송신

		if( listStepHeight.size() > 0 ) 
		{
			m_socketModule.SendMeasureResult( bSuccess, *listStepHeight.begin(), 0 );
		} else {
			m_socketModule.SendMeasureResult( false, 0, 0 );
		}
	}


	dispay_message("▼▼▼ 소공경 검사 > 완료\n");
}


//////////////////////////////////////////////////////////////////////////
//
//	대공경 면에서 slope angle와 step height 를 측정한다.
//
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::measureLargeHoleRegion()
{
	start_global_counter();
	dispay_message("▲ 대공경 검사 >");

	bool bSuccess =true;


	// 결과 저장 폴더 생성
	makeSaveFolder();


	//////////////////////////////////////////////////////////////////////////
	// Auto focus
	//////////////////////////////////////////////////////////////////////////
	start_local_counter();
	if( m_operationMode==AUTO_MODE )
	{
		//
		//moveLens(6413.0);

		// 레이저 AF 실시
		if( !autoFocus(10) )
		{
			dispay_message(" 대공경 검사 > 실패 : AF");

			bSuccess = false;
		}


		// 레이저 AF 실시
		if( !autoFocus(50) )
		{
			dispay_message(" 대공경 검사 > 실패 : AF");

			// AF 실패 시 안전을 위해 10배 렌즈로 변경
			if( !changeLens(10) )
			{
				dispay_message(" 대공경 검사 > 실패 : AF 실패 후 10배로 변경");
			}

			bSuccess = false;
		}

		// 측정 range를 설정
		// Mask 두께를 고려하여 30 um로 설정
// 		if( !setMeasureRange( false, 30, 30 ) )
		if( !setMeasureRange( false, m_stRecip3D.dScanRangeDown, m_stRecip3D.dScanRangeUp ) )
		{
			dispay_message(" 측정 범위 설정 : 실패");
			bSuccess = false;

			// AF 실패 시 안전을 위해 10배 렌즈로 변경
			if( !changeLens(10) )
			{
				dispay_message(" 측정 범위 설정 > 실패 : 10배로 변경");
			}
		}
	}
	end_local_counter("대공경 검사 > AF");


	//////////////////////////////////////////////////////////////////////////
	// 반복 측정
	//////////////////////////////////////////////////////////////////////////
	list<double> listStepHeight;
	list<double> listSlopeAngle;

	if( bSuccess )
	{
		m_inspector.clearResult();


		for( UINT i=0 ; i<m_nIteration ; i++ )
		{
			//////////////////////////////////////////////////////////////////////////
			// 정지 명령 확인
			//
			if( m_bStop )
			{
				m_bStop = false;
				bSuccess = false;

				i = m_nIteration;

				dispay_message(" 대공경 검사 > 중지");

				break;
			}
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			//
			// 3D 측정
			//
			start_local_counter();
			double dDistance=0;
			if( !measure3d(&dDistance) )
			{
				bSuccess = false;
				break;
			}
			end_local_counter("대공경 검사 > 3D 측정");
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 측정 데이터 저장
			//
			start_local_counter();

			m_3dMeasure.m_strPathHeightMap.Format(m_strSaveFilePath+"\\height map %d.csv", m_nCntSaveImage );
			m_3dMeasure.m_strPathLaserMap.Format(m_strSaveFilePath+"\\laser map %d.csv", m_nCntSaveImage );
			m_3dMeasure.m_strPathLaserImage.Format(m_strSaveFilePath+"\\laser image %d.bmp", m_nCntSaveImage );
			m_3dMeasure.m_strPathDicImage.Format(m_strSaveFilePath+"\\dic image %d.bmp", m_nCntSaveImage );
			m_3dMeasure.m_strPathCcdImage.Format(m_strSaveFilePath+"\\ccd image %d.bmp", m_nCntSaveImage );
			m_3dMeasure.m_strPathVkImage.Format(m_strSaveFilePath+"\\Vk image %d.vk4", m_nCntSaveImage );
			m_3dMeasure.m_strPathThickness.Format(m_strSaveFilePath+"\\thickness %d.csv", m_nCntSaveImage );

			m_nCntSaveImage++;

			dispay_message("save " + m_3dMeasure.m_strPathHeightMap);
			if( !m_3dMeasure.saveHeightMap() )
			{
				dispay_message("m_3dMeasure.saveHeightMap() : error");
			}

			dispay_message("save " +  m_3dMeasure.m_strPathLaserMap);
			m_3dMeasure.saveLaserMap();

			dispay_message("save " +  m_3dMeasure.m_strPathLaserImage);
			m_3dMeasure.saveLaserImage();

			dispay_message("save " +  m_3dMeasure.m_strPathDicImage);
			m_3dMeasure.saveDicImage();

			dispay_message("save " + m_3dMeasure.m_strPathCcdImage);
			m_3dMeasure.saveCcdImage();

			dispay_message("save " + m_3dMeasure.m_strPathVkImage);
			m_3dMeasure.saveVkImage();

			end_local_counter("대공경 검사 > 데이터 저장");
			//
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 거리센서 
			//
			// 3D 광학계 렌즈 위치 조정
			if(m_bUseDistanceSensor)
			{
				start_local_counter();
				m_lensChanger.goNone();
				end_local_counter("대공경 검사 > 거리센서 데이터 회득 > None 렌즈로 변경");

				Sleep(100);

				// 측정
				start_local_counter();
				dDistance = measureDistance(50);
				end_local_counter("대공경 검사 > 거리센서 데이터 회득 > 데이터 획득");

				// 렌즈 원복
				start_local_counter();
				m_lensChanger.go50x();
				end_local_counter("대공경 검사 > 거리센서 데이터 회득 > 50배 렌즈로 변경");
			} else {
				dDistance = 0;
			}
			//
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 3D 데이터 읽기
			//
			start_local_counter();
			// 높이 데이터 가져오기
			int widthHeight, heightHeight;
			double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);

			// 레이저 이미지 가져오기
			int widthLaser, heightLaser;
			BYTE *pLaserImage = m_3dMeasure.getLaserImage(&widthLaser, &heightLaser);

			// DIC 이미지 가져오기
			int widthDic, heightDic;
			BYTE *pDicImage = m_3dMeasure.getDicImage( &widthDic, &heightDic );
			end_local_counter("대공경 검사 > 3D 데이터 얻기");
			//
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 분석
			start_local_counter();

			// 두께값 획득
			dispay_message("대공경 > 두께 얻기");
			if( m_bUseThicknessSensor )
			{
				m_inspector.setTargetThickness( m_thicknessSensor.GetThickness() );
			} else {
				m_inspector.setTargetThickness( m_targetThickness );
			}


			// 측정
			dispay_message("대공경 > 검사하기");
			if( !m_inspector.measureSlopeAngleAtLargeHoleRegion(	pHeightMap,					// 높이 map
																	pLaserImage,				// laser image
																	pDicImage,					// DIC image
																	widthHeight, heightHeight,	// 이미지 크기
																	dDistance,					// 거리센서 정보
																	&listSlopeAngle,			// slope angle
																	&listStepHeight ) )			// step height
			{
				dispay_message(" 대공경 검사 > 실패 > 측정");
			}
			end_local_counter("대공경 검사 > 영상처리");
			//
			//////////////////////////////////////////////////////////////////////////



			CString strMsg;
			strMsg.Format( "대공경검사>measureLargeHoleRegion : %d 회", i+1 );
			dispay_message(strMsg);
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// Save file
	start_local_counter();
// 	if( bSuccess )
// 	{
// 		int nMesurePoint = m_inspector.getMeasurePointNum();
// 		double *pMeasureAnlge = m_inspector.getMeasurePointAngle();
// 
// 
// 		CStdioFile file;
// 		CString strPath;
// 		strPath.Format(PATH_DATA_HEIGHT_ANGLE);
// 
// 		if( file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
// 		{
// 			CString strTemp;
// 
// 			// angle
// 			file.WriteString("Slope angle\n");
// 			for( int j=0 ; j<nMesurePoint ; j++ )
// 			{
// 				strTemp.Format("%.1f 도,", pMeasureAnlge[j]);
// 				file.WriteString(strTemp);
// 			}
// 			file.WriteString("\n");
// 
// 
// 
// 			int n=1;
// 			for( list<double>::iterator ite = listSlopeAngle.begin() ; ite!=listSlopeAngle.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 			// angle
// 			file.WriteString("\nstep height\n");
// 			n=1;
// 			for( list<double>::iterator ite = listStepHeight.begin() ; ite!=listStepHeight.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 			file.Close();
// 		} 
// 		else 
// 		{
// 			dispay_message(" 대공경 검사 > 실패 > 결과 저장");
// 		}
// 
// 
// 		// 임시
// 		CStdioFile file2;
// 		CString strPath2;
// // 		strPath2.Format("c:\\test data\\S&H %d.csv", m_nCnt++);
// 		strPath2.Format(m_strSaveFilePath+"\\S&H %d.csv", m_nCnt++);
// 
// 		if( file2.Open(strPath2, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
// 		{
// 			CString strTemp;
// 
// 			// 구분자(각도)
// 			file2.WriteString("Slope angle\n");
// 			for( int j=0 ; j<nMesurePoint ; j++ )
// 			{
// 				strTemp.Format("%.1f 도,", pMeasureAnlge[j]);
// 				file2.WriteString(strTemp);
// 			}
// 			file2.WriteString("\n");
// 
// 
// 
// 			// angle
// 			int n=1;
// 			for( list<double>::iterator ite = listSlopeAngle.begin() ; ite!=listSlopeAngle.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file2.WriteString(strTemp);
// 
// 				//
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 			// step height
// 			file2.WriteString("\nstep height\n");
// 			n=1;
// 			for( list<double>::iterator ite = listStepHeight.begin() ; ite!=listStepHeight.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file2.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 			// island의 높이
// 			file2.WriteString("\n3D data(Height of Island)\n");
// 			n=1;
// 			for( list<double>::iterator ite = m_inspector.m_list3DdataIslandHeight.begin() ; ite!=m_inspector.m_list3DdataIslandHeight.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file2.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 
// 			// 구멍 주변부 높이
// 			file2.WriteString("\n3D data(Height of hole edge )\n");
// 			n=1;
// 			for( list<double>::iterator ite = m_inspector.m_list3DdataHoleEdgeHeight.begin() ; ite!=m_inspector.m_list3DdataHoleEdgeHeight.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file2.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 
// 			// 
// 			file2.WriteString("\n3D data(Disntance of angle )\n");
// 			n=1;
// 			for( list<double>::iterator ite = m_inspector.m_list3DdataIslandEdgeDistance.begin() ; ite!=m_inspector.m_list3DdataIslandEdgeDistance.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file2.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 
// 			// 거리센서
// 			file2.WriteString("\nDistance sensor data\n");
// 			n=1;
// 			for( list<double>::iterator ite = m_inspector.m_listDistanceData.begin() ; ite!=m_inspector.m_listDistanceData.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file2.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 
// 			// 
// 			file2.WriteString("\nStep height(Assume thickness)\n");
// 			n=1;
// 			for( list<double>::iterator ite = m_inspector.m_listThickness.begin() ; ite!=m_inspector.m_listThickness.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", *ite);
// 				file2.WriteString(strTemp);
// 
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 
// 			// 두께
// 			file2.WriteString("\nThickness\n");
// 			n=1;
// 			for( list<double>::iterator ite = m_inspector.m_listThickness.begin() ; ite!=m_inspector.m_listThickness.end() ; ite++ )
// 			{
// 				strTemp.Format("%f,", m_inspector.getMaskInfo()->dThickness);
// 				file2.WriteString(strTemp);
// 
// 				//
// 				if( n%nMesurePoint == 0 )
// 				{
// 					file2.WriteString("\n");
// 				}
// 
// 				n++;
// 			}
// 
// 
// 			file2.Close();
// 		} 
// 		else 
// 		{
// 			dispay_message(" 대공경 검사 > 실패 > 결과 저장");
// 		}	
// 	}
	end_local_counter("대공경 검사 > 결과 파일 저장하기");
	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// XML 송부건
	// <message>
	//		<body>
	//			<RESULT>
	//			<SLOPEANGLE>
	//			<STEPHEIGHT>
	//		<header>
	//			<messageName>
	//			<transactionId>
	//			<timeStamp>
	//////////////////////////////////////////////////////////////////////////
	start_local_counter();
// 	Sleep(5000);
// 	list<double> listStepHeight;
// 	list<double> listSlopeAngle;
	if( m_operationMode==AUTO_MODE)
	{
		// 렌즈 변경
		if( !changeLens(10) )
		{
			dispay_message(" 대공경 검사 > 실패 > 10배 렌즈 변경");
		}


		// 결과 데이터 송신
		if( m_stSite==SITE_LGD )
		{
			if(  listStepHeight.size()  > 0 )
			{
// 				m_socketModule.SendMeasureResult( bSuccess, *m_inspector.m_listThickness.begin(), *m_inspector.m_listThickness.begin() );
				m_socketModule.SendMeasureResult( true, 10, 45 );
			} else {
				m_socketModule.SendMeasureResult( false, 0, 0 );
			}
		} 
		else if( m_stSite==SITE_LGIT )
		{
			int nMesurePoint = m_inspector.getMeasurePointNum();

			// Step height : 총합 계산
			int i=0;
			int nStepHeight = 0;
			double tempStepHeightSum[24];
			for( int j=0 ; j<24 ; j++ )
			{
				tempStepHeightSum[j] = 0;
			}

			for( list<double>::iterator ite = m_inspector.m_listThickness.begin() ; ite!=m_inspector.m_listThickness.end() ; ite++ )
			{
				int iPos = i%nMesurePoint;
				if( iPos==0 )
				{
					nStepHeight++;
				}
				
				tempStepHeightSum[iPos] += *ite;

				i++;
			}


			// Step angle : 총합 계산
			i=0;
			int nAngle = 0;
			double tempAngleSum[24];
			for( int j=0 ; j<24 ; j++ )
			{
				tempAngleSum[j] = 0;
			}

			for( list<double>::iterator ite = listSlopeAngle.begin() ; ite!=listSlopeAngle.end() ; ite++ )
			{
				int iPos = i%nMesurePoint;

				if( iPos==0 )
				{
					nAngle++;
				}

				tempAngleSum[iPos] += *ite;

				i++;
			}


			//
			if( listStepHeight.size()  > 0 )
			{
				PARAM_MASK_INFO *pMaskInfo = m_inspector.getMaskInfo();
				if( pMaskInfo->shape== HOLE_SHAPE_ELIPSE_45)	// 기울어진 타원은 45도, 135도, 225도, 315도 위치의 값을 보낸다.
				{
					m_socketModule.SendMeasureResult(	bSuccess, 
														tempStepHeightSum[3]/nStepHeight, 
														tempStepHeightSum[9]/nStepHeight, 
														tempStepHeightSum[15]/nStepHeight, 
														tempStepHeightSum[21]/nStepHeight,
														tempAngleSum[3]/nAngle, 
														tempAngleSum[9]/nAngle, 
														tempAngleSum[15]/nAngle, 
														tempAngleSum[21]/nAngle);
				} else {
					m_socketModule.SendMeasureResult(	bSuccess, 
														tempStepHeightSum[0]/nStepHeight, 
														tempStepHeightSum[6]/nStepHeight, 
														tempStepHeightSum[12]/nStepHeight, 
														tempStepHeightSum[18]/nStepHeight,
														tempAngleSum[0]/nAngle, 
														tempAngleSum[6]/nAngle, 
														tempAngleSum[12]/nAngle, 
														tempAngleSum[18]/nAngle);
				}
			} else {
				m_socketModule.SendMeasureResult( false, 0, 0, 0, 0, 0, 0, 0, 0 );
			}
		}
	}
	end_local_counter("대공경 검사 > 결과 데이터 전송하기");


	dispay_message("▼ 대공경 검사 > 완료");
	end_global_counter("대공경 검사");
}


bool CMy3DmeasureDlg::sequenceCheckMaskMovement()
{
	dispay_message("▼ sequenceCheckMaskMovement : 시작");

	bool bSuccess = true;


	//
	// 1. 
	//
	if( m_operationMode==AUTO_MODE )
	{
		m_3dMeasure.move(6200*1000);

		// 10배 렌즈 AF
		if( !autoFocus(10) )
		{
			dispay_message(" sequenceCheckMaskMovement : 실패");
			bSuccess = false;
		}

		// 50배 렌즈로 변경
		if( bSuccess )
		{
			if( !autoFocus(50) )
			{
				dispay_message(" sequenceCheckMaskMovement : 실패");
				bSuccess = false;
			}
		}

		// 측정 범위 설정
		if( bSuccess )
		{
			if( !setMeasureRange( false, 30, 30 ) )
			{
				dispay_message(" sequenceCheckMaskMovement : 실패");
				bSuccess = false;
			}
		}

		// 3D 측정
		if( bSuccess )
		{
			double dDistance;
			if( !measure3d(&dDistance) )
			{
				dispay_message(" sequenceCheckMaskMovement : 실패");
				bSuccess = false;
			}
		}
	}


	//
	// 2. 
	//
	double dDx=0, dDy=0;	// um 단위
	if( bSuccess )
	{
		int widthCcd, heightCcd;
		BYTE *pCcdImage = m_3dMeasure.getCcdImage( &widthCcd, &heightCcd );

		int widthDic, heightDic;
		BYTE *pDicImage = m_3dMeasure.getDicImage( &widthDic, &heightDic );

// 		m_inspector.setParamer(m_dHolePitchX, m_dHolePitchY, m_dHoleWidth, m_dHoleHeight, (HOLE_SHAPE)m_ctrlHoleShape.GetCurSel());

		if( !m_inspector.checkMaskMovementDic( pDicImage, widthDic, heightDic, &dDx, &dDy ) )
		{
			dispay_message(" sequenceCheckMaskMovement : 실패");

			bSuccess = false;
		} else {
			CString strMsg;
			strMsg.Format(" sequenceCheckMaskMovement : X방향 = %.2f um, y방향 %.2f um", dDx, dDy );	// mm 단위로 변환하여 송부
			dispay_message(strMsg);
		}

	}
	

	//
	// 3.
	//
	if( m_operationMode==AUTO_MODE )
	{
		//
// 		changeLens(10);

		//
		m_socketModule.SendAutoFocusResult( bSuccess, dDx/1000, dDy/1000 );	// 단윈변환 : um > mm
	}


	dispay_message("▲ sequenceCheckMaskMovement : 완료");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	Calibration
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::sequenceCalibration()
{
	dispay_message("▼ sequenceCalibration >");

	bool bSuccess = true;

	// 거리센서 값
	double dDistance = 0;


	if( m_operationMode==AUTO_MODE )
	{
		//
		// 1. Auto focus
		//
		//moveLens(5441.0);
// 		moveLens(6369.0);

		// 1.1 10배
		dispay_message(" sequenceCalibration > autofocus 10x >");

		if( autoFocus(10) )
		{
			dispay_message(" sequenceCalibration > autofocus 10x > 성공");
		} else 
		{
			dispay_message(" sequenceCalibration > autofocus 10x > 실패");

			bSuccess = false;
		}


		// 1.2 50배
		if( bSuccess )
		{
			dispay_message(" sequenceCalibration > autofocus 50x >");

			if( autoFocus(50) )
			{
				dispay_message(" sequenceCalibration > autofocus 50x > 성공");
			} else
			{
				dispay_message(" sequenceCalibration > autofocus 50x > 실패");

				bSuccess = false;
			}

		}



		//
		// 2. 측정( 3D & distance )
		//
		if( bSuccess )
		{
			dispay_message(" sequenceCalibration > 3D 측정 >");

			if( !setMeasureRange( false, 10, 10 ) )
			{
				dispay_message(" sequenceCalibration > 3D 측정 > range : 실패");
				bSuccess = false;
			}

			if( measure3d(&dDistance) )
			{
				dispay_message(" sequenceCalibration > 3D 측정 > 성공");
			} else 
			{
				dispay_message(" sequenceCalibration > 3D 측정 > 실패");

				bSuccess = false;
			}
		}
	} 



	//
	// 1.2 거리센서 데이터 획득
	//	
	dDistance = measureDistance(50);
	//dDistance = m_distanceSensor.sensing(50)*1000;
	//dDistance = 0;


	//
	// 2. cal
	//
	// 2.1 3D 데이터 획득
	int width, height;
	double *p3dData = m_3dMeasure.getHeightMap(&width, &height);


	// 2.2 계산
	if( bSuccess )
	{
		dispay_message(" sequenceCalibration > 계산 >");

		m_inspector.m_list3DdataCal.clear();
		m_inspector.m_listDistanceDataCal.clear();

		if( m_inspector.calibration( p3dData, width, height, dDistance, m_dCalibrationThickness ) )
		{
			dispay_message(" sequenceCalibration > 계산 > 완료");

				//////////////////////////////////////////////////////////////////////////
				// save
				//////////////////////////////////////////////////////////////////////////
				// 1)
				CStdioFile file;
				CString strPath;
				strPath.Format("c:\\test data\\Cal %d.csv", m_nCntCal++);

				if( file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
				{
					CString strTemp;

					file.WriteString("3D data\n");
					for( list<double>::iterator ite = m_inspector.m_list3DdataCal.begin() ; ite!=m_inspector.m_list3DdataCal.end() ; ite++ )
					{
						strTemp.Format("%f\n", *ite);
						file.WriteString(strTemp);
					}

					file.WriteString("distance data\n");
					for( list<double>::iterator ite = m_inspector.m_listDistanceDataCal.begin() ; ite!=m_inspector.m_listDistanceDataCal.end() ; ite++ )
					{
						strTemp.Format("%f\n", *ite);
						file.WriteString(strTemp);
					}

					file.Close();
				}


		} else 
		{
			dispay_message(" sequenceCalibration > 계산 > 실패");

			bSuccess = false;
		}
	}


	//
	// 3. 
	//
	if( m_operationMode==AUTO_MODE )
	{
		// 렌즈 변경
		changeLens(10);

		// 결과 메세지 송부
		m_socketModule.SendCalibrationResult( bSuccess );

	}

	dispay_message("▲ sequenceCalibration > 완료");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	렌즈 배율 변경 시퀀스
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::sequenceCharngeLens()
{
	dispay_message("▼ sequenceCharngeLens >");


	if( m_iLensMag_src == _LENS_CMD_FROM_USER_ )	// 사용자의 명령일 경우
	{
		if( changePgmState(STATE_LENS_CHANGE) )
		{
			switch( m_iLensMag_taget )
			{
			case 10:	// 10배
				if( m_lensChanger.go10x() )
				{
					dispay_message(" sequenceCharngeLens > x10");
				} else {
					dispay_message(" sequenceCharngeLens > x10 > error");
				}
				break;

			case 20:	// 20배
				if( m_lensChanger.go20x() )
				{
					dispay_message(" sequenceCharngeLens > x20");
				} else {
					dispay_message(" sequenceCharngeLens > x20 > error");
				}
				break;

			case 50:	// 50배
				if( m_lensChanger.go50x() )
				{
					dispay_message(" sequenceCharngeLens > x50");
				} else {
					dispay_message(" sequenceCharngeLens > x50 > error");
				}
				break;

			case 150:	// 150배
				if( m_lensChanger.go150x() )
				{
					dispay_message(" sequenceCharngeLens > x150");
				} else {
					dispay_message(" sequenceCharngeLens > x150 > error");
				}
				break;

			default:	// 이상
				dispay_message(" sequenceCharngeLens > error");
				return false;
			}

			changePgmState(STATE_NONE);
		}
	} 
	else if( m_iLensMag_src == _LENS_CMD_FROM_OP_ )	// OP SW의 명령일 경우 > 응답이 필요함
	{
		dispay_message("sequenceCharngeLens > from OP > 시작");

		if( changePgmState( STATE_LENS_CHANGE ) )
		{
			if( m_lensChanger.go10x() )
			{
				dispay_message("sequenceCharngeLens > from OP > 성공");

				m_socketModule.SendLensChangeResult( true );
			} 
			else 
			{
				dispay_message("sequenceCharngeLens > from OP > 실패(동작 X)");

				m_socketModule.SendLensChangeResult( false );
			}

			dispay_message("렌즈 변경 명령 수신 > 끝");

			changePgmState( STATE_NONE );
		} 
		else 
		{
			m_socketModule.SendLensChangeResult( false );
			dispay_message("sequenceCharngeLens > from OP > 실패(다른 동작중)");
		}	

		dispay_message("sequenceCharngeLens > from OP > 끝");
	}
	

	dispay_message("▲ sequenceCharngeLens > 완료");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	측정 시퀀스
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::sequenceMeasure()
{
	switch( m_iModeMeasureMethod )
	{
		// 직접 방식 slope angle 측정
// 		case MEASURE_METHOD_SLOPEANGLE:
// 			measureSlopeAngle();
// 			break;

		// 직접 방식 step height 측정
		case MEASURE_METHOD_SMALL_HOLE:
			measureSmallHoleRegion();
			break;

		// 간접 방식 slope angle & step height 측정
		case MEASURE_METHOD_LARGE_HOLE:
			measureLargeHoleRegion();
			break;

		// 두께 calibration
		case MEASURE_METHOD_THICKNESS:
			measureThickness();
			break;
	}

	return true;
}


void CMy3DmeasureDlg::measureThickness()
{
	dispay_message("▼ 두께 측정 시작");


	//////////////////////////////////////////////////////////////////////////
	// Auto focus
	//////////////////////////////////////////////////////////////////////////
	if( m_operationMode==AUTO_MODE )
	{
		//
 		//moveLens(5909.0);

		//
		if( !autoFocus(10) )
		{
			dispay_message(" measureThickness > autofocus 10x > 실패");

			return;
		}

		// 레이저 AF 실시
		if( !autoFocus(50) )
		{
			dispay_message(" measureThickness > autofocus 50x > 실패");
			return;
		}

		// 측정 range를 설정
		// Mask 두께를 고려하여 30 um로 설정
		if( !setMeasureRange( false, 15, 15 ) )
		{
			dispay_message(" sequenceCheckMaskMovement : 실패");
			return;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// 측정
	//////////////////////////////////////////////////////////////////////////
	list<double> listThicknessData;
	list<double> list3dData;


// 	m_inspector.m_list3Ddata.clear();
// 	m_inspector.m_listDistanceData.clear();
	m_inspector.clearResult();


	for( UINT i=0 ; i<m_nIteration ; i++ )
	{
		if( m_bStop )
		{
			m_bStop = false;
			i = m_nIteration;
			break;
		}

		//
		if( !m_3dMeasure.measure() )
		{
			i = m_nIteration;
			break;
		}
		dispay_message("3D측정시작");


		// 측정 완료까지 대기
		// 거리 측정
		float fDistanceSum = 0.0;
		UINT nData = 0;
		while( m_3dMeasure.isMeasuring() )
		{
			fDistanceSum += (float)measureDistance(10);
			nData++;
		}
		float fDistance = fDistanceSum/nData;


		//
		m_3dMeasure.read3dData();
		dispay_message("3D 데이터 획득 완료");


		//
		int widthHeight, heightHeight;
		double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);

		double dThickness=0.0;
		m_inspector.measureT(pHeightMap, widthHeight, heightHeight, fDistance, &dThickness);

		listThicknessData.push_back(dThickness);


		CString strMsg;
		strMsg.Format("두께는 %.3f um 입니다.", dThickness );

		dispay_message(strMsg);
	}


	//////////////////////////////////////////////////////////////////////////
	// save
	//////////////////////////////////////////////////////////////////////////
	// 1)
	CStdioFile file;
	CString strPath;
	strPath.Format(PATH_DATA_THICKNESS);

	if( file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
	{
		CString strTemp;
		file.WriteString("Thickness data\n");
		for( list<double>::iterator ite = listThicknessData.begin() ; ite!=listThicknessData.end() ; ite++ )
		{
			strTemp.Format("%f\n", *ite);
			file.WriteString(strTemp);
		}

		file.WriteString("3D data\n");
		for( list<double>::iterator ite = m_inspector.m_list3DdataIslandHeight.begin() ; ite!=m_inspector.m_list3DdataIslandHeight.end() ; ite++ )
		{
			strTemp.Format("%f\n", *ite);
			file.WriteString(strTemp);
		}

		file.WriteString("distance data\n");
		for( list<double>::iterator ite = m_inspector.m_listDistanceData.begin() ; ite!=m_inspector.m_listDistanceData.end() ; ite++ )
		{
			strTemp.Format("%f\n", *ite);
			file.WriteString(strTemp);
		}

		file.Close();
	}


	// 2)
	CStdioFile file2;
	CString strPath2;
	strPath2.Format("c:\\test data\\T %d.csv", m_nCnt++);

	if( file2.Open(strPath2, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
	{
		CString strTemp;
		file2.WriteString("Thickness data\n");
		for( list<double>::iterator ite = listThicknessData.begin() ; ite!=listThicknessData.end() ; ite++ )
		{
			strTemp.Format("%f\n", *ite);
			file2.WriteString(strTemp);
		}

		file2.WriteString("3D data\n");
		for( list<double>::iterator ite = m_inspector.m_list3DdataIslandHeight.begin() ; ite!=m_inspector.m_list3DdataIslandHeight.end() ; ite++ )
		{
			strTemp.Format("%f\n", *ite);
			file2.WriteString(strTemp);
		}

		file2.WriteString("distance data\n");
		for( list<double>::iterator ite = m_inspector.m_listDistanceData.begin() ; ite!=m_inspector.m_listDistanceData.end() ; ite++ )
		{
			strTemp.Format("%f\n", *ite);
			file2.WriteString(strTemp);
		}

		file2.Close();
	}


	//////////////////////////////////////////////////////////////////////////
	// XML 송부건
	// <message>
	//		<body>
	//			<RESULT>
	//			<SLOPEANGLE>
	//			<STEPHEIGHT>
	//		<header>
	//			<messageName>
	//			<transactionId>
	//			<timeStamp>
	//////////////////////////////////////////////////////////////////////////
	if( m_operationMode==AUTO_MODE)
	{
		// 렌즈 변경
		if( !changeLens(10) )
		{
			dispay_message(" 두께 검사 > 실패 > 10배 렌즈 변경");
		}

		m_socketModule.SendMeasureResult( true, 0, 0 );
	}

	dispay_message("▲ 두께 측정 완료");
}


double CMy3DmeasureDlg::measureDistance(int nTime)
{
	double dDistanceSum = 0.0;
	UINT nData = 0;

	// 데이터 획득
	//for( int k=0 ; k< nTime ; k++ )
	{
		// 거리센서 재연결 : 기존 데이터 초기화 목적
		m_distanceSensor.Disconnect();

		CString strIp = "30, " + m_distanceSensorIP + ", TCP/IP, 300, 0.0, 3.0,-10.0, 0.001";

// 		if( !m_distanceSensor.Connect("30, 169.254.168.150, TCP/IP, 300, 0.0, 3.0,-10.0, 0.001") )
		if( !m_distanceSensor.Connect(strIp) )
		{
			Sleep(500);	// 바로 읽으면 데이터 획득전에 값을 읽어오는 듯 하다.

			m_distanceSensor.Disconnect();

			Sleep(500);	// 바로 읽으면 데이터 획득전에 값을 읽어오는 듯 하다.

// 			if( !m_distanceSensor.Connect("30, 169.254.168.150, TCP/IP, 300, 0.0, 3.0,-10.0, 0.001") )
			if( !m_distanceSensor.Connect(strIp) )
			{
				dispay_message("===== 거리센서 연결 > 실패 =======");
			}

			Sleep(100);	// 바로 읽으면 데이터 획득전에 값을 읽어오는 듯 하다.
		}

		dDistanceSum = m_distanceSensor.sensing(nTime)*1000;

		CString strTemp;
		strTemp.Format("거리센서 측정 : %.3f\n", dDistanceSum);
		dispay_message(strTemp);

		m_distanceSensor.Disconnect();
	}	

	return dDistanceSum;
}


//////////////////////////////////////////////////////////////////////////
//
// 측정 중단
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::stopMeasure()
{
	m_bStop = true;
}



void CMy3DmeasureDlg::update3dPosition()
{
	long temp;
	if( m_3dMeasure.getCurPos(&temp) )
	{
		m_dCurPos = temp * 0.001;
	}
	if( m_3dMeasure.getUpperPos(&temp) )
	{
		m_dUpperPos = temp * 0.001;
	}
	if( m_3dMeasure.getLowerPos(&temp) )
	{
		m_dLowerPos = temp * 0.001;
	}
	if( m_3dMeasure.getDistance(&temp) )
	{
		m_d3dRangeDistance = temp * 0.001;
	}


	SendMessage(WM_UPDATEDATA_FALSE);
// 	UpdateData(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
// 적용 위치별 수정사항 적용
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::setSite()
{
	if( m_stSite==SITE_LGD )
	{
		m_socketModule.m_site = _SOCKET_MODULE_LGD;
		m_protocol.m_iSite = BHPROTOCOL_LGD;

		SetDlgItemText(IDC_STATIC_PROGRAM_SITE, "LG display");
	} 
	else if(m_stSite==SITE_LGIT)
	{
		m_socketModule.m_site = _SOCKET_MODULE_LGIT;
		m_protocol.m_iSite = BHPROTOCOL_LGIT;

		SetDlgItemText(IDC_STATIC_PROGRAM_SITE, "LG innoteck");
	} 
}


//////////////////////////////////////////////////////////////////////////
//
// Data를 저장할 folder 생성
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::makeSaveFolder()
{
	// 시간 구하기
	COleDateTime tm = COleDateTime::GetCurrentTime();

	m_strSaveFilePath.Format("d:\\3d result\\%d_%2d_%2d_%2d_%2d_%2d", tm.GetYear(), tm.GetMonth(), tm.GetDay(), tm.GetHour(), tm.GetMinute(), tm.GetSecond() );

	m_strSaveFilePath.Format("d:\\Log\\result\\%d\\%2d\\%2d\\%s\\%s", tm.GetYear(), tm.GetMonth(), tm.GetDay(), m_strRecipe, m_strQRCode );

	CString tmp = m_strSaveFilePath;
	CString tok;
	while(0 <= tmp.Find("\\"))
	{
		tok += tmp.Left(tmp.Find("\\")+1);
		tmp = tmp.Right(tmp.GetLength() - (tmp.Find("\\")+1));

		CreateDirectory(tok, NULL);
	}

	CreateDirectory(m_strSaveFilePath, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Data를 저장할 folder 생성
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::makeLogFolder()
{
	// 시간 구하기
	CString tmp = PATH_LOG;
	CString tok;
	while(0 <= tmp.Find("\\"))
	{
		tok += tmp.Left(tmp.Find("\\")+1);
		tmp = tmp.Right(tmp.GetLength() - (tmp.Find("\\")+1));

		CreateDirectory(tok, NULL);
	}
}


// 10배 렌즈 -> AF
// 50배 렌즈 -> AF
bool CMy3DmeasureDlg::autoFocus()
{
	// Live view 로 화면 전환
	if( !m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() ) )
	{
		dispay_message("Auto focus > 실패 > view 전환");

		return false;
	}


	// 10배 렌즈로 변환
	if( !m_lensChanger.go10x() )
	{
		dispay_message("Auto focus > 실패 > 10배 렌즈 이동");
		return false;
	}


	// Laser AF 실시
	if( !m_3dMeasure.autofocusLaser() )
	{
		// image AF 실시
		if( !m_3dMeasure.autofocusImage() )
		{
			dispay_message("Auto focus > 실패 > 10배 렌즈 AF");
			return false;
		}
	}


	// 50배 렌즈로 변경
	if( !m_lensChanger.go50x() )
	{
		dispay_message("Auto focus > 실패 > 50배 렌즈 이동");
		return false;
	}


	if( m_operationMode==MANUAL_MODE)
	{
		// 레이저 AF 실시
		if( !m_3dMeasure.autofocusLaser() )
		{
			dispay_message("Auto focus > 실패 > 50배 렌즈 AF");
			return false;
		}
	}


	// 측정 범위 설정
	if( !m_3dMeasure.setMeasureRangeOffset( (long)(m_3dMeaserRangeLowerOffset*1000), (long)(m_3dMeaserRangeUpperOffset*1000)) )
	{
		dispay_message("Auto focus > 실패 > 측정 range 설정");
		return false;
	}


	// 화면 갱신
	update3dPosition();


	return true;
}

void CMy3DmeasureDlg::OnBnClickedButtonSaveLightImage()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("레이저 영상 저장하기 시작");

	m_3dMeasure.saveLaserImage();

	dispay_message("레이저 영상 저장하기 완료");
}


void CMy3DmeasureDlg::OnBnClickedRadioHeight()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioLaser()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioDic()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonTest()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//
	if( changePgmState(STATE_RUNING) )
	{
		CString strMsg;
		CString strFilePath;
		list<double> listStepHeight;
		list<double> listSlopeAngle;


		int nTime = 1;
		for( int i=0 ; i<nTime ; i++ )
		{
			//////////////////////////////////////////////////////////////////////////
			// 데이터 읽기
			start_local_counter();

			// Height map
			strFilePath.Format("c:\\Test data\\height map %d.csv", i);
			m_3dMeasure.loadHeightMap(strFilePath);

			int widthHeight, heightHeight;
			double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);


			// Laser image
			strFilePath.Format("c:\\Test data\\laser map %d.csv", i);
			m_3dMeasure.loadLaserMap(strFilePath);

			int widthLaser, heightLaser;
			BYTE *pLaserImage = m_3dMeasure.getLaserImage(&widthLaser, &heightLaser);


			// 

			strFilePath.Format("c:\\Test data\\dic image %d.bmp", i);
			m_3dMeasure.loadDicImage(strFilePath);

			int widthDic, heightDic;
			BYTE *pDicImage = m_3dMeasure.getDicImage( &widthDic, &heightDic );

			end_local_counter("소공경검사 > 데이터 읽기");
			//////////////////////////////////////////////////////////////////////////


			double dSlopeAngle=0;


			// 경우 1 : 직접 방식 step height
			m_inspector.measureStepHeightAtSmallHoleRegion( pHeightMap, pLaserImage, pDicImage, widthHeight, heightHeight, &listStepHeight );


			for( list<double>::iterator ite_height = listStepHeight.begin() ; ite_height!=listStepHeight.end() ; ite_height++ )
			{
				strMsg.Format("step height=%f", *ite_height );
				dispay_message(strMsg);
			}

// 			for( int i=0 ; i<4 ; i++ )
// 			{
// 				strMsg.Format("Mask width=%f", m_inspector.m_holeInfo.dRealWidthMask[i] );
// 				dispay_message(strMsg);
// 			}

		}


		//////////////////////////////////////////////////////////////////////////
		//
		//
		//
		// 결과 파일 저장 
		start_local_counter();
		{
			CStdioFile file2;
			CString strPath2;
			strPath2.Format("c:\\test data\\Manual test result.csv", m_nCnt++);

			if( file2.Open(strPath2, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
			{
				CString strTemp;

				// angle
				file2.WriteString("step height\n");
				file2.WriteString("0도,45도,90도,135도,180도,225도,270도,315도\n");
				int i=0;
				for( list<double>::iterator ite = listStepHeight.begin() ; ite!=listStepHeight.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					if( i%8 == 7 )
					{
						strTemp.Format("\n");
						file2.WriteString(strTemp);
					}

					i++;
				}
			}
		}
		end_local_counter("대공경검사 > 결과 파일 저장");
		//
		//////////////////////////////////////////////////////////////////////////


		changePgmState(STATE_NONE);
	}
}


void CMy3DmeasureDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	// 3D 연결상태
	if( m_3dMeasure.m_bStateConnection==TRUE )
	{
		SetDlgItemText(IDC_STATIC_3D_CONNECT, "연결 됨" );
		m_stc3dConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_3D_CONNECT, "연결 않됨" );
		m_stc3dConnection.SetBkColor(RGB(255,0,0));
	}

	// 3D 측정상태
	if( m_3dMeasure.m_bStateMeasure==TRUE )
	{
		SetDlgItemText(IDC_STATIC_3D_MEASURE, "측정중" );
		m_stc3dMeasure.SetBkColor(RGB(0,0,255));
	} else {
		SetDlgItemText(IDC_STATIC_3D_MEASURE, "대기중" );
		m_stc3dMeasure.SetBkColor(RGB(255,255,255));
	}

	// 거리센서 연결상태
	if( m_distanceSensor.IsConnected() )
	{
		SetDlgItemText(IDC_STATIC_DISTANCE_CONNECT, "연결 : O" );
	} else {
		SetDlgItemText(IDC_STATIC_DISTANCE_CONNECT, "연결 : X" );
	}

	// 거리센서 연결상태
	if( m_distanceSensor.IsGapSensing() )
	{
		CString str;
		str.Format("측정 : O(%.6f um)", m_distanceSensor.GetGapAvg()*1000 );
		SetDlgItemText(IDC_STATIC_DISTANCE_MEASURE, str );
	} else {
		SetDlgItemText(IDC_STATIC_DISTANCE_MEASURE, "측정 : X" );
	}


	// 리볼버 연결상태
	if( m_lensChanger.IsConnected() )
	{
		SetDlgItemText(IDC_STATIC_REVOLVER_CONNECT, "연결됨" );
		m_stcRevoverConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_REVOLVER_CONNECT, "연결않됨" );
		m_stcRevoverConnection.SetBkColor(RGB(255,0,0));
	}

	// 리볼버 servo상태
	if( m_lensChanger.IsServoOn() )
	{
		SetDlgItemText(IDC_STATIC_REVOLVER_SERVO, "서보 켜짐" );
		m_stcRevoverServo.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_REVOLVER_SERVO, "서보 꺼점" );
		m_stcRevoverServo.SetBkColor(RGB(255,0,0));
	}

	// 리볼버 동작 상태 표시
	if( m_lensChanger.IsRunning() )
	{
		SetDlgItemText(IDC_STATIC_REVOLVER_STATE, "구동중" );
		m_stcRevoverRun.SetBkColor(RGB(0,0,255));
	} else {
		SetDlgItemText(IDC_STATIC_REVOLVER_STATE, "대기중" );
		m_stcRevoverRun.SetBkColor(RGB(255,255,255));
	}

	// 소켓 통신 상태 표시
	if( m_socketModule.IsConnection() )
	{
		SetDlgItemText(IDC_STATIC_SOCKET_CONNECTION, "연결" );
		m_stcSocketConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_SOCKET_CONNECTION, "대기중" );
		m_stcSocketConnection.SetBkColor(RGB(255,0,0));
	}

	// 두께 측정용 접촉센서 상태 표시
	if( m_thicknessSensor.IsConnected() )
	{
		SetDlgItemText(IDC_STATIC_CONTACT_SENSOR_CONNECTION, "연결" );
		m_stcContactSensorConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_CONTACT_SENSOR_CONNECTION, "대기중" );
		m_stcContactSensorConnection.SetBkColor(RGB(255,0,0));
	}

	// 프로그램 상태 표시
	switch( m_state )
	{
		case STATE_NONE:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "대기중");
			m_stcProgram.SetBkColor(RGB(255,255,255));
			break;

		case STATE_CALIBRATION:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "Calibation...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_AUTOFOCUS:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "Auto focus...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_MEASURE:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "측정중...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_RUNING:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "동작중...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		default:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "???");
			m_stcProgram.SetBkColor(RGB(255,0,0));
	}

	CDialogEx::OnTimer(nIDEvent);
}


//////////////////////////////////////////////////////////////////////////
//
// 3D 측정 완료 시
//
//////////////////////////////////////////////////////////////////////////
LRESULT CMy3DmeasureDlg::OnReceive(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here

	dispay_message("3D 측정 > 완료");

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// 3D 측정 완료 시
//
//////////////////////////////////////////////////////////////////////////
LRESULT CMy3DmeasureDlg::OnUpdateData(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here

	UpdateData(FALSE);

	return 0;
}


void CMy3DmeasureDlg::OnBnClickedButtonConnectDistSensor()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("거리센서 연결 > ");


// 	CString str;
// 	str.Format(_T("30, 169.254.168.150, TCP/IP, 300, 0.0, 3.0,-10.0, 0.001"));

	// Get ip
// 	CString strIp = "30, " + m_distanceSensorIP + ", TCP/IP, 300, 0.0, 3.0,-10.0, 0.001";
	CString strIp = "33, " + m_distanceSensorIP + ", TCP/IP, 300, 0.0, 3.0,-10.0, 0.001";

// 	if( !m_distanceSensor.Connect(strIp) )
	if( !m_distanceSensor.Connect(m_distanceSensorIP) )
	{
		dispay_message("거리센서 연결 > 실패");
	}


	// 
	dispay_message("거리센서 연결 > 완료");



}


void CMy3DmeasureDlg::OnBnClickedButtonDisconnectDistSensor()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_distanceSensor.Disconnect();
}


void CMy3DmeasureDlg::OnBnClickedButtonMeasureDist()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( m_distanceSensor.IsConnected() )
	{
		m_distanceSensor.StartContinousSensing();
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonMeasureDistOnece()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.



	if( m_distanceSensor.IsConnected() )
	{
		double ret = m_distanceSensor.sensing(10);

		CString str;
		str.Format("거리값 : %.5f", ret );
		dispay_message(str);
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonStopMeasureDist()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( m_distanceSensor.IsConnected() )
	{
		m_distanceSensor.StopSensing();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 표준 두께 시료를 이용한 calibration
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonCalibration()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if( changePgmState( STATE_CALIBRATION ) )
	{
		SetEvent( m_thread.m_hEventCalibration );
	}
}


void CMy3DmeasureDlg::OnEnChangeEditCalibrationThick()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioThicknessMethodDirect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioThicknessMethodIndirect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonServoOn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_lensChanger.servoOn();
}


void CMy3DmeasureDlg::OnBnClickedButtonServoOff()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_lensChanger.servoOff();
}


//////////////////////////////////////////////////////////////////////////
//
// 렌즈 배율을 10배로 변경한다.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// 렌즈 배율 변경
	if( IsPgmStateFree() )	// 유휴 상태일 때 실행
	{
		// 명려을 내리는 자
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// 목표 배율 설정
		m_iLensMag_taget = 10;

		// 변경 스레드 실행
		SetEvent( m_thread.m_hEventChangeLens );
	}

// 	if( changePgmState(STATE_RUNING) )
// 	{
// 		m_lensChanger.go10x();
// 
// 		changePgmState(STATE_NONE);
// 	}
}


//////////////////////////////////////////////////////////////////////////
//
// 렌즈 배율을 20배로 변경한다.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens20()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 렌즈 배율 변경
	if( IsPgmStateFree() )	// 유휴 상태일 때 실행
	{
		// 명려을 내리는 자
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// 목표 배율 설정
		m_iLensMag_taget = 20;

		// 변경 스레드 실행
		SetEvent( m_thread.m_hEventChangeLens );
	}


// 	if( changePgmState(STATE_RUNING) )
// 	{
// 		m_lensChanger.go20x();
// 
// 		changePgmState(STATE_NONE);
// 	}
}


//////////////////////////////////////////////////////////////////////////
//
// 렌즈 배율을 50배로 변경한다.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens50()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 렌즈 배율 변경
	if( IsPgmStateFree() )		// 유휴 상태일 때 실행
	{
		// 명려을 내리는 자
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// 목표 배율 설정
		m_iLensMag_taget = 50;

		// 변경 스레드 실행
		SetEvent( m_thread.m_hEventChangeLens );
	}

// 	if( changePgmState(STATE_RUNING) )
// 	{
// 		m_lensChanger.go50x();
// 
// 		changePgmState(STATE_NONE);
// 	}
}


//////////////////////////////////////////////////////////////////////////
//
// 렌즈 배율을 150배로 변경한다.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens150()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// 렌즈 배율 변경
	if( IsPgmStateFree() )	// 유휴 상태일 때 실행
	{
		// 명려을 내리는 자
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// 목표 배율 설정
		m_iLensMag_taget = 150;

		// 변경 스레드 실행
		SetEvent( m_thread.m_hEventChangeLens );
	}

// 	if( changePgmState(STATE_RUNING) )
// 	{
// 		m_lensChanger.go150x();
// 
// 		changePgmState(STATE_NONE);
// 	}
}


void CMy3DmeasureDlg::OnBnClickedButtonConnectRevolver()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("렌즈 체인저 통신 연결 > ");

	if( m_lensChanger.IsConnected()==false )
	{
		int iPortNum = m_ctrlRevolverPortNum.GetCurSel() + 1;

		m_lensChanger.m_setPortNum(iPortNum);


		if( !m_lensChanger.connect() )
		{
			dispay_message("렌즈 체인저 통신 연결 > 실패");
		} 

		dispay_message("렌즈 체인저 통신 연결 > 완료");
	}
	else
	{
		dispay_message("렌즈 체인저 통신 연결 > 실패 > 이미 연결됨");
	}

	
}


void CMy3DmeasureDlg::OnBnClickedButtonDisconnectRevolver()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("렌즈 체인저 통신 해제 >");


	if( !m_lensChanger.disconnect() )
	{
		dispay_message("렌즈 체인저 통신 해제 > 실패");
	}

	dispay_message("렌즈 체인저 통신 해제 > 완료");
}


///////////////////////////////////////////////////////////
//
// Lens를 Home 위치로 이동한다.
//
///////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonGoHome()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_lensChanger.goHome();
}


void CMy3DmeasureDlg::OnBnClickedButton3dLaserAutofofus()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("Laser AF >");

	if( changePgmState(STATE_RUNING))
	{
		if( !m_3dMeasure.autofocusLaser() )
		{
			dispay_message("Laser AF > 실패");
		} 

		changePgmState(STATE_NONE);
	} else 
	{
		dispay_message("Laser AF > 실패");
	}

	dispay_message("Laser AF > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButton3dImageAutofofus()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("Image AF >");

	if( changePgmState(STATE_RUNING))
	{
		if( !m_3dMeasure.autofocusImage() )
		{
			dispay_message("Image AF > 실패");
		} 

		changePgmState(STATE_NONE);
	} else 
	{
		dispay_message("Image AF > 실패");
	}
		
	
	dispay_message("Image AF > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButton3dUp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( !m_3dMeasure.moveUp(1000) )
	{
		dispay_message("Move > 실패");
	}
}


void CMy3DmeasureDlg::OnBnClickedButton3dDown()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( !m_3dMeasure.moveDown(1000) )
	{
		dispay_message("Move > 실패");
	}
}


void CMy3DmeasureDlg::OnEnChangeEditIteration()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonCalculate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bStop = false;

	if( changePgmState(STATE_MEASURE) )
	{
		SetEvent( m_thread.m_hEventMeasure );
	}

// 	for( int i=0 ; i<m_nIteration ; i++ )
// 	{
// 		//
// 		if( !m_3dMeasure.measure() )
// 		{
// 			i = m_nIteration;
// 			break;
// 		}
// 		dispay_message("3D측정시작");
// 
// 
// 		// 측정 완료까지 대기
// 		while( m_3dMeasure.isMeasuring() )
// 		{
// 
// 		}
// 
// 
// 		//
// 		m_3dMeasure.read3dData();
// 		dispay_message("3D 데이터 획득 완료");
// 
// 
// 		//
// 		int widthHeight, heightHeight;
// 		double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);
// 
// 
// 		int widthLaser, heightLaser;
// 		BYTE *pLaserImage = m_3dMeasure.getLaserImage(&widthLaser, &heightLaser);
// 
// 
// 		double dSlopeAngle=0;
// 		double dStepHeight=0;
// 		m_inspector.setParamer(467.0, 467.0, 214, 207, HOLE_SHAPE_RECTANGLE_45);
// 		m_inspector.measureA( pHeightMap, pLaserImage, widthHeight, heightHeight, &dSlopeAngle, &dStepHeight );
// 
// 
// 		CString strMsg;
// 		strMsg.Format("Slope angle은 %.3f 도 입니다.", dSlopeAngle );
// 		dispay_message(strMsg);
// 	}
}


void CMy3DmeasureDlg::OnBnClickedButtonStopCaculation()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bStop = true;

	changePgmState(STATE_NONE);
}


void CMy3DmeasureDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	//
	if( m_hThreadMeasure )
	{
		delete m_hThreadMeasure;
	}
	m_hThreadMeasure = NULL;

	//
	if( m_hThreadAutofocus )
	{
		delete m_hThreadAutofocus;
	}
	m_hThreadAutofocus = NULL;

	//
	if( m_hThreadCalibration )
	{
		delete m_hThreadCalibration;
	}
	m_hThreadCalibration = NULL;

	//
	if( m_hThreadChangeLens )
	{
		delete m_hThreadChangeLens;
	}
	m_hThreadChangeLens = NULL;

	m_pLogger->release();
}


void CMy3DmeasureDlg::OnBnClickedRadioServerSocket()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioClientSocket()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonSocketCreate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("소켓생성 >");

	m_socketModule.SetParameter( this->GetSafeHwnd(), m_socketPort, m_socketIp );


	switch(m_soketType)
	{
	case 0:
		if( !m_socketModule.Create(_SERVER_) )
		{
			dispay_message("소켓생성 > 실패 : 서버 생성");
		}
		break;
	case 1:
		if( !m_socketModule.Create(_CLIENT_) )
		{
			dispay_message("소켓생성 > 실패 : 클라이언트 생성");
		}
		break;
	default:
		break;;
	}


	dispay_message("소켓생성 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonSocketConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("소켓 연결 >");


	if( m_socketModule.GetStatus()==_CLIENT_ )
	{
		if( !m_socketModule.Start() )
		{
			dispay_message("소켓 연결 > 실패 : 클라이언트 연결");
		}
	} 

	if( m_socketModule.GetStatus()==_SERVER_ )
	{
		if( m_socketModule.Start() )
		{
			dispay_message("소켓 연결 > 대기중 : 클라이언트 연결 대기중");
		} else {
			dispay_message("소켓 연결 > 실패");
		}
	} 


	dispay_message("소켓 연결 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonSocketDisconnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_socketModule.Stop();
}


LRESULT CMy3DmeasureDlg::OnAcceptSocket(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here

	dispay_message("클라이언트가 연결됨");

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// 운영 프로그램으로 부터 명령 수신 시
//
//////////////////////////////////////////////////////////////////////////
LRESULT CMy3DmeasureDlg::OnReceiveSocket(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here
	unsigned int szReceive=0;

	BYTE *pReceiveData = m_socketModule.OnReceive(&szReceive);	

	CString str;
	str.Format("size of received data = %d", szReceive);
	dispay_message(str);


	//
	// 레시피 설정 명령 수신시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_RECIPE)
	{
		if( changePgmState( STATE_RUNING ) )
		{
			//////////////////////////////////////////////////////////////////////////
			// 검사 제품정보 설정
			PARAM_MASK_INFO maskInfo;
			maskInfo.iPitchX	= (int)(m_protocol.m_dPitchX/m_inspector.getResolution());
			maskInfo.iPitchY	= (int)(m_protocol.m_dPitchY/m_inspector.getResolution());
			maskInfo.iWidth		= (int)(m_protocol.m_dSizeX/m_inspector.getResolution());
			maskInfo.iHeight	= (int)(m_protocol.m_dSizeY/m_inspector.getResolution());
			maskInfo.shape		= (HOLE_SHAPE)m_protocol.m_iShape;			// 네모/마름모/원
			maskInfo.arrangement = (HOLE_ARRANGEMENT)m_protocol.m_iArrage;	// 사각/대각
			maskInfo.dThickness = m_protocol.m_dThickness;

			// 이노텍용 임시 코드
			if( maskInfo.shape== HOLE_SHAPE_AUTO_RG)
			{
				maskInfo.shape = HOLE_SHAPE_AUTO;
			}

			m_inspector.setParamImage( maskInfo );
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 수신 내용 UI 표시
			m_dHolePitchX = maskInfo.iPitchX;
			m_dHolePitchY = maskInfo.iPitchY;
			m_dHoleWidth = maskInfo.iWidth;
			m_dHoleHeight = maskInfo.iHeight;
			m_ctrlHoleShape.SetCurSel(maskInfo.shape);
			m_ctrlHoleInfoArrange.SetCurSel(maskInfo.arrangement);
			m_targetThickness = maskInfo.dThickness;

			UpdateData(FALSE);
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 결과 송신
			m_socketModule.SendSetReceipeResult( true );
			//////////////////////////////////////////////////////////////////////////

			changePgmState( STATE_NONE );

			//
			CString str;
			str.Format("Parameter 설정 수신 > 성공 : pitch x = %f um, pitch y = %f um, size x = %f um, size x = %f um, shape = %d, arrange = %d, thickness = %f", 
				m_protocol.m_dPitchX, m_protocol.m_dPitchY, m_protocol.m_dSizeX, m_protocol.m_dSizeY, m_protocol.m_iShape, m_protocol.m_iArrage, m_targetThickness);
			dispay_message(str);
		} 
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// 결과 송신
			m_socketModule.SendSetReceipeResult( false );
			//////////////////////////////////////////////////////////////////////////

			dispay_message("Parameter 설정 수신 > 실패 : 다른 동작중");
		}
	}


	//
	// Calibration 명령 수신
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_CALIBRATION)
	{
		if( changePgmState( STATE_CALIBRATION ) )
		{
			// Calibration thread 실행
			SetEvent( m_thread.m_hEventCalibration );
		} 
		else 
		{
			m_socketModule.SendCalibrationResult( false );
			dispay_message("CAL 명령 수신 > 실패 : 다른 동작중");
		}
	}


	//
	// 프로그램 상태 요청 수신 시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_STATE)
	{
// 		dispay_message("통신 > 현재 상태 확인 요청");

		// UI 갱신
		ToggleStateCheck();

		// 3D sensor에서 렌즈 위치 정보 얻기
// 		int iLensPosAtSensor = 0;
// 		m_3dMeasure.GetCurrentLensPos(&iLensPosAtSensor);

		// 렌즈 체인져에서 렌즈 위치 정보 얻기
		int iLensPosAtChanger = m_lensChanger.getCurrentLens();

// 		int iLensPos = 0;
// 		if( iLensPosAtSensor == iLensPosAtChanger )
// 		{
// 			iLensPos = iLensPosAtSensor;
// 		} else {
// 			iLensPos = -1;
// 		}
	
		// 렌즈 상태 정보 올리기
		if( m_state==STATE_NONE)
		{	// 검사중이 아닐때
			m_socketModule.SendState(false, m_lensChanger.getCurrentLens());
// 			m_socketModule.SendState(false, iLensPos);
		} else {
			// 검사중일 때
 			m_socketModule.SendState(true, m_lensChanger.getCurrentLens());
// 			m_socketModule.SendState(true, iLensPos);
		}
	}


	//
	// 측정 중단 명령 수신 시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_STOP)
	{
		dispay_message("통신 > 중지 요청");
	
		if( m_state==STATE_MEASURE)
		{
			stopMeasure();
			m_socketModule.SendStop(true);
		} else {
			m_socketModule.SendStop(false);
		}
	}


	//
	// 10배 렌즈 변경 요청 수신 시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_LENS10X)
	{
		// 렌즈 배율 변경
		if( IsPgmStateFree() )	// 유휴 상태일 때 실행
		{
			// 명려을 내리는 자
			m_iLensMag_src = _LENS_CMD_FROM_OP_;

			// 목표 배율 설정
			m_iLensMag_taget = 10;

			SetEvent( m_thread.m_hEventChangeLens );
		} else {
			m_socketModule.SendLensChangeResult( false );

			dispay_message("렌즈 변경 명령 수신 > 실패 : 다른 동작중");
		}

// 		dispay_message("통신 > 10배로 렌즈 변경 요청");
// 
// 		if( changePgmState( STATE_RUNING ) )
// 		{
// 			if( m_lensChanger.go10x() )
// 			{
// 				m_socketModule.SendLensChangeResult( true );
// 			} 
// 			else 
// 			{
// 				dispay_message("렌즈 변경 명령 수신 > 처리 실패");
// 
// 				m_socketModule.SendLensChangeResult( false );
// 			}
// 
// 			dispay_message("렌즈 변경 명령 수신 > 끝");
// 
// 			changePgmState( STATE_NONE );
// 		} 
// 		else 
// 		{
// 			m_socketModule.SendLensChangeResult( false );
// 			dispay_message("렌즈 변경 명령 수신 > 실패 : 다른 동작중");
// 		}	
	}


	//
	// 대공경 측정 명력 요청 수신 시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_MESURE_LARGE)
	{
		dispay_message("통신 > 대공경 측정 요청");

		//
		if( changePgmState( STATE_MEASURE ) )
		{
			//
			m_iModeMeasureMethod = MEASURE_METHOD_LARGE_HOLE;
			UpdateData(FALSE);

			m_strRecipe = m_protocol.m_strRecipe;
			m_strQRCode = m_protocol.m_strQRCode;
			
			m_bUseThicknessSensor = m_protocol.m_bUseThickness;
			m_stRecip3D.dScanRangeUp = m_protocol.m_d3dRangeUp;
			m_stRecip3D.dScanRangeDown = m_protocol.m_d3dRangDown;
// 			m_dUpperPos = m_protocol.m_d3dRangeUp;	// UI 갱신
// 			m_dLowerPos = m_protocol.m_d3dRangDown;	// UI 갱신

			m_3dMeaserRangeUpperOffset = m_protocol.m_d3dRangeUp;
			m_3dMeaserRangeLowerOffset = m_protocol.m_d3dRangDown;

			UpdateData(FALSE);


			//
			SetEvent( m_thread.m_hEventMeasure );

// 			{
// 				m_socketModule.SendMeasureResult(true, 4, 0);
// 				changePgmState(STATE_NONE);
// 			}

			//
			CString str;
			str.Format(	"통신 > 측정 요청 > 대공경 검사 시작 : [레시피 %s] [QR %s] [두께센서 사용 %s] [측정범위 %f, %f]", 
						m_protocol.m_strRecipe,
						m_protocol.m_strQRCode,
						(m_bUseThicknessSensor)? "예" :"아니오", 
						m_protocol.m_d3dRangeUp, 
						m_protocol.m_d3dRangDown);
			dispay_message(str);
		} 
		else 
		{
			m_socketModule.SendMeasureResult( false, 0, 0 );

			dispay_message("통신 > 측정 요청 > 대공경 검사 실패 > 타작업 진행중");
		}

	}


	//
	// 소공경 측정 명력 요청 수신 시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_MESURE_SMALL)
	{
		dispay_message("통신 > 소공경 측정 요청");

		//
		if( changePgmState( STATE_MEASURE ) )
		{
			// 측정
			m_iModeMeasureMethod = MEASURE_METHOD_SMALL_HOLE;
			UpdateData(FALSE);

			SetEvent( m_thread.m_hEventMeasure );

// 			{
// 				m_socketModule.SendMeasureResult(true, 4, 0);
// 				changePgmState(STATE_NONE);
// 			}

			// display
			dispay_message("통신 > 측정 요청 > 소공경 검사 시작");
		} 
		else 
		{
			m_socketModule.SendMeasureResult( false, 0, 0 );

			dispay_message("통신 > 측정 요청 > 대공경 검사 실패 > 타작업 진행중");
		}

	}


	//
	// Auot focus 명령 요청 수신 시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_AUTOFOCUS)
	{
		if( changePgmState( STATE_AUTOFOCUS ) )
		{
			SetEvent( m_thread.m_hEventAutofocus );
		} else {
			m_socketModule.SendAutoFocusResult( false, 0, 0 );

			dispay_message("AF 명령 수신 > 실패 : 다른 동작중");
		}
	}

	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_FAIL)
	{
	}


	//
	// 두께 측정 명령 요청 수신 시
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_THICKNESS)
	{
		dispay_message("통신 > 두께 측정 요청");

		if( changePgmState( STATE_RUNING ) )
		{
// 			if( m_stSite == SITE_LGD )
// 			{
// 				OnBnClickedButtonMeasureDistOnece();	// 측정 
// 
// 				changePgmState( STATE_NONE );	// 상태 갱신
// 			}


// 			if( m_stSite == SITE_LGD )
			{
				m_requests.measureThickness = TRUE;	// 두께 측정 ?
				m_requests.measureThicknessCAL = m_protocol.m_bCAL;	// Calibration ?
				m_requests.measureThicknessPos = m_protocol.m_thicknessIndex;	// 위치 ?

				if( m_protocol.m_bCAL )
				{
					m_thicknessSensor.setState(DS_MODE_CALIBRATION, m_protocol.m_thicknessIndex);
				} else {
					m_thicknessSensor.setState(DS_MODE_MEASURE, m_protocol.m_thicknessIndex);
				}

				// 측정 시작
				if( !m_thicknessSensor.measureAll() )
				{
					changePgmState( STATE_NONE );
					m_requests.measureThickness = FALSE;	// 두께 측정 ?
					m_socketModule.SendThicknessResult( -99 );	// 결과 송부

					dispay_message("통신 > 두께 측정 요청 > 실패 > 측정불가");
				}
			}
		} else {
			dispay_message("통신 > 두께 측정 요청 > 실패 > 다른 작업 진행중");
		}
	}


	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// 두께 측정 센서에서 데이터 획득 시...
//
//////////////////////////////////////////////////////////////////////////
LRESULT CMy3DmeasureDlg::OnReceiveThichness(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here
	CString str;
	str.Format("접촉센서 > 측정결과 > 거리값 > %f um", m_thicknessSensor.m_dValue );
	dispay_message(str);


	if( m_requests.measureThickness )	// 운용 SW로부터 두께 측정 명령을 받았으면...
	{
		m_socketModule.SendThicknessResult( m_thicknessSensor.GetThickness(m_requests.measureThicknessPos) );	// 결과 송부

		str.Format("접촉센서 > 측정결과 > 두께값 > %f um", m_thicknessSensor.GetThickness(m_requests.measureThicknessPos) );
		dispay_message(str);

		// 측정 요청 초기화
		m_requests.measureThickness = FALSE;


		changePgmState( STATE_NONE );
	}


	return 0;
}


// void CMy3DmeasureDlg::OnBnClickedRadioMeasureSlopeangle()
// {
// 	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
// 	UpdateData(TRUE);
// }


void CMy3DmeasureDlg::OnBnClickedRadioMeasureStepHeight()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioMeasureSlopeangleNStepHeight()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}

void CMy3DmeasureDlg::OnBnClickedRadioMeasureThickness()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonStopCalculation()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CMy3DmeasureDlg::OnBnClickedButton3dSetUpperPos()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if( !m_3dMeasure.setUpperPos((long)(m_dUpperPos*1000)) )
	{
		dispay_message("setUpperPos > 실패");
	}

	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedButton3dSetLowerPos()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if( !m_3dMeasure.setLowerPos((long)(m_dLowerPos*1000)) )
	{
		dispay_message("setLowerPos > 실패");
	}

	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedButton3dSetDistance()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if( !m_3dMeasure.setDistance((long)(m_d3dRangeDistance*1000)) )
	{
		dispay_message("setDistance > 실패");
	}

	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedRadioModeAuto()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioModeManual()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit3dLowerDistance()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit3dUpperDistance()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButton3dAutoRange()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_3dMeasure.setMeasureRangeOffset( (long)(m_3dMeaserRangeLowerOffset*1000), (long)(m_3dMeaserRangeUpperOffset*1000));


	if( m_3dMeaserRangeLowerOffset<=100 && m_3dMeaserRangeUpperOffset<= 100 )
	{
		m_stRecip3D.dScanRangeUp = m_3dMeaserRangeUpperOffset;
		m_stRecip3D.dScanRangeDown = m_3dMeaserRangeLowerOffset;
	} else {
		AfxMessageBox("측정 범위는 100 um 이내로 가능합니다.");
	}


	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedButton3dSelectViewType()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() );
}


void CMy3DmeasureDlg::OnEnChangeEdit6()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit3()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit4()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit5()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonSetHoleShapeInfo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("구멍 형상 정보 입력 >");


	m_stMaskInfo.iPitchX	= (int)m_dHolePitchX;
	m_stMaskInfo.iPitchY	= (int)m_dHolePitchY;
	m_stMaskInfo.iWidth		= (int)m_dHoleWidth;
	m_stMaskInfo.iHeight	= (int)m_dHoleHeight;
	m_stMaskInfo.shape		= (HOLE_SHAPE)m_ctrlHoleShape.GetCurSel();
	m_stMaskInfo.arrangement = (HOLE_ARRANGEMENT)m_ctrlHoleInfoArrange.GetCurSel();
	m_stMaskInfo.dThickness = m_targetThickness;
	m_stMaskInfo.dStepHeightOffset = m_offet;

	m_inspector.setParamImage( m_stMaskInfo );


	dispay_message("구멍 형상 정보 입력 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedButtonAutoFocus()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	dispay_message("▼ Auto focus 절차 >");

	if( changePgmState(STATE_RUNING) )
	{
		if( !autoFocus() )
		{
			dispay_message("Auto focus 절차 > 실패");
		}

		changePgmState(STATE_NONE);
	} else 
	{
		dispay_message("Auto focus 절차 > 실패");
	}


	dispay_message("▲ Auto focus 절차 > 완료");
}


void CMy3DmeasureDlg::OnBnClickedCheckDisplayProcessingImage()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

	if( m_ctrlDisplayProcessingImage)
	{
		m_inspector.m_bDisplayImage = true;
	} else 
	{
		m_inspector.m_bDisplayImage = false;
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonTestMaskMovement()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	if( changePgmState(STATE_RUNING) )
	{
		sequenceCheckMaskMovement();

		changePgmState(STATE_NONE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 대공경 검사
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonTest2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//
	dispay_message("Test 2 >");


	if( changePgmState(STATE_RUNING) )
	{
		CString strMsg;
		CString strFilePath;
		list<double> listStepHeight;
		list<double> listSlopeAngle;


		makeSaveFolder();

		// 이전 데이터 삭제
		m_inspector.clearResult();


		// 측정
		int nTime = 1;
		for( int i=0 ; i<nTime ; i++ )
		{

			//////////////////////////////////////////////////////////////////////////
			// 데이터 읽기
			start_local_counter();

			// Height map
			strFilePath.Format("c:\\Test data\\height map %d.csv", i);
			m_3dMeasure.loadHeightMap(strFilePath);

			int widthHeight, heightHeight;
			double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);


			// Laser image
			strFilePath.Format("c:\\Test data\\laser map %d.csv", i);
			m_3dMeasure.loadLaserMap(strFilePath);

			int widthLaser, heightLaser;
			BYTE *pLaserImage = m_3dMeasure.getLaserImage(&widthLaser, &heightLaser);


			// 

			strFilePath.Format("c:\\Test data\\dic image %d.bmp", i);
			m_3dMeasure.loadDicImage(strFilePath);

			int widthDic, heightDic;
			BYTE *pDicImage = m_3dMeasure.getDicImage( &widthDic, &heightDic );

			end_local_counter("대공경검사 > 데이터 읽기");
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 측정
			start_local_counter();

			double dSlopeAngle=0;

			if( m_bUseThicknessSensor )
			{
				m_inspector.setTargetThickness( m_thicknessSensor.GetThickness() );
			} else {
				m_inspector.setTargetThickness( m_targetThickness );
			}

			if( !m_inspector.measureSlopeAngleAtLargeHoleRegion( pHeightMap, pLaserImage, pDicImage, widthHeight, heightHeight, m_distanceSensor.GetGap()*1000, &listSlopeAngle, &listStepHeight ) )
			{
				dispay_message("Test 2 > 실패");
			}

			end_local_counter("대공경검사 > 측정");
			//////////////////////////////////////////////////////////////////////////

		}


		// 결과 표시 UI
		list<double>::iterator ite1;
		list<double>::iterator ite2 = listStepHeight.begin();
		list<double>::iterator ite3 = m_inspector.m_listThickness.begin();
		for( ite1 = listSlopeAngle.begin() ; ite1!=listSlopeAngle.end() ; ite1++, ite2++, ite3++ )
		{
			strMsg.Format("angle=%f, height=%f(%f)", *ite1,  *ite2, *ite3 );
			dispay_message(strMsg);
		}

		// 결과 파일 저장 
		start_local_counter();
		{
			int nMesurePoint = m_inspector.getMeasurePointNum();
			double *pMeasureAnlge = m_inspector.getMeasurePointAngle();


			CStdioFile file2;
			CString strPath2;
			strPath2.Format("c:\\test data\\Manual test result.csv", m_nCnt++);

			if( file2.Open(strPath2, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
			{
				CString strTemp;


				// angle
				file2.WriteString("Slope angle\n");
// 				file2.WriteString("0도,45도,90도,135도,180도,225도,270도,315도\n");
// 				file2.WriteString("0도,15도,30도,45도,60도,75도,90도,105도,120도,135도,150도,165도,180도,195도,210도,225도,240도,255도,270도,285도,300도,315도,330도,345도\n");
				for( int j=0 ; j<nMesurePoint ; j++ )
				{
					strTemp.Format("%.1f도,", pMeasureAnlge[j]);
					file2.WriteString(strTemp);
				}
				file2.WriteString("\n");


				int n=1;
				for( list<double>::iterator ite = listSlopeAngle.begin() ; ite!=listSlopeAngle.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}

				// angle
				file2.WriteString("\nstep height\n");
				n=1;
				for( list<double>::iterator ite = listStepHeight.begin() ; ite!=listStepHeight.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}

				// 
				file2.WriteString("\n3D data(Height of Island)\n");
				n=1;
				for( list<double>::iterator ite = m_inspector.m_list3DdataIslandHeight.begin() ; ite!=m_inspector.m_list3DdataIslandHeight.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}


				// 
				file2.WriteString("\n3D data(Height of hole edge )\n");
				n=1;
				for( list<double>::iterator ite = m_inspector.m_list3DdataHoleEdgeHeight.begin() ; ite!=m_inspector.m_list3DdataHoleEdgeHeight.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}


				// 
				file2.WriteString("\n3D data(Disntance of angle )\n");
				n=1;
				for( list<double>::iterator ite = m_inspector.m_list3DdataIslandEdgeDistance.begin() ; ite!=m_inspector.m_list3DdataIslandEdgeDistance.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}


				// 
				file2.WriteString("\nDistance sensor data\n");
				n=1;
				for( list<double>::iterator ite = m_inspector.m_listDistanceData.begin() ; ite!=m_inspector.m_listDistanceData.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}


				// 
				file2.WriteString("\nStep height(Assume thickness)\n");
				n=1;
				for( list<double>::iterator ite = m_inspector.m_listThickness.begin() ; ite!=m_inspector.m_listThickness.end() ; ite++ )
				{
					strTemp.Format("%f,", *ite);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}

				// 두께
				file2.WriteString("\nThickness\n");
				n=1;
				for( list<double>::iterator ite = m_inspector.m_listThickness.begin() ; ite!=m_inspector.m_listThickness.end() ; ite++ )
				{
					strTemp.Format("%f,", m_inspector.getMaskInfo()->dThickness);
					file2.WriteString(strTemp);

					//
					if( n%nMesurePoint == 0 )
					{
						file2.WriteString("\n");
					}

					n++;
				}


				file2.Close();
			}
		}
		end_local_counter("대공경검사 > 결과 파일 저장");


		changePgmState(STATE_NONE);



		dispay_message("Test 2 > 완료");
	} else 
	{
		dispay_message("Test 2 > 실패");
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButton2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
// 	CDistanceSensor *pSensor;
// 
// 	pSensor = new CDistanceSensor;
// 	pSensor->Create(NULL,NULL,WS_CHILD, CRect(0,0,0,0), this, NULL );

	/*
	tinyxml2::XMLDocument xmlDocument;
	tinyxml2::XMLNode *pNode;

	pNode = xmlDocument.NewElement("message");
	xmlDocument.InsertFirstChild( pNode );

	//
	tinyxml2::XMLElement *pElement = xmlDocument.NewElement("body");
	pNode->InsertEndChild(pElement);

	tinyxml2::XMLElement *pElement2 = xmlDocument.NewElement("test");
	pElement2->SetText("REQUEST");
	pElement->InsertEndChild(pElement2);

	//
	pElement = xmlDocument.NewElement("header");
	pNode->InsertEndChild(pElement);

	pElement2 = xmlDocument.NewElement("messageName");
	pElement2->SetText("REQUEST");
	pElement->InsertEndChild(pElement2);


	// File로 저장하기
	// 	tinyxml2::XMLError err = xmlDocument.SaveFile(XML_FILE_PATH);

	tinyxml2::XMLPrinter xmlPrinter;
	tinyxml2::XMLDocument xmlDocument2;
	xmlDocument.Print(&xmlPrinter);
	char *pTmp = (char*)xmlPrinter.CStr();

	CString str;
	str.Format("%s", pTmp);
	CString str1;
	str1.Format("%c", 2 );
	CString str2;
	str2.Format("%c", 3 );

	CString str3 = str1+str+str2;*/


	// 시간 구하기
/*	COleDateTime tm = COleDateTime::GetCurrentTime();
	
	CString str;
	str.Format(	"d:\\%d_%2d_%2d_%2d_%2d_%2d", 
				tm.GetYear(), 
				tm.GetMonth(), 
				tm.GetDay(), 
				tm.GetHour(), 
				tm.GetMinute(), 
				tm.GetSecond() );


	// 폴더 생성하기
	CreateDirectory( str, NULL );
	*/
		// 시간 구하기
// 	makeSaveFolder();
// 
// 	m_stc3dConnection.SetBkColor(RGB(0,255,0));

// 	ToggleStateCheck();


	int iLensPosAtSensor = 0;
	m_3dMeasure.getZoom(&iLensPosAtSensor);

	CString strMessage;
	strMessage.Format("Lens position(sensor) = %d, Lens position(changer) = %d", iLensPosAtSensor, m_lensChanger.getCurrentLens() );
	dispay_message(strMessage);
}


///////////////////////////////////////////////////////////
//
// Lens가 없는 위치로 레볼버를 움직인다.
//
///////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLensNone()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_lensChanger.goNone();
}


void CMy3DmeasureDlg::OnEnChangeDistanceSensorIp()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
// 3D 측정 시 거리센서 측정 여부 선택
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedCheckUseDistance()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
// 두께 측정용 센서와 연결
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonThicksensorOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( !m_thicknessSensor.m_Comm )
	{
		m_thicknessSensor.close();

		// 통신 PORT 설정
		m_thicknessSensor.SetPortNum( GetPortNumber_thickSensor() );

		// 통신 PORT 열기
		if( m_thicknessSensor.open() )
		{
			dispay_message("접촉센서 > Open > 성공");
		} else {
			dispay_message("접촉센서 > Open > 실패");
		}
	} else {
		dispay_message("접촉센서 > Open > 이미 연결됨");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 두께 측정 센서에 측정 요청을 날린다.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonThicksensorRead()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if( m_thicknessSensor.measureAll() )
	{
		dispay_message("접촉센서 > 측정 요청 > 성공");
	} else {
		dispay_message("접촉센서 > 측정 요청 > 실패");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// 측정 step height에 대한 offset 적용
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnEnChangeEidtMeasureOffset()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);

// 	m_inspector.setOffset(m_offet);
}


void CMy3DmeasureDlg::OnBnClickedCheckThicknessSensorUse()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEditTargetThickness()
{
	// TODO:  RICHEDIT 컨트롤인 경우, 이 컨트롤은
	// CDialogEx::OnInitDialog() 함수를 재지정 
	//하고 마스크에 OR 연산하여 설정된 ENM_CHANGE 플래그를 지정하여 CRichEditCtrl().SetEventMask()를 호출하지 않으면
	// 이 알림 메시지를 보내지 않습니다.

	// TODO:  여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonInitialize()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	// 3D 광학계 연결
	OnBnClickedButtonConnect();


	// 레볼버 연결
	OnBnClickedButtonConnectRevolver();

	// 10배 렌즈로 변경
	AfxMessageBox("3D 광학계 렌즈를 10배로 변경합니다. 렌즈 변경 시 충돌 우려는 없는지 반드시 확인해 주세요");
	OnBnClickedButtonLens10();


	// 두께 측정 센서 연결
	if( m_bUseThicknessSensor )
	{
		OnBnClickedButtonThicksensorOpen();
	}


	// 운영 SW 연결
	OnBnClickedButtonSocketCreate();
	OnBnClickedButtonSocketConnect();

}


void CMy3DmeasureDlg::OnBnClickedButtonRecipe()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogRecipe dlg;


	// 기존 정보 로딩
	dlg.init( &m_stMaskInfo );


	//
	if( dlg.DoModal()==IDOK )
	{
		m_stMaskInfo.iPositionNum = dlg.getData(m_stMaskInfo.dPositionAngle, m_stMaskInfo.bPositionPeak);
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveVk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	TCHAR BASED_CODE szFilter[] = _T("VK4 file (*.vk4)|*.vk4|");
	CFileDialog dlgMeasurementDataSave(FALSE, _T("vk4"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	if (dlgMeasurementDataSave.DoModal() != IDOK)
	{
		return;
	}

	CString strPath = dlgMeasurementDataSave.GetPathName();
	VKResult result = (VKResult)VK_SaveMeasurementResult(strPath);
	switch(result)
	{
	case VKResult_OK:
		break;
	case VKResult_NotAccepted:
		AfxMessageBox(_T("Invalid status."));
		return;
	case VKResult_InvalidArgument:
		AfxMessageBox(_T("Invalid parameter."));
		return;
	default:
		AfxMessageBox(_T("Failed to save."));
		return;
	}

}


int CMy3DmeasureDlg::GetPortNumber_thickSensor()
{
	int iPortNum = m_comboComPort_thickSensor.GetCurSel() + 1;

	return iPortNum;
}


void CMy3DmeasureDlg::ToggleStateCheck()
{
	if( m_bStateCheck )
	{
		m_stcStateCheck.SetBkColor( RGB(255,255,255) );

		m_bStateCheck = FALSE;
	} else {
		m_stcStateCheck.SetBkColor( RGB(0,255,0) );

		m_bStateCheck = TRUE;
	}


}
