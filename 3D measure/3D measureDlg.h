
// 3D measureDlg.h : ��� ����
//

#pragma once

#include "3dMesure/C3dMeasure.h"
#include "3dMesure/MeConfocalSensor.h"
#include "Inspection.h"
#include "afxwin.h"
#include "LensChanger.h"
#include "XML/tinyxml2.h"
#include "Socket/SocketModule.h"
#include "BhThread.h"
#include "Protocol.h"
#include "DistanceSensor.h"

#include "common/Logger.h"
using namespace CPlusPlusLogging;

#include "TitleWnd.h"

#define WM_END_MEASURE		WM_USER+1
#define WM_RECEIVE_DATA		WM_USER+2
#define WM_ACCEPT_SOCKET	WM_USER+3
#define WM_UPDATEDATA_FALSE	WM_USER+4

#define PATH_DATA_HEIGHT_ANGLE	"c:\\Test data\\Step height & Slope angle.csv"
#define PATH_DATA_STEP_HEIGHT	"c:\\Test data\\Step height.csv"
#define PATH_LOG	"d:\\log\\3D\\"

typedef enum {
	DIRECT_METHOD	= 0,
	INDIRECT_METHOD	= 1,
} thickness_method;


#define _LENS_CMD_FROM_OP_		0
#define _LENS_CMD_FROM_USER_	1


typedef enum {
// 	MEASURE_METHOD_SLOPEANGLE	= 0,
	MEASURE_METHOD_SMALL_HOLE	= 0,
	MEASURE_METHOD_LARGE_HOLE	= 1,
	MEASURE_METHOD_THICKNESS	= 2,
} MESUREING_METHOD;


typedef enum {
	MANUAL_MODE	= 0,
	AUTO_MODE	= 1,
} MODE_OPERATION;


typedef enum {
	STATE_NONE		= 0,
	STATE_RUNING	= 1,
	STATE_AUTOFOCUS	= 2,
	STATE_CALIBRATION	= 3,
	STATE_MEASURE	= 4,
	STATE_ERROR	= 5,
	STATE_LENS_CHANGE	= 6,
} PGM_STATE;

typedef enum {
	SITE_LGD		= 0,
	SITE_LGIT	= 1,
} PGM_SITE;

typedef struct _REQUEST_OMM
{
	//
	BOOL measureThickness;
	int measureThicknessPos;
	BOOL measureThicknessCAL;

	_REQUEST_OMM()
	{
		measureThickness = FALSE;
		measureThicknessPos = 0;
		measureThicknessCAL = FALSE;
	}
}REQUEST_OMM;


typedef struct _RECIPE_3D_
{
	//
	double dScanRangeUp;
	double dScanRangeDown;

	_RECIPE_3D_()
	{
		dScanRangeUp = 10.0;
		dScanRangeDown = 30.0;
	}
}RECIPE_3D;


// CMy3DmeasureDlg ��ȭ ����
class CMy3DmeasureDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CMy3DmeasureDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_MY3DMEASURE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()


