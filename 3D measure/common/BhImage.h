// BhImage.h: interface for the CBhImage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BHIMAGE_H__373CCD7B_9DC5_477B_92C0_6CB9EA369FA1__INCLUDED_)
#define AFX_BHIMAGE_H__373CCD7B_9DC5_477B_92C0_6CB9EA369FA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _GRAY_	0
#define _RGB_	1

class CBhImage  
{
public:
	void*	Create( int width, int height, int bpp, int imageType);

// 	BOOL	Open(CString fileName);
	BOOL	Open(LPCSTR fileName);
	void	Save(CString strFileName);
	void	Copy(CBhImage *img);

	void	Draw( HDC hdc, int x, int y );							// 이미지를 뿌려준다.
	void	Draw( HDC hdc, int x, int y, int width, int height );	// 주어진 폭과 높이에 맞게 이미지를 뿌려준다.
	void	Invalidate();

	BYTE*	GetImage() { return m_pImageBuffer; };	// 이미지에 대한 포인터
	int		GetColor();
	BYTE	GetPixelGrey(int x, int y);		// 이미지 픽셀의 값을 얻는다(Gray).
	RGBQUAD GetPixelColor(int x, int y);
	CSize	GetImageSize();					// 이미지의 버퍼를 얻는다.
	long	GetSize();						// 이미지를 저장하는 버퍼의 크기를 얻는다(이미지 크기와는 다를 수 있다).
	int		GetHeight();					// 이미지의 높이를 얻는다.
	int		GetWidth();						// 이미지의 폭을 얻는다.
	int		GetWidthStep();					// 이미지 버퍼의 폭을 얻는다.
	BOOL	GetROI( BYTE *pImg, CSize szImg, CRect rtROI );
	BOOL	SetROI( BYTE *pImg, CSize szImg, CRect rtROI );

	void	SetImage(BYTE value);								// 주어진 값으로 버퍼를 채운다.
	void	SetImage(BYTE *pImage, int iWidth, int iHeight);	// 주어진 버퍼로 이미지를 채운다.
	void	SetColorImage(BYTE *pImage, int iWidth, int iHeight);	// 주어진 버퍼로 이미지를 채운다.
	void	SetImage(BYTE *pImage, CSize szImage );	// 주어진 버퍼로 이미지를 채운다.
	void	SetPixelGrey(int x, int y, BYTE value);				// 

	BOOL	IsValid();					// 이미지가 로딩되어 있는가?
	BOOL	IsValidPoint(int x, int y);	// (x,y)가 이미지 상에 존재하는 점인가?
	void	Cut( int iStartX, int iStartY, int iWidth, int iHeight );
	
	CBhImage();
	virtual ~CBhImage();
protected:
	void	Init();					// 버퍼 초기화
	BOOL	Decode();

	void	FlipImage();			// 실제로 사용하는 이미지 버퍼로 DIP의 내용을 옮긴다.
	void	UnFlipImage();			// 실제 사용 이미지 버퍼의 내용을 DIP로 옮긴다.

	BYTE*		GetBits();			// 이미지 버퍼에 해당하는 영역의 포인터를 가져온다.
	DWORD		GetPaletteSize();	// 팔레트의 크기를 얻는다.
	RGBQUAD*	GetPalette() const;	// 팔레트의 포인터를 얻는다.

protected:
	void				*m_pDib;			// contains the header, the palette, the pixels
    BITMAPINFOHEADER    m_infoHeader;		// standard header
	BYTE				*m_pImage;			// the pixels
	BYTE				*m_pImageBuffer;	// addition - 실제로 보이는 이미지와 동일한 순서의 배열
	int					m_widthStep;		// valid width
	char				m_strFileName[512];
	bool				m_bIsImage;

	int m_nDumy;
};

#endif // !defined(AFX_BHIMAGE_H__373CCD7B_9DC5_477B_92C0_6CB9EA369FA1__INCLUDED_)
