#pragma once

#include "HalconCpp.h"
using namespace HalconCpp;

#include <list>
using namespace std;

enum HOLE_SHAPE
{
	// ���� : HOLE_SHAPE_[��������]_���۹�ġ
	HOLE_SHAPE_RECTANGLE,
	HOLE_SHAPE_DIAMOND,
	HOLE_SHAPE_CIRCLE,
	HOLE_SHAPE_ELIPSE,
	HOLE_SHAPE_AUTO_RG,
	HOLE_SHAPE_AUTO_B,
	HOLE_SHAPE_AUTO,
	HOLE_SHAPE_ELIPSE_45,
};


// #define HOLE_ARRANGE_RECT 0
// #define HOLE_ARRANGE_DIAMONE 1

enum HOLE_ARRANGEMENT
{
	RECTANGLE_ARRANGEMENT,
	DIAMOND_ARRANGEMENT,
};

struct PARAM_MASK_INFO
{
	HOLE_SHAPE shape;				// ���� ����
	HOLE_ARRANGEMENT arrangement;	// ���� �迭

	int iPitchX; // ���۰���[����:X, ����:pixel]
	int iPitchY; // ���۰���[����:Y, ����:pixel]
	int iWidth;	 // ����ũ��[����:X, ����:pixel]
	int iHeight; // ����ũ��[����:Y, ����:pixel]

	double dRealPitchX;	// ���۰���[����:X, ����:um]
	double dRealPitchY;	// ���۰���[����:Y, ����:um]
	double dRealWidth;	// ����ũ��[����:X, ����:um]
	double dRealHeight;	// ����ũ��[����:Y, ����:um]

	double dThickness;			// Mask �β�[����:um]
	double dStepHeightOffset;	// Step height offset[����:um]

	// ���� ��ġ ����
	int iPositionNum;			// ���� ����
	double dPositionAngle[36];	// ���� ����
	BOOL bPositionPeak[36];		// ���� ��ġ�� peak ����


	void calcRealValue( double dPixelResolution )
	{
		dRealPitchX = iPitchX*dPixelResolution;
		dRealPitchY = iPitchY*dPixelResolution; 
		dRealWidth  = iWidth*dPixelResolution; 
		dRealHeight = iHeight*dPixelResolution; 

	};

	PARAM_MASK_INFO()
	{
		iPitchX = 467;
		iPitchY = 467;
		iWidth	= 214;
		iHeight = 207;

		shape = HOLE_SHAPE_RECTANGLE;
		arrangement = RECTANGLE_ARRANGEMENT;

		dRealPitchX = 0;
		dRealPitchY = 0;
		dRealWidth = 0;
		dRealHeight = 0;

		dThickness = 25.0;
		dStepHeightOffset = 0;

		iPositionNum = 24;
		for( int i=0 ; i<iPositionNum ; i++ )
		{
			dPositionAngle[i] = 15*i;
			bPositionPeak[i] = FALSE;

			if( dPositionAngle[i]==0 ) bPositionPeak[i] = TRUE;
			if( dPositionAngle[i]==90 ) bPositionPeak[i] = TRUE;
			if( dPositionAngle[i]==180 ) bPositionPeak[i] = TRUE;
			if( dPositionAngle[i]==270 ) bPositionPeak[i] = TRUE;
		}
	};

};

struct PARAM_INSPECTION
{
	double dResolution;
};

enum Direction
{
	// ���� : HOLE_SHAPE_[��������]_���۹�ġ
	Right, RightUp, Up, LeftUp, Left, LeftBottom, Bottom, RightBottom
};

struct INFO_HOLESHAPE
{
	int iWidthMask[4];
	int iWidthHole[4];

	double dRealWidthMask[4];
	double dRealWidthHole[4];


	INFO_HOLESHAPE()
	{
		for( int i=0 ; i<4 ; i++ )
		{
			iWidthMask[i]		= 0;
			iWidthHole[i]		= 0;
			dRealWidthMask[i]	= 0.0;
			dRealWidthHole[i]	= 0.0;
		}
	}
};

class CInspection
{
public:
	CInspection(void);
	~CInspection(void);

// 	bool setParamer( double xPitch, double yPitch , double holeWidth, double holeHeight, HOLE_SHAPE shape );
	bool setParamImage( PARAM_MASK_INFO parameter );
	double getResolution() {return m_dResolution;};
	PARAM_MASK_INFO *getMaskInfo(){return &m_maskInfo;};

	// ���� ��ġ �� ���� ����
	int getMeasurePointNum();
	double* getMeasurePointAngle();
	BOOL* getMeasurePointIsland();
	
	// Calibration
	bool calibration( double *p3dData, int width, int height, double distData, double dThickness );

	// ����ũ �̵��� ���
	bool checkMaskMovement( BYTE *pCcdImage, int width, int height, double *dxMask, double *dyMask );
	bool checkMaskMovementDic( BYTE *pDicImage, int width, int height, double *dxMask, double *dyMask );

	// ����� �˻�
	bool measureSlopeAngleAtLargeHoleRegion( double *pHeightMap, BYTE *pLasertMap, BYTE *pDicImage, int width, int height, double dDistance, list<double> *listSlopeAngle, list<double> *listStepHeight );

