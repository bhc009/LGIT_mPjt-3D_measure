// ClientSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ClientSocket.h"
#include "SocketModule.h"

//#define WM_RECEIVE_DATA                WM_USER+1


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClientSocket

CClientSocket::CClientSocket()
{
}

CClientSocket::~CClientSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CClientSocket, CSocket)
	//{{AFX_MSG_MAP(CClientSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CClientSocket member functions

//////////////////////////////////////////////////////////////////////////
// Message를 전달할 윈도우의 핸들 설정
void CClientSocket::SetHWnd(HWND hWnd)
{
	m_hWndParent = hWnd;
}

void CClientSocket::OnReceive(int nErrorCode)
{
	//..
	SendMessage( m_hWndParent, WM_RECEIVE_DATA, 0, (LPARAM)this );
}
