
// 3D measureDlg.cpp : ���� ����
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


// ���� thread
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

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CMy3DmeasureDlg ��ȭ ����




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


// CMy3DmeasureDlg �޽��� ó����

BOOL CMy3DmeasureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	m_3dMeasure.setParam( this->GetSafeHwnd(), WM_END_MEASURE );

	SetTimer( 1, 200, NULL );


	// ���α׷� ���(�ڵ� �Ǵ� ����)
	m_operationMode = 0;


	// �Ÿ������� IF port ����
	m_ctrlRevolverPortNum.SetCurSel(9);	// COM3 ����
	m_distanceSensorIP = "169.254.168.150";
	m_distanceSensorIP = "192.6.94.3";	// E6


	// 3D ī�޶� �� ī�޶� ����
	m_ctrl3dViewType.SetCurSel(0);


	// �˻��� ���� ����
	m_ctrlHoleShape.SetCurSel(HOLE_SHAPE_DIAMOND);
	m_ctrlHoleInfoArrange.SetCurSel(DIAMOND_ARRANGEMENT);


	// ���˼���
	m_comboComPort_thickSensor.SetCurSel(19);


	//////////////////////////////////////////////////////////////////////////
	// 
	makeLogFolder();

	// ���� ���� �ε�
	loadInit();

	// Dialog item ����
	UpdateData(FALSE);

	// site ���� ����
	setSite();
	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// ��ǰ ���� �Է�
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


	// ���˼����� ���� ������ ����
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

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CMy3DmeasureDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CMy3DmeasureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMy3DmeasureDlg::OnBnClickedButtonConnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("3D ���а� ���� > ");

	if( m_3dMeasure.connect() )
	{
		update3dPosition();
	} else {
		dispay_message("3D ���а� ���� > ����");
	}


	dispay_message("3D ���а� ���� > �Ϸ�");
		
}


void CMy3DmeasureDlg::OnBnClickedButtonDisconnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_3dMeasure.disconnect();
}