	// �Ұ��� �˻�
	bool measureStepHeightAtSmallHoleRegion( double *pHeightMap, BYTE *pLasertMap, BYTE *pDicImage, int width, int height, list<double> *dStepHeight );
	
	// �β� ����
	bool measureT( double *pHeightMap, int width, int height, double dDistance, double *dThickness );
	
	bool m_bDisplayImage;

	void clearResult();
	list<double> m_listPeakToPeakDistance;
	list<double> m_list3DdataIslandHeight;	// Island�� 3D ������
	list<double> m_list3DdataHoleEdgeHeight;	// Hole edge�� 3D ������
	list<double> m_list3DdataIslandEdgeDistance;	// Island�� 3D ������

	list<double> m_listDistanceData;
	list<double> m_listThickness;

	list<double> m_list3DdataCal;
	list<double> m_listDistanceDataCal;

	INFO_HOLESHAPE m_holeInfo;

	UINT m_nCnt;
// 	double m_dThickness;

	void setOffset(double dOffset);
	void setTargetThickness( double dThickness );



protected:
// 	double m_dThicknessOffset;	// ���� step height�� ���� offset ��

protected:
	void DISPLAY( const HalconCpp::HImage & image, CString strMesage = "" );
	void DISPLAY( const HalconCpp::HRegion & region, CString strMesage = "" );
	void DISPLAY( const HalconCpp::HObject & object, CString strMesage = "" );
	void DISPLAY( const HalconCpp::HImage & image, HalconCpp::HRegion & region, CString strMesage = "");
	void DISPLAY( const HalconCpp::HImage & image, const HalconCpp::HXLDCont & xld, CString strMesage = "" );

	double getPitchXPixel();
	double getPitchYPixel();
	double getPixelToMirco(double dPixel);

	bool threshold(double *pData, int width, int height);

	// ����ũ �̵��� ���
	bool checkMaskMovement( HImage& ccdImage, HTuple *dxMask, HTuple *dyMask );
	bool checkMaskMovementDic( HImage& dicImage, HTuple *dxMask, HTuple *dyMask );

	// ����� �˻�
	bool measureLargeHoleDiamond(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HTuple& Row, const HalconCpp::HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak);
	bool measureSlopeAngleFromPeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights);
	bool measureSlopeAngleFromOnePeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights);
	bool measureSlopeAngleGreen(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HTuple& Row, const HalconCpp::HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak);
	bool measureLargeHoleAutoRG(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HTuple& Row, const HalconCpp::HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak);
	bool measureLargeHoleAutoB(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HTuple& Row, const HalconCpp::HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak);
	bool measureLargeHoleAuto(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HTuple& Row, const HalconCpp::HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak);


	bool measureSlopeAngles(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak);
	bool measureOneLargeHoleFullIsland(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak);
	bool measureOneLargeHoleHalfIsland(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& islandRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak);


	// �Ұ��� �˻�
	bool measureStepHeightAtSmallHoleRegion(const HImage& laserImage, const HImage& heightImage, const HImage& dicImage, const HTuple& height, const HTuple& width, HTuple *pStepHeight );
	bool measureStepHieghts(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& islandRegion, HTuple *pHeightHole, HTuple *pHeightPeak);

	// �β� ����
	bool calculateThickness( double d3dData, double dDistanceData, double *dResult );

	//
	bool measureMaskWidth( const HImage& laserImage );
	bool measureHoleWidth( const HImage& laserImage );

	bool findHoleEdge( HImage& laserImage, HImage& heightImage, HImage& dicImage, HRegion& holeRegion, HTuple width, HTuple height, HTuple *result_height, HTuple *result_row, HTuple *result_col, int direction );
	bool findPeakRegion( const HImage& heightImage, HRegion& rgRoi, double dLowOffset, double dHighOffset, double *peakPosRow, double *peakPosCol, double *dPeakHeight );	// �־��� ���������� ���� ���� ���츮�� ã�´�.
	bool findPeakRegion( const HImage& heightImage, HRegion& rgRoi, double dLowOffset, double dHighOffset, double *peakPosRow, double *peakPosCol, double *dPeakHeight, HRegion& resultRegion );	// �־��� ���������� ���� ���� ���츮�� ã�´�.
	bool findNearRegionIndex( HRegion regions, HTuple rowTarget, HTuple colTarget, HTuple *index );	// Ư�� ��ġ�� ���� ����� ������ ã�´�.
	bool findEdgePoint( HTuple tData, int *pPos );
	bool findHoleRegion( HImage& laserImage, HImage& heightImage,  HRegion& rgnIsland, HRegion& resultRegion);
	bool findIslandRegion( HImage& laserImage, HImage& heightImage, HRegion& resultRegion);
	bool findIslandRegionSmallIsland( HImage& laserImage, HImage& heightImage, HRegion& resultRegion);
	bool findIslandRegion_atHeightMap( HImage& laserImage, HImage& heightImage, HRegion& resultRegion);

	bool extractHoleInfo( HRegion holeRegion, double *pWidth );


	PARAM_MASK_INFO m_maskInfo;
	double m_dMarginRatio;
	double m_dResolution;

	double m_dTotalDistance;
};
