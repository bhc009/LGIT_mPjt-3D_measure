// ServerSocket.cpp : implementation file
//

#include "stdafx.h"
#include "ServerSocket.h"
#include "SocketModule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CServerSocket

CServerSocket::CServerSocket()
{
}

CServerSocket::~CServerSocket()
{
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CServerSocket, CSocket)
	//{{AFX_MSG_MAP(CServerSocket)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CServerSocket member functions

BOOL CServerSocket::Create(UINT nPortNum, HWND hWnd)
{
	if (!CAsyncSocket::Create(nPortNum, SOCK_STREAM))
	{
		return FALSE;
	}

	if (!CAsyncSocket::Listen())
	{
		return FALSE;
	}

	SetHWnd( hWnd );

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Message를 전달할 윈도우의 핸들 설정
void CServerSocket::SetHWnd(HWND hWnd)
{
	m_hWndParent = hWnd;
}

void CServerSocket::OnAccept(int nErrorCode)
{
	m_pClientSocket.Close();

	Accept( m_pClientSocket );

	m_pClientSocket.SetHWnd(m_hWndParent);

	SendMessage( m_hWndParent, WM_ACCEPT_SOCKET, 0, 0 );
}