void CMy3DmeasureDlg::OnBnClickedButtonMeasure()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("3D ���� > ");

 	if( m_state==STATE_NONE )
	{
		if( m_3dMeasure.measure() )
		{
// 			dispay_message("3D ���� : ...");
		} else {
			changePgmState(STATE_NONE);

			dispay_message("3D ���� > ����");
		}
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveHeight()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("���� ������ ���� > ");

	m_3dMeasure.saveHeightMap();

	dispay_message("���� ������ ���� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadHeight()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("���� ������ �б� >");

	m_3dMeasure.loadHeightMap();

	dispay_message("���� ������ �б� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveLaser()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("������ ������ ���� > ");

	m_3dMeasure.saveLaserMap();

	dispay_message("������ ������ ���� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadLaser()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("������ ������ �б� > ");

	m_3dMeasure.loadLaserMap();

	dispay_message("������ ������ �б� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveDct()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("Save DIC > ");

	m_3dMeasure.saveDicImage();

	dispay_message("Save DIC > OK");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadDct()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("DIC �����б� > ");

	if( m_3dMeasure.loadDicImage() )
	{
		dispay_message("DIC �����б� > �Ϸ�");

	} else {
		dispay_message("DIC �����б� > ����");

	}
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveCcd()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("Save CCD > ");

	if( !m_3dMeasure.saveCcdImage() )
	{
		dispay_message("Save CCD > ����");

		return;
	}

	dispay_message("Save CCD > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonLoadCcd()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("Load CCD image > ");

	if( !m_3dMeasure.loadCcdImage() )
	{
		dispay_message("Load CCD image > ����");

		return;
	}

	dispay_message("Load CCD image > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonReadData()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("3D ������ �ҷ����� > ");

	if( !m_3dMeasure.read3dData() )
	{
		dispay_message("3D ������ �ҷ����� > ����");
	}

	dispay_message("3D ������ �ҷ����� > �Ϸ�");
}


//////////////////////////////////////////////////////////////////////////
//
// �ð� counter ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::start_global_counter()
{
	QueryPerformanceCounter(&m_globalCounterStart);
}



//////////////////////////////////////////////////////////////////////////
//
// �ð� counter �Ϸ�
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
// �ð� counter ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::start_local_counter()
{
	QueryPerformanceCounter(&m_localCounterStart);
}



//////////////////////////////////////////////////////////////////////////
//
// �ð� counter �Ϸ�
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
// Message�� ȭ�鿡 ǥ���Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::dispay_message(CString strMsg)
{
	// dislplay message
	m_ctrlMessage.InsertString(0, strMsg );


	// Debug view message
	OutputDebugString(strMsg+"\n");


	// Log file
	// �ð� ���
	CTime today = CTime::GetCurrentTime();


	// ���ϸ� ����
	CString strName;
	strName.Format( "log%d_%d_%d_%d.log", 
					today.GetYear(),
					today.GetMonth(),
					today.GetDay(),
					today.GetHour());

	CString strPath;
	strPath = PATH_LOG;

	std::string strFileName = CT2CA(strPath + strName);


	// ���ϸ� ����
	m_pLogger->setFileName(strFileName);	// ���ϸ� ����


	// Log ����
	m_pLogger->info(strMsg);
}


//////////////////////////////////////////////////////////////////////////
//
//	���α׷� ���ۻ��� ����
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
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "�����");
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
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "������...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_RUNING:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "������...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_LENS_CHANGE:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "����������...");
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
//	���α׷� ���ۻ��� ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::loadInit( )
{
	dispay_message("ini ���� �б� > ����");

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
			AfxMessageBox("loadInit( )���� site ������ ���� ����");
		}

		// 	�β� ���� ��� ����
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		i = _tstoi(s);
		if( i==0 )
		{
			m_bUseThicknessSensor = FALSE;
		} else {
			m_bUseThicknessSensor = TRUE;
		}

		// �β� ���� ��Ʈ
		is.getline(s, sizeof(s), ',');
		is.getline(s, sizeof(s));
		i = _tstoi(s);

		int port = max( 0, min( 19, i - 1 ) );

		m_comboComPort_thickSensor.SetCurSel(i-1);

	}


	dispay_message("ini ���� �б� > �Ϸ�");
}


//////////////////////////////////////////////////////////////////////////
//
//	Auto focus �ǽ�
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::autoFocus( UINT mag )
{
	// Live view �� ȭ�� ��ȯ
	if( !m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() ) )
	{
		dispay_message(" autoFocus : view ��ȯ ����");

		return false;
	}


	//
	switch( mag )
	{
		case 10:
			// 10�� ����� ��ȯ
			if( !m_lensChanger.go10x() )
			{
				dispay_message(" autoFocus : ����>10�� ���� �̵�");

				return false;
			}


			// Laser AF �ǽ�
 			//if( !m_3dMeasure.autofocusLaser() )
			{
				// image AF �ǽ�
				if( !m_3dMeasure.autofocusImage() )
				{
					dispay_message(" autoFocus : ����>10�� ���� AF");

					return false;
				}
			}
			break;

		case 20:
			// 20�� ����� ����
			if( !m_lensChanger.go20x() )
			{
				dispay_message(" autoFocus : ����>20�� ���� �̵�");

				return false;
			}

			// ������ AF �ǽ�
			if( !m_3dMeasure.autofocusLaser() )
			{
				// image AF �ǽ�
				if( !m_3dMeasure.autofocusImage() )
				{
					dispay_message(" autoFocus : ����>20�� ���� AF");

					return false;
				}
			}
			break;

		case 50:
			// 50�� ����� ����
			if( !m_lensChanger.go50x() )
			{
				dispay_message(" autoFocus : ����>50�� ���� �̵�");
				return false;
			}

			// ������ AF �ǽ�
			//if( !m_3dMeasure.autofocusLaser() )
			{
				// image AF �ǽ�
				if( !m_3dMeasure.autofocusImage() )
				{
					dispay_message(" autoFocus : ����>50�� ���� AF");
					return false;
				}
			}
			break;

		case 150:
			// 50�� ����� ����
			if( !m_lensChanger.go150x() )
			{
				dispay_message(" autoFocus : ����>150�� ���� �̵�");
				return false;
			}

			// ������ AF �ǽ�
			if( !m_3dMeasure.autofocusLaser() )
			{
				dispay_message(" autoFocus : ����>150�� ���� AF");
				return false;
			}
			break;

		default:
			dispay_message(" autoFocus : ����>���� �̻�");
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
		// 10�� ����� ��ȯ
		if( !m_lensChanger.go10x() )
		{
			dispay_message(" changeLens : ����>10�� ���� �̵�");

			return false;
		}
		break;

	case 20:
		// 20�� ����� ����
		if( !m_lensChanger.go20x() )
		{
			dispay_message(" changeLens : ����>20�� ���� �̵�");

			return false;
		}
		break;

	case 50:
		// 50�� ����� ����
		if( !m_lensChanger.go50x() )
		{
			dispay_message(" changeLens : ����>50�� ���� �̵�");
			return false;
		}
		break;

	case 150:
		// 50�� ����� ����
		if( !m_lensChanger.go150x() )
		{
			dispay_message(" changeLens : ����>150�� ���� �̵�");
			return false;
		}
		break;

	default:
		dispay_message(" changeLens : ����>���� �̻�");
		return false;

	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	3D ���а� ���� ���� ����
//
//	- bAuto �� ture�� ��� �ڵ����� ����( ���̾�α� �� )
//	- dLowOffset, dHighOffset [���� : um]
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



	// ���� ���� ����
	if( !m_3dMeasure.setMeasureRangeOffset( lowOffset, highOffset) )
	{
		dispay_message(" setMeasureRange : ����...");
		return false;
	}


	// ȭ�� ����
	update3dPosition();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	3D ī�޶� ��ġ �̵�
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
//	3D ���� �ǽ�
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::measure3d( double *dDistance )
{
	// 1. ���� ����
	dispay_message(" measure3d > ");

	if( !m_3dMeasure.measure() )
	{
		dispay_message(" measure3d > ���� : ��������");

		return false;
	}


	// 2. ���� �Ϸ���� ��� & �Ÿ����� ������ ȹ��
	double fDistanceSum = 0.0;
	UINT nData = 0;
	while( m_3dMeasure.isMeasuring() )
	{
		/* 
		// 3D ������ �Ÿ����� ȹ��
		m_distanceSensor.StopSensing();
		m_distanceSensor.StartSingleSensing(500);

		Sleep(100);	// �ٷ� ������ ������ ȹ������ ���� �о���� �� �ϴ�.

		fDistanceSum += m_distanceSensor.GetGapAvg()*1000;
		nData++;

		CString strTemp;
		strTemp.Format("�Ÿ����� ���� : %.3f\n", m_distanceSensor.GetGapAvg()*1000);
		dispay_message(strTemp);

		Sleep(50);
		*/
	}
//	*dDistance = fDistanceSum/nData;

	*dDistance = 0;


	// 3. Read 3D data
	if( !m_3dMeasure.read3dData() )
	{
		dispay_message(" measure3d > ���� : 3D ������ ȹ��");
		
		return false;
	}

	dispay_message(" measure3d > �Ϸ�");

	return true;
}


void CMy3DmeasureDlg::measureSlopeAngle()
{
	dispay_message("slope angle ���� ����");

	dispay_message("slope angle ���� �Ϸ�");

}


bool CMy3DmeasureDlg::measure3d()
{
	dispay_message("3D ���� >");


	//
	if( !m_3dMeasure.measure() )
	{
		dispay_message("3D ���� > ���� > scan ����");

		return false;
	}


	// ���� �Ϸ���� ���
	while( m_3dMeasure.isMeasuring() )
	{
	}


	//
	if( !m_3dMeasure.read3dData() )
	{
		dispay_message("3D ���� > ���� > ������ ȹ��");

		return false;
	}


	dispay_message("3D ���� > �Ϸ�");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	step height�� ���� �����Ѵ�.
//
//
//
void CMy3DmeasureDlg::measureSmallHoleRegion()
{
	dispay_message("���� �Ұ��� �˻� >");

	// ��� ���� ���� ����
	makeSaveFolder();

	bool bSuccess = true;


	//////////////////////////////////////////////////////////////////////////
	// Auto focus
	//////////////////////////////////////////////////////////////////////////
	if( m_operationMode==AUTO_MODE )
	{
			// Live view �� ȭ�� ��ȯ
		if( !m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() ) )
		{
			dispay_message(" autoFocus : view ��ȯ ����");
	
			bSuccess = false;
		}

// 		m_3dMeasure.move(6200*1000);

		// 10�� ����� ����
		if( bSuccess )
		{
			if( !changeLens(10) )
			{
				dispay_message(" �Ұ��� �˻� > ���� > changeLens 10");
				bSuccess = false;
			}
		}

		// 10�� ���� AF
		if( bSuccess )
		{
			if( !autoFocus(10) )
			{
				dispay_message(" �Ұ��� �˻� > ���� > auto focus 10");
				bSuccess = false;
			}
		}

		// 50�� ����� ����
		if( bSuccess )
		{
			if( !changeLens(50) )
			{
				dispay_message(" �Ұ��� �˻� > ���� > changeLens 50");
				bSuccess = false;
			}
		}

		if( !autoFocus(50) )
		{
			dispay_message(" �Ұ��� �˻� > ���� > auto focus 50");

			bSuccess = false;
		}


		// ���� range�� ����
		// Mask �β��� ����Ͽ� 30 um�� ����
// 		if( !setMeasureRange( false, 30, 30 ) )
		if( !setMeasureRange( false, m_stRecip3D.dScanRangeDown, m_stRecip3D.dScanRangeUp ) )
		{
			dispay_message(" sequenceCheckMaskMovement : ����");
			bSuccess = false;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// �ݺ� ����
	//////////////////////////////////////////////////////////////////////////
	list<double> listStepHeight;
	list<double> listMaskWidth;

	if( bSuccess )
	{
		for( UINT i=0 ; i<m_nIteration ; i++ )
		{
			// ���� üũ
			if( m_bStop )
			{
				m_bStop = false;
				bSuccess = false;

				i = m_nIteration;

				dispay_message(" �Ұ��� �˻� > ����");
			
				break;
			}


			//////////////////////////////////////////////////////////////////////////
			// 3D ����
			//
			double dDistance=0;
			measure3d(&dDistance);
			//
			//////////////////////////////////////////////////////////////////////////



			//////////////////////////////////////////////////////////////////////////
			// ���� ������ ����
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

			end_local_counter("�Ұ��� �˻� > ������ ����");
			//
			//////////////////////////////////////////////////////////////////////////

			

			//////////////////////////////////////////////////////////////////////////
			//
			//
			start_local_counter();

			// step height ����
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
				dispay_message(" �Ұ��� �˻� > ���� > ����");
			}

			end_local_counter("�Ұ��� �˻� > ����ó��");
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
			dispay_message(" �Ұ��� �˻� > ���� > ��� ����");
		}
	}


	//
	if( m_operationMode==AUTO_MODE )
	{
		// ���� ����
		if( !changeLens(10) )
		{
			dispay_message(" �Ұ��� �˻� > ���� > 10�� ����� ����");
		}

		// ��� ������ �۽�

		if( listStepHeight.size() > 0 ) 
		{
			m_socketModule.SendMeasureResult( bSuccess, *listStepHeight.begin(), 0 );
		} else {
			m_socketModule.SendMeasureResult( false, 0, 0 );
		}
	}


	dispay_message("���� �Ұ��� �˻� > �Ϸ�\n");
}


//////////////////////////////////////////////////////////////////////////
//
//	����� �鿡�� slope angle�� step height �� �����Ѵ�.
//
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::measureLargeHoleRegion()
{
	start_global_counter();
	dispay_message("�� ����� �˻� >");

	bool bSuccess =true;


	// ��� ���� ���� ����
	makeSaveFolder();


	//////////////////////////////////////////////////////////////////////////
	// Auto focus
	//////////////////////////////////////////////////////////////////////////
	start_local_counter();
	if( m_operationMode==AUTO_MODE )
	{
		//
		//moveLens(6413.0);

		// ������ AF �ǽ�
		if( !autoFocus(10) )
		{
			dispay_message(" ����� �˻� > ���� : AF");

			bSuccess = false;
		}


		// ������ AF �ǽ�
		if( !autoFocus(50) )
		{
			dispay_message(" ����� �˻� > ���� : AF");

			// AF ���� �� ������ ���� 10�� ����� ����
			if( !changeLens(10) )
			{
				dispay_message(" ����� �˻� > ���� : AF ���� �� 10��� ����");
			}

			bSuccess = false;
		}

		// ���� range�� ����
		// Mask �β��� ����Ͽ� 30 um�� ����
// 		if( !setMeasureRange( false, 30, 30 ) )
		if( !setMeasureRange( false, m_stRecip3D.dScanRangeDown, m_stRecip3D.dScanRangeUp ) )
		{
			dispay_message(" ���� ���� ���� : ����");
			bSuccess = false;

			// AF ���� �� ������ ���� 10�� ����� ����
			if( !changeLens(10) )
			{
				dispay_message(" ���� ���� ���� > ���� : 10��� ����");
			}
		}
	}
	end_local_counter("����� �˻� > AF");


	//////////////////////////////////////////////////////////////////////////
	// �ݺ� ����
	//////////////////////////////////////////////////////////////////////////
	list<double> listStepHeight;
	list<double> listSlopeAngle;

	if( bSuccess )
	{
		m_inspector.clearResult();


		for( UINT i=0 ; i<m_nIteration ; i++ )
		{
			//////////////////////////////////////////////////////////////////////////
			// ���� ��� Ȯ��
			//
			if( m_bStop )
			{
				m_bStop = false;
				bSuccess = false;

				i = m_nIteration;

				dispay_message(" ����� �˻� > ����");

				break;
			}
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			//
			// 3D ����
			//
			start_local_counter();
			double dDistance=0;
			if( !measure3d(&dDistance) )
			{
				bSuccess = false;
				break;
			}
			end_local_counter("����� �˻� > 3D ����");
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// ���� ������ ����
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

			end_local_counter("����� �˻� > ������ ����");
			//
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// �Ÿ����� 
			//
			// 3D ���а� ���� ��ġ ����
			if(m_bUseDistanceSensor)
			{
				start_local_counter();
				m_lensChanger.goNone();
				end_local_counter("����� �˻� > �Ÿ����� ������ ȸ�� > None ����� ����");

				Sleep(100);

				// ����
				start_local_counter();
				dDistance = measureDistance(50);
				end_local_counter("����� �˻� > �Ÿ����� ������ ȸ�� > ������ ȹ��");

				// ���� ����
				start_local_counter();
				m_lensChanger.go50x();
				end_local_counter("����� �˻� > �Ÿ����� ������ ȸ�� > 50�� ����� ����");
			} else {
				dDistance = 0;
			}
			//
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// 3D ������ �б�
			//
			start_local_counter();
			// ���� ������ ��������
			int widthHeight, heightHeight;
			double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);

			// ������ �̹��� ��������
			int widthLaser, heightLaser;
			BYTE *pLaserImage = m_3dMeasure.getLaserImage(&widthLaser, &heightLaser);

			// DIC �̹��� ��������
			int widthDic, heightDic;
			BYTE *pDicImage = m_3dMeasure.getDicImage( &widthDic, &heightDic );
			end_local_counter("����� �˻� > 3D ������ ���");
			//
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// �м�
			start_local_counter();

			// �β��� ȹ��
			dispay_message("����� > �β� ���");
			if( m_bUseThicknessSensor )
			{
				m_inspector.setTargetThickness( m_thicknessSensor.GetThickness() );
			} else {
				m_inspector.setTargetThickness( m_targetThickness );
			}


			// ����
			dispay_message("����� > �˻��ϱ�");
			if( !m_inspector.measureSlopeAngleAtLargeHoleRegion(	pHeightMap,					// ���� map
																	pLaserImage,				// laser image
																	pDicImage,					// DIC image
																	widthHeight, heightHeight,	// �̹��� ũ��
																	dDistance,					// �Ÿ����� ����
																	&listSlopeAngle,			// slope angle
																	&listStepHeight ) )			// step height
			{
				dispay_message(" ����� �˻� > ���� > ����");
			}
			end_local_counter("����� �˻� > ����ó��");
			//
			//////////////////////////////////////////////////////////////////////////



			CString strMsg;
			strMsg.Format( "�����˻�>measureLargeHoleRegion : %d ȸ", i+1 );
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
// 				strTemp.Format("%.1f ��,", pMeasureAnlge[j]);
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
// 			dispay_message(" ����� �˻� > ���� > ��� ����");
// 		}
// 
// 
// 		// �ӽ�
// 		CStdioFile file2;
// 		CString strPath2;
// // 		strPath2.Format("c:\\test data\\S&H %d.csv", m_nCnt++);
// 		strPath2.Format(m_strSaveFilePath+"\\S&H %d.csv", m_nCnt++);
// 
// 		if( file2.Open(strPath2, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
// 		{
// 			CString strTemp;
// 
// 			// ������(����)
// 			file2.WriteString("Slope angle\n");
// 			for( int j=0 ; j<nMesurePoint ; j++ )
// 			{
// 				strTemp.Format("%.1f ��,", pMeasureAnlge[j]);
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
// 			// island�� ����
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
// 			// ���� �ֺ��� ����
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
// 			// �Ÿ�����
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
// 			// �β�
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
// 			dispay_message(" ����� �˻� > ���� > ��� ����");
// 		}	
// 	}
	end_local_counter("����� �˻� > ��� ���� �����ϱ�");
	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	// XML �ۺΰ�
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
		// ���� ����
		if( !changeLens(10) )
		{
			dispay_message(" ����� �˻� > ���� > 10�� ���� ����");
		}


		// ��� ������ �۽�
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

			// Step height : ���� ���
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


			// Step angle : ���� ���
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
				if( pMaskInfo->shape== HOLE_SHAPE_ELIPSE_45)	// ������ Ÿ���� 45��, 135��, 225��, 315�� ��ġ�� ���� ������.
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
	end_local_counter("����� �˻� > ��� ������ �����ϱ�");


	dispay_message("�� ����� �˻� > �Ϸ�");
	end_global_counter("����� �˻�");
}


bool CMy3DmeasureDlg::sequenceCheckMaskMovement()
{
	dispay_message("�� sequenceCheckMaskMovement : ����");

	bool bSuccess = true;


	//
	// 1. 
	//
	if( m_operationMode==AUTO_MODE )
	{
		m_3dMeasure.move(6200*1000);

		// 10�� ���� AF
		if( !autoFocus(10) )
		{
			dispay_message(" sequenceCheckMaskMovement : ����");
			bSuccess = false;
		}

		// 50�� ����� ����
		if( bSuccess )
		{
			if( !autoFocus(50) )
			{
				dispay_message(" sequenceCheckMaskMovement : ����");
				bSuccess = false;
			}
		}

		// ���� ���� ����
		if( bSuccess )
		{
			if( !setMeasureRange( false, 30, 30 ) )
			{
				dispay_message(" sequenceCheckMaskMovement : ����");
				bSuccess = false;
			}
		}

		// 3D ����
		if( bSuccess )
		{
			double dDistance;
			if( !measure3d(&dDistance) )
			{
				dispay_message(" sequenceCheckMaskMovement : ����");
				bSuccess = false;
			}
		}
	}


	//
	// 2. 
	//
	double dDx=0, dDy=0;	// um ����
	if( bSuccess )
	{
		int widthCcd, heightCcd;
		BYTE *pCcdImage = m_3dMeasure.getCcdImage( &widthCcd, &heightCcd );

		int widthDic, heightDic;
		BYTE *pDicImage = m_3dMeasure.getDicImage( &widthDic, &heightDic );

// 		m_inspector.setParamer(m_dHolePitchX, m_dHolePitchY, m_dHoleWidth, m_dHoleHeight, (HOLE_SHAPE)m_ctrlHoleShape.GetCurSel());

		if( !m_inspector.checkMaskMovementDic( pDicImage, widthDic, heightDic, &dDx, &dDy ) )
		{
			dispay_message(" sequenceCheckMaskMovement : ����");

			bSuccess = false;
		} else {
			CString strMsg;
			strMsg.Format(" sequenceCheckMaskMovement : X���� = %.2f um, y���� %.2f um", dDx, dDy );	// mm ������ ��ȯ�Ͽ� �ۺ�
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
		m_socketModule.SendAutoFocusResult( bSuccess, dDx/1000, dDy/1000 );	// ������ȯ : um > mm
	}


	dispay_message("�� sequenceCheckMaskMovement : �Ϸ�");


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
	dispay_message("�� sequenceCalibration >");

	bool bSuccess = true;

	// �Ÿ����� ��
	double dDistance = 0;


	if( m_operationMode==AUTO_MODE )
	{
		//
		// 1. Auto focus
		//
		//moveLens(5441.0);
// 		moveLens(6369.0);

		// 1.1 10��
		dispay_message(" sequenceCalibration > autofocus 10x >");

		if( autoFocus(10) )
		{
			dispay_message(" sequenceCalibration > autofocus 10x > ����");
		} else 
		{
			dispay_message(" sequenceCalibration > autofocus 10x > ����");

			bSuccess = false;
		}


		// 1.2 50��
		if( bSuccess )
		{
			dispay_message(" sequenceCalibration > autofocus 50x >");

			if( autoFocus(50) )
			{
				dispay_message(" sequenceCalibration > autofocus 50x > ����");
			} else
			{
				dispay_message(" sequenceCalibration > autofocus 50x > ����");

				bSuccess = false;
			}

		}



		//
		// 2. ����( 3D & distance )
		//
		if( bSuccess )
		{
			dispay_message(" sequenceCalibration > 3D ���� >");

			if( !setMeasureRange( false, 10, 10 ) )
			{
				dispay_message(" sequenceCalibration > 3D ���� > range : ����");
				bSuccess = false;
			}

			if( measure3d(&dDistance) )
			{
				dispay_message(" sequenceCalibration > 3D ���� > ����");
			} else 
			{
				dispay_message(" sequenceCalibration > 3D ���� > ����");

				bSuccess = false;
			}
		}
	} 



	//
	// 1.2 �Ÿ����� ������ ȹ��
	//	
	dDistance = measureDistance(50);
	//dDistance = m_distanceSensor.sensing(50)*1000;
	//dDistance = 0;


	//
	// 2. cal
	//
	// 2.1 3D ������ ȹ��
	int width, height;
	double *p3dData = m_3dMeasure.getHeightMap(&width, &height);


	// 2.2 ���
	if( bSuccess )
	{
		dispay_message(" sequenceCalibration > ��� >");

		m_inspector.m_list3DdataCal.clear();
		m_inspector.m_listDistanceDataCal.clear();

		if( m_inspector.calibration( p3dData, width, height, dDistance, m_dCalibrationThickness ) )
		{
			dispay_message(" sequenceCalibration > ��� > �Ϸ�");

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
			dispay_message(" sequenceCalibration > ��� > ����");

			bSuccess = false;
		}
	}


	//
	// 3. 
	//
	if( m_operationMode==AUTO_MODE )
	{
		// ���� ����
		changeLens(10);

		// ��� �޼��� �ۺ�
		m_socketModule.SendCalibrationResult( bSuccess );

	}

	dispay_message("�� sequenceCalibration > �Ϸ�");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	���� ���� ���� ������
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::sequenceCharngeLens()
{
	dispay_message("�� sequenceCharngeLens >");


	if( m_iLensMag_src == _LENS_CMD_FROM_USER_ )	// ������� ����� ���
	{
		if( changePgmState(STATE_LENS_CHANGE) )
		{
			switch( m_iLensMag_taget )
			{
			case 10:	// 10��
				if( m_lensChanger.go10x() )
				{
					dispay_message(" sequenceCharngeLens > x10");
				} else {
					dispay_message(" sequenceCharngeLens > x10 > error");
				}
				break;

			case 20:	// 20��
				if( m_lensChanger.go20x() )
				{
					dispay_message(" sequenceCharngeLens > x20");
				} else {
					dispay_message(" sequenceCharngeLens > x20 > error");
				}
				break;

			case 50:	// 50��
				if( m_lensChanger.go50x() )
				{
					dispay_message(" sequenceCharngeLens > x50");
				} else {
					dispay_message(" sequenceCharngeLens > x50 > error");
				}
				break;

			case 150:	// 150��
				if( m_lensChanger.go150x() )
				{
					dispay_message(" sequenceCharngeLens > x150");
				} else {
					dispay_message(" sequenceCharngeLens > x150 > error");
				}
				break;

			default:	// �̻�
				dispay_message(" sequenceCharngeLens > error");
				return false;
			}

			changePgmState(STATE_NONE);
		}
	} 
	else if( m_iLensMag_src == _LENS_CMD_FROM_OP_ )	// OP SW�� ����� ��� > ������ �ʿ���
	{
		dispay_message("sequenceCharngeLens > from OP > ����");

		if( changePgmState( STATE_LENS_CHANGE ) )
		{
			if( m_lensChanger.go10x() )
			{
				dispay_message("sequenceCharngeLens > from OP > ����");

				m_socketModule.SendLensChangeResult( true );
			} 
			else 
			{
				dispay_message("sequenceCharngeLens > from OP > ����(���� X)");

				m_socketModule.SendLensChangeResult( false );
			}

			dispay_message("���� ���� ��� ���� > ��");

			changePgmState( STATE_NONE );
		} 
		else 
		{
			m_socketModule.SendLensChangeResult( false );
			dispay_message("sequenceCharngeLens > from OP > ����(�ٸ� ������)");
		}	

		dispay_message("sequenceCharngeLens > from OP > ��");
	}
	

	dispay_message("�� sequenceCharngeLens > �Ϸ�");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	���� ������
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CMy3DmeasureDlg::sequenceMeasure()
{
	switch( m_iModeMeasureMethod )
	{
		// ���� ��� slope angle ����
// 		case MEASURE_METHOD_SLOPEANGLE:
// 			measureSlopeAngle();
// 			break;

		// ���� ��� step height ����
		case MEASURE_METHOD_SMALL_HOLE:
			measureSmallHoleRegion();
			break;

		// ���� ��� slope angle & step height ����
		case MEASURE_METHOD_LARGE_HOLE:
			measureLargeHoleRegion();
			break;

		// �β� calibration
		case MEASURE_METHOD_THICKNESS:
			measureThickness();
			break;
	}

	return true;
}


void CMy3DmeasureDlg::measureThickness()
{
	dispay_message("�� �β� ���� ����");


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
			dispay_message(" measureThickness > autofocus 10x > ����");

			return;
		}

		// ������ AF �ǽ�
		if( !autoFocus(50) )
		{
			dispay_message(" measureThickness > autofocus 50x > ����");
			return;
		}

		// ���� range�� ����
		// Mask �β��� ����Ͽ� 30 um�� ����
		if( !setMeasureRange( false, 15, 15 ) )
		{
			dispay_message(" sequenceCheckMaskMovement : ����");
			return;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// ����
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
		dispay_message("3D��������");


		// ���� �Ϸ���� ���
		// �Ÿ� ����
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
		dispay_message("3D ������ ȹ�� �Ϸ�");


		//
		int widthHeight, heightHeight;
		double *pHeightMap = m_3dMeasure.getHeightMap(&widthHeight, &heightHeight);

		double dThickness=0.0;
		m_inspector.measureT(pHeightMap, widthHeight, heightHeight, fDistance, &dThickness);

		listThicknessData.push_back(dThickness);


		CString strMsg;
		strMsg.Format("�β��� %.3f um �Դϴ�.", dThickness );

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
	// XML �ۺΰ�
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
		// ���� ����
		if( !changeLens(10) )
		{
			dispay_message(" �β� �˻� > ���� > 10�� ���� ����");
		}

		m_socketModule.SendMeasureResult( true, 0, 0 );
	}

	dispay_message("�� �β� ���� �Ϸ�");
}


double CMy3DmeasureDlg::measureDistance(int nTime)
{
	double dDistanceSum = 0.0;
	UINT nData = 0;

	// ������ ȹ��
	//for( int k=0 ; k< nTime ; k++ )
	{
		// �Ÿ����� �翬�� : ���� ������ �ʱ�ȭ ����
		m_distanceSensor.Disconnect();

		CString strIp = "30, " + m_distanceSensorIP + ", TCP/IP, 300, 0.0, 3.0,-10.0, 0.001";

// 		if( !m_distanceSensor.Connect("30, 169.254.168.150, TCP/IP, 300, 0.0, 3.0,-10.0, 0.001") )
		if( !m_distanceSensor.Connect(strIp) )
		{
			Sleep(500);	// �ٷ� ������ ������ ȹ������ ���� �о���� �� �ϴ�.

			m_distanceSensor.Disconnect();

			Sleep(500);	// �ٷ� ������ ������ ȹ������ ���� �о���� �� �ϴ�.

// 			if( !m_distanceSensor.Connect("30, 169.254.168.150, TCP/IP, 300, 0.0, 3.0,-10.0, 0.001") )
			if( !m_distanceSensor.Connect(strIp) )
			{
				dispay_message("===== �Ÿ����� ���� > ���� =======");
			}

			Sleep(100);	// �ٷ� ������ ������ ȹ������ ���� �о���� �� �ϴ�.
		}

		dDistanceSum = m_distanceSensor.sensing(nTime)*1000;

		CString strTemp;
		strTemp.Format("�Ÿ����� ���� : %.3f\n", dDistanceSum);
		dispay_message(strTemp);

		m_distanceSensor.Disconnect();
	}	

	return dDistanceSum;
}


//////////////////////////////////////////////////////////////////////////
//
// ���� �ߴ�
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
// ���� ��ġ�� �������� ����
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
// Data�� ������ folder ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::makeSaveFolder()
{
	// �ð� ���ϱ�
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
// Data�� ������ folder ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::makeLogFolder()
{
	// �ð� ���ϱ�
	CString tmp = PATH_LOG;
	CString tok;
	while(0 <= tmp.Find("\\"))
	{
		tok += tmp.Left(tmp.Find("\\")+1);
		tmp = tmp.Right(tmp.GetLength() - (tmp.Find("\\")+1));

		CreateDirectory(tok, NULL);
	}
}


// 10�� ���� -> AF
// 50�� ���� -> AF
bool CMy3DmeasureDlg::autoFocus()
{
	// Live view �� ȭ�� ��ȯ
	if( !m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() ) )
	{
		dispay_message("Auto focus > ���� > view ��ȯ");

		return false;
	}


	// 10�� ����� ��ȯ
	if( !m_lensChanger.go10x() )
	{
		dispay_message("Auto focus > ���� > 10�� ���� �̵�");
		return false;
	}


	// Laser AF �ǽ�
	if( !m_3dMeasure.autofocusLaser() )
	{
		// image AF �ǽ�
		if( !m_3dMeasure.autofocusImage() )
		{
			dispay_message("Auto focus > ���� > 10�� ���� AF");
			return false;
		}
	}


	// 50�� ����� ����
	if( !m_lensChanger.go50x() )
	{
		dispay_message("Auto focus > ���� > 50�� ���� �̵�");
		return false;
	}


	if( m_operationMode==MANUAL_MODE)
	{
		// ������ AF �ǽ�
		if( !m_3dMeasure.autofocusLaser() )
		{
			dispay_message("Auto focus > ���� > 50�� ���� AF");
			return false;
		}
	}


	// ���� ���� ����
	if( !m_3dMeasure.setMeasureRangeOffset( (long)(m_3dMeaserRangeLowerOffset*1000), (long)(m_3dMeaserRangeUpperOffset*1000)) )
	{
		dispay_message("Auto focus > ���� > ���� range ����");
		return false;
	}


	// ȭ�� ����
	update3dPosition();


	return true;
}

void CMy3DmeasureDlg::OnBnClickedButtonSaveLightImage()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("������ ���� �����ϱ� ����");

	m_3dMeasure.saveLaserImage();

	dispay_message("������ ���� �����ϱ� �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedRadioHeight()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioLaser()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioDic()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonTest()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
			// ������ �б�
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

			end_local_counter("�Ұ���˻� > ������ �б�");
			//////////////////////////////////////////////////////////////////////////


			double dSlopeAngle=0;


			// ��� 1 : ���� ��� step height
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
		// ��� ���� ���� 
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
				file2.WriteString("0��,45��,90��,135��,180��,225��,270��,315��\n");
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
		end_local_counter("�����˻� > ��� ���� ����");
		//
		//////////////////////////////////////////////////////////////////////////


		changePgmState(STATE_NONE);
	}
}


void CMy3DmeasureDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	// 3D �������
	if( m_3dMeasure.m_bStateConnection==TRUE )
	{
		SetDlgItemText(IDC_STATIC_3D_CONNECT, "���� ��" );
		m_stc3dConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_3D_CONNECT, "���� �ʵ�" );
		m_stc3dConnection.SetBkColor(RGB(255,0,0));
	}

	// 3D ��������
	if( m_3dMeasure.m_bStateMeasure==TRUE )
	{
		SetDlgItemText(IDC_STATIC_3D_MEASURE, "������" );
		m_stc3dMeasure.SetBkColor(RGB(0,0,255));
	} else {
		SetDlgItemText(IDC_STATIC_3D_MEASURE, "�����" );
		m_stc3dMeasure.SetBkColor(RGB(255,255,255));
	}

	// �Ÿ����� �������
	if( m_distanceSensor.IsConnected() )
	{
		SetDlgItemText(IDC_STATIC_DISTANCE_CONNECT, "���� : O" );
	} else {
		SetDlgItemText(IDC_STATIC_DISTANCE_CONNECT, "���� : X" );
	}

	// �Ÿ����� �������
	if( m_distanceSensor.IsGapSensing() )
	{
		CString str;
		str.Format("���� : O(%.6f um)", m_distanceSensor.GetGapAvg()*1000 );
		SetDlgItemText(IDC_STATIC_DISTANCE_MEASURE, str );
	} else {
		SetDlgItemText(IDC_STATIC_DISTANCE_MEASURE, "���� : X" );
	}


	// ������ �������
	if( m_lensChanger.IsConnected() )
	{
		SetDlgItemText(IDC_STATIC_REVOLVER_CONNECT, "�����" );
		m_stcRevoverConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_REVOLVER_CONNECT, "����ʵ�" );
		m_stcRevoverConnection.SetBkColor(RGB(255,0,0));
	}

	// ������ servo����
	if( m_lensChanger.IsServoOn() )
	{
		SetDlgItemText(IDC_STATIC_REVOLVER_SERVO, "���� ����" );
		m_stcRevoverServo.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_REVOLVER_SERVO, "���� ����" );
		m_stcRevoverServo.SetBkColor(RGB(255,0,0));
	}

	// ������ ���� ���� ǥ��
	if( m_lensChanger.IsRunning() )
	{
		SetDlgItemText(IDC_STATIC_REVOLVER_STATE, "������" );
		m_stcRevoverRun.SetBkColor(RGB(0,0,255));
	} else {
		SetDlgItemText(IDC_STATIC_REVOLVER_STATE, "�����" );
		m_stcRevoverRun.SetBkColor(RGB(255,255,255));
	}

	// ���� ��� ���� ǥ��
	if( m_socketModule.IsConnection() )
	{
		SetDlgItemText(IDC_STATIC_SOCKET_CONNECTION, "����" );
		m_stcSocketConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_SOCKET_CONNECTION, "�����" );
		m_stcSocketConnection.SetBkColor(RGB(255,0,0));
	}

	// �β� ������ ���˼��� ���� ǥ��
	if( m_thicknessSensor.IsConnected() )
	{
		SetDlgItemText(IDC_STATIC_CONTACT_SENSOR_CONNECTION, "����" );
		m_stcContactSensorConnection.SetBkColor(RGB(0,255,0));
	} else {
		SetDlgItemText(IDC_STATIC_CONTACT_SENSOR_CONNECTION, "�����" );
		m_stcContactSensorConnection.SetBkColor(RGB(255,0,0));
	}

	// ���α׷� ���� ǥ��
	switch( m_state )
	{
		case STATE_NONE:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "�����");
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
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "������...");
			m_stcProgram.SetBkColor(RGB(0,0,255));
			break;

		case STATE_RUNING:
			SetDlgItemText(IDC_STATIC_PROGRAM_STATE, "������...");
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
// 3D ���� �Ϸ� ��
//
//////////////////////////////////////////////////////////////////////////
LRESULT CMy3DmeasureDlg::OnReceive(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here

	dispay_message("3D ���� > �Ϸ�");

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// 3D ���� �Ϸ� ��
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("�Ÿ����� ���� > ");


// 	CString str;
// 	str.Format(_T("30, 169.254.168.150, TCP/IP, 300, 0.0, 3.0,-10.0, 0.001"));

	// Get ip
// 	CString strIp = "30, " + m_distanceSensorIP + ", TCP/IP, 300, 0.0, 3.0,-10.0, 0.001";
	CString strIp = "33, " + m_distanceSensorIP + ", TCP/IP, 300, 0.0, 3.0,-10.0, 0.001";

// 	if( !m_distanceSensor.Connect(strIp) )
	if( !m_distanceSensor.Connect(m_distanceSensorIP) )
	{
		dispay_message("�Ÿ����� ���� > ����");
	}


	// 
	dispay_message("�Ÿ����� ���� > �Ϸ�");



}


void CMy3DmeasureDlg::OnBnClickedButtonDisconnectDistSensor()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_distanceSensor.Disconnect();
}


