#pragma once


// CDistanceSensor
#include "common/PYH_Comm.h"

#define BUFFER_LENGTH	10000
#define MAX_DISTANCE_COUNT	10

#define WM_RECEIVE_THICKNESS_SENSOR (WM_USER+10)
#define WM_CLOSE_THICKNESS_SENSOR (WM_USER+11)

#define DS_MODE_CALIBRATION	0
#define DS_MODE_MEASURE	1

class CDistanceSensor : public CWnd
{
	DECLARE_DYNAMIC(CDistanceSensor)

public:
	CDistanceSensor();
	virtual ~CDistanceSensor();


public:
	CPYH_Comm* m_Comm;
	HWND m_hWndParent;

	int m_iMode;	// ���� ��� : CAL or FMM
	int m_iPos;		// ���� ��ġ : 0 ~
	
	double m_dValue;
	char *m_pData;
	int m_nData;
	BOOL m_bConnection;
	CString m_strPortNum;

	int m_cntValid;
	double m_calibrationDistance[MAX_DISTANCE_COUNT];	// chuck�� ���� �� �Ÿ�
	double m_measureDistance[MAX_DISTANCE_COUNT];		// FMM�� ���� �� �Ÿ�
	double m_distanceDifference[MAX_DISTANCE_COUNT];	// FMM�� �������� �������� ���� �� ����
	double m_thickness[MAX_DISTANCE_COUNT];				// �β� ��

public:
	bool open();
	bool close();
	double measure();
	bool measureAll();
	bool read(int len);
	void clear();
	BOOL IsConnected() {return m_bConnection;};

	void setState( int iMode, int iPos );
	void SetPortNum( int iNum );

	double getDistance();

	BOOL setCalibrationkDistance(int iPos, double dValue);
	BOOL setMeasureDistance(int iPos, double dValue);
	BOOL calcThickness();
	double GetThickness( int iPos );
	double GetThickness( );


private:
	double decode();
	int convert( char c );



protected:
	afx_msg LRESULT OnReceive( WPARAM, LPARAM );
	DECLARE_MESSAGE_MAP()
};


