#pragma once

#include "VkViewerRemoteOperation.h"
#include "VkViewerRemoteOperationCompatible.h"

// enum 3D_STATE
// {
// 	DEFAULT,
// 	CONNECTED,
// 	DISCONNECTED;
// 	MEASURING,
// };

#define PATH_SAVE			"c:\\Test data"	// 아래의 위치와 일치해야함
#define PATH_HEIGHT_MAP		"c:\\Test data\\Height map.csv"
#define PATH_LASER_MAP		"c:\\Test data\\Laser map.csv"
#define PATH_LASER_IMAGE	"c:\\Test data\\Laser image.bmp"
#define PATH_DIC_IMAGE		"c:\\Test data\\DIC image.bmp"
#define PATH_CCD_IMAGE		"c:\\Test data\\CCD image.bmp"
#define PATH_DATA_THICKNESS "c:\\Test data\\thickness.csv"
#define PATH_VK_IMAGE		"c:\\Test data\\Vk image"

#define _3D_WIDTH_	1024
#define _3D_HEIGHT_	768
#define _3D_RESOLUTION_	(0.2783935546875)	// um/pixel

#define _3DM_CONNECT_	0
#define _3DM_MEASURE_	1

#include "..\common\BhImage.h"


class C3dMeasure
{
public:
	C3dMeasure(void);
	~C3dMeasure(void);

	CString m_strPathHeightMap; 
	CString m_strPathLaserMap; 
	CString m_strPathLaserImage; 
	CString m_strPathDicImage; 
	CString m_strPathCcdImage; 
	CString m_strPathVkImage; 
	CString m_strPathThickness; 


	BOOL m_bStateConnection;
	BOOL m_bStateMeasure;

	HANDLE m_hEventCheckEndMeasure;
	CWinThread *m_hThreadCheckEndMeasure;

	HWND m_hWndParent;
	UINT m_uiMessage;

	BOOL connect();
	BOOL disconnect();
	BOOL isMeasuring();

	BOOL initializeLensPos();
	BOOL GetCurrentLensPos(int *pLensPos);

	BOOL setState( int type, BOOL bValue );
	BOOL setViewType( int iType );

	BOOL autofocusImage();
	BOOL autofocusLaser();

	BOOL moveUp(long nanoStep);
	BOOL moveDown(long nanoStep);
	BOOL move(long nanoStep);

	BOOL measure();
	BOOL measureStop();
	BOOL measureEnd();

	BOOL read3dData();

	BOOL saveHeightMap();
	BOOL loadHeightMap();
	BOOL loadHeightMap(CString strFilePath);

	BOOL saveLaserMap();
	BOOL loadLaserMap();
	BOOL loadLaserMap(CString strFilePath);
	BOOL saveLaserImage();

	BOOL saveDicImage();
	BOOL loadDicImage();
	BOOL loadDicImage(CString strFilePath);

	BOOL saveCcdImage();
	BOOL loadCcdImage();
	BOOL loadCcdImage(CString strFilePath);

	BOOL saveVkImage();

	BOOL setParam( HWND hWnd, UINT uiMsg);

	double* getHeightMap( int *pWidth, int *pHeight );
	unsigned short *getLaserMap( int *pWidth, int *pHeight );
	BYTE *getDicImage( int *pWidth, int *pHeight );
	BYTE *getLaserImage( int *pWidth, int *pHeight );
	BYTE *getCcdImage( int *pWidth, int *pHeight );


	bool getCurPos(long *pos);
	bool getUpperPos(long *pos);
	bool setUpperPos(long dPos);
	bool getLowerPos(long *pos);
	bool setLowerPos(long dPos);
	bool getDistance(long *dist);
	bool setDistance(long dDistance);
	bool setMeasureRangeOffset( long lLowerOffset, long lUpperOffset );

	bool getZoom(int *pZoom);

protected:
	BOOL GetHeightBasePosition( double &height_base_position );

	BOOL SetMeasurementResultDataSize(MeasurementQuality measurementQuality);

	BOOL SaveHeightDataToCsvFile(double* pData, long width, long height );
	VKResult GetDataSize(long* plWidth, long* plHeight);
	BOOL getHeightData(long* pSrc, double* pDst, long width, long height, double height_base_position );

	bool MakeFolder();

	int m_nLastMeasurementResultDataWidth;
	int m_nLastMeasurementResultDataHeight;
	double m_dHeightBasePosition;

	long* m_pHeight_origin;			// 높이 데이터( 0.1 nm 단위, 상대적 높이)
	double* m_pHeightMap;			// 높이 데이터( 1 um 단위, 절대적 높이)
	unsigned short* m_pLaserMap;	// Laser 밝기값
	BYTE* m_pLaserImage;			// Laser image
	BYTE *m_pDicMap;				// DIC image
	BYTE *m_pCddImage;				// ccd image
};