void CMy3DmeasureDlg::OnBnClickedButtonMeasureDist()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if( m_distanceSensor.IsConnected() )
	{
		m_distanceSensor.StartContinousSensing();
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonMeasureDistOnece()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.



	if( m_distanceSensor.IsConnected() )
	{
		double ret = m_distanceSensor.sensing(10);

		CString str;
		str.Format("�Ÿ��� : %.5f", ret );
		dispay_message(str);
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonStopMeasureDist()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if( m_distanceSensor.IsConnected() )
	{
		m_distanceSensor.StopSensing();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ǥ�� �β� �÷Ḧ �̿��� calibration
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonCalibration()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	if( changePgmState( STATE_CALIBRATION ) )
	{
		SetEvent( m_thread.m_hEventCalibration );
	}
}


void CMy3DmeasureDlg::OnEnChangeEditCalibrationThick()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioThicknessMethodDirect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioThicknessMethodIndirect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonServoOn()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_lensChanger.servoOn();
}


void CMy3DmeasureDlg::OnBnClickedButtonServoOff()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_lensChanger.servoOff();
}


//////////////////////////////////////////////////////////////////////////
//
// ���� ������ 10��� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens10()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	// ���� ���� ����
	if( IsPgmStateFree() )	// ���� ������ �� ����
	{
		// ����� ������ ��
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// ��ǥ ���� ����
		m_iLensMag_taget = 10;

		// ���� ������ ����
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
// ���� ������ 20��� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens20()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	// ���� ���� ����
	if( IsPgmStateFree() )	// ���� ������ �� ����
	{
		// ����� ������ ��
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// ��ǥ ���� ����
		m_iLensMag_taget = 20;

		// ���� ������ ����
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
// ���� ������ 50��� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens50()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	// ���� ���� ����
	if( IsPgmStateFree() )		// ���� ������ �� ����
	{
		// ����� ������ ��
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// ��ǥ ���� ����
		m_iLensMag_taget = 50;

		// ���� ������ ����
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
// ���� ������ 150��� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLens150()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	// ���� ���� ����
	if( IsPgmStateFree() )	// ���� ������ �� ����
	{
		// ����� ������ ��
		m_iLensMag_src = _LENS_CMD_FROM_USER_;

		// ��ǥ ���� ����
		m_iLensMag_taget = 150;

		// ���� ������ ����
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("���� ü���� ��� ���� > ");

	if( m_lensChanger.IsConnected()==false )
	{
		int iPortNum = m_ctrlRevolverPortNum.GetCurSel() + 1;

		m_lensChanger.m_setPortNum(iPortNum);


		if( !m_lensChanger.connect() )
		{
			dispay_message("���� ü���� ��� ���� > ����");
		} 

		dispay_message("���� ü���� ��� ���� > �Ϸ�");
	}
	else
	{
		dispay_message("���� ü���� ��� ���� > ���� > �̹� �����");
	}

	
}


void CMy3DmeasureDlg::OnBnClickedButtonDisconnectRevolver()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("���� ü���� ��� ���� >");


	if( !m_lensChanger.disconnect() )
	{
		dispay_message("���� ü���� ��� ���� > ����");
	}

	dispay_message("���� ü���� ��� ���� > �Ϸ�");
}


///////////////////////////////////////////////////////////
//
// Lens�� Home ��ġ�� �̵��Ѵ�.
//
///////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonGoHome()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_lensChanger.goHome();
}


void CMy3DmeasureDlg::OnBnClickedButton3dLaserAutofofus()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("Laser AF >");

	if( changePgmState(STATE_RUNING))
	{
		if( !m_3dMeasure.autofocusLaser() )
		{
			dispay_message("Laser AF > ����");
		} 

		changePgmState(STATE_NONE);
	} else 
	{
		dispay_message("Laser AF > ����");
	}

	dispay_message("Laser AF > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButton3dImageAutofofus()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("Image AF >");

	if( changePgmState(STATE_RUNING))
	{
		if( !m_3dMeasure.autofocusImage() )
		{
			dispay_message("Image AF > ����");
		} 

		changePgmState(STATE_NONE);
	} else 
	{
		dispay_message("Image AF > ����");
	}
		
	
	dispay_message("Image AF > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButton3dUp()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if( !m_3dMeasure.moveUp(1000) )
	{
		dispay_message("Move > ����");
	}
}


void CMy3DmeasureDlg::OnBnClickedButton3dDown()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if( !m_3dMeasure.moveDown(1000) )
	{
		dispay_message("Move > ����");
	}
}


void CMy3DmeasureDlg::OnEnChangeEditIteration()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonCalculate()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
// 		dispay_message("3D��������");
// 
// 
// 		// ���� �Ϸ���� ���
// 		while( m_3dMeasure.isMeasuring() )
// 		{
// 
// 		}
// 
// 
// 		//
// 		m_3dMeasure.read3dData();
// 		dispay_message("3D ������ ȹ�� �Ϸ�");
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
// 		strMsg.Format("Slope angle�� %.3f �� �Դϴ�.", dSlopeAngle );
// 		dispay_message(strMsg);
// 	}
}


