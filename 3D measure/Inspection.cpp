#include "StdAfx.h"
#include "Inspection.h"

// #include "HalconCpp.h"
using namespace HalconCpp;

CInspection::CInspection(void)
{
	m_dMarginRatio	= 0.1;
	m_dResolution	= 0.2783935546875;	// um/pixel

	m_bDisplayImage = false;

	m_dTotalDistance = 6611.0944535860999;

	m_nCnt = 0;
}


CInspection::~CInspection(void)
{
}

/*
bool CInspection::setParamer( double xPitch, double yPitch , double holeWidth, double holeHeight, HOLE_SHAPE shape )
{
m_maskInfo.iPitchX		= xPitch;
m_maskInfo.iPitchY		= yPitch;
m_maskInfo.iWidth	= holeWidth;
m_maskInfo.iHeight	= holeHeight;

m_holeShape = shape;

switch( m_holeShape )
{
case HOLE_SHAPE_RECTANGLE_DIAMOND:
case HOLE_SHAPE_RECTANGLE_RECTANGLE:
m_dArea = m_maskInfo.iWidth*m_maskInfo.iHeight;
break;

case HOLE_SHAPE_DIAMOND_DIAMOND:
case HOLE_SHAPE_DIAMOND_RECTANGLE:
m_dArea = m_maskInfo.iWidth*m_maskInfo.iHeight/2;
break;

case HOLE_SHAPE_CIRCLE_DIAMOND:
case HOLE_SHAPE_CIRCLE_RECTANGLE:
m_dArea = PI * pow(m_maskInfo.iWidth/2, 2);
break;
}

return true;
}
*/

bool CInspection::threshold(double *pData, int width, int height)
{
	HObject ho_image;

	// 생성
	GenImageConst(&ho_image, "real", width, height);


	// 
	// 	HTuple ptr, lWidth, lHeight, hType;
	// 	GetImagePointer1(ho_image, &ptr, &hType, &lWidth, &lHeight

	// 
	for( int y=0 ; y<height ; y++ )
	{
		for( int x=0 ; x<width ; x++ )
		{
			SetGrayval(ho_image, y, x, pData[x + y*width]);
		}
	}

	// 	WriteImage( ho_image, "hobj", 0, "d:\\test data\\hImage");
	// 
	// 
	// 	HWindow w(0,0,1024,768);
	// 
	// 	w.DispObj(ho_image);
	// 
	// 	w.Click();


	// 	HImage imageIn(ho_image);
	// 	HImage imageIn("D:\\Test data\\Laser image.bmp");
	// 	double angle=0.f;
	// 	double peakHeight=0.f;
	// 	double holeHeight=0.f;
	// 	measureSlopeAngle( imageIn, imageIn, height, width, &angle, &holeHeight, &peakHeight );

	return true;
}


