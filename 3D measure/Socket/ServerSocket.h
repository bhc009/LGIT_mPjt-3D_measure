#if !defined(AFX_SERVERSOCKET_H__59B71016_7D12_40B4_A44D_3961CBFF2762__INCLUDED_)
#define AFX_SERVERSOCKET_H__59B71016_7D12_40B4_A44D_3961CBFF2762__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ServerSocket.h : header file
//

//#define _SERVER_	0
//#define _CLIENT_	1

#include "ClientSocket.h"


/////////////////////////////////////////////////////////////////////////////
// CServerSocket command target
class CServerSocket : public CSocket//CAsyncSocket
{
// Attributes
public:

// Operations
public:
	CServerSocket();
	virtual ~CServerSocket();

// Overrides
public:
	BOOL Create(UINT nPortNum, HWND hWnd );
	void SetHWnd(HWND hWnd);
	virtual void OnAccept( int nErrorCode );

	CClientSocket m_pClientSocket;

protected:

	HWND m_hWndParent;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerSocket)
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CServerSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERSOCKET_H__59B71016_7D12_40B4_A44D_3961CBFF2762__INCLUDED_)