void CMy3DmeasureDlg::OnBnClickedButtonStopCaculation()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_bStop = true;

	changePgmState(STATE_NONE);
}


void CMy3DmeasureDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.

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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioClientSocket()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonSocketCreate()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("���ϻ��� >");

	m_socketModule.SetParameter( this->GetSafeHwnd(), m_socketPort, m_socketIp );


	switch(m_soketType)
	{
	case 0:
		if( !m_socketModule.Create(_SERVER_) )
		{
			dispay_message("���ϻ��� > ���� : ���� ����");
		}
		break;
	case 1:
		if( !m_socketModule.Create(_CLIENT_) )
		{
			dispay_message("���ϻ��� > ���� : Ŭ���̾�Ʈ ����");
		}
		break;
	default:
		break;;
	}


	dispay_message("���ϻ��� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonSocketConnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("���� ���� >");


	if( m_socketModule.GetStatus()==_CLIENT_ )
	{
		if( !m_socketModule.Start() )
		{
			dispay_message("���� ���� > ���� : Ŭ���̾�Ʈ ����");
		}
	} 

	if( m_socketModule.GetStatus()==_SERVER_ )
	{
		if( m_socketModule.Start() )
		{
			dispay_message("���� ���� > ����� : Ŭ���̾�Ʈ ���� �����");
		} else {
			dispay_message("���� ���� > ����");
		}
	} 


	dispay_message("���� ���� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonSocketDisconnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_socketModule.Stop();
}


LRESULT CMy3DmeasureDlg::OnAcceptSocket(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here

	dispay_message("Ŭ���̾�Ʈ�� �����");

	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// � ���α׷����� ���� ��� ���� ��
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
	// ������ ���� ��� ���Ž�
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_RECIPE)
	{
		if( changePgmState( STATE_RUNING ) )
		{
			//////////////////////////////////////////////////////////////////////////
			// �˻� ��ǰ���� ����
			PARAM_MASK_INFO maskInfo;
			maskInfo.iPitchX	= (int)(m_protocol.m_dPitchX/m_inspector.getResolution());
			maskInfo.iPitchY	= (int)(m_protocol.m_dPitchY/m_inspector.getResolution());
			maskInfo.iWidth		= (int)(m_protocol.m_dSizeX/m_inspector.getResolution());
			maskInfo.iHeight	= (int)(m_protocol.m_dSizeY/m_inspector.getResolution());
			maskInfo.shape		= (HOLE_SHAPE)m_protocol.m_iShape;			// �׸�/������/��
			maskInfo.arrangement = (HOLE_ARRANGEMENT)m_protocol.m_iArrage;	// �簢/�밢
			maskInfo.dThickness = m_protocol.m_dThickness;

			// �̳��ؿ� �ӽ� �ڵ�
			if( maskInfo.shape== HOLE_SHAPE_AUTO_RG)
			{
				maskInfo.shape = HOLE_SHAPE_AUTO;
			}

			m_inspector.setParamImage( maskInfo );
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// ���� ���� UI ǥ��
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
			// ��� �۽�
			m_socketModule.SendSetReceipeResult( true );
			//////////////////////////////////////////////////////////////////////////

			changePgmState( STATE_NONE );

			//
			CString str;
			str.Format("Parameter ���� ���� > ���� : pitch x = %f um, pitch y = %f um, size x = %f um, size x = %f um, shape = %d, arrange = %d, thickness = %f", 
				m_protocol.m_dPitchX, m_protocol.m_dPitchY, m_protocol.m_dSizeX, m_protocol.m_dSizeY, m_protocol.m_iShape, m_protocol.m_iArrage, m_targetThickness);
			dispay_message(str);
		} 
		else
		{
			//////////////////////////////////////////////////////////////////////////
			// ��� �۽�
			m_socketModule.SendSetReceipeResult( false );
			//////////////////////////////////////////////////////////////////////////

			dispay_message("Parameter ���� ���� > ���� : �ٸ� ������");
		}
	}


	//
	// Calibration ��� ����
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_CALIBRATION)
	{
		if( changePgmState( STATE_CALIBRATION ) )
		{
			// Calibration thread ����
			SetEvent( m_thread.m_hEventCalibration );
		} 
		else 
		{
			m_socketModule.SendCalibrationResult( false );
			dispay_message("CAL ��� ���� > ���� : �ٸ� ������");
		}
	}


	//
	// ���α׷� ���� ��û ���� ��
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_STATE)
	{
// 		dispay_message("��� > ���� ���� Ȯ�� ��û");

		// UI ����
		ToggleStateCheck();

		// 3D sensor���� ���� ��ġ ���� ���
// 		int iLensPosAtSensor = 0;
// 		m_3dMeasure.GetCurrentLensPos(&iLensPosAtSensor);

		// ���� ü�������� ���� ��ġ ���� ���
		int iLensPosAtChanger = m_lensChanger.getCurrentLens();

// 		int iLensPos = 0;
// 		if( iLensPosAtSensor == iLensPosAtChanger )
// 		{
// 			iLensPos = iLensPosAtSensor;
// 		} else {
// 			iLensPos = -1;
// 		}
	
		// ���� ���� ���� �ø���
		if( m_state==STATE_NONE)
		{	// �˻����� �ƴҶ�
			m_socketModule.SendState(false, m_lensChanger.getCurrentLens());
// 			m_socketModule.SendState(false, iLensPos);
		} else {
			// �˻����� ��
 			m_socketModule.SendState(true, m_lensChanger.getCurrentLens());
// 			m_socketModule.SendState(true, iLensPos);
		}
	}


	//
	// ���� �ߴ� ��� ���� ��
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_STOP)
	{
		dispay_message("��� > ���� ��û");
	
		if( m_state==STATE_MEASURE)
		{
			stopMeasure();
			m_socketModule.SendStop(true);
		} else {
			m_socketModule.SendStop(false);
		}
	}


	//
	// 10�� ���� ���� ��û ���� ��
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_LENS10X)
	{
		// ���� ���� ����
		if( IsPgmStateFree() )	// ���� ������ �� ����
		{
			// ����� ������ ��
			m_iLensMag_src = _LENS_CMD_FROM_OP_;

			// ��ǥ ���� ����
			m_iLensMag_taget = 10;

			SetEvent( m_thread.m_hEventChangeLens );
		} else {
			m_socketModule.SendLensChangeResult( false );

			dispay_message("���� ���� ��� ���� > ���� : �ٸ� ������");
		}

// 		dispay_message("��� > 10��� ���� ���� ��û");
// 
// 		if( changePgmState( STATE_RUNING ) )
// 		{
// 			if( m_lensChanger.go10x() )
// 			{
// 				m_socketModule.SendLensChangeResult( true );
// 			} 
// 			else 
// 			{
// 				dispay_message("���� ���� ��� ���� > ó�� ����");
// 
// 				m_socketModule.SendLensChangeResult( false );
// 			}
// 
// 			dispay_message("���� ���� ��� ���� > ��");
// 
// 			changePgmState( STATE_NONE );
// 		} 
// 		else 
// 		{
// 			m_socketModule.SendLensChangeResult( false );
// 			dispay_message("���� ���� ��� ���� > ���� : �ٸ� ������");
// 		}	
	}


	//
	// ����� ���� ��� ��û ���� ��
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_MESURE_LARGE)
	{
		dispay_message("��� > ����� ���� ��û");

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
// 			m_dUpperPos = m_protocol.m_d3dRangeUp;	// UI ����
// 			m_dLowerPos = m_protocol.m_d3dRangDown;	// UI ����

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
			str.Format(	"��� > ���� ��û > ����� �˻� ���� : [������ %s] [QR %s] [�β����� ��� %s] [�������� %f, %f]", 
						m_protocol.m_strRecipe,
						m_protocol.m_strQRCode,
						(m_bUseThicknessSensor)? "��" :"�ƴϿ�", 
						m_protocol.m_d3dRangeUp, 
						m_protocol.m_d3dRangDown);
			dispay_message(str);
		} 
		else 
		{
			m_socketModule.SendMeasureResult( false, 0, 0 );

			dispay_message("��� > ���� ��û > ����� �˻� ���� > Ÿ�۾� ������");
		}

	}


	//
	// �Ұ��� ���� ��� ��û ���� ��
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_MESURE_SMALL)
	{
		dispay_message("��� > �Ұ��� ���� ��û");

		//
		if( changePgmState( STATE_MEASURE ) )
		{
			// ����
			m_iModeMeasureMethod = MEASURE_METHOD_SMALL_HOLE;
			UpdateData(FALSE);

			SetEvent( m_thread.m_hEventMeasure );

// 			{
// 				m_socketModule.SendMeasureResult(true, 4, 0);
// 				changePgmState(STATE_NONE);
// 			}

			// display
			dispay_message("��� > ���� ��û > �Ұ��� �˻� ����");
		} 
		else 
		{
			m_socketModule.SendMeasureResult( false, 0, 0 );

			dispay_message("��� > ���� ��û > ����� �˻� ���� > Ÿ�۾� ������");
		}

	}


	//
	// Auot focus ��� ��û ���� ��
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_AUTOFOCUS)
	{
		if( changePgmState( STATE_AUTOFOCUS ) )
		{
			SetEvent( m_thread.m_hEventAutofocus );
		} else {
			m_socketModule.SendAutoFocusResult( false, 0, 0 );

			dispay_message("AF ��� ���� > ���� : �ٸ� ������");
		}
	}

	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_FAIL)
	{
	}


	//
	// �β� ���� ��� ��û ���� ��
	//
	if( m_protocol.Decode(pReceiveData, szReceive) == BHPROTOCOL_THICKNESS)
	{
		dispay_message("��� > �β� ���� ��û");

		if( changePgmState( STATE_RUNING ) )
		{
// 			if( m_stSite == SITE_LGD )
// 			{
// 				OnBnClickedButtonMeasureDistOnece();	// ���� 
// 
// 				changePgmState( STATE_NONE );	// ���� ����
// 			}


// 			if( m_stSite == SITE_LGD )
			{
				m_requests.measureThickness = TRUE;	// �β� ���� ?
				m_requests.measureThicknessCAL = m_protocol.m_bCAL;	// Calibration ?
				m_requests.measureThicknessPos = m_protocol.m_thicknessIndex;	// ��ġ ?

				if( m_protocol.m_bCAL )
				{
					m_thicknessSensor.setState(DS_MODE_CALIBRATION, m_protocol.m_thicknessIndex);
				} else {
					m_thicknessSensor.setState(DS_MODE_MEASURE, m_protocol.m_thicknessIndex);
				}

				// ���� ����
				if( !m_thicknessSensor.measureAll() )
				{
					changePgmState( STATE_NONE );
					m_requests.measureThickness = FALSE;	// �β� ���� ?
					m_socketModule.SendThicknessResult( -99 );	// ��� �ۺ�

					dispay_message("��� > �β� ���� ��û > ���� > �����Ұ�");
				}
			}
		} else {
			dispay_message("��� > �β� ���� ��û > ���� > �ٸ� �۾� ������");
		}
	}


	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// �β� ���� �������� ������ ȹ�� ��...
