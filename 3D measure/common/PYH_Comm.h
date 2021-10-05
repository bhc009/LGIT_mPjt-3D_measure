#if !defined(AFX_PYH_COMM_H__BC230D73_1ABD_4A40_813F_27B124A805C6__INCLUDED_)
#define AFX_PYH_COMM_H__BC230D73_1ABD_4A40_813F_27B124A805C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PYH_Comm.h : header file
//

#include <afxmt.h>

#define	MAXBUF		50000
#define	InBufSize	50000
#define	OutBufSize	50000
#define ASCII_XON	0x11
#define ASCII_XOFF	0x13
// #define	WM_MYRECEIVE	(WM_USER+10)
#define	WM_MYCLOSE		(WM_USER+11)

/////////////////////////////////////////////////////////////////////////////
// CPYH_Comm command target

class CPYH_Comm : public CCmdTarget
{
	DECLARE_DYNCREATE(CPYH_Comm)

		CPYH_Comm() {};           // protected constructor used by dynamic creation
		~CPYH_Comm();

// Attributes
public:
	HANDLE	m_hComDev;
	HWND	m_hWnd;

	BOOL	m_bIsOpenned;
	CString	m_sComPort;
	CString	m_sBaudRate;
	CString	m_sParity;
	CString	m_sDataBit;
	CString	m_sStopBit;
	BOOL	m_bFlowChk;

	UINT m_msgReceive;
	UINT m_msgClose;

	OVERLAPPED m_OLR,m_OLW;
	char	m_sInBuf[MAXBUF*2];
	int		m_nLength;
	CEvent*	m_pEvent;

// Operations
public:
	void Clear();
	int Receive(LPSTR inbuf, int len);
	BOOL Send(LPCTSTR outbuf, int len);
	BOOL Create(HWND hWnd);
	void HandleClose();
	void Close();
	void ResetSerial();
	CPYH_Comm(CString port,CString baudrate,CString parity,CString databit,CString stopbit);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPYH_Comm)
	//}}AFX_VIRTUAL

// Implementation
protected:
	

	// Generated message map functions
	//{{AFX_MSG(CPYH_Comm)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
UINT CommThread(LPVOID p);
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PYH_COMM_H__BC230D73_1ABD_4A40_813F_27B124A805C6__INCLUDED_)
