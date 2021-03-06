#if !defined(AFX_CLIENTSOCKET_H__7F6AE248_0AF7_46D6_A9BE_716EAAF0C738__INCLUDED_)
#define AFX_CLIENTSOCKET_H__7F6AE248_0AF7_46D6_A9BE_716EAAF0C738__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ClientSocket.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CClientSocket command target

class CClientSocket : public CSocket//CAsyncSocket
{
// Attributes
public:

// Operations
public:
	CClientSocket();
	virtual ~CClientSocket();

// Overrides
public:
	virtual void OnReceive( int nErrorCode );

	void SetHWnd(HWND hWnd);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClientSocket)
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CClientSocket)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

// Implementation
protected:
	HWND m_hWndParent;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLIENTSOCKET_H__7F6AE248_0AF7_46D6_A9BE_716EAAF0C738__INCLUDED_)