//
//////////////////////////////////////////////////////////////////////////
LRESULT CMy3DmeasureDlg::OnReceiveThichness(WPARAM wParam, LPARAM lParam) 
{
	// TODO: Add your control notification handler code here
	CString str;
	str.Format("���˼��� > ������� > �Ÿ��� > %f um", m_thicknessSensor.m_dValue );
	dispay_message(str);


	if( m_requests.measureThickness )	// ��� SW�κ��� �β� ���� ����� �޾�����...
	{
		m_socketModule.SendThicknessResult( m_thicknessSensor.GetThickness(m_requests.measureThicknessPos) );	// ��� �ۺ�

		str.Format("���˼��� > ������� > �β��� > %f um", m_thicknessSensor.GetThickness(m_requests.measureThicknessPos) );
		dispay_message(str);

		// ���� ��û �ʱ�ȭ
		m_requests.measureThickness = FALSE;


		changePgmState( STATE_NONE );
	}


	return 0;
}


// void CMy3DmeasureDlg::OnBnClickedRadioMeasureSlopeangle()
// {
// 	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
// 	UpdateData(TRUE);
// }


void CMy3DmeasureDlg::OnBnClickedRadioMeasureStepHeight()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioMeasureSlopeangleNStepHeight()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}

void CMy3DmeasureDlg::OnBnClickedRadioMeasureThickness()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonStopCalculation()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
}


