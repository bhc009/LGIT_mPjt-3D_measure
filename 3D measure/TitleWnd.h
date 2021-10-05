#pragma once


// CTitleWnd

class CTitleWnd : public CStatic
{
	DECLARE_DYNAMIC(CTitleWnd)

	COLORREF	m_crBkColor;
	COLORREF	m_crTextColor;

	CBrush		m_brBkgnd;

	CFont		m_Font;
	int			m_nFontSize;

public:
	CTitleWnd();
	virtual ~CTitleWnd();

	void SetBkColor(COLORREF crBkColor);
	void SetFontSize(int nSize);
	void SetTextColor(COLORREF crTextColor = 0xffffffff);
	COLORREF GetTextColor();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnDestroy();
};