public:
	PGM_STATE m_state;	// ���α׷� ����
	PGM_SITE m_stSite;

	REQUEST_OMM m_requests;
	RECIPE_3D m_stRecip3D;

	C3dMeasure m_3dMeasure;				// 3D ���� ���
	CMeConfocalSensor m_distanceSensor;	// �Ÿ� ���� ���
	CLensChanger m_lensChanger;			// ���� ���� ���
	CInspection	m_inspector;			// ��� ���
	CBhThread m_thread;					// Dlg ������ event ������ �ʵǾ ��¿ �� ����...
	CSocketModule m_socketModule;		// ��� ���
	CProtocol m_protocol;
	CDistanceSensor m_thicknessSensor;	// �β� ������ ���˼���


	CWinThread *m_hThreadMeasure;	// ���� ������
	CWinThread *m_hThreadAutofocus;	// auto focus ������
	CWinThread *m_hThreadCalibration;	// calibration ������
	CWinThread *m_hThreadChangeLens;	// calibration ������

	PARAM_MASK_INFO m_stMaskInfo;

	Logger* m_pLogger;	// Log class


	//////////////////////////////////////////////////////////////////////////
	//
	LARGE_INTEGER m_globalCounterStart, m_globalCounterEnd, m_globalCounterFreq;
	LARGE_INTEGER m_localCounterStart, m_localCounterEnd, m_localCounterFreq;
	void start_global_counter();
	void end_global_counter(CString strMessage);
	void start_local_counter();
	void end_local_counter(CString strMessage);
	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//
	void dispay_message(CString strMsg);
	bool changePgmState( PGM_STATE statusIn );
	bool IsPgmStateFree();
	void loadInit();
	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//
	bool autoFocus();
	bool autoFocus( UINT mag );		// Auto focus �ǽ�
	bool changeLens( UINT mag );	// ���� ����

	int m_iLensMag_src;
	int m_iLensMag_taget;




	bool setMeasureRange( bool bAuto, double dLowOffset, double dHighOffset );	// 3D ���� range ����
	BOOL moveLens( double dMicro );
	bool measure3d( double *dDistance );										// 3D ����

	bool sequenceCheckMaskMovement();	// mask �̵��� üũ
	bool sequenceCalibration();			// calibration ����
	bool sequenceMeasure();				// ���� ����
	bool sequenceCharngeLens();				// ���� ����

	bool measure3d();
	void measureSlopeAngle();
	void measureSmallHoleRegion();
	void measureLargeHoleRegion();
	void measureThickness();
	double measureDistance(int nTime);

	void stopMeasure();

	void update3dPosition();
	//
	//////////////////////////////////////////////////////////////////////////


	void setSite();
	void makeSaveFolder();
	void makeLogFolder();

	int GetPortNumber_thickSensor();

	void ToggleStateCheck();
	BOOL m_bStateCheck;

	//////////////////////////////////////////////////////////////////////////
	//
	// Dlg items
	//
	int m_operationMode;	// ��� ���

	CListBox m_ctrlMessage;		// �޼��� â
	BOOL m_ctrlDisplayProcessingImage;

	// 3D ī�޶� ����
	CComboBox m_ctrl3dViewType;	// 3D ī�޶� ����
	double m_dCurPos;
	double m_dUpperPos;
	double m_dLowerPos;
	double m_d3dRangeDistance;
	double m_3dMeaserRangeLowerOffset;	// um
	double m_3dMeaserRangeUpperOffset;	// um

	// ��ǰ ����
	CComboBox m_ctrlHoleShape;	// Hole ���� ����
	double m_dHolePitchX;	// Hole pitch ����
	double m_dHolePitchY;	// Hole pitch ����
	double m_dHoleWidth;	// Hole �� ����
	double m_dHoleHeight;	// Hole ���� ����
	double m_targetThickness;
	CComboBox m_ctrlHoleInfoArrange;
	double m_offet;


	// calibration
	double m_dCalibrationThickness;	// Calibration �÷��� �β�


	// ���� ����
	UINT m_nIteration;	// ���� �ݺ� Ƚ��
	int m_iModeMeasureMethod;	// ���� ��� ����
	bool m_bStop;		// ���� �ߴ� flag


	// ���� ����
	int m_soketType;
	CString m_socketIp;
	UINT m_socketPort;


	// ������ ����
	CComboBox m_ctrlRevolverPortNum;


	// ���� ����
	BOOL m_bUseThicknessSensor;


	// �Ÿ�����
	CString m_distanceSensorIP;
	BOOL m_bUseDistanceSensor;


	// �ӽ�
	int m_nCnt;
	unsigned int m_nCntSaveImage;
	int m_nCntCal;
	CString m_strSaveFilePath;
	CString m_strRecipe;
	CString m_strQRCode;


	// ǥ�õ�
	CTitleWnd m_stc3dConnection;
	CTitleWnd m_stc3dMeasure;
	CTitleWnd m_stcRevoverConnection;
	CTitleWnd m_stcRevoverServo;
	CTitleWnd m_stcRevoverRun;
	CTitleWnd m_stcProgram;
	CTitleWnd m_stcSocketConnection;
	CTitleWnd m_stcContactSensorConnection;
	CTitleWnd m_stcStateCheck;