void CMy3DmeasureDlg::OnBnClickedButton3dSetUpperPos()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	if( !m_3dMeasure.setUpperPos((long)(m_dUpperPos*1000)) )
	{
		dispay_message("setUpperPos > ����");
	}

	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedButton3dSetLowerPos()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	if( !m_3dMeasure.setLowerPos((long)(m_dLowerPos*1000)) )
	{
		dispay_message("setLowerPos > ����");
	}

	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedButton3dSetDistance()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

	if( !m_3dMeasure.setDistance((long)(m_d3dRangeDistance*1000)) )
	{
		dispay_message("setDistance > ����");
	}

	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedRadioModeAuto()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedRadioModeManual()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit3dLowerDistance()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit3dUpperDistance()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButton3dAutoRange()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_3dMeasure.setMeasureRangeOffset( (long)(m_3dMeaserRangeLowerOffset*1000), (long)(m_3dMeaserRangeUpperOffset*1000));


	if( m_3dMeaserRangeLowerOffset<=100 && m_3dMeaserRangeUpperOffset<= 100 )
	{
		m_stRecip3D.dScanRangeUp = m_3dMeaserRangeUpperOffset;
		m_stRecip3D.dScanRangeDown = m_3dMeaserRangeLowerOffset;
	} else {
		AfxMessageBox("���� ������ 100 um �̳��� �����մϴ�.");
	}


	update3dPosition();
}


