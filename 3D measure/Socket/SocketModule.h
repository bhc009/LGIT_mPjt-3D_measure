// SocketModule.h: interface for the CSocketModule class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOCKETMODULE_H__2E84EFE1_69C0_4E57_AE42_1E38797A1918__INCLUDED_)
#define AFX_SOCKETMODULE_H__2E84EFE1_69C0_4E57_AE42_1E38797A1918__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "ClientSocket.h"
#include "ServerSocket.h"

#include "../XML/tinyxml2.h"


#define WM_RECEIVE_DATA				WM_USER+2
#define WM_ACCEPT_SOCKET			WM_USER+3


// Client to master
#define _TERMINATE_CONNECTION_		0x00
#define _SUCCESS_INITILAIZE_		0x01
#define _FAIL_INITIALIZE_			0x02
#define _PROCESSING_RESULT_			0x03


// Master to client
#define _SET_PROCESSING_			0x00
#define _START_PROCESSING_			0x01
#define _STOP_PROCESSING_			0x02
#define _SEND_MISS_ID_				0x03
#define _SEND_REFERENCE_			0x04


// #define _FT_DEFAULT_	0

// #define _FT_SERVER_	1
// #define _FT_CLIENT_	2

// #define _FT_WAIT_			1
// #define _FT_CONNECT_		2
// #define _FT_DISCONNECT_	3


#define _SERVER_	0
#define _CLIENT_	1

#define _SOCKET_BUFFER_SIZE_	9999

#define _PACKET_SIZE	4+16*20
#define _MASTER_PACKET_SIZE	999
#define _CLIENT_PACKET_SIZE	999

#define _CLIENT_PACKET_HEADER_	2 + 1 + 8 + 1	// [Length] + [mode] + [frameNum] + [FeatureNum]
#define _CLIENT_PACKET_DATA_	4 + 1 + 15*15	// [position] + [status] + [imageData]

#define _SOCKET_MODULE_LGD	0
#define _SOCKET_MODULE_LGIT	1



class CSocketModule : CWnd
{
public:
	CSocketModule();
	virtual ~CSocketModule();

	void SetParameter( HWND hWnd, DWORD iPortNum, CString strMasterIp );
	int GetStatus() { return m_iStatus; };


	// Start
	BOOL Create( int iType );
	BOOL Start();
	void Stop();


	// Send & Receive
	void Send(BYTE *strSend, int nSize);
	BYTE* OnReceive( unsigned int *iLength );


	// Socket
	CServerSocket	*m_pServerSocket;
	CClientSocket	*m_pClientSocket;


	// 
	int m_site;
	void SendMeasureResult( bool bSuccess, double dStepHeight, double dSlopeAngle );
	void SendMeasureResult( bool bSuccess, double dStepHeight1, double dStepHeight2, double dStepHeight3, double dStepHeight4, double dSlopeAngle1, double dSlopeAngle2, double dSlopeAngle3, double dSlopeAngle4 );
	void SendAutoFocusResult( bool bSuccess, double dDx, double dDy );
	void SendCalibrationResult( bool bSuccess );
	void SendLensChangeResult( bool bSuccess );
	void SendSetReceipeResult( bool bSuccess );
	void SendState( bool bRun, int lens );
	void SendStop( bool bValid );
	void SendThicknessResult( double dThickness );

	BOOL IsConnection() { return m_bConnection; };

protected:
	void DeleteSocket();

	HWND	m_hWndParent;
	CString m_MasterIp;
	DWORD   m_MasterPort;

	int m_iStatus;			// Server인가 client인가?

	BYTE *m_receiveData;
	BYTE *m_sendData;

	// Packet size
	int m_szSendPacket;		// 보내는 packet의 크기
	int m_szReceivePacket;	// 받는 packet의 크기

	//
	BOOL m_bConnection;
	
	// Generated message map functions
	//{{AFX_MSG(CSocketDlg)
	//afx_msg LONG OnAccept( UINT, LONG );
	//afx_msg LONG OnReceive( UINT, LONG );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_SOCKETMODULE_H__2E84EFE1_69C0_4E57_AE42_1E38797A1918__INCLUDED_)