public:
	afx_msg LRESULT OnReceive( WPARAM, LPARAM );
	afx_msg LRESULT OnUpdateData( WPARAM, LPARAM );
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	afx_msg void OnBnClickedButtonMeasure();
	afx_msg void OnBnClickedButtonSaveHeight();
	afx_msg void OnBnClickedButtonLoadHeight();
	afx_msg void OnBnClickedButtonSaveLaser();
	afx_msg void OnBnClickedButtonLoadLaser();
	afx_msg void OnBnClickedButtonSaveDct();
	afx_msg void OnBnClickedButtonLoadDct();
	afx_msg void OnBnClickedButtonReadData();
	afx_msg void OnBnClickedButtonSaveLightImage();
	afx_msg void OnBnClickedRadioHeight();
	afx_msg void OnBnClickedRadioLaser();
	afx_msg void OnBnClickedRadioDic();
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedButtonConnectDistSensor();
	afx_msg void OnBnClickedButtonMeasureDist();
	afx_msg void OnBnClickedButtonDisconnectDistSensor();
	afx_msg void OnBnClickedButtonStopMeasureDist();
	afx_msg void OnBnClickedButtonMeasureDistOnece();
	afx_msg void OnBnClickedButtonCalibration();
	afx_msg void OnEnChangeEditCalibrationThick();
	afx_msg void OnBnClickedRadioThicknessMethodDirect();
	afx_msg void OnBnClickedRadioThicknessMethodIndirect();
	afx_msg void OnBnClickedButtonServoOn();
	afx_msg void OnBnClickedButtonLens10();
	afx_msg void OnBnClickedButtonLens20();
	afx_msg void OnBnClickedButtonLens50();
	afx_msg void OnBnClickedButtonLens150();
	afx_msg void OnBnClickedButtonConnectRevolver();
	afx_msg void OnBnClickedButtonDisconnectRevolver();
	afx_msg void OnBnClickedButtonServoOff();
	afx_msg void OnBnClickedButtonGoHome();
	afx_msg void OnBnClickedButton3dLaserAutofofus();
	afx_msg void OnBnClickedButton3dImageAutofofus();
	afx_msg void OnBnClickedButton3dUp();
	afx_msg void OnBnClickedButton3dDown();
	afx_msg void OnEnChangeEditIteration();
	afx_msg void OnBnClickedButtonCalculate();
	afx_msg void OnBnClickedButtonStopCaculation();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonMeasureThickness();
	afx_msg void OnBnClickedButtonSocketCreate();
	afx_msg void OnBnClickedRadioServerSocket();
	afx_msg void OnBnClickedRadioClientSocket();
	afx_msg void OnBnClickedButtonSocketConnect();
	afx_msg void OnBnClickedButtonSocketDisconnect();

	afx_msg LRESULT OnAcceptSocket( WPARAM, LPARAM );
	afx_msg LRESULT OnReceiveSocket( WPARAM, LPARAM );
	afx_msg LRESULT OnReceiveThichness( WPARAM, LPARAM );

	// �������
// 	afx_msg void OnBnClickedRadioMeasureSlopeangle();
	afx_msg void OnBnClickedRadioMeasureStepHeight();
	afx_msg void OnBnClickedRadioMeasureSlopeangleNStepHeight();
	afx_msg void OnBnClickedRadioMeasureThickness();
	afx_msg void OnBnClickedButtonStopCalculation();
	afx_msg void OnBnClickedButton3dSetUpperPos();
	afx_msg void OnBnClickedButton3dSetLowerPos();
	afx_msg void OnBnClickedButton3dSetDistance();
	afx_msg void OnBnClickedRadioModeAuto();
	afx_msg void OnBnClickedRadioModeManual();
	afx_msg void OnEnChangeEdit3dLowerDistance();
	afx_msg void OnEnChangeEdit3dUpperDistance();
	afx_msg void OnBnClickedButton3dAutoRange();
	afx_msg void OnBnClickedButton3dSelectViewType();
	afx_msg void OnEnChangeEdit6();
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnEnChangeEdit4();
	afx_msg void OnEnChangeEdit5();
	afx_msg void OnBnClickedButtonSetHoleShapeInfo();
	afx_msg void OnBnClickedButtonAutoFocus();
	afx_msg void OnBnClickedCheckDisplayProcessingImage();
	afx_msg void OnBnClickedButtonSaveCcd();
	afx_msg void OnBnClickedButtonLoadCcd();
	afx_msg void OnBnClickedButtonTestMaskMovement();
	afx_msg void OnBnClickedButtonTest2();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonLensNone();
	// �Ÿ������� IP �ּ�
	afx_msg void OnEnChangeDistanceSensorIp();
	afx_msg void OnBnClickedCheckUseDistance();
	afx_msg void OnBnClickedButtonThicksensorOpen();
	afx_msg void OnBnClickedButtonThicksensorRead();
	afx_msg void OnEnChangeEidtMeasureOffset();
	afx_msg void OnBnClickedCheckThicknessSensorUse();
	afx_msg void OnEnChangeEditTargetThickness();
	afx_msg void OnBnClickedButtonInitialize();
	afx_msg void OnBnClickedButtonRecipe();
	afx_msg void OnBnClickedButtonSaveVk();
	CComboBox m_comboComPort_thickSensor;
};