void CMy3DmeasureDlg::OnBnClickedButton3dSelectViewType()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_3dMeasure.setViewType( m_ctrl3dViewType.GetCurSel() );
}


void CMy3DmeasureDlg::OnEnChangeEdit6()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit3()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit4()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEdit5()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonSetHoleShapeInfo()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("���� ���� ���� �Է� >");


	m_stMaskInfo.iPitchX	= (int)m_dHolePitchX;
	m_stMaskInfo.iPitchY	= (int)m_dHolePitchY;
	m_stMaskInfo.iWidth		= (int)m_dHoleWidth;
	m_stMaskInfo.iHeight	= (int)m_dHoleHeight;
	m_stMaskInfo.shape		= (HOLE_SHAPE)m_ctrlHoleShape.GetCurSel();
	m_stMaskInfo.arrangement = (HOLE_ARRANGEMENT)m_ctrlHoleInfoArrange.GetCurSel();
	m_stMaskInfo.dThickness = m_targetThickness;
	m_stMaskInfo.dStepHeightOffset = m_offet;

	m_inspector.setParamImage( m_stMaskInfo );


	dispay_message("���� ���� ���� �Է� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedButtonAutoFocus()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	dispay_message("�� Auto focus ���� >");

	if( changePgmState(STATE_RUNING) )
	{
		if( !autoFocus() )
		{
			dispay_message("Auto focus ���� > ����");
		}

		changePgmState(STATE_NONE);
	} else 
	{
		dispay_message("Auto focus ���� > ����");
	}


	dispay_message("�� Auto focus ���� > �Ϸ�");
}