bool CInspection::setParamImage( PARAM_MASK_INFO parameter )
{
	m_maskInfo = parameter;

	m_maskInfo.calcRealValue(m_dResolution);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 측정 시 측정하는 point 갯수 얻기
//
//////////////////////////////////////////////////////////////////////////
int CInspection::getMeasurePointNum()
{
	return m_maskInfo.iPositionNum;
}



double* CInspection::getMeasurePointAngle()
{
	return m_maskInfo.dPositionAngle;
}


BOOL* CInspection::getMeasurePointIsland()
{
	return m_maskInfo.bPositionPeak;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::calibration( double *p3dData, int width, int height, double distData, double dThickness )
{
	if( !p3dData || width<1 || height<1 )
	{
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// 1. 3D 데이터 결정
	//////////////////////////////////////////////////////////////////////////
	int iHalfRange = 50;
	int cx = width/2;
	int cy = height/2;

	int nData=0;
	double dSum = 0;
	for( int y=max(cy-iHalfRange, 0) ; y<=min( cy+iHalfRange, height-1 ) ; y++ )
	{
		for( int x=max( cx-iHalfRange, 0) ; x<=min( cx+iHalfRange, width-1 ) ; x++ )
		{
			dSum += p3dData[ x + y*width ]/1000.0;
			nData++;
		}
	}

	if( nData<=0 )
	{
		return false;
	}

	double dAvg3dData = 1000.0*dSum/(double)nData;	// 3D에서 측정된 평균 높이값


	//////////////////////////////////////////////////////////////////////////
	// 
	//////////////////////////////////////////////////////////////////////////
	m_dTotalDistance = dAvg3dData + distData + dThickness;

	m_list3DdataCal.push_back(dAvg3dData);
	m_listDistanceDataCal.push_back(distData);

	return true;
}


bool CInspection::checkMaskMovement( BYTE *pCcdImage, int width, int height, double *dxMask, double *dyMask )
{
	if( !pCcdImage || width<1 || height<1 )
	{
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// Make Ccd image
	//////////////////////////////////////////////////////////////////////////
	BYTE *pRed = new BYTE[width*height];
	BYTE *pGreen = new BYTE[width*height];
	BYTE *pBlue = new BYTE[width*height];
	for( int y=0 ; y<height ; y++ )
	{
		for( int x=0 ; x<width ; x++ )
		{
			pRed[ x + y*width ] = pCcdImage[ 3*x + 2 + y*3*width];
			pGreen[ x + y*width ] = pCcdImage[ 3*x + 1 + y*3*width];
			pBlue[ x + y*width ] = pCcdImage[ 3*x + 0 + y*3*width];
		}
	}

	HImage ccdImage;
	ccdImage.GenImage3("byte", width, height, pRed, pGreen, pBlue );

	delete[] pRed;
	delete[] pGreen;
	delete[] pBlue;

	DISPLAY(ccdImage);


	//////////////////////////////////////////////////////////////////////////
	// 
	//////////////////////////////////////////////////////////////////////////
	HTuple xMovement, yMovement;
	if( !checkMaskMovement( ccdImage, &xMovement, &yMovement ) )
	{
		return false;
	}

	*dxMask = xMovement;
	*dyMask = yMovement;


	return true;
}


bool CInspection::checkMaskMovementDic( BYTE *pDicImage, int width, int height, double *dxMask, double *dyMask )
{
	if( !pDicImage || width<1 || height<1 )
	{
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// Make DIC image
	//////////////////////////////////////////////////////////////////////////
	BYTE *pRed = new BYTE[width*height];
	BYTE *pGreen = new BYTE[width*height];
	BYTE *pBlue = new BYTE[width*height];
	for( int y=0 ; y<height ; y++ )
	{
		for( int x=0 ; x<width ; x++ )
		{
			pRed[ x + y*width ] = pDicImage[ 3*x + 2 + y*3*width];
			pGreen[ x + y*width ] = pDicImage[ 3*x + 1 + y*3*width];
			pBlue[ x + y*width ] = pDicImage[ 3*x + 0 + y*3*width];
		}
	}

	HImage dicImage;
	dicImage.GenImage3("byte", width, height, pRed, pGreen, pBlue );

	delete[] pRed;
	delete[] pGreen;
	delete[] pBlue;

	DISPLAY(dicImage);


	//////////////////////////////////////////////////////////////////////////
	// 
	//////////////////////////////////////////////////////////////////////////
	HTuple xMovement, yMovement;
	if( !checkMaskMovementDic( dicImage, &xMovement, &yMovement ) )
	{
		return false;
	}

	*dxMask = xMovement;
	*dyMask = yMovement;


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Slope angle을 측정
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleAtLargeHoleRegion( double *pHeightMap, BYTE *pLasertMap,BYTE *pDicImage, int width, int height, double dDistance, list<double> *listSlopeAngle, list<double> *listStepHeight )
{
	// 조건 점검
	if( !pHeightMap || !pLasertMap || width<1 || height<1 || !listSlopeAngle || !listStepHeight )
	{
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// make height image 변환
	//////////////////////////////////////////////////////////////////////////
	float *pHieght = new float[width*height];
	for( int i=0 ; i<width*height ; i++ )
	{
		pHieght[i] = (float)pHeightMap[i];
	}

	HImage heightImage("real", width, height, pHieght);
	heightImage.WriteImage("hobj", 0, "c:\\test data\\hImage");

	delete[] pHieght;
	DISPLAY(heightImage, "입력 데이터 > 높이 map");


	//////////////////////////////////////////////////////////////////////////
	// Make laser image 변환
	//////////////////////////////////////////////////////////////////////////
	HImage laserImage("byte", width, height, pLasertMap);

	DISPLAY(laserImage, "입력 데이터 > 레이저 영상");


	//////////////////////////////////////////////////////////////////////////
	// Make DIC image
	//////////////////////////////////////////////////////////////////////////
	BYTE *pRed = new BYTE[width*height];
	BYTE *pGreen = new BYTE[width*height];
	BYTE *pBlue = new BYTE[width*height];
	for( int y=0 ; y<height ; y++ )
	{
		for( int x=0 ; x<width ; x++ )
		{
			pRed[ x + y*width ] = pDicImage[ 3*x + 2 + y*3*width];
			pGreen[ x + y*width ] = pDicImage[ 3*x + 1 + y*3*width];
			pBlue[ x + y*width ] = pDicImage[ 3*x + 0 + y*3*width];
		}
	}

	HImage dicImage;
	dicImage.GenImage3("byte", width, height, pRed, pGreen, pBlue );

	delete[] pRed;
	delete[] pGreen;
	delete[] pBlue;

	DISPLAY(dicImage, "입력 데이터 > DIC 영상");


	//////////////////////////////////////////////////////////////////////////
	// Calculate slope angle
	//////////////////////////////////////////////////////////////////////////
	HTuple tAngles;
	HTuple tHeightHoles;
	HTuple tHeightPeaks;
	HTuple peakToPeaks;

	int iShape = m_maskInfo.shape;
	switch( iShape )
	{
	case HOLE_SHAPE_RECTANGLE:	// 사각 형태의 구멍인 경우
		if( !measureSlopeAngleFromPeak( laserImage, heightImage, dicImage, &tAngles, &tHeightHoles, &tHeightPeaks) )
		{
			return false;
		}


	case HOLE_SHAPE_DIAMOND:	// 다이아몬드 형태의 구멍인 경우
		if( !measureLargeHoleDiamond( laserImage, heightImage, height, width, &tAngles, &tHeightHoles, &tHeightPeaks, &peakToPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_CIRCLE:	// 원형 형태의 구멍인 경우
		if( !measureSlopeAngleFromPeak( laserImage, heightImage, dicImage, &tAngles, &tHeightHoles, &tHeightPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_ELIPSE:	// 타원 형태의 구멍인 경우
		if( !measureSlopeAngleGreen( laserImage, heightImage, height, width, &tAngles, &tHeightHoles, &tHeightPeaks, &peakToPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_AUTO_RG:	// LGD E5 auto model(Red & Green)
		if( !measureLargeHoleAutoRG( laserImage, heightImage, height, width, &tAngles, &tHeightHoles, &tHeightPeaks, &peakToPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_AUTO_B:	// LGD E5 auto model(Red & Green)
		if( !measureLargeHoleAutoB( laserImage, heightImage, height, width, &tAngles, &tHeightHoles, &tHeightPeaks, &peakToPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_AUTO:	// LGD E5 auto model(Red & Green)
		if( !measureLargeHoleAuto( laserImage, heightImage, height, width, &tAngles, &tHeightHoles, &tHeightPeaks, &peakToPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_ELIPSE_45:	// 기울어진 타원 : LGIT
		if( !measureSlopeAngleFromPeak( laserImage, heightImage, dicImage, &tAngles, &tHeightHoles, &tHeightPeaks) )
		{
			return false;
		}

		break;
	}


	//
	// 결과 데이터 저장하기
	//
	for( int i=0 ; i<tAngles.Length() ; i++ )
	{
		listSlopeAngle->push_back(tAngles[i]);
	}

	for( int i=0 ; i<tHeightHoles.Length() ; i++ )
	{
		// 3D와 거리센서 연동에 의한 step height 계산
		double dStepHeight1 = m_dTotalDistance - dDistance - tHeightHoles[i];
		dStepHeight1 += m_maskInfo.dStepHeightOffset;
		listStepHeight->push_back( dStepHeight1 );			// 센서 연동 두께 측정

		// Mask 두께 가정에 따른 step height 계산
		double dStepHeight2 = m_maskInfo.dThickness - ( tHeightHoles[i] - tHeightPeaks[i] );
		dStepHeight2 += m_maskInfo.dStepHeightOffset;
		m_listThickness.push_back( dStepHeight2 );	// 두께 가정 방식
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	소공경 면에서 step height 를 측정한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureStepHeightAtSmallHoleRegion( double *pHeightMap, BYTE *pLasertMap, BYTE *pDicImage, int width, int height, list<double> *dStepHeight )
{
	if( !pHeightMap || !pLasertMap || !pDicImage || width<1 || height<1 || !dStepHeight )
	{
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// make height image
	//////////////////////////////////////////////////////////////////////////
	float *pHieght = new float[width*height];
	for( int i=0 ; i<width*height ; i++ )
	{
		pHieght[i] = (float)pHeightMap[i];
	}

	HImage heightImage("real", width, height, pHieght);
	heightImage.WriteImage("hobj", 0, "c:\\test data\\hImage");

	delete[] pHieght;

	DISPLAY(heightImage, "입력영상 > 높이 map");


	//////////////////////////////////////////////////////////////////////////
	// Make laser image
	//////////////////////////////////////////////////////////////////////////
	HImage laserImage("byte", width, height, pLasertMap);

	DISPLAY(laserImage, "입력영상 > Laser 영상");


	//////////////////////////////////////////////////////////////////////////
	// Make DIC image
	//////////////////////////////////////////////////////////////////////////
	BYTE *pRed = new BYTE[width*height];
	BYTE *pGreen = new BYTE[width*height];
	BYTE *pBlue = new BYTE[width*height];
	for( int y=0 ; y<height ; y++ )
	{
		for( int x=0 ; x<width ; x++ )
		{
			pRed[ x + y*width ] = pDicImage[ 3*x + 2 + y*3*width];
			pGreen[ x + y*width ] = pDicImage[ 3*x + 1 + y*3*width];
			pBlue[ x + y*width ] = pDicImage[ 3*x + 0 + y*3*width];
		}
	}

	HImage dicImage;
	dicImage.GenImage3("byte", width, height, pRed, pGreen, pBlue );

	delete[] pRed;
	delete[] pGreen;
	delete[] pBlue;

	DISPLAY(dicImage, "입력영상 > Dic 영상");



	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	HTuple stepHeights;
	if( !measureStepHeightAtSmallHoleRegion(laserImage, heightImage, dicImage, height, width, &stepHeights ) )
	{
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	// * Mask hole 검사
	//
	//////////////////////////////////////////////////////////////////////////
	// 	m_bDisplayImage = true;
	//measureMaskWidth( laserImage );


	//
	// Results
	//
	for( int i=0 ; i<stepHeights.Length() ; i++ )
	{
		dStepHeight->push_back(stepHeights[i]);
	}


	return true;
}


bool CInspection::measureT( double *pHeightMap, int width, int height, double dDistance, double *dThickness )
{
	if( !pHeightMap || width<1 || height<1 )
	{
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// make height image
	//////////////////////////////////////////////////////////////////////////
	float *pHieght = new float[width*height];
	for( int i=0 ; i<width*height ; i++ )
	{
		pHieght[i] = (float)pHeightMap[i];
	}

	HImage heightImage("real", width, height, pHieght);
	delete[] pHieght;

	if( m_bDisplayImage )
	{
		HWindow w1(0,0,1024,768);
		w1.DispImage(heightImage);
		w1.Click();
	}


	//////////////////////////////////////////////////////////////////////////
	// make height image
	//////////////////////////////////////////////////////////////////////////
	int iHalfRange = 50;
	double dSum = 0.0;
	int nSum = 0;
	int cx = width/2;
	int cy = height/2;

	for( int y=max(0, cy-iHalfRange ) ; y<=min( cy+iHalfRange, height-1) ; y++ )
	{
		for( int x=max( cx-iHalfRange, 0 ) ; x<=min( cx+iHalfRange, width-1 ) ; x++ )
		{
			dSum += pHeightMap[ x + y*width ]/1000.0;
			nSum++;
		}
	}
	double dAverage = 1000.0*dSum/nSum;

	*dThickness = m_dTotalDistance - dAverage - dDistance;

	m_list3DdataIslandHeight.push_back(dAverage);
	m_listDistanceData.push_back(dDistance);

	return true;
}


void CInspection::clearResult()
{
	m_list3DdataIslandHeight.clear();				// Island의 3D 측정값
	m_list3DdataHoleEdgeHeight.clear();	// Hole edge의 3D 측정값
	m_list3DdataIslandEdgeDistance.clear();		// Island의 3D 측정값

	m_listPeakToPeakDistance.clear();
// 	m_list3Ddata.clear();
	m_listDistanceData.clear();
	m_listThickness.clear();
	m_list3DdataCal.clear();
	m_listDistanceDataCal.clear();
}


// Step height에 대한 offset
void CInspection::setOffset(double dOffset)
{
// 	m_dThicknessOffset = dOffset;
	m_maskInfo.dStepHeightOffset = dOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// 검사 대상 제품의 두께 설정
//
//////////////////////////////////////////////////////////////////////////
void CInspection::setTargetThickness( double dThickness )
{
	m_maskInfo.dThickness = dThickness;
}

//////////////////////////////////////////////////////////////////////////
//
// 구멍을 찾고 구멍의 에지의 높이와 구멍간 봉우리의 높이를 계산한다.
//
//	1. 관심 구멍 1개를 찾는다.
//	2. 관심 구멍을 기준으로 방향별로 대한 step height와 slope angle을 계산한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleDiamond(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	//
	// 전처리
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");


	//
	//
	// 1. Find hole position
	//
	//
// 	// 1.1 Threshold
// 	HRegion thresholdRegion = laserImage.Threshold( 0, 30 );
// 	// 	HRegion thresholdRegion = smoothimage.Threshold( 0, 30 );
// 
// 	DISPLAY(thresholdRegion, "threshold 30");
// 
// 
// 	// 1.2 Opening : 노이즈 제거
// 	HRegion openRegion = thresholdRegion.OpeningCircle(3.5);
// 
// 	// 	DISPLAY(openRegion);
// 
// 
// 	// 1.3 Closing : 노이즈 제거
// 	HRegion closeRegion = openRegion.ClosingCircle(3.5);
// 
// 	// 	DISPLAY(closeRegion);
// 
// 
// 	// 1.4 Connection
// 	HRegion rgCandidateholes = closeRegion.Connection();
// 
// 	DISPLAY(rgCandidateholes, "Connection");
// 
// 	//
// 	if( rgCandidateholes.CountObj()<1 )
// 	{
// 		return false;
// 	}

	// Island 영역 찾기
	HRegion rgIsland1;
	if( !findIslandRegion_atHeightMap( smoothLaser, smoothHeight, rgIsland1) )
	{
		return false;
	}
	HRegion rgIsland = rgIsland1.FillUp();

	DISPLAY(laserImage, rgIsland, "Island 영역 찾기 > 후보영역");

	// 구멍 영역 찾기
	HRegion rgHoles1;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles1) )
	{
		return false;
	}
	HRegion rgCandidateholes = rgHoles1.FillUp();

	//
	// 1.5 형상정보 기반하여 부적합한 구멍 영역 제거
	//
	HTuple hv_Feature, hv_min, hv_max;
	hv_Feature[0] = "width";
	hv_Feature[1] = "height";
	hv_min[0] = m_maskInfo.iWidth*0.8;
	hv_min[1] = m_maskInfo.iHeight*0.8;
	hv_max[0] = m_maskInfo.iWidth*1.2;
	hv_max[1] = m_maskInfo.iHeight*1.2;

	HRegion rgValidHoles = rgCandidateholes.SelectShape( hv_Feature, "and", hv_min, hv_max );

	if( rgValidHoles.CountObj()<1 )
	{
		return false;
	}
	DISPLAY(rgValidHoles, "유효 구멍");


	//
	// 1.6 ROI 구멍 찾기
	//
	//
	HTuple indexRoiHole;
	findNearRegionIndex( rgValidHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
// 	findNearRegionIndex( rgValidHoles, imageWidth/2, imageHeight/2, &indexRoiHole);
	HRegion rgnRoiHole = rgValidHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgnRoiHole, "구멍 찾기 > ROI 구멍");



	//
	//
	// 2. ROI 구멍을 기준으로 8방향 slope angle와 step heigh를 계산한다.
	//
	//
	HTuple hh;	// Hole edge의 높이
	HTuple hp;	// peak의 높이
	HTuple ptpDist;	// peak 간 거리
	HTuple angDist;	// peak 간 거리

	measureSlopeAngles(	laserImage,		// 레이저 영상
						smoothHeight,	// 높이 영상
						rgnRoiHole,		// 관심 구멍
						rgCandidateholes.DilationCircle(2.5),	// 무효영역(관심구멍 이외 구멍 영역 )
						&hh, 
						&hp, 
						&angDist, 
						&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge와 peak와 높이차
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// 각도 계산 + (라디안 -> 도)

	pAngle->Append( ang );
	pHeightHole->Append( hh );
	pHeightPeak->Append( hp );
	pPeakToPeak->Append(ptpDist_um);


	// save
	for( int i=0 ; i<hp.Length() ; i++)
	{
		m_list3DdataIslandHeight.push_back( (double)(hp[i]) );
	}
	for( int i=0 ; i<hh.Length() ; i++)
	{
		m_list3DdataHoleEdgeHeight.push_back( (double)(hh[i]) );
	}
	for( int i=0 ; i<angDist_um.Length() ; i++)
	{
		m_list3DdataIslandEdgeDistance.push_back( (double)(angDist_um[i]) );
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 입력받은 Hole에 대해서 주변의 slope angle 계산
//
//
//	[out]
//		HTuple *pHeightHole	: 구멍 경계의 높이			=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pHeightPeak	: Peak 의 높이				=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pLateralDist: 구멍 경계와 peak 간 거리	=> 4개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//
//		pAngleDistance : Slope angle를 계산하기 위해 필요한 lateral distance
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngles(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak)
{
	// 이미지 크기
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );


	// Hole의 중심점 추출
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


	// Hole의 경계 영역 추출
	HRegion edgeRegion = holeRegion.DilationCircle(1.5).Difference(holeRegion);


// 	// 검색 각도 : 우상/좌하, 좌상/우하, 좌/우, 상/하
// 	int searchAngle[8] = { 0, 45, 90, 135, 180, 225, 270, 315 };
// 
// 	// 각도별 아일랜드 위치 여부 체크 : 1이면 peak가 있다
// 	int peakPos[8];
// 	if( m_maskInfo.arrangement==RECTANGLE_ARRANGEMENT )
// 	{
// 		for( int i=0 ; i<8 ; i++ )
// 		{
// 			peakPos[i] = i%2;
// 		}
// 	} else {
// 		for( int i=0 ; i<8 ; i++ )
// 		{
// 			peakPos[i] = (i+1)%2;
// 		}
// 	}
	

	// 각도별로 island(peak)영역인지 bridge 영역인지 정보를 설정한다.
	int searchAngle[24];
	int peakPos[24];
	for( int i=0 ; i<24 ; i++ )
	{
		searchAngle[i] = i*15;	// 각도 입력

		// Island 여부 입력( 1이면 island 방향이다. )
		if( m_maskInfo.arrangement==RECTANGLE_ARRANGEMENT )
		{
			if( (searchAngle[i]+45)%90==0 )
			{
				peakPos[i] = 1;
			} else 
			{
				peakPos[i] = 0;
			}
		} else {
			if( searchAngle[i]%90==0 )
			{
				peakPos[i] = 1;
			} else 
			{
				peakPos[i] = 0;
			}
		}
	}


	// 8방향에 대해 hole와 island or bridge의 높이 추출
	for( int i=0 ; i<24 ; i++ )
	{
		//
		// 전처리 : 영상 회전
		// 
		// 영상 회전
		CString strMessage;
		strMessage.Format("회전 %d도 ", searchAngle[i]);

		// 회전 matrix 생성
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// 높이 map 회전
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > 높이값");

		// 높이 map에 대한 ROI 설정 : 회전 후 정보가 없는 영역이 발생하는 부분에 대한 대응방안
		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
		DISPLAY(heightMapRot, strMessage+" > 높이값(roi)");


		// 구멍영역 회전
		HRegion regionHoleRot;
		AffineTransRegion(holeRegion, &regionHoleRot, matRotation, "constant");
		DISPLAY(regionHoleRot, strMessage+" > 구멍위치");

		// 무효영역 회전 ( = 다른 구멍 영역)
		HRegion regionInvalidRot;
		AffineTransRegion(invalidRegion, &regionInvalidRot, matRotation, "constant");
		DISPLAY(regionInvalidRot, strMessage+" > 무효영역");


		// Peak 초기위치 설정
		double targetX, targetY;
		if( m_maskInfo.arrangement==RECTANGLE_ARRANGEMENT )
		{
			if( peakPos[i] )
			{
				targetX = colHoleCenter + m_maskInfo.iPitchX/2*1.414;
				targetY = rowHoleCenter;
			} else {
				targetX = colHoleCenter + m_maskInfo.iPitchX/2;
				targetY = rowHoleCenter;
			}
		} else {
			if( peakPos[i] )
			{
				targetX = colHoleCenter + m_maskInfo.iPitchX/2;
				targetY = rowHoleCenter;
			} else {
				targetX = colHoleCenter + m_maskInfo.iPitchX/4*1.414;
				targetY = rowHoleCenter;
			}
		}


		//
		// Hole edge 찾기
		//
		// 검색범위 설정
		HRegion searchRegion;
		searchRegion.GenRectangle1(targetY-15, targetX-m_maskInfo.iPitchX/2, targetY+15, targetX+m_maskInfo.iPitchX/2 );
		DISPLAY(heightMapRot, searchRegion, strMessage+" > Hole edge 탐색영역");

		// Hole 경계 영역 결정 및 높이값 획득
		HRegion regionRoiHoleEdge = regionHoleRot.DilationCircle(7.5).Difference(regionHoleRot).Intersection(searchRegion);
		DISPLAY(heightMapRot, regionRoiHoleEdge, strMessage+" > Hole edge 탐색결과");

		if( regionRoiHoleEdge.CountObj()==0 )
		{
			continue;
		}

		// Hole edge의 위치값
		HTuple rowRoiHoleEdge, colRoiHoleEdge;
		regionRoiHoleEdge.GetRegionPoints(&rowRoiHoleEdge, &colRoiHoleEdge);

		// Hole edge의 높이값
		HTuple heightRoiHoleEdge = heightMapRot.GetGrayval(rowRoiHoleEdge, colRoiHoleEdge);


		//
		// Peak 찾기
		//
		HTuple rowRoiPeak, colRoiPeak, heightRoiPeak;
		if( peakPos[i] )
		{	// Island 인 경우
			// Peak 영역 결정 및 높이값 획득
			// 검색범위 설정
			HRegion regionSearchPeak( targetY-m_maskInfo.iPitchX/4, targetX-m_maskInfo.iPitchX/3, targetY+m_maskInfo.iPitchX/3, targetX+m_maskInfo.iPitchX/3  );
			HRegion regionSearchPeakReduce = regionSearchPeak.Intersection(roiRegionRot).Difference(regionInvalidRot);	// 탐색범위가 회전된 영상에서 벚어나지 않도록 조정
			DISPLAY(heightMapRot, regionSearchPeakReduce, strMessage+" > 아일랜드 탐색 영역");
			double rowPeak, colPeak, heightPeak;

			findPeakRegion( heightMapRot, regionSearchPeakReduce, 3, 3, &rowPeak, &colPeak, &heightPeak );

			rowRoiPeak = rowPeak;
			colRoiPeak = colPeak;
			heightRoiPeak = heightPeak;
		} 
		else 
		{	// Bridge 인 경우
			// Bridge
			int iRange = 20;
			HTuple vallyHeights;
			for( int j=-iRange ; j<iRange ; j++)
			{
				HRegion regionProjection;
				regionProjection.GenRectangle1( targetY-30, targetX+j, targetY+30, targetX+j );

				//DISPLAY(heightMapRot, regionProjection, "Projection range");

				HTuple row, col, height;
				regionProjection.GetRegionPoints(&row, &col);
				height = heightMapRot.GetGrayval(row, col);

				vallyHeights.Append(height.TupleMean());
			}
			HTuple index = vallyHeights.TupleSortIndex();
			int indexVally = index[0];

			rowRoiPeak = targetY;
			colRoiPeak = targetX-iRange+indexVally;
			heightRoiPeak = vallyHeights[indexVally];
		}

		HRegion regionVally;
		regionVally.GenCircle(rowRoiPeak, colRoiPeak, HTuple(2.5));
		DISPLAY(heightMapRot, regionVally, strMessage+" > Bridge 의 위치");



		// 결과
		pHeightHole->Append(heightRoiHoleEdge.TupleMean());	// 구멍 경계 높이
		pHeightPeak->Append(heightRoiPeak);	// Peak의 높이

		HTuple distanceHolePeak;
		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiPeak, colRoiPeak, &distanceHolePeak );	
		pAngleDistance->Append(distanceHolePeak);	// 구멍경계와 peak의 거리

		pPeakToPeak->Append(HTuple(0));	// 사용 X
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 입력받은 Hole에 대해서 주변의 slope angle 과 주요높이정보(구멍경계, island, bridge )
//
//
//	[in]
//		islandRegion : 대상홀 주변 island 경계 영역
//	[out]
//		HTuple *pHeightHole	: 구멍 경계의 높이			=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pHeightPeak	: Peak 의 높이				=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pLateralDist: 구멍 경계와 peak 간 거리	=> 4개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureOneLargeHoleFullIsland(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& islandRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak)
{
	double dIslandRange = 5.5;	// 구멍주변 영역의 두께 설정

	// 이미지 크기
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );


	// Hole의 중심점 추출
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


// 	// Hole의 경계 영역 추출
// 	HRegion edgeRegion = holeRegion.DilationCircle(1.5).Difference(holeRegion);


// 	int searchAngle[24];
// 	int peakPos[24];
// 	for( int i=0 ; i<24 ; i++ )
// 	{
// 		searchAngle[i] = i*15;	// 각도 입력
// 		peakPos[i] = 1;
// 	}

	int nMeasure = getMeasurePointNum();
	double *pAngle = getMeasurePointAngle();

	// 8방향에 대해 hole와 island or bridge의 높이 추출
	// 	for( int i=0 ; i<8 ; i++ )
// 	for( int i=0 ; i<24 ; i++ )
	for( int i=0 ; i<nMeasure ; i++ )
	{
		//
		// 전처리 : 영상 회전
		// 
		// 영상 회전
		CString strMessage;
// 		strMessage.Format("회전 %d도 ", searchAngle[i]);
		strMessage.Format("회전 %.1f도 ", pAngle[i]);

		// 회전 matrix 생성
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
// 		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );
		HomMat2dRotate(matIdentity, -pAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// 회전한 높이 map 생성
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > 높이값");

		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");
		// 		DISPLAY(roiRegionRot, strMessage+" > roi");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
// 		DISPLAY(heightMapRot, strMessage+" > 높이값(roi)");


		// 회전한 hole region 생성
		HRegion rgHoleRot;
		AffineTransRegion(holeRegion, &rgHoleRot, matRotation, "constant");
		DISPLAY(rgHoleRot, strMessage+" > 검사 구멍 영역");

		// 회전한 island region 생성
		HRegion rgIslandRot;
		AffineTransRegion(islandRegion, &rgIslandRot, matRotation, "constant");
		DISPLAY(rgIslandRot, strMessage+" > Island 영역");


		// Island 초기위치 설정
		HRegion searchRegion;
		double targetX, targetY, dSearchLength;
		if( m_maskInfo.arrangement==RECTANGLE_ARRANGEMENT )
		{
			double dLength = 0;
// 			double theta = searchAngle[i]%90;
			double theta = (int)(pAngle[i])%90;

			if( theta<=45 )
			{
				double dx =(double)(m_maskInfo.iPitchX/2);
				double dy = dx*tan( theta*3.14/180 );
				dLength = sqrt( dx*dx + dy*dy );
			} else {
				double dx = (double)(m_maskInfo.iPitchX/2);
				double dy = dx*tan( (90-theta)*3.14/180 );
				dLength = sqrt( dx*dx + dy*dy );
			}

			targetX = colHoleCenter + dLength;
			targetY = rowHoleCenter;

// 			searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/2, targetY+10, targetX+m_maskInfo.iPitchX/2 );
			searchRegion.GenRectangle1(targetY-10, targetX-dLength, targetY+10, targetX+dLength );
		} else {
			// 일단 원으로 가정하자

			// 45도 방향 길이
			double dLength = sqrt( (float)( (m_maskInfo.iPitchX/2)*(m_maskInfo.iPitchX/2) + (m_maskInfo.iPitchY/2)*(m_maskInfo.iPitchY/2) ) );

			targetX = colHoleCenter + dLength/2;
			targetY = rowHoleCenter;

// 			searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/4, targetY+10, targetX+m_maskInfo.iPitchX/4 );
			searchRegion.GenRectangle1(targetY-10, targetX-dLength/2, targetY+10, targetX+dLength/2 );
		}


		//
		// Hole edge 찾기
		//
		// 검색범위 설정
// 		HRegion searchRegion;
// 		searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/2, targetY+10, targetX+m_maskInfo.iPitchX/2 );
		DISPLAY(heightMapRot, searchRegion, strMessage+" > Hole edge 탐색영역");

		// Hole 경계 영역 결정 및 높이값 획득
		HRegion regionRoiHoleEdge = rgHoleRot.DilationCircle(dIslandRange).Difference(rgHoleRot).Intersection(searchRegion);
		DISPLAY(heightMapRot, regionRoiHoleEdge, strMessage+" > Hole edge 탐색결과");

		if( regionRoiHoleEdge.Area()==0 )
		{
			return false;
		}

		// Hole edge의 위치값
		HTuple rowRoiHoleEdge, colRoiHoleEdge;
		regionRoiHoleEdge.GetRegionPoints(&rowRoiHoleEdge, &colRoiHoleEdge);

		// Hole edge의 높이값
		HTuple tHeightOfHoleEdge = heightMapRot.GetGrayval(rowRoiHoleEdge, colRoiHoleEdge);

 		if( tHeightOfHoleEdge.Length()==0 )
		{
			return false;
		}

		//
		// Island edge 찾기
		//
		// 검색범위 설정
		// Hole 경계 영역 결정 및 높이값 획득
		HRegion rgIslandRoi = rgIslandRot.Intersection(searchRegion).Connection();

		if( rgIslandRoi.Area()==0 )
		{
			return false;
		}

		HTuple indexRoiHole;
		findNearRegionIndex( rgIslandRoi, rgHoleRot.Row(), rgHoleRot.Column(), &indexRoiHole);
		HRegion rgIslandRoi1 = rgIslandRoi.SelectObj(indexRoiHole+1);

		DISPLAY(heightMapRot, rgIslandRoi1, strMessage+" > Island 탐색결과");


		// Island 의 위치값
		HTuple rowRoiIsland, colRoiIsland;
		rgIslandRoi1.GetRegionPoints(&rowRoiIsland, &colRoiIsland);
// 		rgIslandRoi.GetRegionPoints(&rowRoiIsland, &colRoiIsland);

		// Island 의 높이값
		HTuple tHeightOfIsland = heightMapRot.GetGrayval(rowRoiIsland, colRoiIsland);

		if( tHeightOfIsland.Length()==0 )
		{
			return false;
		}


		// 결과
		pHeightHole->Append(tHeightOfHoleEdge.TupleMean());	// 구멍 경계 높이
		pHeightPeak->Append(tHeightOfIsland.TupleMean());	// Peak의 높이

		HTuple tSlopeAnlgeDistance;
		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiIsland.TupleMean(), colRoiIsland.TupleMean(), &tSlopeAnlgeDistance );	
		pAngleDistance->Append(tSlopeAnlgeDistance);	// 구멍경계와 peak의 거리
	}


	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// 입력받은 Hole에 대해서 주변의 slope angle 과 주요높이정보(구멍경계, island, bridge )
//
//
//	[in]
//		islandRegion : 대상홀 주변 island 경계 영역
//	[out]
//		HTuple *pHeightHole	: 구멍 경계의 높이			=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pHeightPeak	: Peak 의 높이				=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pLateralDist: 구멍 경계와 peak 간 거리	=> 4개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureOneLargeHoleHalfIsland(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& islandRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak)
{
	double dIslandRange = 5.5;	// 구멍주변 영역의 두께 설정

	// 이미지 크기
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );

	// Hole의 중심점 추출
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


	// 측정 정보 설정 : 측정 방향(각도), 방향별 island or bridge
/*	int searchAngle[24];
	bool bIsIsland[24];
	for( int i=0 ; i<24 ; i++ )
	{
		searchAngle[i] = i*15;	// 각도 입력
		bIsIsland[i] = true;
// 		if( (searchAngle[i]>45 && searchAngle[i]<135 ) || (searchAngle[i]>225 && searchAngle[i]<315 ) )
// 		{
// 			bIsIsland[i] = false;
// 		}
		if( ( searchAngle[i]>=45-30 && searchAngle[i]<=45+30 ) || 
			( searchAngle[i]>=135-30 && searchAngle[i]<=135+30 ) || 
			( searchAngle[i]>=225-30 && searchAngle[i]<=225+30 ) || 
			( searchAngle[i]>=315-30 && searchAngle[i]<=315+30 ) )
		{
			bIsIsland[i] = false;
		}
	}
*/


	int nMeasureNum = getMeasurePointNum();
	double *pMeasureAnlge = getMeasurePointAngle();
	BOOL *pMeasureIsland = getMeasurePointIsland();


	// 24방향에 대해 hole와 island or bridge의 높이 추출
// 	for( int i=0 ; i<24 ; i++ )
	for( int i=0 ; i<nMeasureNum ; i++ )
	{
		//
		// 전처리 : 영상 회전
		// 
		// 영상 회전
		CString strMessage;
// 		strMessage.Format("회전 %d도 ", searchAngle[i]);
		strMessage.Format("회전 %.1f 도 ", pMeasureAnlge[i]);

		// 회전 matrix 생성
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
// 		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );
		HomMat2dRotate(matIdentity, -pMeasureAnlge[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// 회전한 높이 map 생성
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > 회전한 높이값");

		// 회전 ROI 생성
		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
// 		DISPLAY(heightMapRot, strMessage+" > 높이값(roi)");

		// 회전한 hole region 생성
		HRegion rgHoleRot;
		AffineTransRegion(holeRegion, &rgHoleRot, matRotation, "constant");
		DISPLAY(rgHoleRot, strMessage+" > 회전한 구멍위치");

		// 회전한 island region 생성
		HRegion rgIslandRot;
		AffineTransRegion(islandRegion, &rgIslandRot, matRotation, "constant");
		DISPLAY(rgIslandRot, strMessage+" > 회전한 island 영역");

		// 회전한 무효 region 생성
		HRegion rgInvlaidRot;
		AffineTransRegion(invalidRegion, &rgInvlaidRot, matRotation, "constant");
		DISPLAY(rgInvlaidRot, strMessage+" > 회전한 무효 영역");


		// Island(Bridge)의 예상 위치 계산
		HRegion searchRegion;
		double targetX, targetY;
		if( m_maskInfo.arrangement==RECTANGLE_ARRANGEMENT )
		{
			double dLength = 0;
// 			double theta = searchAngle[i]%90;
			double theta = (int)(pMeasureAnlge[i])%90;

			if( theta<=45 )
			{
				double dx =(double)(m_maskInfo.iPitchX/2);
				double dy = dx*tan( theta*3.14/180 );
				dLength = sqrt( dx*dx + dy*dy );
			} else {
				double dx = (double)(m_maskInfo.iPitchX/2);
				double dy = dx*tan( (90-theta)*3.14/180 );
				dLength = sqrt( dx*dx + dy*dy );
			}

			targetX = colHoleCenter + dLength;
			targetY = rowHoleCenter;

// 			searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/2, targetY+10, targetX+m_maskInfo.iPitchX/2 );
			searchRegion.GenRectangle1(targetY-10, targetX-dLength, targetY+10, targetX+dLength );
		} else {
			// 일단 원으로 가정하자

			// 45도 방향 길이
			double dLength = sqrt( (float)( (m_maskInfo.iPitchX/2)*(m_maskInfo.iPitchX/2) + (m_maskInfo.iPitchY/2)*(m_maskInfo.iPitchY/2) ) );

			targetX = colHoleCenter + dLength/2;
			targetY = rowHoleCenter;

// 			searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/4, targetY+10, targetX+m_maskInfo.iPitchX/4 );
			searchRegion.GenRectangle1(targetY-10, targetX-dLength/2, targetY+10, targetX+dLength/2 );
		}


		//
		// Hole edge 찾기
		//
		// 검색 영역 생성
// 		HRegion searchRegion;
// 		searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/2, targetY+10, targetX+m_maskInfo.iPitchX/2 );
		DISPLAY(heightMapRot, searchRegion, strMessage+" > Hole edge 찾기 > 탐색영역");

		// Hole 경계 영역 결정 및 높이값 획득
		HRegion regionRoiHoleEdge = rgHoleRot.DilationCircle(dIslandRange).Difference(rgHoleRot).Intersection(searchRegion);
		DISPLAY(heightMapRot, regionRoiHoleEdge, strMessage+" > Hole edge 찾기 > 최종 선택");

		if( regionRoiHoleEdge.Area()==0 )
		{
			return false;
		}

		// Hole edge의 위치값 추출
		HTuple rowRoiHoleEdge, colRoiHoleEdge;
		regionRoiHoleEdge.GetRegionPoints(&rowRoiHoleEdge, &colRoiHoleEdge);

		// Hole edge의 높이값 추출
		HTuple tHeightOfHoleEdge = heightMapRot.GetGrayval(rowRoiHoleEdge, colRoiHoleEdge);


		//
		// Island edge 찾기
		//
		HTuple tHeightOfPeak;
		HTuple rowRoiPeak, colRoiPeak;

// 		if( bIsIsland[i] )	
		if( pMeasureIsland[i] )	
		{	// Island 검출일 경우
			// Island 후보 영역을 찾는다.
			HRegion rgIslandsCandidate = rgIslandRot.Intersection(searchRegion).Connection();
			DISPLAY(heightMapRot, rgIslandsCandidate, strMessage+" > Island 찾기 > 후보영역");

			if( rgIslandsCandidate.Area()==0 )
			{
				return false;
			}

			// 후보 영역중 가장 적합한 영역을 선택한다.( 구멍에 가장 가까운 것 )
			HTuple indexRoiHole;
			findNearRegionIndex( rgIslandsCandidate, rowHoleCenter, colHoleCenter, &indexRoiHole);
			HRegion rgIslandRoi = rgIslandsCandidate.SelectObj(indexRoiHole+1);

			DISPLAY(heightMapRot, rgIslandRoi, strMessage+" > Island 찾기 > 최종 선택");


			// Island의 위치/높이값을 추출한다.
			HTuple rowIsland, colIsland;
			rgIslandRoi.GetRegionPoints(&rowIsland, &colIsland);

			// Island 의 높이값
			tHeightOfPeak = heightMapRot.GetGrayval(rowIsland, colIsland);

			rowRoiPeak = rowIsland;
			colRoiPeak = colIsland;
		} 
		else	
		{	// Bridge 검출일 경우

			// 높이를 세로방향으로 projection.
			int iRange = 20;
			HTuple bridgeHeightProjection;
			for( int j=-iRange ; j<iRange ; j++)
			{
				HRegion regionProjection;
				regionProjection.GenRectangle1( targetY-30, targetX+j, targetY+30, targetX+j );
				HRegion rgSearchValid = regionProjection.Difference( rgInvlaidRot.DilationCircle(2.5) );

				//DISPLAY(heightMapRot, regionProjection, "Projection range");

				HTuple row, col, height;
				regionProjection.GetRegionPoints(&row, &col);
				height = heightMapRot.GetGrayval(row, col);

				bridgeHeightProjection.Append(height.TupleMean());
			}

			// Projection으로 부터 bridge를 찾는다.( 가장 높은 지점(3D 데이터로를 가장 작은 값) )
			HTuple index = bridgeHeightProjection.TupleSortIndex();
			int indexVally = index[0];

			// Brige 영역의 위치, 높이값 추출
			HTuple rowRoiBridge = targetY;
			HTuple colRoiBridge = targetX-iRange+indexVally;

			// * 화면 표시
			HRegion regionVally;
			regionVally.GenCircle(rowRoiBridge, colRoiBridge, HTuple(2.5));
			DISPLAY(heightMapRot, regionVally, strMessage+" > bridge 찾기 > 최종 영역");

			rowRoiPeak = rowRoiBridge;
			colRoiPeak = colRoiBridge;
			tHeightOfPeak = heightMapRot.GetGrayval(rowRoiBridge, colRoiBridge);
		}


		if( tHeightOfHoleEdge.Length()==0 || tHeightOfPeak.Length()==0 )
		{
			return false;
		}


		// 결과
		pHeightHole->Append(tHeightOfHoleEdge.TupleMean());	// 구멍 경계부의 높이
		pHeightPeak->Append(tHeightOfPeak.TupleMean());		// Island or bridge 의 높이

		HTuple tSlopeAnlgeDistance;
// 		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiPeak.TupleMean(), colRoiPeak.TupleMean(), &tSlopeAnlgeDistance );	
		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiPeak.TupleMean(), colRoiPeak.TupleMin(), &tSlopeAnlgeDistance );
		pAngleDistance->Append(tSlopeAnlgeDistance);	// 구멍경계와 peak의 거리
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 입력받은 Hole에 대해서 주변의 step height
//
//
//	[out]
//		HTuple *pHeightHole	: 구멍 경계의 높이			=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pHeightPeak	: Peak 의 높이				=> 8개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//		HTuple *pLateralDist: 구멍 경계와 peak 간 거리	=> 4개(우상/좌하, 좌상/우하, 좌/우, 상/하)
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureStepHieghts(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak)
{
	// 이미지 크기
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );


	// Hole의 중심점 추출
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


	// Hole의 경계 영역 추출
	HRegion edgeRegion = holeRegion.DilationCircle(1.5).Difference(holeRegion);


	// 검색 각도 : 우상/좌하, 좌상/우하, 좌/우, 상/하
	int searchAngle[8] = { 0, 45, 90, 135, 180, 225, 270, 315 };

	// 각도별 아일랜드 위치 여부 체크 : 1이면 peak가 있다
	int peakPos[8];
	if( m_maskInfo.arrangement==RECTANGLE_ARRANGEMENT )
	{
		for( int i=0 ; i<8 ; i++ )
		{
			peakPos[i] = i%2;
		}
	} else {
		for( int i=0 ; i<8 ; i++ )
		{
			peakPos[i] = (i+1)%2;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// 8방향에 대해 hole와 vally의 높이 추출
	for( int i=0 ; i<8 ; i++ )
	{
		//
		// (1) 전처리 : 영상 회전
		// 
		// 영상 회전
		CString strMessage;
		strMessage.Format("회전 %d도 ", searchAngle[i]);

		// 회전 matrix 생성
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// 높이 map 회전
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > 높이값");

		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");
// 		DISPLAY(heightMapRotAll, roiRegionRot, strMessage+" > roi");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
		DISPLAY(heightMapRot, strMessage+" > 높이값(roi)");


		//
		HRegion regionHoleRot;
		AffineTransRegion(holeRegion, &regionHoleRot, matRotation, "constant");
		DISPLAY(heightMapRot, regionHoleRot, strMessage+" > 구멍위치");

		//
		HRegion regionInvalidRot;
		AffineTransRegion(invalidRegion, &regionInvalidRot, matRotation, "constant");
		DISPLAY(heightMapRot, regionInvalidRot, strMessage+" > 무효영역");


		//
		// 검색 영역 생성
		HRegion rgnSearchTotal = regionHoleRot.DilationCircle(30).Difference(regionHoleRot);
		DISPLAY(heightMapRot, rgnSearchTotal, strMessage+" > 검색범위(Total)");

		if( rgnSearchTotal.CountObj()==0 ) return false;


		//
		//
		//
		int nHalfRange = 10;
		HTuple rowsHoleEdge, colsHoleEdge, rowsRiv, colsRiv, tHeightsHoleEdge, tHeightsRiv;
		for( int i=-nHalfRange ; i<=nHalfRange ; i++ )
		{
			//
			HTuple tRowsSearch, tColsSearch;
			HRegion rgnSearchLine(rowHoleCenter+i, colHoleCenter, rowHoleCenter+i, colHoleCenter + m_maskInfo.iPitchX );
			HRegion rgnSearch = rgnSearchTotal.Intersection(rgnSearchLine);
			rgnSearch.GetRegionPoints(&tRowsSearch, &tColsSearch);
// 			DISPLAY(rgnSearch, strMessage+" > 검색범위");

			if( rgnSearch.CountObj()==0 ) return false;


			// Hole 주변 높이 검출 : 최저점 검출
			HTuple tHeightsRoi = heightMapRot.GetGrayval(tRowsSearch, tColsSearch);


			//
			HTuple tSortedIndex = tHeightsRoi.TupleSortIndex();
			int iIndexDown = tSortedIndex[tSortedIndex.Length() - 1];

			rowsHoleEdge.Append( tRowsSearch[iIndexDown] );
			colsHoleEdge.Append( tColsSearch[iIndexDown] );
			tHeightsHoleEdge.Append( tHeightsRoi[iIndexDown] );

			
			// Riv 높이 검출 : 꺾이는 부분 검출
			int iPos=0;
			findEdgePoint( tHeightsRoi, &iPos );
			rowsRiv.Append( tRowsSearch[iPos] );
			colsRiv.Append( tColsSearch[iPos] );
			tHeightsRiv.Append( tHeightsRoi[iPos] );

// 			int iIndexUp = tSortedIndex[0];
// 			rowsRiv.Append( tRowsSearch[iIndexUp] );
// 			colsRiv.Append( tColsSearch[iIndexUp] );
// 			tHeightsRiv.Append( tHeightsRoi[iIndexUp] );
		}
		
		HRegion rgnHoleEdge;
		rgnHoleEdge.GenRegionPoints(rowsHoleEdge, colsHoleEdge);
		DISPLAY(heightMapRot, rgnHoleEdge, strMessage+" > 구멍 경계부");

		HRegion rgnRivEdge;
		rgnRivEdge.GenRegionPoints(rowsRiv, colsRiv);
		DISPLAY(heightMapRot, rgnRivEdge, strMessage+" > 리브 경계부");


		pHeightHole->Append(tHeightsHoleEdge.TupleMean());
		pHeightPeak->Append(tHeightsRiv.TupleMean());
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Slope angle을 계산한다.
//
//	1. 구멍 영역 검출
//	2. 검출 구멍을 대상으로 방향별 step height와 slope angle 계산
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleFromPeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights)
{
	int iSize = 4000;	// 이노텍 시료용 임시 파라메터
	double dSizeMargin = 0.2;
	double dGrayDiffLimit = 2;
	bool bValid = false;

	double dIslandRange = 10.5;

	// 구멍의 형태
	int iShape = m_maskInfo.shape;


	// 영상의 크기
	HTuple imageHeight, imageWidth;
	laserImage.GetImageSize( &imageWidth, &imageHeight );


	//
	// 0. 전처리
	//	노이즈 제거
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island 영역을 찾는다.
	//

	// Island 영역 찾기
	HRegion rgIsland1;
	if( !findIslandRegionSmallIsland( smoothLaser, smoothHeight, rgIsland1) )
	{
		return false;
	}
	HRegion rgIsland = rgIsland1.FillUp();

	DISPLAY(laserImage, rgIsland, "Island 영역 찾기 > 후보영역");


	// 유효 island 선택



	//
	// 2. 구멍 영역을 찾는다.
	//

	// 2.1
	HRegion rgHoles1;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles1) )
	{
		return false;
	}
	HRegion rgHoles = rgHoles1.FillUp();

	// 2.2 가운데 구멍 선택
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "구멍 찾기 > 최종 선택 구멍");




	//
	//
	// 2. ROI 구멍을 기준으로 8방향 slope angle와 step heigh를 계산한다.
	//
	//
	HTuple hh;	// Hole edge의 높이
	HTuple hp;	// peak의 높이
	HTuple ptpDist;	// peak 간 거리
	HTuple angDist;	// peak 간 거리

//  	for( int i=0 ; i<holeRegions.CountObj() ; i++ )
  	for( int i=0 ; i<rgHoles.CountObj() ; i++ )
	{
//  		HRegion tmpRegion = holeRegions.SelectObj(i+1);
		HRegion tmpRegion = rgHoles.SelectObj(i+1).DilationCircle(1.5);
 
 		if( tmpRegion.Column() > m_maskInfo.iPitchX && tmpRegion.Column() < imageWidth - m_maskInfo.iPitchX &&
 			tmpRegion.Row() > m_maskInfo.iPitchY && tmpRegion.Row() < imageHeight - m_maskInfo.iPitchY)
		{
			measureSlopeAngles(	laserImage,		// 레이저 영상
								smoothHeight,	// 높이 영상
				//				roiHoleRegion,		// 관심 구멍
 								tmpRegion,
// 								holeRegions.DilationCircle(2.5),	// 무효영역(관심구멍 이외 구멍 영역 )
								rgHoles.DilationCircle(2.5),	// 무효영역(관심구멍 이외 구멍 영역 )
								&hh, 
								&hp, 
								&angDist, 
								&ptpDist );
		}
 	}
 
//  	for( int i=0 ; i<24 ; i++ )
	// 2019.01.09 불요 부분 삭제
// 	for( int i=0 ; i<getMeasurePointNum() ; i++ )
// 	{
//  		hh.Append(0.0);			// Hole edge의 높이
//  		hp.Append(0.0);			// peak의 높이
//  		ptpDist.Append(0.0);	// peak 간 거리
//  		angDist.Append(0.0);	// peak 간 거리
//  	}

	HTuple gap = hh.TupleSub(hp);					// hole edge와 peak와 높이차
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// 각도 계산 + (라디안 -> 도)

	pAngles->Append( ang );
	pHoleHeights->Append( hh );
	pPeakHeights->Append( hp );


	// save
	for( int i=0 ; i<hp.Length() ; i++)
	{
		m_list3DdataIslandHeight.push_back( (double)(hp[i]) );
	}
	for( int i=0 ; i<hh.Length() ; i++)
	{
		m_list3DdataHoleEdgeHeight.push_back( (double)(hh[i]) );
	}
	for( int i=0 ; i<angDist_um.Length() ; i++)
	{
		m_list3DdataIslandEdgeDistance.push_back( (double)(angDist_um[i]) );
	}


	return true;
}
//////////////////////////////////////////////////////////////////////////
//
// Slope angle을 계산한다.
//
//	1. 구멍 영역 검출
//	2. 검출 구멍을 대상으로 방향별 step height와 slope angle 계산
//
//////////////////////////////////////////////////////////////////////////
/*
bool CInspection::measureSlopeAngleFromPeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights)
{
	int iSize = 4000;	// 이노텍 시료용 임시 파라메터
	double dSizeMargin = 0.2;
	double dGrayDiffLimit = 2;
	bool bValid = false;

	// 구멍의 형태
	int iShape = m_maskInfo.shape;

	// 영상의 크기
	HTuple imageHeight, imageWidth;
	laserImage.GetImageSize( &imageWidth, &imageHeight );


	//////////////////////////////////////////////////////////////////////////
	// 0. 전처리 - 노이즈 제거
	//////////////////////////////////////////////////////////////////////////
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored").MeanImage(3,3);
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");


	//////////////////////////////////////////////////////////////////////////
	// 2. Hole edge region 찾기 : Laser + dic 영상 조합
	//////////////////////////////////////////////////////////////////////////
	// 1) threshold for hole
	HRegion holesRegion1 = smoothLaser.Threshold(0,30).OpeningCircle(3.5).ClosingCircle(4.5).DilationCircle(3);

	// 	HRegion holesRegion2 = dicImage.Threshold(0,40).ClosingCircle(4.5).DilationCircle(8);
	HRegion holesRegion2 = dicImage.Threshold(0,40).OpeningCircle(5.5).ErosionCircle(7.5).FillUp();

	HRegion holesRegion  = holesRegion1.Intersection( holesRegion2 );;

	DISPLAY(smoothLaser,holesRegion);

	// 2) 형상 정보를 기준으로 real holes 선택
	HTuple paramShapeHole, paramMinHole, paramMaxHole;

	paramShapeHole[0] = "width";
	paramMinHole[0] = m_maskInfo.iWidth*0.6;
	paramMaxHole[0] = m_maskInfo.iWidth*1.2;

	paramShapeHole[1] = "height";
	paramMinHole[1] = m_maskInfo.iHeight*0.6;
	paramMaxHole[1] = m_maskInfo.iHeight*1.2;

	switch ( iShape )
	{
	case HOLE_SHAPE_RECTANGLE:
		paramShapeHole[2] = "rectangularity";	// 원형 hole의 경우
		paramMinHole[2] = 0.8;
		paramMaxHole[2] = 1;
		break;

	case HOLE_SHAPE_DIAMOND:
		break;

	case HOLE_SHAPE_CIRCLE:
		paramShapeHole[2] = "circularity";	// 원형 hole의 경우
		paramMinHole[2] = 0.5;
		paramMaxHole[2] = 1;
		break;
	}

	HRegion holeRegions = holesRegion.ClosingCircle(4.5).DilationCircle(9.5).Connection().SelectShape(paramShapeHole, "and", paramMinHole, paramMaxHole);
	DISPLAY(smoothLaser,holeRegions, "검사 대상 구멍");



	// find roi hole 1개
	HTuple rowRoiHole, colRoiHole, roiHoleIndex;
	if( !findNearRegionIndex(holeRegions, imageHeight/2, imageWidth/2, &roiHoleIndex ) )
	{
		return false;
	}

	HRegion roiHoleRegion = holeRegions.SelectObj(roiHoleIndex+1);

	DISPLAY(smoothLaser, roiHoleRegion, "검사 대상 구멍");



	//
	//
	// 2. ROI 구멍을 기준으로 8방향 slope angle와 step heigh를 계산한다.
	//
	//
	HTuple hh;	// Hole edge의 높이
	HTuple hp;	// peak의 높이
	HTuple ptpDist;	// peak 간 거리
	HTuple angDist;	// peak 간 거리

 	for( int i=0 ; i<holeRegions.CountObj() ; i++ )
 	{
 		HRegion tmpRegion = holeRegions.SelectObj(i+1);
 
 		if( tmpRegion.Column() > m_maskInfo.iPitchX && tmpRegion.Column() < imageWidth - m_maskInfo.iPitchX &&
 			tmpRegion.Row() > m_maskInfo.iPitchY && tmpRegion.Row() < imageHeight - m_maskInfo.iPitchY)
		{
			measureSlopeAngles(	laserImage,		// 레이저 영상
								smoothHeight,	// 높이 영상
				//				roiHoleRegion,		// 관심 구멍
 								tmpRegion,
								holeRegions.DilationCircle(2.5),	// 무효영역(관심구멍 이외 구멍 영역 )
								&hh, 
								&hp, 
								&angDist, 
								&ptpDist );
		}
 	}
 
//  	for( int i=0 ; i<24 ; i++ )
	for( int i=0 ; i<getMeasurePointNum() ; i++ )
	{
 		hh.Append(0.0);			// Hole edge의 높이
 		hp.Append(0.0);			// peak의 높이
 		ptpDist.Append(0.0);	// peak 간 거리
 		angDist.Append(0.0);	// peak 간 거리
 	}

	HTuple gap = hh.TupleSub(hp);					// hole edge와 peak와 높이차
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// 각도 계산 + (라디안 -> 도)

	pAngles->Append( ang );
	pHoleHeights->Append( hh );
	pPeakHeights->Append( hp );


	// save
	for( int i=0 ; i<hp.Length() ; i++)
	{
		m_list3DdataIslandHeight.push_back( (double)(hp[i]) );
	}
	for( int i=0 ; i<hh.Length() ; i++)
	{
		m_list3DdataHoleEdgeHeight.push_back( (double)(hh[i]) );
	}
	for( int i=0 ; i<angDist_um.Length() ; i++)
	{
		m_list3DdataIslandEdgeDistance.push_back( (double)(angDist_um[i]) );
	}


	return true;
}*/
/*
bool CInspection::measureSlopeAngleFromPeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights)
{
int iSize = 4000;	// 이노텍 시료용 임시 파라메터
double dSizeMargin = 0.2;
double dGrayDiffLimit = 2;
bool bValid = false;

// 구멍의 형태
int iShape = m_maskInfo.shape;

// 영상의 크기
HTuple imageHeight, imageWidth;
laserImage.GetImageSize( &imageWidth, &imageHeight );


//////////////////////////////////////////////////////////////////////////
// 0. 전처리
//////////////////////////////////////////////////////////////////////////
HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored").MeanImage(5,5);
HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");


//////////////////////////////////////////////////////////////////////////
// 1. Peak 영역 찾기
//////////////////////////////////////////////////////////////////////////

// 1) Threshold
HRegion rgnThreshold = smoothLaser.Threshold(80,255);
HRegion rgnOpening = rgnThreshold.OpeningCircle(3.5);

DISPLAY(smoothLaser,rgnOpening);


// 2) Connection
HRegion rgnConnection = rgnOpening.Connection();

DISPLAY(smoothLaser,rgnConnection);


// 3) select peaks
HTuple paramShapePeak, paramMinPeak, paramMaxPeak;

switch ( iShape )
{
case HOLE_SHAPE_RECTANGLE:	// 별 모양으로 생겼음
paramShapePeak[0] = "compactness";
paramMinPeak[0] = 0;
paramMaxPeak[0] = 10;

paramShapePeak[1] = "dist_deviation";
paramMinPeak[1] = 0;
paramMaxPeak[1] = 15;

paramShapePeak[2] = "area";
paramMinPeak[2] = 3000;
paramMaxPeak[2] = 99999;
break;

case HOLE_SHAPE_DIAMOND:
break;

case HOLE_SHAPE_CIRCLE:	// Diamond 모양으로 생겼음
paramShapePeak[0] = "rectangularity";
paramMinPeak[0] = 0.5;
paramMaxPeak[0] = 1;

paramShapePeak[1] = "inner_radius";
paramMinPeak[1] = 15;
paramMaxPeak[1] = 150;
break;
}

HRegion rgnPeaks = rgnConnection.SelectShape(paramShapePeak, "and", paramMinPeak, paramMaxPeak);
DISPLAY(smoothLaser,rgnPeaks);

// 3-1) 예외 점검
if( rgnPeaks.CountObj() < 1 )	
{
return false;
}


// 4) Select most center peak
HTuple rowPeak, colPeak, heightPeak, peakIndex;
if( !findNearRegionIndex(rgnPeaks, imageHeight/2, imageWidth/2, &peakIndex ) )
{
return false;
}

HRegion roiPeakRegion = rgnPeaks.SelectObj(peakIndex+1);

// 5) Peak 의 중심 계산
rowPeak = roiPeakRegion.Row();
colPeak = roiPeakRegion.Column();


// 6) Peak의높이계산
// case 1
/////	heightPeak = smoothHeight.GetGrayval(rowPeak,colPeak);

// case 2
HTuple rowsPeak, colsPeak;
roiPeakRegion.GetRegionPoints( &rowsPeak,  &colsPeak );
HTuple peakHeights = smoothHeight.GetGrayval( rowsPeak, colsPeak );
heightPeak = peakHeights.TupleMean();


// * 표시
HRegion rgnPeak( rowPeak, colPeak, HTuple(3) );
DISPLAY(smoothLaser,rgnPeak);


//////////////////////////////////////////////////////////////////////////
// 2. Hole edge region 찾기
//////////////////////////////////////////////////////////////////////////
// 1) threshold for hole
HRegion holesRegion1 = smoothLaser.Threshold(0,30).OpeningCircle(3.5).ClosingCircle(4.5).DilationCircle(3);
HRegion holesRegion2 = dicImage.Threshold(0,50).ClosingCircle(4.5).DilationCircle(8);

HRegion holesRegion  = holesRegion1.Intersection( holesRegion2 );;

DISPLAY(smoothLaser,holesRegion);

// 2) 형상 정보를 기준으로 real holes 선택
HTuple paramShapeHole, paramMinHole, paramMaxHole;

paramShapeHole[0] = "width";
paramMinHole[0] = m_maskInfo.iWidth*0.8;
paramMaxHole[0] = m_maskInfo.iWidth*1.2;

paramShapeHole[1] = "height";
paramMinHole[1] = m_maskInfo.iHeight*0.8;
paramMaxHole[1] = m_maskInfo.iHeight*1.2;

switch ( iShape )
{
case HOLE_SHAPE_RECTANGLE:
paramShapeHole[2] = "rectangularity";	// 원형 hole의 경우
paramMinHole[2] = 0.8;
paramMaxHole[2] = 1;
break;

case HOLE_SHAPE_DIAMOND:
break;

case HOLE_SHAPE_CIRCLE:
paramShapeHole[2] = "circularity";	// 원형 hole의 경우
paramMinHole[2] = 0.5;
paramMaxHole[2] = 1;
break;
}

HRegion holeRegions = holesRegion.Connection().SelectShape(paramShapeHole, "and", paramMinHole, paramMaxHole);
DISPLAY(smoothLaser,holeRegions);

// 3) hole edge 영역 추출
HRegion holeEdgeRegions = holeRegions.DilationCircle(3).Difference(holeRegions);
DISPLAY(smoothLaser,holeEdgeRegions);



//////////////////////////////////////////////////////////////////////////
// 3. 각 방향에 대한 slope angle 계산
//////////////////////////////////////////////////////////////////////////

// 1) 탐색 방향
double seedAngle[2] = { 45*PI/180, 135*PI/180 };	// 


// 2) 
for( int i=0 ; i<2 ; i++ )
{
HRegion region1, region2;
region1.GenEmptyRegion();
region2.GenEmptyRegion();


// a. hole edge 영역 선택
for( int j=-10 ; j<=10 ; j++ )
{
HRegion searchRegion;
if( m_maskInfo.arrangement==RECTANGLE_ARRANGEMENT )
{
searchRegion.GenRectangle2(	rowPeak, colPeak, 
HTuple(seedAngle[i] + j*PI/180), 
HTuple( 0.5*sqrt( (double)(m_maskInfo.iPitchX*m_maskInfo.iPitchX) + (double)(m_maskInfo.iPitchY*m_maskInfo.iPitchY) ) ), 
HTuple(1) );
} else {
searchRegion.GenRectangle2(	rowPeak, colPeak, 
HTuple(seedAngle[i] + j*PI/180), 
HTuple(0.5*m_maskInfo.iPitchX), 
HTuple(1) );
}
// 			DISPLAY(smoothLaser, searchRegion);

HRegion roiRegions = searchRegion.Intersection(holeEdgeRegions).Connection();

if( roiRegions.CountObj()==2 )
{
region1 = region1.Union2(roiRegions.SelectObj(1));
region2 = region2.Union2(roiRegions.SelectObj(2));
}
}


// b. hole edge 영역의 높이를 이용하여 slope angle을 계산한다.
HTuple rows, cols, heights, dist_x, dist_z;

// b-1. 1번째 영역에서 slope angle 계산
region1.GetRegionPoints(&rows, &cols);
heights = heightImage.GetGrayval(rows, cols);

if( heights.Length()>0 )
{
DistancePp( rowPeak, colPeak, rows.TupleMean(), cols.TupleMean(), &dist_x );	// dx
dist_z = heights.TupleMean() - heightPeak;		// dz

pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// 각도 계산
pHoleHeights->Append( heights.TupleMean() );	// Hole edge의 평균 높이 계산
pPeakHeights->Append( heightPeak );
} else 
{	// 예외사항
pAngles->Append( -1.0 );
pHoleHeights->Append( -1.0 );
pPeakHeights->Append( heightPeak );
}

DISPLAY(smoothLaser, region1);


// b-2. 2번째 영역에서 slope angle 계산
region2.GetRegionPoints(&rows, &cols);
heights = heightImage.GetGrayval(rows, cols);

if( heights.Length()>0 )
{
DistancePp( rowPeak, colPeak, rows.TupleMean(), cols.TupleMean(), &dist_x );	// dx
dist_z = heights.TupleMean() - heightPeak;		// dz

pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// 각도 계산
pHoleHeights->Append( heights.TupleMean() );	// Hole edge의 평균 높이 계산
pPeakHeights->Append( heightPeak );
} else 
{	// 예외사항
pAngles->Append( -1.0 );	
pHoleHeights->Append( -1.0 );
pPeakHeights->Append( heightPeak );
}

DISPLAY(smoothLaser, region2);
}


return true;
}*/


//////////////////////////////////////////////////////////////////////////
//
// Slope angle을 계산한다.
//	- 구멍을 찾고 
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleFromOnePeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights)
{
	double dSizeMargin = 0.2;
	double dGrayDiffLimit = 2;
	bool bValid = false;

	// 구멍의 형태
	int iShape = m_maskInfo.shape;

	// 영상의 크기
	HTuple imageHeight, imageWidth;
	laserImage.GetImageSize( &imageWidth, &imageHeight );



	//////////////////////////////////////////////////////////////////////////
	// 0. 전처리
	//////////////////////////////////////////////////////////////////////////
	// Median + Mean
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored").MeanImage(5,5);

	// Median
	HImage smoothHeight = heightImage.MedianImage("circle", 4, "mirrored");



	//////////////////////////////////////////////////////////////////////////
	//
	// 1. Hole 영역 찾기
	//
	//////////////////////////////////////////////////////////////////////////

	//
	// 1) Threshold
	//
	HRegion rgnThreshold = smoothLaser.Threshold(0,30).OpeningCircle(3.5).ClosingCircle(3.5);

	DISPLAY(smoothLaser,rgnThreshold);


	//
	// 2) Connection
	//
	HRegion rgnConnection = rgnThreshold.Connection();

	// 	DISPLAY(smoothLaser,rgnConnection);


	//
	// 3) select real holes
	//
	HTuple paramHoleShape, paramHoleMin, paramHoleMax;

	switch ( iShape )
	{
	case HOLE_SHAPE_RECTANGLE:	// 별 모양으로 생겼음
		break;

	case HOLE_SHAPE_DIAMOND:
		break;

	case HOLE_SHAPE_CIRCLE:	// Diamond 모양으로 생겼음
		break;

	case HOLE_SHAPE_ELIPSE:	// 번데기 모양으로 생겼음
		paramHoleShape[0] = "anisometry";
		paramHoleMin[0] = 1.6;
		paramHoleMax[0] = 2.0;

		paramHoleShape[1] = "width";
		paramHoleMin[1] = m_maskInfo.iWidth*0.8;
		paramHoleMax[1] = m_maskInfo.iWidth*1.2;

		paramHoleShape[2] = "height";
		paramHoleMin[2] = m_maskInfo.iHeight*0.8;
		paramHoleMax[2] = m_maskInfo.iHeight*1.2;
		break;
	}

	HRegion holeRegions = rgnConnection.SelectShape(paramHoleShape, "and", paramHoleMin, paramHoleMax);
	DISPLAY(smoothLaser,holeRegions);


	//
	// 3-1) 예외 점검
	//
	if( holeRegions.CountObj() < 1 )	
	{
		return false;
	}


	//
	// 4) Select most center hole
	//
	HTuple rowRoiHole, colRoiHole, indexRoiHole;
	if( !findNearRegionIndex(holeRegions, imageHeight/2, imageWidth/2 + m_maskInfo.iWidth/2, &indexRoiHole ) )
	{
		return false;
	}

	HRegion roiHoleRegion = holeRegions.SelectObj(indexRoiHole+1);


	// * 표시
	DISPLAY(smoothLaser,roiHoleRegion);


	//
	// 5) Hole edge region 찾기
	//
	HRegion holeEdgeRegion = roiHoleRegion.DilationCircle(3.5).Difference( roiHoleRegion.DilationCircle(0.5) );

	DISPLAY(smoothLaser,holeEdgeRegion);


	//
	// 3. Peak 영역 구하기
	//
	//	- 좌우상하 별도 계산
	//

	// 3.1 왼쪽 peak 영역 찾기
	HRegion peakSearchRegion;
	HTuple rowSeed = roiHoleRegion.Row();
	HTuple colSeed = roiHoleRegion.Column() - m_maskInfo.iPitchX/2;

	// 검색 영역 생성
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(30), colSeed-HTuple(20), rowSeed+HTuple(30), colSeed+HTuple(20) );
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak 위치 계산
	double rowLeftPeak, colLeftPeak, heightLeftPeak;
	findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowLeftPeak, &colLeftPeak, &heightLeftPeak);


	// 3.2 오른쪽 peak 영역 찾기
	rowSeed = roiHoleRegion.Row();
	colSeed = roiHoleRegion.Column() + m_maskInfo.iPitchX/2;

	// 검색 영역 생성
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(30), colSeed-HTuple(20), rowSeed+HTuple(30), colSeed+HTuple(20) );
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak 위치 계산
	double rowRightPeak, colRightPeak, heightRightPeak;
	findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowRightPeak, &colRightPeak, &heightRightPeak);


	// 3.3 위쪽 peak 영역 찾기
	rowSeed = roiHoleRegion.Row() - m_maskInfo.iPitchY/4;
	colSeed = roiHoleRegion.Column();

	// 검색 영역 생성
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(40), colSeed-HTuple(40), rowSeed+HTuple(40), colSeed+HTuple(40) );
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak 위치/높이 계산
	double rowUpPeak, colUpPeak, heightUpPeak;
	HRegion rgnUpPeak;
	if( !findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowUpPeak, &colUpPeak, &heightUpPeak, rgnUpPeak) )
	{
		return false;
	}
	DISPLAY(smoothLaser,rgnUpPeak);

	// Peak의 edge 계산
	HRegion rgnEdgeOfUpPeak = rgnUpPeak.Difference(rgnUpPeak.ErosionCircle(1.5));

	// *region이 없으면 return false;
	if( rgnEdgeOfUpPeak.CountObj()==0 )
	{
		return false;
	}

	DISPLAY(smoothLaser,rgnEdgeOfUpPeak);

	/*	// Edge : 
	HXLDCont xldsUp = smoothHeight.ReduceDomain(peakSearchRegion).EdgesSubPix("shen_junctions", 1, 1, 3);
	HXLDCont xldUp;
	double nLength = 0;
	int a = xldsUp.CountObj();
	for( int j=0 ; j<xldsUp.CountObj() ; j++ )
	{
	if( xldsUp.SelectObj(j+1).LengthXld()>nLength )
	{
	double b= xldsUp.SelectObj(j+1).LengthXld();
	nLength = xldsUp.SelectObj(j+1).LengthXld();
	xldUp = xldsUp.SelectObj(j+1);
	}
	}
	DISPLAY( smoothHeight, xldUp );*/


	// 3.4 아래쪽 peak 영역 찾기
	rowSeed = roiHoleRegion.Row() + m_maskInfo.iPitchY/4;
	colSeed = roiHoleRegion.Column();

	// 검색 영역 생성
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(40), colSeed-HTuple(40), rowSeed+HTuple(40), colSeed+HTuple(40) );	
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak 위치/높이 계산
	double rowBottomPeak, colBottomPeak, heightBottomPeak;
	HRegion rgnDownPeak;
	if( !findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowBottomPeak, &colBottomPeak, &heightBottomPeak, rgnDownPeak) )
	{
		return false;
	}
	DISPLAY(smoothLaser,rgnDownPeak);

	//
	HRegion rgnEdgeOfDownPeak = rgnDownPeak.Difference(rgnDownPeak.ErosionCircle(1.5));

	// * region이 없으면 return false;
	if( rgnEdgeOfDownPeak.CountObj()==0 )
	{
		return false;
	}

	DISPLAY(smoothLaser,rgnEdgeOfDownPeak);

	/*	// Edge 
	HXLDCont xldsBottom = smoothHeight.ReduceDomain(peakSearchRegion).EdgesSubPix("shen_junctions", 1, 1, 3);
	HXLDCont xldBottom;
	nLength = 0;
	int aa = xldsUp.CountObj();
	for( int j=0 ; j<xldsBottom.CountObj() ; j++ )
	{
	if( xldsBottom.SelectObj(j+1).LengthXld()>nLength )
	{
	double b= xldsBottom.SelectObj(j+1).LengthXld();
	nLength = xldsBottom.SelectObj(j+1).LengthXld();
	xldBottom = xldsBottom.SelectObj(j+1);
	}
	}
	DISPLAY( smoothHeight, xldBottom );*/



	//////////////////////////////////////////////////////////////////////////
	// 3. 각 방향에 대한 slope angle 계산
	//////////////////////////////////////////////////////////////////////////

	// 1) 탐색 방향
	// 	double seedAngle[2] = { 0*PI/180, 90*PI/180 };	// 0, 180, 90, 270 도 


	//
	// 4.  
	// 

	// 상하방향에 대한 angle & height
	{
		// a. Search bar 생성
		HRegion searchBarRegion;

		searchBarRegion.GenRectangle2(	roiHoleRegion.Row(), roiHoleRegion.Column(), 
			HTuple(0),	//HTuple(seedAngle[i]), 
			HTuple( m_maskInfo.iPitchX ), 
			HTuple(10) );

		DISPLAY(smoothLaser,searchBarRegion);


		// c. Hole의 ddge 영역 결정
		HRegion tmpHoleEdgeRegions = searchBarRegion.Intersection(holeEdgeRegion).Connection();

		DISPLAY(smoothLaser, tmpHoleEdgeRegions);

		if( tmpHoleEdgeRegions.CountObj()!=2 )
		{
			return false;
		}


		// 좌
		{
			HTuple rowHole, colHole, heightHole;
			HTuple rowPeak, colPeak, heightPeak;

			// Hole 영역 높이 값 추출
			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(1);
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);

			// Peak 영역 높이 값 추출
			rowPeak		= rowHole;		// 구멍과 동일 위치
			colPeak		= colLeftPeak;	//
			heightPeak	= heightLeftPeak;

			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// 각도 계산
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// 각도 계산

				// 높이 데이터 저장
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge의 평균 높이 값
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge의 평균 높이 값
			} 
			else 
			{	// 예외사항
				pAngles->Append( -1.0 );
				pHoleHeights->Append( -1.0 );
				pPeakHeights->Append( heightPeak.TupleMean() );
			}

		}


		// 우 
		{
			HTuple rowHole, colHole, heightHole;
			HTuple rowPeak, colPeak, heightPeak;

			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(2);

			// Hole 높이 값 추출
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);

			// Peak 높이 값 추출
			rowPeak		= rowHole;		// 구멍과 동일 위치
			colPeak		= colRightPeak;	//
			heightPeak	= heightRightPeak;

			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// 각도 계산
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// 각도 계산

				// 높이 데이터 저장
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge의 평균 높이 값
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge의 평균 높이 값
			} 
			else 
			{	// 예외사항
				pAngles->Append( -1.0 );
				pHoleHeights->Append( -1.0 );
				pPeakHeights->Append( heightPeak.TupleMean() );
			}

		}
	}


	// 상하방향에 대한 angle & height
	{
		// a. Search bar 생성
		HRegion searchBarRegion;

		searchBarRegion.GenRectangle2(	roiHoleRegion.Row(), roiHoleRegion.Column(), 
			HTuple(90*PI/180), //HTuple(seedAngle[i]), 
			HTuple( m_maskInfo.iPitchX ), 
			HTuple(20) );

		DISPLAY(smoothLaser,searchBarRegion);


		// c. Hole의 ddge 영역 결정
		HRegion tmpHoleEdgeRegions = searchBarRegion.Intersection(holeEdgeRegion).Connection();

		DISPLAY(smoothLaser, tmpHoleEdgeRegions);

		if( tmpHoleEdgeRegions.CountObj()!=2 )
		{
			return false;
		}


		// 상
		HTuple rowHole, colHole, heightHole;
		HTuple rowPeak, colPeak, heightPeak;
		{
			// 1) Hole 영역 높이 값 추출
			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(1);
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);


			// 2) Peak 영역 결정
			// 2.1) 관심 edge
			HRegion tmpPeakEdgeRegions = searchBarRegion.Intersection(rgnEdgeOfUpPeak).Connection();
			if( tmpPeakEdgeRegions.CountObj()<1 )
			{
				return false;
			}
			DISPLAY(smoothLaser, tmpPeakEdgeRegions);

			// 2.2) 유효 edge
			HTuple index;
			findNearRegionIndex(tmpPeakEdgeRegions, roiHoleRegion.Row(), roiHoleRegion.Column(), &index );

			HRegion peakRegion = tmpPeakEdgeRegions.SelectObj(index+1);
			peakRegion.GetRegionPoints(&rowPeak, &colPeak);

			DISPLAY(smoothLaser, peakRegion);

			// 2.3) 유효 높이
			HTuple tmpRow, tmpCol;
			searchBarRegion.Intersection(rgnUpPeak.ErosionCircle(1.5)).GetRegionPoints(&tmpRow, &tmpCol);
			heightPeak = smoothHeight.GetGrayval(tmpRow, tmpCol);


			// 3) 계산
			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// 각도 계산
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// 각도 계산

				// 높이 데이터 저장
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge의 평균 높이 값
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge의 평균 높이 값
			} 
			else 
			{	// 예외사항
				pAngles->Append( -1.0 );
				pHoleHeights->Append( -1.0 );
				pPeakHeights->Append( heightPeak.TupleMean() );
			}

		}


		// 하
		{
			// 1) Hole 높이 값 추출
			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(2);
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);


			// b. Peak 영역 결정
			HRegion tmpPeakEdgeRegions = searchBarRegion.Intersection(rgnEdgeOfDownPeak).Connection();

			DISPLAY(smoothLaser, tmpPeakEdgeRegions);

			if( tmpPeakEdgeRegions.CountObj()<1 )
			{
				return false;
			}

			// 2.2) 유효 edge
			HTuple index;
			findNearRegionIndex(tmpPeakEdgeRegions, roiHoleRegion.Row(), roiHoleRegion.Column(), &index );

			HRegion peakRegion = tmpPeakEdgeRegions.SelectObj(index+1);
			peakRegion.GetRegionPoints(&rowPeak, &colPeak);
			// 			heightPeak = smoothHeight.GetGrayval(rowPeak, colPeak);

			DISPLAY(smoothLaser, peakRegion);

			// 2.3) 유효 높이
			HTuple tmpRow, tmpCol;
			searchBarRegion.Intersection(rgnDownPeak.ErosionCircle(1.5)).GetRegionPoints(&tmpRow, &tmpCol);
			heightPeak = smoothHeight.GetGrayval(tmpRow, tmpCol);


			// 3) 
			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// 각도 계산
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// 각도 계산

				// 높이 데이터 저장
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge의 평균 높이 값
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge의 평균 높이 값
			} 
			else 
			{	// 예외사항
				pAngles->Append( -1.0 );
				pHoleHeights->Append( -1.0 );
				pPeakHeights->Append( heightPeak.TupleMean() );
			}

		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 구멍을 찾고 구멍의 에지의 높이와 구멍간 봉우리의 높이를 계산한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleGreen(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	bool bValid = false;


	//
	// 전처리
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");


	//
	//
	// 1. Find hole position
	//
	//
	// 1.1 Threshold
	HRegion thresholdRegion = laserImage.Threshold( 0, 30 );
	// 	HRegion thresholdRegion = smoothimage.Threshold( 0, 30 );

	DISPLAY(thresholdRegion);


	// 1.2 Opening : 노이즈 제거
	HRegion openRegion = thresholdRegion.OpeningCircle(3.5);

	// 	DISPLAY(openRegion);


	// 1.3 Closing : 노이즈 제거
	HRegion closeRegion = openRegion.ClosingCircle(3.5);

	// 	DISPLAY(closeRegion);


	// 1.4 Connection
	HRegion rgCandidateholes = closeRegion.Connection();

	DISPLAY(rgCandidateholes);

	//
	if( rgCandidateholes.CountObj()<1 )
	{
		return false;
	}


	//
	// 1.5 형상정보 기반하여 부적합한 구멍 영역 제거
	//
	HTuple hv_Feature, hv_min, hv_max;
	hv_Feature[0] = "width";
	hv_Feature[1] = "height";
	hv_min[0] = m_maskInfo.iWidth*0.8;
	hv_min[1] = m_maskInfo.iHeight*0.8;
	hv_max[0] = m_maskInfo.iWidth*1.2;
	hv_max[1] = m_maskInfo.iHeight*1.2;

	HRegion rgValidHoles = rgCandidateholes.SelectShape( hv_Feature, "and", hv_min, hv_max );

	if( rgValidHoles.CountObj()<4 )
	{
		return false;
	}


	//
	// 1.6 영상 중심과 거리를 기준으로 정렬하여 가장 가까운 구멍 4개를 찾는다.
	//
	// 구멍의 중심 위치 계산
	HTuple imageWidth, imageHeight;
	laserImage.GetImageSize( &imageWidth, &imageHeight );

	HTuple indexRoiHole;
	findNearRegionIndex(rgValidHoles, imageHeight/2, imageWidth/2, &indexRoiHole );

	HRegion rgRoiHole = rgValidHoles.SelectObj(indexRoiHole+1);
	DISPLAY(rgRoiHole);



	//
	// Hole 한개를 기준으로 slope angle 값 계산
	//
	HTuple hh;	// Hole edge의 높이
	HTuple hp;	// peak의 높이
	HTuple ptpDist;	// peak 간 거리
	HTuple angDist;	// peak 간 거리
	measureSlopeAngles(	laserImage, 
						smoothHeight, 
						rgRoiHole, 
						rgValidHoles,
						&hh, 
						&hp, 
						&angDist, 
						&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge와 peak와 높이차
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// 각도 계산 + (라디안 -> 도)

	pAngle->Append( ang );
	pHeightHole->Append( hh );
	pHeightPeak->Append( hp );
	pPeakToPeak->Append(ptpDist_um);


	// save
	for( int i=0 ; i<hp.Length() ; i++)
	{
		m_list3DdataIslandHeight.push_back( (double)(hp[i]) );
	}
	for( int i=0 ; i<hh.Length() ; i++)
	{
		m_list3DdataHoleEdgeHeight.push_back( (double)(hh[i]) );
	}
	for( int i=0 ; i<angDist_um.Length() ; i++)
	{
		m_list3DdataIslandEdgeDistance.push_back( (double)(angDist_um[i]) );
	}



	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 구멍을 찾고 구멍의 에지의 높이와 구멍간 봉우리의 높이를 계산한다.
//
//	1. Island 외곽선을 찾는다.
//	2. 검사할 구멍을 찾는다.
//	3. 검사항 구멍에 가장 가까운 island 외곽선을 선택한다.
//	4. 관심 구멍을 기준으로 방향별로 대한 step height와 slope angle을 계산한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleAutoRG(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	double dIslandRange = 10.5;	// r검색 범위


	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);


	//
	// 전처리
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island 영역을 찾는다.
	//

	// Island 영역 찾기
	HRegion rgIsland;
	if( !findIslandRegion( smoothLaser, smoothHeight, rgIsland) )
	{
		return false;
	}

	// Island 외곽선 찾기
	HRegion rgEdgeOfIslandCandidate = rgIsland.Difference( rgIsland.ErosionCircle(1.5 )).Connection();

	DISPLAY(laserImage, rgEdgeOfIslandCandidate, "Island 외곽 영역 찾기 > 후보영역");


	// 유효 island 외곽선 선택
	HTuple hv_Feature, hv_min, hv_max;
	hv_Feature[0] = "width";
	hv_Feature[1] = "height";
	hv_min[0] = m_maskInfo.iWidth;
	hv_min[1] = m_maskInfo.iHeight;
	hv_max[0] = imageWidth*10;
	hv_max[1] = imageHeight*10;

	HRegion rgEdgeOfIsland = rgEdgeOfIslandCandidate.SelectShape( hv_Feature, "and", hv_min, hv_max );

	if( rgEdgeOfIsland.Area()<1 )
	{
		return false;
	}


	DISPLAY(rgEdgeOfIsland, "Island 외곽 영역 찾기 > 최종 결정 영역");



	//
	// 2. 구멍 영역을 찾는다.
	//

	// 2.1
	HRegion rgHoles;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles) )
	{
		return false;
	}

	// 2.2 가운데 구멍 선택
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "구멍 찾기 > 최종 선택 구멍");



	//
	// 3. 유효 island edge 선택
	//

	// 관심 구멍에서 가장 가까운 에지 성분 선택
	HTuple indexRoiIsland;
	findNearRegionIndex( rgEdgeOfIsland, rgRoiHole.Row(), rgRoiHole.Column(), &indexRoiIsland);
	HRegion rgRoiEdgeOfIsland = rgEdgeOfIsland.SelectObj(indexRoiIsland+1);
	DISPLAY(rgRoiEdgeOfIsland, "Island 영역 찾기 > island edge > 최종 edge");


	// 에지를 두껍게 만든다.
	//	선택된 에지를 두껍게 만들고 원래 island 영역과 겹치는 부분을 살린다.
	HRegion rgRoiEdgeOfIsland2 = rgIsland.Difference( rgIsland.ErosionCircle(dIslandRange )).Intersection(rgRoiEdgeOfIsland.DilationCircle(dIslandRange)).Connection();

	DISPLAY(rgRoiEdgeOfIsland2, "Island 영역 찾기 > island > Connection");


	//
	if( rgRoiEdgeOfIsland2.CountObj()<1 )
	{
		return false;
	}



	//
	//
	// 2. ROI 구멍을 기준으로 8방향 slope angle와 step heigh를 계산한다.
	//
	//
	HTuple hh;	// Hole edge의 높이
	HTuple hp;	// peak의 높이
	HTuple ptpDist;	// peak 간 거리
	HTuple angDist;	// peak 간 거리

	if( !measureOneLargeHoleFullIsland(	laserImage,		// 레이저 영상
										smoothHeight,	// 높이 영상
										rgRoiHole,	//rgnRoiHole,		// 관심 구멍
										rgRoiEdgeOfIsland2,	//rgValidEdgesOfIsland,	// Island 영역
										&hh, 
										&hp, 
										&angDist, 
										&ptpDist ) )
	{
		return false;
	}

	HTuple gap = hh.TupleSub(hp);					// hole edge와 peak와 높이차
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// 각도 계산 + (라디안 -> 도)

	pAngle->Append( ang );
	pHeightHole->Append( hh );
	pHeightPeak->Append( hp );
	pPeakToPeak->Append(ptpDist_um);


	// save
	for( int i=0 ; i<hp.Length() ; i++)
	{
		m_list3DdataIslandHeight.push_back( (double)(hp[i]) );
	}
	for( int i=0 ; i<hh.Length() ; i++)
	{
		m_list3DdataHoleEdgeHeight.push_back( (double)(hh[i]) );
	}
	for( int i=0 ; i<angDist_um.Length() ; i++)
	{
		m_list3DdataIslandEdgeDistance.push_back( (double)(angDist_um[i]) );
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 구멍을 찾고 구멍의 에지의 높이와 구멍간 봉우리의 높이를 계산한다.
//
//	1. 관심 구멍 1개를 찾는다.
//	2. 관심 구멍을 기준으로 방향별로 대한 step height와 slope angle을 계산한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleAutoB(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	double dIslandRange = 10.5;


	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	//
	// 전처리
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island 영역을 찾는다.
	//

	// Island 영역 찾기
	HRegion rgIsland1;
	if( !findIslandRegion( smoothLaser, smoothHeight, rgIsland1) )
	{
		return false;
	}
	HRegion rgIsland = rgIsland1.FillUp();

	// Island 외곽선 찾기
	HRegion rgEdgeOfIslandCandidate = rgIsland.Difference( rgIsland.ErosionCircle(dIslandRange )).Connection();

	DISPLAY(laserImage, rgEdgeOfIslandCandidate, "Island 외곽 영역 찾기 > 후보영역");


	// 유효 island 외곽선 선택
	HTuple hv_Feature, hv_min, hv_max;
	hv_Feature[0] = "width";
	hv_Feature[1] = "height";
	hv_min[0] = m_maskInfo.iWidth/2;
	hv_min[1] = m_maskInfo.iHeight/2;
	hv_max[0] = imageWidth*2;
	hv_max[1] = imageHeight*2;

	HRegion rgEdgeOfIsland = rgEdgeOfIslandCandidate.SelectShape( hv_Feature, "and", hv_min, hv_max );

	if( rgEdgeOfIsland.Area()<1 )
	{
		return false;
	}


	DISPLAY(rgEdgeOfIsland, "Island 외곽 영역 찾기 > 최종 결정 영역");



	//
	// 2. 구멍 영역을 찾는다.
	//

	// 2.1
	HRegion rgHoles;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles) )
	{
		return false;
	}

	// 2.2 가운데 구멍 선택
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "구멍 찾기 > 최종 선택 구멍");



	//
	//
	// 2. ROI 구멍을 기준으로 8방향 slope angle와 step heigh를 계산한다.
	//
	//
	HTuple hh;	// Hole edge의 높이
	HTuple hp;	// peak의 높이
	HTuple ptpDist;	// peak 간 거리
	HTuple angDist;	// peak 간 거리

	measureOneLargeHoleHalfIsland(	laserImage,		// 레이저 영상
									smoothHeight,	// 높이 영상
									rgRoiHole,		//rgnRoiHole,		// 관심 구멍
									rgEdgeOfIsland,	//rgValindIsland,	// Island 영역
									rgHoles,		// 무효 영역
									&hh, 
									&hp, 
									&angDist, 
									&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge와 peak와 높이차
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// 각도 계산 + (라디안 -> 도)

	pAngle->Append( ang );
	pHeightHole->Append( hh );
	pHeightPeak->Append( hp );
	pPeakToPeak->Append(ptpDist_um);


	// save
	for( int i=0 ; i<hp.Length() ; i++)
	{
		m_list3DdataIslandHeight.push_back( (double)(hp[i]) );
	}
	for( int i=0 ; i<hh.Length() ; i++)
	{
		m_list3DdataHoleEdgeHeight.push_back( (double)(hh[i]) );
	}
	for( int i=0 ; i<angDist_um.Length() ; i++)
	{
		m_list3DdataIslandEdgeDistance.push_back( (double)(angDist_um[i]) );
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 구멍을 찾고 구멍의 에지의 높이와 구멍간 봉우리의 높이를 계산한다.
//
//	1. Island 영역을 찾는다.
//		Laser 영상에서 밝은 부분과 높이 영상에서 높이가 높은 부분의 교집합으로 결정한다.
//
//	2. 구멍 영역을 찾는다.
//		Laser 영상에서 어두운 부분과 높이 영상에서 높이가 낮은 부분의 합집합으로 결정한다.
//
//	3. 구멍에 대해서 각도별로 step height와 slope angle을 계산한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleAuto(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	double dIslandRange = 10.5;


	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);


	//
	// 전처리
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island 영역을 찾는다.
	//

	// Island 영역 찾기
	HRegion rgIsland;
	if( !findIslandRegion( smoothLaser, smoothHeight, rgIsland) )
	{
		return false;
	}

	// Island 외곽선 찾기
	HRegion rgEdgeOfIslandCandidate = rgIsland.Difference( rgIsland.ErosionCircle(dIslandRange )).Connection();

	DISPLAY(laserImage, rgEdgeOfIslandCandidate, "Island 외곽 영역 찾기 > 후보영역");


	// 유효 island 외곽선 선택
	HTuple hv_Feature, hv_min, hv_max;
	hv_Feature[0] = "width";
	hv_Feature[1] = "height";
	hv_min[0] = m_maskInfo.iWidth/2;
	hv_min[1] = m_maskInfo.iHeight/2;
	hv_max[0] = imageWidth*99;
	hv_max[1] = imageHeight*99;

	HRegion rgEdgeOfIsland = rgEdgeOfIslandCandidate.SelectShape( hv_Feature, "and", hv_min, hv_max );

	if( rgEdgeOfIsland.Area()<1 )
	{
		return false;
	}


	DISPLAY(rgEdgeOfIsland, "Island 외곽 영역 찾기 > 최종 결정 영역");



	//
	// 2. 구멍 영역을 찾는다.
	//

	// 2.1
	HRegion rgHoles;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles) )
	{
		return false;
	}

	// 2.2 가운데 구멍 선택
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "구멍 찾기 > 최종 선택 구멍");



	//
	//
	// 2. ROI 구멍을 기준으로 8방향 slope angle와 step heigh를 계산한다.
	//
	//
	HTuple hh;	// Hole edge의 높이
	HTuple hp;	// peak의 높이
	HTuple ptpDist;	// peak 간 거리
	HTuple angDist;	// peak 간 거리

	measureOneLargeHoleHalfIsland(	laserImage,		// 레이저 영상
		smoothHeight,	// 높이 영상
		rgRoiHole,		//rgnRoiHole,		// 관심 구멍
		rgEdgeOfIsland,	//rgValindIsland,	// Island 영역
		rgHoles,		// 무효 영역
		&hh, 
		&hp, 
		&angDist, 
		&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge와 peak와 높이차
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um 단위로 변환
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// 각도 계산 + (라디안 -> 도)

	pAngle->Append( ang );
	pHeightHole->Append( hh );
	pHeightPeak->Append( hp );
	pPeakToPeak->Append(ptpDist_um);


	// save
	for( int i=0 ; i<hp.Length() ; i++)
	{
		m_list3DdataIslandHeight.push_back( (double)(hp[i]) );
	}
	for( int i=0 ; i<hh.Length() ; i++)
	{
		m_list3DdataHoleEdgeHeight.push_back( (double)(hh[i]) );
	}
	for( int i=0 ; i<angDist_um.Length() ; i++)
	{
		m_list3DdataIslandEdgeDistance.push_back( (double)(angDist_um[i]) );
	}


	return true;
}




//////////////////////////////////////////////////////////////////////////
//
// Step height 계산 
//
//	- Threshod DIC image
//	- Find hole of interest
//	- 좌, 우, 상, 하, 우상, 좌하, 좌상, 우하 순서로 step height를 계산한다.( 8개 )
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureStepHeightAtSmallHoleRegion(const HImage& laserImage, const HImage& heightImage, const HImage& dicImage, const HTuple& height, const HTuple& width, HTuple *pStepHeight )
{
	HTuple imageWidth, imageHeight;
	laserImage.GetImageSize( &imageWidth, &imageHeight );


	CString str;

	HImage image(dicImage);
	HImage hHeightMapSmooth = heightImage.MedianImage(HTuple("circle"), 3, HTuple("mirrored"));	// 구멍주위 노이즈 성분 제거용


	//////////////////////////////////////////////////////////////////////////
	//
	// 1. 구멍 영역을 찾는다.
	//
	// 1) hole 영역 검출
// 	HRegion rgnThreshold = laserImage.Threshold(0, 50);
	HRegion rgnThreshold = dicImage.Threshold(0, 50);

	DISPLAY(hHeightMapSmooth, rgnThreshold, "구멍 추출 > Threshold");


	// 2) hole 영역 후보 검출
	HRegion rgnHoles = rgnThreshold.ErosionCircle(3.5);
	rgnHoles = rgnHoles.DilationCircle(3.5);
	rgnHoles = rgnHoles.ClosingCircle(25.5);
	rgnHoles = rgnHoles.DilationCircle(1.5);

	DISPLAY(hHeightMapSmooth, rgnHoles, "구멍 추출 > Threshold > 후처리");


	//	3) connection
	HRegion rgnsHole = rgnHoles.Connection();

	if( rgnsHole.CountObj()<1 )
	{
		return false;
	}


	//	3) connection
	HTuple tRoiHoleIndex;
	findNearRegionIndex(rgnsHole, imageHeight/2, imageWidth/2, &tRoiHoleIndex );
	HRegion rgnHoleROI = rgnsHole.SelectObj(tRoiHoleIndex+1);

	DISPLAY(hHeightMapSmooth, rgnHoleROI, "구멍 추출 > 선택");


	// 선택된 hole이 적합하지 추가확인 필요
	// 면적, 폭, 형태, 위치 등...

	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//
	// 2. Step height를 계산한다.
	//
	HTuple tHeightRiv, tHeightHole;

	measureStepHieghts(	laserImage, 
						hHeightMapSmooth, 
						rgnHoleROI, 
						rgnsHole, 
						&tHeightHole, 
						&tHeightRiv );

	for( int i=0 ; i<min(tHeightHole.Length(), tHeightRiv.Length()) ; i++ )
	{
		pStepHeight->Append( tHeightHole[i] - tHeightRiv[i]);
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 두께 값 계산 
//
//	- 3D data와 거리값 이용
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::calculateThickness( double d3dData, double dDistanceData, double *dResult )
{
	if( !dResult )
	{
		return false;
	}

	*dResult = m_dTotalDistance - d3dData - dDistanceData;

	return true;
}


void CInspection::DISPLAY( const HImage & image, CString strMesage )
{
	if( !m_bDisplayImage )
	{
		return;
	}


	HWindow w(0,0,1024,768);
	w.SetColored(6);
	w.DispImage(image);

	w.WriteString(strMesage);

	w.Click();
}


void CInspection::DISPLAY( const HRegion & region, CString strMesage )
{
	if( !m_bDisplayImage )
	{
		return;
	}


	HWindow w(0,0,1024,768);
	w.SetColored(6);
	w.DispRegion(region);

	w.WriteString(strMesage);

	w.Click();
}


void CInspection::DISPLAY( const HObject & object, CString strMesage )
{
	if( !m_bDisplayImage )
	{
		return;
	}


	HWindow w(0,0,1024,768);
	w.SetColored(6);
	w.DispObj(object);

	w.WriteString(strMesage);

	w.Click();
}

void CInspection::DISPLAY( const HImage & image, HRegion & region, CString strMesage )
{
	if( !m_bDisplayImage )
	{
		return;
	}


	HWindow w(0,0,1024,768);
	w.SetColored(6);
	w.DispImage(image);
	w.DispRegion(region);

	w.WriteString(strMesage);

	w.Click();
}


void CInspection::DISPLAY( const HalconCpp::HImage & image, const HalconCpp::HXLDCont & xld, CString strMesage )
{
	if( !m_bDisplayImage )
	{
		return;
	}


	HWindow w(0,0,1024,768);
	w.SetColored(6);
	w.DispImage(image);
	w.DispXld(xld);

	w.WriteString(strMesage);

	w.Click();
}


//////////////////////////////////////////////////////////////////////////
//
// Pixel 단위의 x pitch 값을 반환한다.
//
//////////////////////////////////////////////////////////////////////////
double CInspection::getPitchXPixel()
{
	return m_maskInfo.iPitchX;
}

//////////////////////////////////////////////////////////////////////////
//
// Pixel 단위의 y pitch 값을 반환한다.
//
//////////////////////////////////////////////////////////////////////////
double CInspection::getPitchYPixel()
{
	return m_maskInfo.iPitchY;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixel값을 um 값으로 바꿔서 반환한다.
//
//////////////////////////////////////////////////////////////////////////
double CInspection::getPixelToMirco(double dPixel)
{
	return dPixel*m_dResolution;
}



//////////////////////////////////////////////////////////////////////////
//
// 영상의 중심에 mask의 구멍이 위치하지 않기 위한 mask 이동량을 계산한다.
//
//	- ccd 영상을 입력받아 사용한다.
//	
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::checkMaskMovement( HImage& ccdImage, HTuple *dxMask, HTuple *dyMask )
{
	// Parameter
	int thresholdHoleRegion = 100;


	//
	// 처리 영상 생성
	//
	// 영상 크기
	Hlong imageWidth, imageHeight;
	ccdImage.GetImageSize(&imageWidth, &imageHeight);

	// Red 성분 이미지 추출
	HImage imageR, imageG, imageB;
	imageB = ccdImage.Decompose3(&imageG, &imageR );

	DISPLAY(imageR);


	//
	// Hole 찾기
	//
	// 영상 노이즈 제거
	HImage imageSmooth = imageR.MedianImage("circle", 5, "mirrored");

	// threshold
	HRegion rgThreshold = imageSmooth.Threshold(0, thresholdHoleRegion);
	DISPLAY(imageR, rgThreshold);

	// Connect region
	HRegion rgCandidates = rgThreshold.Connection();

	//
	HRegion rgHoles = rgCandidates.SelectShape( "area", "and", 10000, 30000 );

	if( rgHoles.CountObj()<1 )
	{
		OutputDebugString("checkMaskMovement : 실패1");
		return false;
	}

	DISPLAY(imageR, rgHoles);


	// 가장 가운데 hole 찾기
	HTuple rowHoles, colHoles;
	rgHoles.AreaCenter( &rowHoles, &colHoles );	// Hole들의 중심 좌표 획득

	double rowImageCenter = imageHeight/2;
	double colImageCenter = imageWidth/2;
	double rowHole=0, colHole=0;
	double minDistance = DBL_MAX;
	bool bValid = false;
	int index=0;
	for( int i=0 ; i<rgHoles.CountObj() ; i++ )
	{
		HTuple tempDistance;
		DistancePp(rowHoles[i], colHoles[i], rowImageCenter, colImageCenter, &tempDistance );

		if( tempDistance<minDistance )
		{
			// 최소거리 갱신
			minDistance = tempDistance;	

			// 가운데 hole 위치 갱신
			rowHole = rowHoles[i];
			colHole = colHoles[i];

			// 
			index = i;
			bValid = true;
		}
	}

	if( !bValid )
	{
		OutputDebugString("checkMaskMovement : 실패2");
		return false;
	}

	DISPLAY(rgHoles.SelectObj(index+1));


	//
	// Mask 이동량 계산하기
	//
	double rowTarget;
	double colTarget;
	double dRow = abs( rowImageCenter - rowHole );
	double dCol = abs( colImageCenter - colHole );


	int iArrage = m_maskInfo.arrangement;
	switch( iArrage )
	{
	case DIAMOND_ARRANGEMENT:
		// 구멍의 목표위치가 상하인지 좌우인지 구분하여 처리한다.
		if( dRow > dCol )	// 상하 hole의 경우
		{
			// Row
			if( rowHole>rowImageCenter )
			{
				rowTarget = rowImageCenter + getPitchYPixel()/2;
			} else {
				rowTarget = rowImageCenter - getPitchYPixel()/2;
			}

			// Col : X좌표는 이미지 중심
			colTarget = colImageCenter;
		} else {	// 좌우 hole의 경우
			// Row : Y 좌표는 이미지 중심
			rowTarget = rowImageCenter;

			// Col
			if( colHole>colImageCenter )
			{
				colTarget = colImageCenter + getPitchXPixel()/2;
			} else {
				colTarget = colImageCenter - getPitchXPixel()/2;
			}

		}

		break;

	case RECTANGLE_ARRANGEMENT:
		// Row
		if( rowHole>rowImageCenter )
		{
			rowTarget = rowImageCenter + getPitchYPixel()/2;
		} else {
			rowTarget = rowImageCenter - getPitchYPixel()/2;
		}

		// Col
		if( colHole>colImageCenter )
		{
			colTarget = colImageCenter + getPitchXPixel()/2;
		} else {
			colTarget = colImageCenter - getPitchXPixel()/2;
		}

		break;

	default:
		OutputDebugString("checkMaskMovement : 실패3");
		return false;
	}


	// 결과값 계산
	*dxMask = getPixelToMirco( colTarget - colHole );
	*dyMask = getPixelToMirco( rowTarget - rowHole );


	//
	CString strMsg;
	strMsg.Format("checkMaskMovement : dx=%.2f pixel, dy=%.2f pixel", colTarget - colHole, rowTarget - rowHole );
	OutputDebugString(strMsg);


	return true;
}



//////////////////////////////////////////////////////////////////////////
//
// 영상의 중심에 mask의 구멍이 위치하지 않기 위한 mask 이동량을 계산한다.
//
//	- DIC 영상을 입력받아 사용한다.
//	
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::checkMaskMovementDic( HImage& dicImage, HTuple *dxMask, HTuple *dyMask )
{
	// Parameter
	int thresholdHoleRegion = 50;	// @이노텍


	int iShape = m_maskInfo.shape;


	//
	// 처리 영상 생성
	//
	// 영상 크기
	Hlong imageWidth, imageHeight;
	dicImage.GetImageSize(&imageWidth, &imageHeight);


	//
	// Hole 찾기
	//
	// 영상 노이즈 제거
	HImage imageSmooth = dicImage.MedianImage("circle", 3, "mirrored");

	// threshold
	HRegion rgThreshold = imageSmooth.Threshold(0, thresholdHoleRegion);
	DISPLAY(dicImage, rgThreshold);

	// erosion
	// 	HRegion rgErosion = rgThreshold.ErosionCircle(15.5);
	HRegion rgErosion = rgThreshold.OpeningCircle(3.5);
	DISPLAY(dicImage, rgErosion);

	// Connect region
	HRegion rgCandidates = rgErosion.Connection();

	//
	HTuple paramShape;
	HTuple paramMin;
	HTuple paramMax;

	switch( iShape )
	{
	case HOLE_SHAPE_RECTANGLE:
	case HOLE_SHAPE_DIAMOND:
		paramShape[0] = "compactness";
		paramShape[1] = "area";
		paramMin[0] = 0;
		paramMin[1] = 3000;
		paramMax[0] = 2;
		paramMax[1] = 999999;
		break;

	case HOLE_SHAPE_CIRCLE:
		paramShape[0] = "circularity";
		paramMin[0] = 0.8;
		paramMax[0] = 1;

		paramShape[1] = "width";
		paramMin[1] = m_maskInfo.iWidth*0.8;
		paramMax[1] = m_maskInfo.iWidth*1.2;

		paramShape[2] = "height";
		paramMin[2] = m_maskInfo.iHeight*0.8;
		paramMax[2] = m_maskInfo.iHeight*1.2;
		break;

	case HOLE_SHAPE_ELIPSE:
		paramShape[0] = "area";
		paramMin[0] = m_maskInfo.iWidth*m_maskInfo.iHeight*0.5;
		paramMax[0] = m_maskInfo.iWidth*m_maskInfo.iHeight;
		break;
	}

	HRegion rgHoles = rgCandidates.SelectShape( paramShape, "and", paramMin, paramMax );

	if( rgHoles.CountObj()<1 )
	{
		OutputDebugString("checkMaskMovement : 실패1");
		return false;
	}

	DISPLAY(dicImage, rgHoles);


	// 가장 가운데 hole 찾기
	double dTargetPosY, dTargetPosX;

	switch( iShape )
	{
	case HOLE_SHAPE_RECTANGLE:
	case HOLE_SHAPE_DIAMOND:
	case HOLE_SHAPE_CIRCLE:
		dTargetPosY = imageHeight/2;
		dTargetPosX = imageWidth/2;
		break;

	case HOLE_SHAPE_ELIPSE:
		dTargetPosY = 768/2;
		dTargetPosX = 670;
		break;
	}

	// 4) Select most center peak
	HTuple roiHoleIndex;
	if( !findNearRegionIndex(rgHoles, dTargetPosY, dTargetPosX, &roiHoleIndex ) )
	{
		return false;
	}

	HRegion roiHoleRegion = rgHoles.SelectObj(roiHoleIndex+1);

	DISPLAY(dicImage, roiHoleRegion);


	// 	HTuple rowHoles, colHoles;
	// 	rgHoles.AreaCenter( &rowHoles, &colHoles );	// Hole들의 중심 좌표 획득
	// 
	// 	double rowImageCenter = imageHeight/2;
	// 	double colImageCenter = imageWidth/2;
	// 	double rowHole=0, colHole=0;
	// 	double minDistance = DBL_MAX;
	// 	bool bValid = false;
	// 	int index=0;
	// 	for( int i=0 ; i<rgHoles.CountObj() ; i++ )
	// 	{
	// 		HTuple tempDistance;
	// 		DistancePp(rowHoles[i], colHoles[i], rowImageCenter, colImageCenter, &tempDistance );
	// 
	// 		if( tempDistance<minDistance )
	// 		{
	// 			// 최소거리 갱신
	// 			minDistance = tempDistance;	
	// 
	// 			// 가운데 hole 위치 갱신
	// 			rowHole = rowHoles[i];
	// 			colHole = colHoles[i];
	// 
	// 			// 
	// 			index = i;
	// 			bValid = true;
	// 		}
	// 	}
	// 
	// 	if( !bValid )
	// 	{
	// 		OutputDebugString("checkMaskMovement : 실패2");
	// 		return false;
	// 	}

	// 	DISPLAY(rgHoles.SelectObj(index+1));


	//
	// Mask 이동량 계산하기
	//
	double rowTarget;
	double colTarget;
	// 	double dRow = abs( rowImageCenter - rowHole );
	// 	double dCol = abs( colImageCenter - colHole );
	double dRow = fabs( dTargetPosY - (double)roiHoleRegion.Row() );
	double dCol = fabs( dTargetPosX - (double)roiHoleRegion.Column() );

	int iArrange = m_maskInfo.arrangement;
	switch( iShape )
	{
	case HOLE_SHAPE_RECTANGLE:
	case HOLE_SHAPE_DIAMOND:
	case HOLE_SHAPE_CIRCLE:
		switch( iArrange )
		{
		case DIAMOND_ARRANGEMENT:
			// 구멍의 목표위치가 상하인지 좌우인지 구분하여 처리한다.
			if( dRow > dCol )	// 상하 hole의 경우
			{
				// 			// Row
				// 			if( rowHole>rowImageCenter )
				// 			{
				// 				rowTarget = rowImageCenter + getPitchYPixel()/2;
				// 			} else {
				// 				rowTarget = rowImageCenter - getPitchYPixel()/2;
				// 			}
				// 
				// 			// Col : X좌표는 이미지 중심
				// 			colTarget = colImageCenter;
				// Row
				if( roiHoleRegion.Row()>dTargetPosY )
				{
					rowTarget = dTargetPosY + getPitchYPixel()/2;
				} else {
					rowTarget = dTargetPosY - getPitchYPixel()/2;
				}

				// Col : X좌표는 이미지 중심
				colTarget = dTargetPosX;
			} else {	// 좌우 hole의 경우
				// 			// Row : Y 좌표는 이미지 중심
				// 			rowTarget = rowImageCenter;
				// 
				// 			// Col
				// 			if( colHole>colImageCenter )
				// 			{
				// 				colTarget = colImageCenter + getPitchXPixel()/2;
				// 			} else {
				// 				colTarget = colImageCenter - getPitchXPixel()/2;
				// 			}

				// Row : Y 좌표는 이미지 중심
				rowTarget = dTargetPosY;

				// Col
				if( roiHoleRegion.Column()>dTargetPosX )
				{
					colTarget = dTargetPosX + getPitchXPixel()/2;
				} else {
					colTarget = dTargetPosX - getPitchXPixel()/2;
				}
			}

			break;

		case RECTANGLE_ARRANGEMENT:
			// 		// Row
			// 		if( rowHole>rowImageCenter )
			// 		{
			// 			rowTarget = rowImageCenter + getPitchYPixel()/2;
			// 		} else {
			// 			rowTarget = rowImageCenter - getPitchYPixel()/2;
			// 		}
			// 
			// 		// Col
			// 		if( colHole>colImageCenter )
			// 		{
			// 			colTarget = colImageCenter + getPitchXPixel()/2;
			// 		} else {
			// 			colTarget = colImageCenter - getPitchXPixel()/2;
			// 		}

			// Row
			if( roiHoleRegion.Row()>dTargetPosY )
			{
				rowTarget = dTargetPosY + getPitchYPixel()/2;
			} else {
				rowTarget = dTargetPosY - getPitchYPixel()/2;
			}

			// Col
			if( roiHoleRegion.Column()>dTargetPosX )
			{
				colTarget = dTargetPosX + getPitchXPixel()/2;
			} else {
				colTarget = dTargetPosX - getPitchXPixel()/2;
			}		
			break;

		default:
			OutputDebugString("checkMaskMovement : 실패3");
			return false;
		}
		break;

	case HOLE_SHAPE_ELIPSE:
		rowTarget = 340;
		colTarget = 670;
		break;

	}


	// 결과값 계산
	*dxMask = getPixelToMirco( colTarget - roiHoleRegion.Column() );
	*dyMask = getPixelToMirco( rowTarget - roiHoleRegion.Row() );


	//
	CString strMsg;
	strMsg.Format("checkMaskMovement : dx=%.2f pixel, dy=%.2f pixel", colTarget - roiHoleRegion.Column(), rowTarget - roiHoleRegion.Row() );
	OutputDebugString(strMsg);


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	Mask 구멍의 width 측정
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureMaskWidth( const HImage& laserImage )
{
	HTuple imageWidth, imageHeight;
	laserImage.GetImageSize(&imageWidth, &imageHeight);


	// Threshold
	HRegion region = laserImage.Threshold(0, 127);


	// Remove noise
	HRegion filterdRegion = region.OpeningCircle(3.5).ClosingCircle(3.5);


	// Blob
	HRegion regions = filterdRegion.Connection();

	DISPLAY(laserImage, regions);


	// Remove false
	HTuple paramShape;
	paramShape[0] = "width";
	paramShape[1] = "height";

	HTuple paramMin;
	paramMin[0] = m_maskInfo.iWidth*0.75;
	paramMin[1] = m_maskInfo.iHeight*0.75;

	HTuple paramMax;
	paramMax[0] = m_maskInfo.iWidth*1.25;
	paramMax[1] = m_maskInfo.iHeight*1.25;

	HRegion holeRegions = regions.SelectShape(paramShape, "and", paramMin, paramMax);

	DISPLAY(laserImage, holeRegions);


	// 중심에 가장 가까운 구멍 1개 선택
	HTuple roiHoleIndex;
	findNearRegionIndex(holeRegions, imageHeight/2, imageWidth/2, &roiHoleIndex );

	HRegion roiHoleRegion = holeRegions.SelectObj(roiHoleIndex+1);
	DISPLAY(laserImage, roiHoleRegion);


	// 구멍의 폭 정보 추출
	double dWidth[4];
	extractHoleInfo(roiHoleRegion, dWidth);
	for( int i=0 ; i<4 ; i++ )
	{
		m_holeInfo.iWidthMask[i]		= (int)(dWidth[i]);
		m_holeInfo.dRealWidthMask[i]	= dWidth[i]*m_dResolution;
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	Mask 구멍의 width 측정
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureHoleWidth( const HImage& laserImage )
{
	HTuple imageWidth, imageHeight;
	laserImage.GetImageSize(&imageWidth, &imageHeight);


	// Threshold
	HRegion region = laserImage.Threshold(0, 50);


	// Remove noise
	HRegion filterdRegion = region.OpeningCircle(3.5).ClosingCircle(3.5);


	// Blob
	HRegion regions = filterdRegion.Connection();

	DISPLAY(laserImage, regions);


	// Remove false
	HTuple paramShape;
	paramShape[0] = "width";
	paramShape[1] = "height";

	HTuple paramMin;
	paramMin[0] = m_maskInfo.iWidth*0.75;
	paramMin[1] = m_maskInfo.iHeight*0.75;

	HTuple paramMax;
	paramMax[0] = m_maskInfo.iWidth*1.25;
	paramMax[1] = m_maskInfo.iHeight*1.25;

	HRegion holeRegions = regions.SelectShape(paramShape, "and", paramMin, paramMax);

	if( holeRegions.CountObj()==0 )
	{
		return false;
	}

	DISPLAY(laserImage, holeRegions);


	// 중심에 가장 가까운 구멍 1개 선택
	HTuple roiHoleIndex;
	findNearRegionIndex(holeRegions, imageHeight/2, imageWidth/2, &roiHoleIndex );

	HRegion roiHoleRegion = holeRegions.SelectObj(roiHoleIndex+1);
	DISPLAY(laserImage, roiHoleRegion);


	// 구멍의 폭 정보 추출
	double dWidth[4];

	extractHoleInfo(roiHoleRegion, dWidth);

	for( int i=0 ; i<4 ; i++ )
	{
		m_holeInfo.iWidthHole[i]		= (int)dWidth[i];
		m_holeInfo.dRealWidthHole[i]	= dWidth[i]*m_dResolution;
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Hole edge 영역의 높이와 위치를 찾는다.
//
//	- 영상들과 hole 영역(한개) 정보를 입력 받는다.
//	- Hole 영역의 edge 영역에서 direction(상/하/좌/우) 방향에서 edge를 찾고, 해당 위치와 3D(높이) 값을 리턴한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findHoleEdge( HImage& laserImage, HImage& heightImage, HImage& dicImage, HRegion& holeRegion, HTuple width, HTuple height, HTuple *result_height, HTuple *result_row, HTuple *result_col, int direction )
{
	// Parameter
	int halfMargin = 10;	// 모서리를 기준으로 2N + 1개의 edge data를 수집한다.


	// 1. Edge 영역을 찾는다.
	// - Morphology를 이용하여 edge 영역 추출
	HRegion rgDilation = holeRegion.DilationCircle(1.5);
	HRegion rgHoleEdge = rgDilation.Difference(holeRegion);

	DISPLAY(laserImage, rgHoleEdge);


	// 2. Edge 중에서 모서리 영역을 찾는다.
	// - 가정 : 마름모 형상의 구멍
	HTuple tpEdgeRows, tpEdgeCols;
	rgHoleEdge.GetRegionPoints( &tpEdgeRows, &tpEdgeCols );

	if( direction==0 )	// 왼쪽 에지
	{
		// 왼쪽 corner point를 찾는다.
		HTuple tpLeftCornerRows;
		HTuple tpLeftCornerCol = tpEdgeCols.TupleMin();	// 제일 왼쪽 x
		HTuple tpLeftCornerIndices = tpEdgeCols.TupleFind(tpLeftCornerCol);	// 
		for( int i=0 ; i<tpLeftCornerIndices.Length() ; i++ )
		{
			tpLeftCornerRows.Append(tpEdgeRows.TupleSelect(tpLeftCornerIndices[i]));
		}
		double tpLeftCornerRow = tpLeftCornerRows.TupleMean();


		// corner point를 기준으로 주변 edge를 추출
		HRegion searchRegion = HRegion( double(tpLeftCornerRow-halfMargin), double(tpLeftCornerCol-halfMargin), double(tpLeftCornerRow+halfMargin), double(tpLeftCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge 영역들의 높이 영역 획득
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge 영역의 평균 높이

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpLeftCornerCol;
		*result_row = tpLeftCornerRow;
	} 
	else if( direction==1 )	// 오른쪽 에지
	{
		// 오른쪽 corner point를 찾는다.
		HTuple tpRightCornerRows;
		HTuple tpRightCornerCol = tpEdgeCols.TupleMax();	// 제일 왼쪽 x
		HTuple tpRightCornerIndices = tpEdgeCols.TupleFind(tpRightCornerCol);	// 
		for( int i=0 ; i<tpRightCornerIndices.Length() ; i++ )
		{
			tpRightCornerRows.Append(tpEdgeRows.TupleSelect(tpRightCornerIndices[i]));
		}
		double tpRightCornerRow = tpRightCornerRows.TupleMean();


		// corner point를 기준으로 주변 edge를 추출
		HRegion searchRegion = HRegion( double(tpRightCornerRow-halfMargin), double(tpRightCornerCol-halfMargin), double(tpRightCornerRow+halfMargin), double(tpRightCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge 영역들의 높이 영역 획득
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge 영역의 평균 높이

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpRightCornerCol;
		*result_row = tpRightCornerRow;
	}
	else if( direction==2 )	// 위쪽 에지
	{
		// 위쪽 corner point를 찾는다.
		HTuple tpTopCornerRow = tpEdgeRows.TupleMin();
		HTuple tpTopCornerCols;	// 제일 위쪽 x
		HTuple tpTopCornerIndices = tpEdgeRows.TupleFind(tpTopCornerRow);	// 
		for( int i=0 ; i<tpTopCornerIndices.Length() ; i++ )
		{
			tpTopCornerCols.Append(tpEdgeCols.TupleSelect(tpTopCornerIndices[i]));
		}
		double tpTopCornerCol = tpTopCornerCols.TupleMean();


		// corner point를 기준으로 주변 edge를 추출
		HRegion searchRegion = HRegion( double(tpTopCornerRow-halfMargin), double(tpTopCornerCol-halfMargin), double(tpTopCornerRow+halfMargin), double(tpTopCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge 영역들의 높이 영역 획득
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge 영역의 평균 높이

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpTopCornerCol;
		*result_row = tpTopCornerRow;
	}
	else if( direction==3 )	// 아래쪽 에지
	{
		// 위쪽 corner point를 찾는다.
		HTuple tpBottomCornerRow = tpEdgeRows.TupleMax();
		HTuple tpBottomCornerCols;	// 제일 위쪽 x
		HTuple tpBottomCornerIndices = tpEdgeRows.TupleFind(tpBottomCornerRow);	// 
		for( int i=0 ; i<tpBottomCornerIndices.Length() ; i++ )
		{
			tpBottomCornerCols.Append(tpEdgeCols.TupleSelect(tpBottomCornerIndices[i]));
		}
		double tpBottomCornerCol = tpBottomCornerCols.TupleMean();


		// corner point를 기준으로 주변 edge를 추출
		HRegion searchRegion = HRegion( double(tpBottomCornerRow-halfMargin), double(tpBottomCornerCol-halfMargin), double(tpBottomCornerRow+halfMargin), double(tpBottomCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge 영역들의 높이 영역 획득
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge 영역의 평균 높이

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpBottomCornerCol;
		*result_row = tpBottomCornerRow;
	}



	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 주어진 ROI 영역내에서 island(bridge) 정보를 찾는다.
//
//	- Island는 높이가 가장 높다라고 가정하고, 높이가 가장 높은 영역을 선택한다.
//	- 
//
// 주어진 영역 내에서 peak(가장 낮은 높이)를 찾고, 해당 높이를 기준으로 threshod 후 가장 큰 blob를 찾는다.
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findPeakRegion( const HImage& heightImage, HRegion& rgRoi, double dLowOffset, double dHighOffset, double *peakPosRow, double *peakPosCol, double *dPeakHeight )
{
	Hlong imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	if( imageWidth<1 || imageHeight<1 )
	{
		return false;
	}


	// 영역내 gray 값 
	double minHeight, maxHeight, rangeHeight;
// 	heightImage.MinMaxGray(rgRoi, 0, &minHeight, &maxHeight, &rangeHeight);

	HImage roiImageSmooth = heightImage.MedianImage("circle", 2, "mirrored");	// 노이즈 제거
	roiImageSmooth.MinMaxGray(rgRoi, 0, &minHeight, &maxHeight, &rangeHeight);


	// ROI 설정
	HImage roiImage = heightImage.ReduceDomain(rgRoi);


	// Peak(가장 높은 위치) 값을 기준으로 threshold
	HRegion vallyRegion = roiImage.Threshold( minHeight - dLowOffset, minHeight + dHighOffset );

	// 	DISPLAY(heightImage, vallyRegion);


	// Blob
	HRegion vallyRegions = vallyRegion.Connection();

	if( vallyRegions.CountObj()<1 )
	{
		return false;
	}

	// 가장 큰 region(peak) 선택
	HTuple vallAreas = vallyRegions.Area();
	HTuple maxIndex = vallAreas.TupleSortIndex();
	HRegion realPeakRegion = vallyRegions.SelectObj( maxIndex[vallAreas.Length()-1] + 1 );

	DISPLAY(heightImage, realPeakRegion);


	// Island 영역의 좌표값 획득
	HTuple realPeakRow, realPeakCol;
	realPeakRegion.GetRegionPoints( &realPeakRow, &realPeakCol );

	// Island의 중심점
	realPeakRegion.AreaCenter(peakPosRow, peakPosCol);

	// Island 영역의 높이값 획득
	HTuple grayValues = heightImage.GetGrayval(realPeakRow, realPeakCol );

	*dPeakHeight = grayValues.TupleMean();

	// 추가 : slope angle 계산용 좌표
	double dCol_IslandEdge = *peakPosCol;		// 구멍중심의 col
	int iRow_IslandEdge = (int)(*peakPosRow);	// 구멍 중심의 row
	for( int i=0 ; i<realPeakRow.Length() ; i++ )	// 모든 구멍 point에 대해서...
	{
		double dColTemp = realPeakCol[i];	// col 좌표
		int iRowTemp = realPeakRow[i];	// row 좌표
		if( iRowTemp == iRow_IslandEdge )	// Row 좌표가 구멍중심과 동일하면...
		{
			if( dColTemp<dCol_IslandEdge )
			{
				dCol_IslandEdge = dColTemp;
			}
		}
	}

	*peakPosCol = dCol_IslandEdge;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// 주어진 영역 내에서 peak(가장 낮은 높이)를 찾고, 해당 높이를 기준으로 threshod 후 가장 큰 blob를 찾는다.
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findPeakRegion( const HImage& heightImage, HRegion& rgRoi, double dLowOffset, double dHighOffset, double *peakPosRow, double *peakPosCol, double *dPeakHeight, HRegion& resultRegion )
{
	Hlong imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	if( imageWidth<1 || imageHeight<1 )
	{
		return false;
	}


	// 영역내 gray 값 
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgRoi, 0, &minHeight, &maxHeight, &rangeHeight);


	// ROI 설정
	HImage roiImage = heightImage.ReduceDomain(rgRoi);


	// Peak(가장 높은 위치) 값을 기준으로 threshold
	HRegion vallyRegion = roiImage.Threshold( minHeight - dLowOffset, minHeight + dHighOffset );

	// 	DISPLAY(heightImage, vallyRegion);


	// Blob
	HRegion vallyRegions = vallyRegion.Connection();

	if( vallyRegions.CountObj()<1 )
	{
		return false;
	}

	// 가장 큰 region(peak) 선택
	HTuple vallAreas = vallyRegions.Area();
	HTuple maxIndex = vallAreas.TupleSortIndex();
	HRegion realPeakRegion = vallyRegions.SelectObj( maxIndex[vallAreas.Length()-1] + 1 );

	// 	DISPLAY(heightImage, realPeakRegion);


	//
	realPeakRegion.AreaCenter(peakPosRow, peakPosCol);



	HTuple realPeakRow, realPeakCol;
	realPeakRegion.GetRegionPoints( &realPeakRow, &realPeakCol );

	HTuple grayValues = heightImage.GetGrayval(realPeakRow, realPeakCol );

	*dPeakHeight = grayValues.TupleMean();


	// 결과 region 생성
	resultRegion = realPeakRegion;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Target 위치에 가장 가까운 영역에 대한 index를 추출한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findNearRegionIndex( HRegion regions, HTuple rowTarget, HTuple colTarget, HTuple *index )
{
	// Region들의 중심좌표 획득
	HTuple rowsRegions, colsRegions;
	regions.AreaCenter( &rowsRegions, &colsRegions );	


	double rowHole=0, colHole=0;
	double minDistance = DBL_MAX;

	bool bValid = false;

	for( int i=0 ; i<regions.CountObj() ; i++ )
	{
		// Target 위치와 거리 계산
		HTuple distance;
		DistancePp(rowsRegions[i], colsRegions[i], rowTarget, colTarget, &distance );


		// 최소 거리여부 확인
		if( distance<minDistance )
		{
			// 최소거리 갱신
			minDistance = distance;	

			// index 갱신
			*index = i;
			bValid = true;
		}
	}


	return bValid;
}



//////////////////////////////////////////////////////////////////////////
//
// 
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findEdgePoint( HTuple tData, int *pPos )
{
	int iLength = tData.Length();

	double *pScore = new double[iLength];
	memset( pScore, 0, sizeof(double));

	double dMaxScore = -1.0;

	for( int i=2 ; i<iLength ; i++ )
	{
		double dAngleLeft = atan2( tData[i-2] - tData[i-1], 0.27 );
		double dAnglleRight = fabs(atan2(tData[i+1] - tData[i], 0.27 ));

		pScore[i] = (PI/2 - dAnglleRight) * dAngleLeft;

		if( pScore[i]>dMaxScore )
		{
			dMaxScore = pScore[i];
			*pPos = i;
		}
	}

	delete[] pScore;

	return true;
}



//////////////////////////////////////////////////////////////////////////
//
// Hole 영역을 찾는다.
//
//	1) Laser 영상에서 구멍을 찾는다.
//		. 구멍은 어둡다.
//
//	2) Height 영상에서 구멍을 찾는다.
//		. 구멍은 island 보다 낮다.
//
//	3) 1)과 2)의 합집합을 구한다.
//		. 구멍 후보점들을 생성한다.
//
//	4) 가장 적합한 구멍을 선택한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findHoleRegion( HImage& laserImage, HImage& heightImage, HRegion& rgnIsland, HRegion& resultRegion)
{
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	HRegion rgAll;
	rgAll.GenRectangle1(HTuple(0), HTuple(0), imageHeight-1, imageWidth -1 );


	//
	// Laser 영상에서 구멍 영역을 찾는다.
	//

	// 1. Threshold : Laser image
	double minLaser, maxLaser, rangeLaser;
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// 밝기값 범위를 얻는다.
	double thresholdLaser = minLaser+10;							// 영상 밝기를 기준으로 threshold값을 결정한다.

	HRegion rgHoleCandidate1 = laserImage.Threshold( 0, thresholdLaser ).OpeningCircle(3.5).ClosingCircle(3.5);

	DISPLAY(laserImage, rgHoleCandidate1, "findHoleRegion > threshold laser image");


// 	// 2. 노이즈 제거
// 	HRegion rgHoleCandidates = rgThresholdHole.OpeningCircle(3.5).ClosingCircle(3.5).Connection();
// 
// 	DISPLAY(laserImage, rgHoleCandidates, "findHoleRegion > 후보 구멍 @ laser image");



	//
	// Height 영상에서 구멍 영역을 찾는다.
	//

	//
	HTuple tpRowIsland, tpColIsland;
	rgnIsland.GetRegionPoints( &tpRowIsland, &tpColIsland );

	HTuple tpHeightIsland = heightImage.GetGrayval(tpRowIsland, tpColIsland);
	double dHeightIsland = tpHeightIsland.TupleMean();

	HRegion rgHoleCandidate2 = heightImage.Threshold(dHeightIsland + m_maskInfo.dThickness, dHeightIsland + 200 );

	
	DISPLAY(laserImage, rgHoleCandidate2, "findHoleRegion > threshold height image");



	//
	//
	//

	//
	HRegion rgHolesCandidate = rgHoleCandidate1.Union2(rgHoleCandidate2).Connection().FillUp();

	DISPLAY(laserImage, rgHolesCandidate, "findHoleRegion > 구멍 후보");


	//
	// 유효 구멍 생성
	//
	HTuple hv_Feature, hv_min, hv_max;

	// 폭 기준 설정
	hv_Feature[0] = "width";
	hv_min[0] = m_maskInfo.iWidth*0.8;
	hv_max[0] = m_maskInfo.iWidth*1.3;

	// 높이 기준 설정
	hv_Feature[1] = "height";
	hv_min[1] = m_maskInfo.iHeight*0.8;
	hv_max[1] = m_maskInfo.iHeight*1.3;

	// 면적 기준 설정
	hv_Feature[2] = "area";
	double dEstimateArea = 0;
	if( m_maskInfo.shape == HOLE_SHAPE_RECTANGLE ||
		m_maskInfo.shape == HOLE_SHAPE_AUTO_RG ||
		m_maskInfo.shape == HOLE_SHAPE_AUTO_B ||
		m_maskInfo.shape == HOLE_SHAPE_AUTO )
	{
		dEstimateArea = m_maskInfo.iWidth * m_maskInfo.iHeight;
	}
	else if( m_maskInfo.shape == HOLE_SHAPE_DIAMOND )
	{
		dEstimateArea = 0.5 * m_maskInfo.iWidth * m_maskInfo.iHeight;
	}
	else if( m_maskInfo.shape == HOLE_SHAPE_CIRCLE )
	{
		dEstimateArea = 3.14 * (0.5*m_maskInfo.iWidth) * (0.5*m_maskInfo.iHeight);
	}
	else if( m_maskInfo.shape == HOLE_SHAPE_ELIPSE || m_maskInfo.shape == HOLE_SHAPE_ELIPSE_45 )
	{
		dEstimateArea = 0.8 * m_maskInfo.iWidth * m_maskInfo.iHeight;
	}
	else {
		dEstimateArea = m_maskInfo.iWidth * m_maskInfo.iHeight;
	}
	hv_min[2] = dEstimateArea*0.8;
	hv_max[2] = dEstimateArea*1.3;

	// 기울기 기준 설정
	//	기울어진 구멍에 대한 예외사항
	if( m_maskInfo.shape == HOLE_SHAPE_ELIPSE_45 )
	{
		hv_Feature[3] = "rect2_phi";
		hv_min[3] = -3.14/2.0 * (3.0/4.0);	// -90도 * (3/4)
		hv_max[3] = -3.14/2.0 * (1.0/4.0);	// -90도 * (1/4)
	}


	// filtering
	resultRegion = rgHolesCandidate.SelectShape( hv_Feature, "and", hv_min, hv_max );

	if( resultRegion.CountObj()<1 )
	{
		return false;
	}


	DISPLAY(resultRegion, "findHoleRegion > 유효한 구멍들");


	// 
// 	HTuple indexRoiHole;
// 	findNearRegionIndex( rgHolesValid, imageHeight/2, imageWidth/2, &indexRoiHole);
// 	resultRegion = rgHolesValid.SelectObj(indexRoiHole+1);
// 
// 	DISPLAY(resultRegion, "findHoleRegion > 최종 선택 구멍");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Island 영역을 찾는다.
//
//	1) Laser image에서 밝은 영역을 찾는다.
//		. Island는 평탄해서 laser 영상에서 가장 밝게 나타난다.
//
//	2) Height image에서 높은 영역을 찾는다.
//		. Island는 시료에서 가장 위치가 높다.
//
//	3) 1)과 2)의 교집합을 island 영역으로 결정한다.
//	
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findIslandRegion( HImage& laserImage, HImage& heightImage, HRegion& resultRegion)
{
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	HRegion rgAll;
	rgAll.GenRectangle1(HTuple(0), HTuple(0), imageHeight-1, imageWidth -1 );


	//////////////////////////////////////////////////////////////////////////
	// 1.1 Threshold : Laser image
	//////////////////////////////////////////////////////////////////////////
	double minLaser, maxLaser, rangeLaser;
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// 밝기값 범위를 얻는다.

	double thresholdLaser = (maxLaser+2*minLaser)/3;							// 영상 밝기를 기준으로 threshold값을 결정한다.
	HRegion rgThresholdLaser = laserImage.Threshold( thresholdLaser, 255 ).ClosingCircle(3.5);	// island 영역을 찾기위해 threshold를 한다.

	DISPLAY(laserImage, rgThresholdLaser, "findIslandRegion > threshold laser image"); 


	//////////////////////////////////////////////////////////////////////////
	// 1.2 Threshold : height image
	//////////////////////////////////////////////////////////////////////////
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgThresholdLaser, 10, &minHeight, &maxHeight, &rangeHeight);	// 높이값 범위 획득

	HRegion rgThresholdHeight = heightImage.Threshold( minHeight-5, minHeight+5 ).ClosingCircle(3.5);	// island 영역을 찾기위해 threshold를 한다.(3D센서값은 높을 수록 작다)

	DISPLAY(laserImage, rgThresholdHeight, "findIslandRegion > threshold height map");


	//////////////////////////////////////////////////////////////////////////
	// 1.3 intersection : Laser & height
	//////////////////////////////////////////////////////////////////////////
	//resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight).FillUp();	// 내부 구멍을 채우면 island가 전부 이어져 있는 경우 구멍을 못땀
	resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight);

	DISPLAY(laserImage, resultRegion, "findIslandRegion > result");

	if( resultRegion.Contlength()<1 )
	{
		return false;
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Island 영역을 찾는다.
//
//	1) Laser image에서 밝은 영역을 찾는다.
//		. Island는 평탄해서 laser 영상에서 가장 밝게 나타난다.
//
//	2) Height image에서 높은 영역을 찾는다.
//		. Island는 시료에서 가장 위치가 높다.
//
//	3) 1)과 2)의 교집합을 island 영역으로 결정한다.
//	
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findIslandRegion_atHeightMap( HImage& laserImage, HImage& heightImage, HRegion& resultRegion)
{
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	HRegion rgAll;
	rgAll.GenRectangle1(HTuple(0), HTuple(0), imageHeight-1, imageWidth -1 );


	//////////////////////////////////////////////////////////////////////////
	// 1.1 Threshold : Laser image
	//////////////////////////////////////////////////////////////////////////
	double minLaser, maxLaser, rangeLaser;
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// 밝기값 범위를 얻는다.

	double thresholdLaser = (maxLaser+2*minLaser)/3;							// 영상 밝기를 기준으로 threshold값을 결정한다.
	HRegion rgThresholdLaser = laserImage.Threshold( thresholdLaser, 255 ).ClosingCircle(3.5);	// island 영역을 찾기위해 threshold를 한다.

	DISPLAY(laserImage, rgThresholdLaser, "findIslandRegion > threshold laser image"); 


	//////////////////////////////////////////////////////////////////////////
	// 1.2 Threshold : height image
	//////////////////////////////////////////////////////////////////////////
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgAll, 0, &minHeight, &maxHeight, &rangeHeight);	// 높이값 범위 획득

	HRegion rgThresholdHeight = heightImage.Threshold( minHeight-5, minHeight+5 ).ClosingCircle(3.5);	// island 영역을 찾기위해 threshold를 한다.(3D센서값은 높을 수록 작다)

	DISPLAY(laserImage, rgThresholdHeight, "findIslandRegion > threshold height map");


	//////////////////////////////////////////////////////////////////////////
	// 1.3 intersection : Laser & height
	//////////////////////////////////////////////////////////////////////////
	//resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight).FillUp();	// 내부 구멍을 채우면 island가 전부 이어져 있는 경우 구멍을 못땀
	resultRegion = rgThresholdHeight;

	DISPLAY(laserImage, resultRegion, "findIslandRegion > result");

	if( resultRegion.Contlength()<1 )
	{
		return false;
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Island 영역을 찾는다.
//
//	1) Laser image에서 밝은 영역을 찾는다.
//		. Island는 평탄해서 laser 영상에서 가장 밝게 나타난다.
//
//	2) Height image에서 높은 영역을 찾는다.
//		. Island는 시료에서 가장 위치가 높다.
//
//	3) 1)과 2)의 교집합을 island 영역으로 결정한다.
//	
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findIslandRegionSmallIsland( HImage& laserImage, HImage& heightImage, HRegion& resultRegion)
{
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	HRegion rgAll;
	rgAll.GenRectangle1(HTuple(0), HTuple(0), imageHeight-1, imageWidth -1 );


	//////////////////////////////////////////////////////////////////////////
	// 1.1 Threshold : Laser image
	//////////////////////////////////////////////////////////////////////////
	double minLaser, maxLaser, rangeLaser;
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// 밝기값 범위를 얻는다.

	double thresholdLaser = (maxLaser+2*minLaser)/3;							// 영상 밝기를 기준으로 threshold값을 결정한다.
	HRegion rgThresholdLaser = laserImage.Threshold( thresholdLaser, 255 ).ClosingCircle(1.5);	// island 영역을 찾기위해 threshold를 한다.

	DISPLAY(laserImage, rgThresholdLaser, "findIslandRegion > threshold laser image"); 


	//////////////////////////////////////////////////////////////////////////
	// 1.2 Threshold : height image
	//////////////////////////////////////////////////////////////////////////
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgThresholdLaser, 5, &minHeight, &maxHeight, &rangeHeight);	// 높이값 범위 획득

	HRegion rgThresholdHeight = heightImage.Threshold( minHeight-30, minHeight+10 ).ClosingCircle(1.5);	// island 영역을 찾기위해 threshold를 한다.(3D센서값은 높을 수록 작다)

	DISPLAY(laserImage, rgThresholdHeight, "findIslandRegion > threshold height map");


	//////////////////////////////////////////////////////////////////////////
	// 1.3 intersection : Laser & height
	//////////////////////////////////////////////////////////////////////////
	//resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight).FillUp();	// 내부 구멍을 채우면 island가 전부 이어져 있는 경우 구멍을 못땀
	resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight);

	DISPLAY(laserImage, resultRegion, "findIslandRegion > result");

	if( resultRegion.Contlength()<1 )
	{
		return false;
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Hole 영역의 정보를 추출하고, 저장한다.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::extractHoleInfo( HRegion holeRegion, double *pWidth )
{
	HTuple rowHole, colHole, radiusHole ;
	holeRegion.SmallestCircle(&rowHole, &colHole, &radiusHole);


	for( int i=0 ; i<4 ; i++ )
	{
		// 검색 region 생성
		HTuple searchAngle = i*PI/4;	// 각도

		HRegion searchRegion;	// 검색 region
		searchRegion.GenRectangle2(rowHole, colHole, searchAngle, radiusHole*HTuple(2), HTuple(1) );

		DISPLAY(holeRegion);
		DISPLAY(searchRegion);


		// ROI
		HRegion roiRegion = searchRegion.Intersection(holeRegion);


		// 시작점과 끝점 추출
		HTuple rowsRoi, colsRoi;
		roiRegion.GetRegionPoints(&rowsRoi, &colsRoi);
		HTuple nRoi = rowsRoi.Length();

		HTuple rowStart = rowsRoi[0];
		HTuple colStart = colsRoi[0];
		HTuple rowEnd = rowsRoi[nRoi-1];
		HTuple colEnd = colsRoi[nRoi-1];


		// 폭 계산
		HTuple distance;
		DistancePp( rowStart, colStart, rowEnd, colEnd, &distance );
		double dDist = distance;

		// 결과 저장
		pWidth[i]		= dDist;
		// 		m_holeInfo.iWidthMask[i]		= dDist;
		// 		m_holeInfo.dRealWidthMask[i]	= dDist*m_dResolution;
	}


	return true;
}