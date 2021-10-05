// TitleWnd.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "TitleWnd.h"


// CTitleWnd

IMPLEMENT_DYNAMIC(CTitleWnd, CStatic)

CTitleWnd::CTitleWnd()
{
	m_nFontSize = 17;

	//m_brBkgnd.CreateStockObject(NULL_BRUSH);
}

CTitleWnd::~CTitleWnd()
{
}


BEGIN_MESSAGE_MAP(CTitleWnd, CStatic)
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CTitleWnd 메시지 처리기입니다.
void CTitleWnd::OnDestroy()
{
	CStatic::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if(m_Font.GetSafeHandle()) m_Font.DeleteObject();
	if(m_brBkgnd.GetSafeHandle()) m_brBkgnd.DeleteObject();
}

void CTitleWnd::SetBkColor(COLORREF crBkColor)
{
	m_crBkColor = crBkColor;

	if(m_brBkgnd.GetSafeHandle()) m_brBkgnd.DeleteObject();
	
	if(m_brBkgnd.GetSafeHandle() == NULL)
		m_brBkgnd.CreateSolidBrush(crBkColor);

	Invalidate(FALSE);
}

void CTitleWnd::SetFontSize(int nSize)
{
	if(m_Font.GetSafeHandle()) m_Font.DeleteObject();

	m_nFontSize = nSize;

	LOGFONT lfont;
	CFont* pFont = GetFont();
	pFont->GetLogFont(&lfont);
	lfont.lfHeight = nSize;
	lfont.lfWeight = FW_BOLD;

	m_Font.CreateFontIndirect(&lfont);

	SetFont(&m_Font);
}

HBRUSH CTitleWnd::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetTextColor(m_crTextColor);

	if (m_brBkgnd.GetSafeHandle() != NULL)
	{
		pDC->SetBkColor(m_crBkColor);
		return (HBRUSH)m_brBkgnd;
	} else {
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)NULL_BRUSH;
	}

	return NULL;
}

void CTitleWnd::SetTextColor(COLORREF crTextColor)
{
	// Set new foreground color
	if (crTextColor != 0xffffffff)
	{
		m_crTextColor = crTextColor;
	}
	else // Set default foreground color
	{
		m_crTextColor = ::GetSysColor(COLOR_BTNTEXT);
	}

	// Repaint control
	Invalidate();
} // End of SetTextColor