void CMy3DmeasureDlg::OnBnClickedCheckDisplayProcessingImage()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	if( changePgmState(STATE_RUNING) )
	{
		sequenceCheckMaskMovement();

		changePgmState(STATE_NONE);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ����� �˻�
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonTest2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	//
	dispay_message("Test 2 >");


	if( changePgmState(STATE_RUNING) )
	{
		CString strMsg;
		CString strFilePath;
		list<double> listStepHeight;
		list<double> listSlopeAngle;


		makeSaveFolder();

		// ���� ������ ����
		m_inspector.clearResult();


		// ����
		int nTime = 1;
		for( int i=0 ; i<nTime ; i++ )
		{

			//////////////////////////////////////////////////////////////////////////
			// ������ �б�
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

			end_local_counter("�����˻� > ������ �б�");
			//////////////////////////////////////////////////////////////////////////


			//////////////////////////////////////////////////////////////////////////
			// ����
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
				dispay_message("Test 2 > ����");
			}

			end_local_counter("�����˻� > ����");
			//////////////////////////////////////////////////////////////////////////

		}


		// ��� ǥ�� UI
		list<double>::iterator ite1;
		list<double>::iterator ite2 = listStepHeight.begin();
		list<double>::iterator ite3 = m_inspector.m_listThickness.begin();
		for( ite1 = listSlopeAngle.begin() ; ite1!=listSlopeAngle.end() ; ite1++, ite2++, ite3++ )
		{
			strMsg.Format("angle=%f, height=%f(%f)", *ite1,  *ite2, *ite3 );
			dispay_message(strMsg);
		}

		// ��� ���� ���� 
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
// 				file2.WriteString("0��,45��,90��,135��,180��,225��,270��,315��\n");
// 				file2.WriteString("0��,15��,30��,45��,60��,75��,90��,105��,120��,135��,150��,165��,180��,195��,210��,225��,240��,255��,270��,285��,300��,315��,330��,345��\n");
				for( int j=0 ; j<nMesurePoint ; j++ )
				{
					strTemp.Format("%.1f��,", pMeasureAnlge[j]);
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

				// �β�
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
		end_local_counter("�����˻� > ��� ���� ����");


		changePgmState(STATE_NONE);



		dispay_message("Test 2 > �Ϸ�");
	} else 
	{
		dispay_message("Test 2 > ����");
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButton2()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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


	// File�� �����ϱ�
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


	// �ð� ���ϱ�
/*	COleDateTime tm = COleDateTime::GetCurrentTime();
	
	CString str;
	str.Format(	"d:\\%d_%2d_%2d_%2d_%2d_%2d", 
				tm.GetYear(), 
				tm.GetMonth(), 
				tm.GetDay(), 
				tm.GetHour(), 
				tm.GetMinute(), 
				tm.GetSecond() );


	// ���� �����ϱ�
	CreateDirectory( str, NULL );
	*/
		// �ð� ���ϱ�
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
// Lens�� ���� ��ġ�� �������� �����δ�.
//
///////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonLensNone()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	m_lensChanger.goNone();
}


void CMy3DmeasureDlg::OnEnChangeDistanceSensorIp()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
// 3D ���� �� �Ÿ����� ���� ���� ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedCheckUseDistance()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
// �β� ������ ������ ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonThicksensorOpen()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if( !m_thicknessSensor.m_Comm )
	{
		m_thicknessSensor.close();

		// ��� PORT ����
		m_thicknessSensor.SetPortNum( GetPortNumber_thickSensor() );

		// ��� PORT ����
		if( m_thicknessSensor.open() )
		{
			dispay_message("���˼��� > Open > ����");
		} else {
			dispay_message("���˼��� > Open > ����");
		}
	} else {
		dispay_message("���˼��� > Open > �̹� �����");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// �β� ���� ������ ���� ��û�� ������.
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnBnClickedButtonThicksensorRead()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if( m_thicknessSensor.measureAll() )
	{
		dispay_message("���˼��� > ���� ��û > ����");
	} else {
		dispay_message("���˼��� > ���� ��û > ����");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ���� step height�� ���� offset ����
//
//////////////////////////////////////////////////////////////////////////
void CMy3DmeasureDlg::OnEnChangeEidtMeasureOffset()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);

// 	m_inspector.setOffset(m_offet);
}


void CMy3DmeasureDlg::OnBnClickedCheckThicknessSensorUse()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnEnChangeEditTargetThickness()
{
	// TODO:  RICHEDIT ��Ʈ���� ���, �� ��Ʈ����
	// CDialogEx::OnInitDialog() �Լ��� ������ 
	//�ϰ� ����ũ�� OR �����Ͽ� ������ ENM_CHANGE �÷��׸� �����Ͽ� CRichEditCtrl().SetEventMask()�� ȣ������ ������
	// �� �˸� �޽����� ������ �ʽ��ϴ�.

	// TODO:  ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	UpdateData(TRUE);
}


void CMy3DmeasureDlg::OnBnClickedButtonInitialize()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	// 3D ���а� ����
	OnBnClickedButtonConnect();


	// ������ ����
	OnBnClickedButtonConnectRevolver();

	// 10�� ����� ����
	AfxMessageBox("3D ���а� ��� 10��� �����մϴ�. ���� ���� �� �浹 ����� ������ �ݵ�� Ȯ���� �ּ���");
	OnBnClickedButtonLens10();


	// �β� ���� ���� ����
	if( m_bUseThicknessSensor )
	{
		OnBnClickedButtonThicksensorOpen();
	}


	// � SW ����
	OnBnClickedButtonSocketCreate();
	OnBnClickedButtonSocketConnect();

}


void CMy3DmeasureDlg::OnBnClickedButtonRecipe()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	CDialogRecipe dlg;


	// ���� ���� �ε�
	dlg.init( &m_stMaskInfo );


	//
	if( dlg.DoModal()==IDOK )
	{
		m_stMaskInfo.iPositionNum = dlg.getData(m_stMaskInfo.dPositionAngle, m_stMaskInfo.bPositionPeak);
	}
}


void CMy3DmeasureDlg::OnBnClickedButtonSaveVk()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
