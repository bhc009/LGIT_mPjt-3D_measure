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

	// ����
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
// ���� �� �����ϴ� point ���� ���
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
	// 1. 3D ������ ����
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

	double dAvg3dData = 1000.0*dSum/(double)nData;	// 3D���� ������ ��� ���̰�


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
// Slope angle�� ����
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleAtLargeHoleRegion( double *pHeightMap, BYTE *pLasertMap,BYTE *pDicImage, int width, int height, double dDistance, list<double> *listSlopeAngle, list<double> *listStepHeight )
{
	// ���� ����
	if( !pHeightMap || !pLasertMap || width<1 || height<1 || !listSlopeAngle || !listStepHeight )
	{
		return false;
	}


	//////////////////////////////////////////////////////////////////////////
	// make height image ��ȯ
	//////////////////////////////////////////////////////////////////////////
	float *pHieght = new float[width*height];
	for( int i=0 ; i<width*height ; i++ )
	{
		pHieght[i] = (float)pHeightMap[i];
	}

	HImage heightImage("real", width, height, pHieght);
	heightImage.WriteImage("hobj", 0, "c:\\test data\\hImage");

	delete[] pHieght;
	DISPLAY(heightImage, "�Է� ������ > ���� map");


	//////////////////////////////////////////////////////////////////////////
	// Make laser image ��ȯ
	//////////////////////////////////////////////////////////////////////////
	HImage laserImage("byte", width, height, pLasertMap);

	DISPLAY(laserImage, "�Է� ������ > ������ ����");


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

	DISPLAY(dicImage, "�Է� ������ > DIC ����");


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
	case HOLE_SHAPE_RECTANGLE:	// �簢 ������ ������ ���
		if( !measureSlopeAngleFromPeak( laserImage, heightImage, dicImage, &tAngles, &tHeightHoles, &tHeightPeaks) )
		{
			return false;
		}


	case HOLE_SHAPE_DIAMOND:	// ���̾Ƹ�� ������ ������ ���
		if( !measureLargeHoleDiamond( laserImage, heightImage, height, width, &tAngles, &tHeightHoles, &tHeightPeaks, &peakToPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_CIRCLE:	// ���� ������ ������ ���
		if( !measureSlopeAngleFromPeak( laserImage, heightImage, dicImage, &tAngles, &tHeightHoles, &tHeightPeaks) )
		{
			return false;
		}

		break;

	case HOLE_SHAPE_ELIPSE:	// Ÿ�� ������ ������ ���
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

	case HOLE_SHAPE_ELIPSE_45:	// ������ Ÿ�� : LGIT
		if( !measureSlopeAngleFromPeak( laserImage, heightImage, dicImage, &tAngles, &tHeightHoles, &tHeightPeaks) )
		{
			return false;
		}

		break;
	}


	//
	// ��� ������ �����ϱ�
	//
	for( int i=0 ; i<tAngles.Length() ; i++ )
	{
		listSlopeAngle->push_back(tAngles[i]);
	}

	for( int i=0 ; i<tHeightHoles.Length() ; i++ )
	{
		// 3D�� �Ÿ����� ������ ���� step height ���
		double dStepHeight1 = m_dTotalDistance - dDistance - tHeightHoles[i];
		dStepHeight1 += m_maskInfo.dStepHeightOffset;
		listStepHeight->push_back( dStepHeight1 );			// ���� ���� �β� ����

		// Mask �β� ������ ���� step height ���
		double dStepHeight2 = m_maskInfo.dThickness - ( tHeightHoles[i] - tHeightPeaks[i] );
		dStepHeight2 += m_maskInfo.dStepHeightOffset;
		m_listThickness.push_back( dStepHeight2 );	// �β� ���� ���
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//	�Ұ��� �鿡�� step height �� �����Ѵ�.
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

	DISPLAY(heightImage, "�Է¿��� > ���� map");


	//////////////////////////////////////////////////////////////////////////
	// Make laser image
	//////////////////////////////////////////////////////////////////////////
	HImage laserImage("byte", width, height, pLasertMap);

	DISPLAY(laserImage, "�Է¿��� > Laser ����");


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

	DISPLAY(dicImage, "�Է¿��� > Dic ����");



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
	// * Mask hole �˻�
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
	m_list3DdataIslandHeight.clear();				// Island�� 3D ������
	m_list3DdataHoleEdgeHeight.clear();	// Hole edge�� 3D ������
	m_list3DdataIslandEdgeDistance.clear();		// Island�� 3D ������

	m_listPeakToPeakDistance.clear();
// 	m_list3Ddata.clear();
	m_listDistanceData.clear();
	m_listThickness.clear();
	m_list3DdataCal.clear();
	m_listDistanceDataCal.clear();
}


// Step height�� ���� offset
void CInspection::setOffset(double dOffset)
{
// 	m_dThicknessOffset = dOffset;
	m_maskInfo.dStepHeightOffset = dOffset;
}


//////////////////////////////////////////////////////////////////////////
//
// �˻� ��� ��ǰ�� �β� ����
//
//////////////////////////////////////////////////////////////////////////
void CInspection::setTargetThickness( double dThickness )
{
	m_maskInfo.dThickness = dThickness;
}

//////////////////////////////////////////////////////////////////////////
//
// ������ ã�� ������ ������ ���̿� ���۰� ���츮�� ���̸� ����Ѵ�.
//
//	1. ���� ���� 1���� ã�´�.
//	2. ���� ������ �������� ���⺰�� ���� step height�� slope angle�� ����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleDiamond(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	//
	// ��ó��
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
// 	// 1.2 Opening : ������ ����
// 	HRegion openRegion = thresholdRegion.OpeningCircle(3.5);
// 
// 	// 	DISPLAY(openRegion);
// 
// 
// 	// 1.3 Closing : ������ ����
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

	// Island ���� ã��
	HRegion rgIsland1;
	if( !findIslandRegion_atHeightMap( smoothLaser, smoothHeight, rgIsland1) )
	{
		return false;
	}
	HRegion rgIsland = rgIsland1.FillUp();

	DISPLAY(laserImage, rgIsland, "Island ���� ã�� > �ĺ�����");

	// ���� ���� ã��
	HRegion rgHoles1;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles1) )
	{
		return false;
	}
	HRegion rgCandidateholes = rgHoles1.FillUp();

	//
	// 1.5 �������� ����Ͽ� �������� ���� ���� ����
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
	DISPLAY(rgValidHoles, "��ȿ ����");


	//
	// 1.6 ROI ���� ã��
	//
	//
	HTuple indexRoiHole;
	findNearRegionIndex( rgValidHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
// 	findNearRegionIndex( rgValidHoles, imageWidth/2, imageHeight/2, &indexRoiHole);
	HRegion rgnRoiHole = rgValidHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgnRoiHole, "���� ã�� > ROI ����");



	//
	//
	// 2. ROI ������ �������� 8���� slope angle�� step heigh�� ����Ѵ�.
	//
	//
	HTuple hh;	// Hole edge�� ����
	HTuple hp;	// peak�� ����
	HTuple ptpDist;	// peak �� �Ÿ�
	HTuple angDist;	// peak �� �Ÿ�

	measureSlopeAngles(	laserImage,		// ������ ����
						smoothHeight,	// ���� ����
						rgnRoiHole,		// ���� ����
						rgCandidateholes.DilationCircle(2.5),	// ��ȿ����(���ɱ��� �̿� ���� ���� )
						&hh, 
						&hp, 
						&angDist, 
						&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge�� peak�� ������
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// ���� ��� + (���� -> ��)

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
// �Է¹��� Hole�� ���ؼ� �ֺ��� slope angle ���
//
//
//	[out]
//		HTuple *pHeightHole	: ���� ����� ����			=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pHeightPeak	: Peak �� ����				=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pLateralDist: ���� ���� peak �� �Ÿ�	=> 4��(���/����, �»�/����, ��/��, ��/��)
//
//		pAngleDistance : Slope angle�� ����ϱ� ���� �ʿ��� lateral distance
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngles(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak)
{
	// �̹��� ũ��
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );


	// Hole�� �߽��� ����
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


	// Hole�� ��� ���� ����
	HRegion edgeRegion = holeRegion.DilationCircle(1.5).Difference(holeRegion);


// 	// �˻� ���� : ���/����, �»�/����, ��/��, ��/��
// 	int searchAngle[8] = { 0, 45, 90, 135, 180, 225, 270, 315 };
// 
// 	// ������ ���Ϸ��� ��ġ ���� üũ : 1�̸� peak�� �ִ�
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
	

	// �������� island(peak)�������� bridge �������� ������ �����Ѵ�.
	int searchAngle[24];
	int peakPos[24];
	for( int i=0 ; i<24 ; i++ )
	{
		searchAngle[i] = i*15;	// ���� �Է�

		// Island ���� �Է�( 1�̸� island �����̴�. )
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


	// 8���⿡ ���� hole�� island or bridge�� ���� ����
	for( int i=0 ; i<24 ; i++ )
	{
		//
		// ��ó�� : ���� ȸ��
		// 
		// ���� ȸ��
		CString strMessage;
		strMessage.Format("ȸ�� %d�� ", searchAngle[i]);

		// ȸ�� matrix ����
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// ���� map ȸ��
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > ���̰�");

		// ���� map�� ���� ROI ���� : ȸ�� �� ������ ���� ������ �߻��ϴ� �κп� ���� �������
		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
		DISPLAY(heightMapRot, strMessage+" > ���̰�(roi)");


		// ���ۿ��� ȸ��
		HRegion regionHoleRot;
		AffineTransRegion(holeRegion, &regionHoleRot, matRotation, "constant");
		DISPLAY(regionHoleRot, strMessage+" > ������ġ");

		// ��ȿ���� ȸ�� ( = �ٸ� ���� ����)
		HRegion regionInvalidRot;
		AffineTransRegion(invalidRegion, &regionInvalidRot, matRotation, "constant");
		DISPLAY(regionInvalidRot, strMessage+" > ��ȿ����");


		// Peak �ʱ���ġ ����
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
		// Hole edge ã��
		//
		// �˻����� ����
		HRegion searchRegion;
		searchRegion.GenRectangle1(targetY-15, targetX-m_maskInfo.iPitchX/2, targetY+15, targetX+m_maskInfo.iPitchX/2 );
		DISPLAY(heightMapRot, searchRegion, strMessage+" > Hole edge Ž������");

		// Hole ��� ���� ���� �� ���̰� ȹ��
		HRegion regionRoiHoleEdge = regionHoleRot.DilationCircle(7.5).Difference(regionHoleRot).Intersection(searchRegion);
		DISPLAY(heightMapRot, regionRoiHoleEdge, strMessage+" > Hole edge Ž�����");

		if( regionRoiHoleEdge.CountObj()==0 )
		{
			continue;
		}

		// Hole edge�� ��ġ��
		HTuple rowRoiHoleEdge, colRoiHoleEdge;
		regionRoiHoleEdge.GetRegionPoints(&rowRoiHoleEdge, &colRoiHoleEdge);

		// Hole edge�� ���̰�
		HTuple heightRoiHoleEdge = heightMapRot.GetGrayval(rowRoiHoleEdge, colRoiHoleEdge);


		//
		// Peak ã��
		//
		HTuple rowRoiPeak, colRoiPeak, heightRoiPeak;
		if( peakPos[i] )
		{	// Island �� ���
			// Peak ���� ���� �� ���̰� ȹ��
			// �˻����� ����
			HRegion regionSearchPeak( targetY-m_maskInfo.iPitchX/4, targetX-m_maskInfo.iPitchX/3, targetY+m_maskInfo.iPitchX/3, targetX+m_maskInfo.iPitchX/3  );
			HRegion regionSearchPeakReduce = regionSearchPeak.Intersection(roiRegionRot).Difference(regionInvalidRot);	// Ž�������� ȸ���� ���󿡼� ����� �ʵ��� ����
			DISPLAY(heightMapRot, regionSearchPeakReduce, strMessage+" > ���Ϸ��� Ž�� ����");
			double rowPeak, colPeak, heightPeak;

			findPeakRegion( heightMapRot, regionSearchPeakReduce, 3, 3, &rowPeak, &colPeak, &heightPeak );

			rowRoiPeak = rowPeak;
			colRoiPeak = colPeak;
			heightRoiPeak = heightPeak;
		} 
		else 
		{	// Bridge �� ���
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
		DISPLAY(heightMapRot, regionVally, strMessage+" > Bridge �� ��ġ");



		// ���
		pHeightHole->Append(heightRoiHoleEdge.TupleMean());	// ���� ��� ����
		pHeightPeak->Append(heightRoiPeak);	// Peak�� ����

		HTuple distanceHolePeak;
		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiPeak, colRoiPeak, &distanceHolePeak );	
		pAngleDistance->Append(distanceHolePeak);	// ���۰��� peak�� �Ÿ�

		pPeakToPeak->Append(HTuple(0));	// ��� X
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// �Է¹��� Hole�� ���ؼ� �ֺ��� slope angle �� �ֿ��������(���۰��, island, bridge )
//
//
//	[in]
//		islandRegion : ���Ȧ �ֺ� island ��� ����
//	[out]
//		HTuple *pHeightHole	: ���� ����� ����			=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pHeightPeak	: Peak �� ����				=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pLateralDist: ���� ���� peak �� �Ÿ�	=> 4��(���/����, �»�/����, ��/��, ��/��)
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureOneLargeHoleFullIsland(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& islandRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak)
{
	double dIslandRange = 5.5;	// �����ֺ� ������ �β� ����

	// �̹��� ũ��
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );


	// Hole�� �߽��� ����
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


// 	// Hole�� ��� ���� ����
// 	HRegion edgeRegion = holeRegion.DilationCircle(1.5).Difference(holeRegion);


// 	int searchAngle[24];
// 	int peakPos[24];
// 	for( int i=0 ; i<24 ; i++ )
// 	{
// 		searchAngle[i] = i*15;	// ���� �Է�
// 		peakPos[i] = 1;
// 	}

	int nMeasure = getMeasurePointNum();
	double *pAngle = getMeasurePointAngle();

	// 8���⿡ ���� hole�� island or bridge�� ���� ����
	// 	for( int i=0 ; i<8 ; i++ )
// 	for( int i=0 ; i<24 ; i++ )
	for( int i=0 ; i<nMeasure ; i++ )
	{
		//
		// ��ó�� : ���� ȸ��
		// 
		// ���� ȸ��
		CString strMessage;
// 		strMessage.Format("ȸ�� %d�� ", searchAngle[i]);
		strMessage.Format("ȸ�� %.1f�� ", pAngle[i]);

		// ȸ�� matrix ����
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
// 		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );
		HomMat2dRotate(matIdentity, -pAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// ȸ���� ���� map ����
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > ���̰�");

		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");
		// 		DISPLAY(roiRegionRot, strMessage+" > roi");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
// 		DISPLAY(heightMapRot, strMessage+" > ���̰�(roi)");


		// ȸ���� hole region ����
		HRegion rgHoleRot;
		AffineTransRegion(holeRegion, &rgHoleRot, matRotation, "constant");
		DISPLAY(rgHoleRot, strMessage+" > �˻� ���� ����");

		// ȸ���� island region ����
		HRegion rgIslandRot;
		AffineTransRegion(islandRegion, &rgIslandRot, matRotation, "constant");
		DISPLAY(rgIslandRot, strMessage+" > Island ����");


		// Island �ʱ���ġ ����
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
			// �ϴ� ������ ��������

			// 45�� ���� ����
			double dLength = sqrt( (float)( (m_maskInfo.iPitchX/2)*(m_maskInfo.iPitchX/2) + (m_maskInfo.iPitchY/2)*(m_maskInfo.iPitchY/2) ) );

			targetX = colHoleCenter + dLength/2;
			targetY = rowHoleCenter;

// 			searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/4, targetY+10, targetX+m_maskInfo.iPitchX/4 );
			searchRegion.GenRectangle1(targetY-10, targetX-dLength/2, targetY+10, targetX+dLength/2 );
		}


		//
		// Hole edge ã��
		//
		// �˻����� ����
// 		HRegion searchRegion;
// 		searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/2, targetY+10, targetX+m_maskInfo.iPitchX/2 );
		DISPLAY(heightMapRot, searchRegion, strMessage+" > Hole edge Ž������");

		// Hole ��� ���� ���� �� ���̰� ȹ��
		HRegion regionRoiHoleEdge = rgHoleRot.DilationCircle(dIslandRange).Difference(rgHoleRot).Intersection(searchRegion);
		DISPLAY(heightMapRot, regionRoiHoleEdge, strMessage+" > Hole edge Ž�����");

		if( regionRoiHoleEdge.Area()==0 )
		{
			return false;
		}

		// Hole edge�� ��ġ��
		HTuple rowRoiHoleEdge, colRoiHoleEdge;
		regionRoiHoleEdge.GetRegionPoints(&rowRoiHoleEdge, &colRoiHoleEdge);

		// Hole edge�� ���̰�
		HTuple tHeightOfHoleEdge = heightMapRot.GetGrayval(rowRoiHoleEdge, colRoiHoleEdge);

 		if( tHeightOfHoleEdge.Length()==0 )
		{
			return false;
		}

		//
		// Island edge ã��
		//
		// �˻����� ����
		// Hole ��� ���� ���� �� ���̰� ȹ��
		HRegion rgIslandRoi = rgIslandRot.Intersection(searchRegion).Connection();

		if( rgIslandRoi.Area()==0 )
		{
			return false;
		}

		HTuple indexRoiHole;
		findNearRegionIndex( rgIslandRoi, rgHoleRot.Row(), rgHoleRot.Column(), &indexRoiHole);
		HRegion rgIslandRoi1 = rgIslandRoi.SelectObj(indexRoiHole+1);

		DISPLAY(heightMapRot, rgIslandRoi1, strMessage+" > Island Ž�����");


		// Island �� ��ġ��
		HTuple rowRoiIsland, colRoiIsland;
		rgIslandRoi1.GetRegionPoints(&rowRoiIsland, &colRoiIsland);
// 		rgIslandRoi.GetRegionPoints(&rowRoiIsland, &colRoiIsland);

		// Island �� ���̰�
		HTuple tHeightOfIsland = heightMapRot.GetGrayval(rowRoiIsland, colRoiIsland);

		if( tHeightOfIsland.Length()==0 )
		{
			return false;
		}


		// ���
		pHeightHole->Append(tHeightOfHoleEdge.TupleMean());	// ���� ��� ����
		pHeightPeak->Append(tHeightOfIsland.TupleMean());	// Peak�� ����

		HTuple tSlopeAnlgeDistance;
		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiIsland.TupleMean(), colRoiIsland.TupleMean(), &tSlopeAnlgeDistance );	
		pAngleDistance->Append(tSlopeAnlgeDistance);	// ���۰��� peak�� �Ÿ�
	}


	return true;
}

//////////////////////////////////////////////////////////////////////////
//
// �Է¹��� Hole�� ���ؼ� �ֺ��� slope angle �� �ֿ��������(���۰��, island, bridge )
//
//
//	[in]
//		islandRegion : ���Ȧ �ֺ� island ��� ����
//	[out]
//		HTuple *pHeightHole	: ���� ����� ����			=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pHeightPeak	: Peak �� ����				=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pLateralDist: ���� ���� peak �� �Ÿ�	=> 4��(���/����, �»�/����, ��/��, ��/��)
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureOneLargeHoleHalfIsland(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& islandRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pAngleDistance, HTuple *pPeakToPeak)
{
	double dIslandRange = 5.5;	// �����ֺ� ������ �β� ����

	// �̹��� ũ��
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );

	// Hole�� �߽��� ����
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


	// ���� ���� ���� : ���� ����(����), ���⺰ island or bridge
/*	int searchAngle[24];
	bool bIsIsland[24];
	for( int i=0 ; i<24 ; i++ )
	{
		searchAngle[i] = i*15;	// ���� �Է�
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


	// 24���⿡ ���� hole�� island or bridge�� ���� ����
// 	for( int i=0 ; i<24 ; i++ )
	for( int i=0 ; i<nMeasureNum ; i++ )
	{
		//
		// ��ó�� : ���� ȸ��
		// 
		// ���� ȸ��
		CString strMessage;
// 		strMessage.Format("ȸ�� %d�� ", searchAngle[i]);
		strMessage.Format("ȸ�� %.1f �� ", pMeasureAnlge[i]);

		// ȸ�� matrix ����
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
// 		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );
		HomMat2dRotate(matIdentity, -pMeasureAnlge[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// ȸ���� ���� map ����
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > ȸ���� ���̰�");

		// ȸ�� ROI ����
		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
// 		DISPLAY(heightMapRot, strMessage+" > ���̰�(roi)");

		// ȸ���� hole region ����
		HRegion rgHoleRot;
		AffineTransRegion(holeRegion, &rgHoleRot, matRotation, "constant");
		DISPLAY(rgHoleRot, strMessage+" > ȸ���� ������ġ");

		// ȸ���� island region ����
		HRegion rgIslandRot;
		AffineTransRegion(islandRegion, &rgIslandRot, matRotation, "constant");
		DISPLAY(rgIslandRot, strMessage+" > ȸ���� island ����");

		// ȸ���� ��ȿ region ����
		HRegion rgInvlaidRot;
		AffineTransRegion(invalidRegion, &rgInvlaidRot, matRotation, "constant");
		DISPLAY(rgInvlaidRot, strMessage+" > ȸ���� ��ȿ ����");


		// Island(Bridge)�� ���� ��ġ ���
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
			// �ϴ� ������ ��������

			// 45�� ���� ����
			double dLength = sqrt( (float)( (m_maskInfo.iPitchX/2)*(m_maskInfo.iPitchX/2) + (m_maskInfo.iPitchY/2)*(m_maskInfo.iPitchY/2) ) );

			targetX = colHoleCenter + dLength/2;
			targetY = rowHoleCenter;

// 			searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/4, targetY+10, targetX+m_maskInfo.iPitchX/4 );
			searchRegion.GenRectangle1(targetY-10, targetX-dLength/2, targetY+10, targetX+dLength/2 );
		}


		//
		// Hole edge ã��
		//
		// �˻� ���� ����
// 		HRegion searchRegion;
// 		searchRegion.GenRectangle1(targetY-10, targetX-m_maskInfo.iPitchX/2, targetY+10, targetX+m_maskInfo.iPitchX/2 );
		DISPLAY(heightMapRot, searchRegion, strMessage+" > Hole edge ã�� > Ž������");

		// Hole ��� ���� ���� �� ���̰� ȹ��
		HRegion regionRoiHoleEdge = rgHoleRot.DilationCircle(dIslandRange).Difference(rgHoleRot).Intersection(searchRegion);
		DISPLAY(heightMapRot, regionRoiHoleEdge, strMessage+" > Hole edge ã�� > ���� ����");

		if( regionRoiHoleEdge.Area()==0 )
		{
			return false;
		}

		// Hole edge�� ��ġ�� ����
		HTuple rowRoiHoleEdge, colRoiHoleEdge;
		regionRoiHoleEdge.GetRegionPoints(&rowRoiHoleEdge, &colRoiHoleEdge);

		// Hole edge�� ���̰� ����
		HTuple tHeightOfHoleEdge = heightMapRot.GetGrayval(rowRoiHoleEdge, colRoiHoleEdge);


		//
		// Island edge ã��
		//
		HTuple tHeightOfPeak;
		HTuple rowRoiPeak, colRoiPeak;

// 		if( bIsIsland[i] )	
		if( pMeasureIsland[i] )	
		{	// Island ������ ���
			// Island �ĺ� ������ ã�´�.
			HRegion rgIslandsCandidate = rgIslandRot.Intersection(searchRegion).Connection();
			DISPLAY(heightMapRot, rgIslandsCandidate, strMessage+" > Island ã�� > �ĺ�����");

			if( rgIslandsCandidate.Area()==0 )
			{
				return false;
			}

			// �ĺ� ������ ���� ������ ������ �����Ѵ�.( ���ۿ� ���� ����� �� )
			HTuple indexRoiHole;
			findNearRegionIndex( rgIslandsCandidate, rowHoleCenter, colHoleCenter, &indexRoiHole);
			HRegion rgIslandRoi = rgIslandsCandidate.SelectObj(indexRoiHole+1);

			DISPLAY(heightMapRot, rgIslandRoi, strMessage+" > Island ã�� > ���� ����");


			// Island�� ��ġ/���̰��� �����Ѵ�.
			HTuple rowIsland, colIsland;
			rgIslandRoi.GetRegionPoints(&rowIsland, &colIsland);

			// Island �� ���̰�
			tHeightOfPeak = heightMapRot.GetGrayval(rowIsland, colIsland);

			rowRoiPeak = rowIsland;
			colRoiPeak = colIsland;
		} 
		else	
		{	// Bridge ������ ���

			// ���̸� ���ι������� projection.
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

			// Projection���� ���� bridge�� ã�´�.( ���� ���� ����(3D �����ͷθ� ���� ���� ��) )
			HTuple index = bridgeHeightProjection.TupleSortIndex();
			int indexVally = index[0];

			// Brige ������ ��ġ, ���̰� ����
			HTuple rowRoiBridge = targetY;
			HTuple colRoiBridge = targetX-iRange+indexVally;

			// * ȭ�� ǥ��
			HRegion regionVally;
			regionVally.GenCircle(rowRoiBridge, colRoiBridge, HTuple(2.5));
			DISPLAY(heightMapRot, regionVally, strMessage+" > bridge ã�� > ���� ����");

			rowRoiPeak = rowRoiBridge;
			colRoiPeak = colRoiBridge;
			tHeightOfPeak = heightMapRot.GetGrayval(rowRoiBridge, colRoiBridge);
		}


		if( tHeightOfHoleEdge.Length()==0 || tHeightOfPeak.Length()==0 )
		{
			return false;
		}


		// ���
		pHeightHole->Append(tHeightOfHoleEdge.TupleMean());	// ���� ������ ����
		pHeightPeak->Append(tHeightOfPeak.TupleMean());		// Island or bridge �� ����

		HTuple tSlopeAnlgeDistance;
// 		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiPeak.TupleMean(), colRoiPeak.TupleMean(), &tSlopeAnlgeDistance );	
		DistancePp( rowRoiHoleEdge.TupleMean(), colRoiHoleEdge.TupleMean(), rowRoiPeak.TupleMean(), colRoiPeak.TupleMin(), &tSlopeAnlgeDistance );
		pAngleDistance->Append(tSlopeAnlgeDistance);	// ���۰��� peak�� �Ÿ�
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// �Է¹��� Hole�� ���ؼ� �ֺ��� step height
//
//
//	[out]
//		HTuple *pHeightHole	: ���� ����� ����			=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pHeightPeak	: Peak �� ����				=> 8��(���/����, �»�/����, ��/��, ��/��)
//		HTuple *pLateralDist: ���� ���� peak �� �Ÿ�	=> 4��(���/����, �»�/����, ��/��, ��/��)
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureStepHieghts(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HRegion& holeRegion, const HalconCpp::HRegion& invalidRegion, HTuple *pHeightHole, HTuple *pHeightPeak)
{
	// �̹��� ũ��
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);
	HRegion regionRoi;
	regionRoi.GenRectangle1(HTuple(1), HTuple(1), imageHeight-1, imageWidth-1 );


	// Hole�� �߽��� ����
	HTuple rowHoleCenter, colHoleCenter;
	holeRegion.AreaCenter(&rowHoleCenter, &colHoleCenter);


	// Hole�� ��� ���� ����
	HRegion edgeRegion = holeRegion.DilationCircle(1.5).Difference(holeRegion);


	// �˻� ���� : ���/����, �»�/����, ��/��, ��/��
	int searchAngle[8] = { 0, 45, 90, 135, 180, 225, 270, 315 };

	// ������ ���Ϸ��� ��ġ ���� üũ : 1�̸� peak�� �ִ�
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
	// 8���⿡ ���� hole�� vally�� ���� ����
	for( int i=0 ; i<8 ; i++ )
	{
		//
		// (1) ��ó�� : ���� ȸ��
		// 
		// ���� ȸ��
		CString strMessage;
		strMessage.Format("ȸ�� %d�� ", searchAngle[i]);

		// ȸ�� matrix ����
		HTuple matIdentity, matRotation;
		HomMat2dIdentity(&matIdentity);
		HomMat2dRotate(matIdentity, -searchAngle[i]*3.14/180, rowHoleCenter, colHoleCenter, &matRotation );

		// ���� map ȸ��
		HImage heightMapRotAll;
		AffineTransImage(heightImage, &heightMapRotAll, matRotation, "constant", "false");
		DISPLAY(heightMapRotAll, strMessage+" > ���̰�");

		HRegion roiRegionRot;
		AffineTransRegion(regionRoi, &roiRegionRot, matRotation, "constant");
// 		DISPLAY(heightMapRotAll, roiRegionRot, strMessage+" > roi");

		HImage heightMapRot = heightMapRotAll.ReduceDomain( roiRegionRot );
		DISPLAY(heightMapRot, strMessage+" > ���̰�(roi)");


		//
		HRegion regionHoleRot;
		AffineTransRegion(holeRegion, &regionHoleRot, matRotation, "constant");
		DISPLAY(heightMapRot, regionHoleRot, strMessage+" > ������ġ");

		//
		HRegion regionInvalidRot;
		AffineTransRegion(invalidRegion, &regionInvalidRot, matRotation, "constant");
		DISPLAY(heightMapRot, regionInvalidRot, strMessage+" > ��ȿ����");


		//
		// �˻� ���� ����
		HRegion rgnSearchTotal = regionHoleRot.DilationCircle(30).Difference(regionHoleRot);
		DISPLAY(heightMapRot, rgnSearchTotal, strMessage+" > �˻�����(Total)");

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
// 			DISPLAY(rgnSearch, strMessage+" > �˻�����");

			if( rgnSearch.CountObj()==0 ) return false;


			// Hole �ֺ� ���� ���� : ������ ����
			HTuple tHeightsRoi = heightMapRot.GetGrayval(tRowsSearch, tColsSearch);


			//
			HTuple tSortedIndex = tHeightsRoi.TupleSortIndex();
			int iIndexDown = tSortedIndex[tSortedIndex.Length() - 1];

			rowsHoleEdge.Append( tRowsSearch[iIndexDown] );
			colsHoleEdge.Append( tColsSearch[iIndexDown] );
			tHeightsHoleEdge.Append( tHeightsRoi[iIndexDown] );

			
			// Riv ���� ���� : ���̴� �κ� ����
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
		DISPLAY(heightMapRot, rgnHoleEdge, strMessage+" > ���� ����");

		HRegion rgnRivEdge;
		rgnRivEdge.GenRegionPoints(rowsRiv, colsRiv);
		DISPLAY(heightMapRot, rgnRivEdge, strMessage+" > ���� ����");


		pHeightHole->Append(tHeightsHoleEdge.TupleMean());
		pHeightPeak->Append(tHeightsRiv.TupleMean());
	}


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Slope angle�� ����Ѵ�.
//
//	1. ���� ���� ����
//	2. ���� ������ ������� ���⺰ step height�� slope angle ���
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleFromPeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights)
{
	int iSize = 4000;	// �̳��� �÷�� �ӽ� �Ķ����
	double dSizeMargin = 0.2;
	double dGrayDiffLimit = 2;
	bool bValid = false;

	double dIslandRange = 10.5;

	// ������ ����
	int iShape = m_maskInfo.shape;


	// ������ ũ��
	HTuple imageHeight, imageWidth;
	laserImage.GetImageSize( &imageWidth, &imageHeight );


	//
	// 0. ��ó��
	//	������ ����
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island ������ ã�´�.
	//

	// Island ���� ã��
	HRegion rgIsland1;
	if( !findIslandRegionSmallIsland( smoothLaser, smoothHeight, rgIsland1) )
	{
		return false;
	}
	HRegion rgIsland = rgIsland1.FillUp();

	DISPLAY(laserImage, rgIsland, "Island ���� ã�� > �ĺ�����");


	// ��ȿ island ����



	//
	// 2. ���� ������ ã�´�.
	//

	// 2.1
	HRegion rgHoles1;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles1) )
	{
		return false;
	}
	HRegion rgHoles = rgHoles1.FillUp();

	// 2.2 ��� ���� ����
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "���� ã�� > ���� ���� ����");




	//
	//
	// 2. ROI ������ �������� 8���� slope angle�� step heigh�� ����Ѵ�.
	//
	//
	HTuple hh;	// Hole edge�� ����
	HTuple hp;	// peak�� ����
	HTuple ptpDist;	// peak �� �Ÿ�
	HTuple angDist;	// peak �� �Ÿ�

//  	for( int i=0 ; i<holeRegions.CountObj() ; i++ )
  	for( int i=0 ; i<rgHoles.CountObj() ; i++ )
	{
//  		HRegion tmpRegion = holeRegions.SelectObj(i+1);
		HRegion tmpRegion = rgHoles.SelectObj(i+1).DilationCircle(1.5);
 
 		if( tmpRegion.Column() > m_maskInfo.iPitchX && tmpRegion.Column() < imageWidth - m_maskInfo.iPitchX &&
 			tmpRegion.Row() > m_maskInfo.iPitchY && tmpRegion.Row() < imageHeight - m_maskInfo.iPitchY)
		{
			measureSlopeAngles(	laserImage,		// ������ ����
								smoothHeight,	// ���� ����
				//				roiHoleRegion,		// ���� ����
 								tmpRegion,
// 								holeRegions.DilationCircle(2.5),	// ��ȿ����(���ɱ��� �̿� ���� ���� )
								rgHoles.DilationCircle(2.5),	// ��ȿ����(���ɱ��� �̿� ���� ���� )
								&hh, 
								&hp, 
								&angDist, 
								&ptpDist );
		}
 	}
 
//  	for( int i=0 ; i<24 ; i++ )
	// 2019.01.09 �ҿ� �κ� ����
// 	for( int i=0 ; i<getMeasurePointNum() ; i++ )
// 	{
//  		hh.Append(0.0);			// Hole edge�� ����
//  		hp.Append(0.0);			// peak�� ����
//  		ptpDist.Append(0.0);	// peak �� �Ÿ�
//  		angDist.Append(0.0);	// peak �� �Ÿ�
//  	}

	HTuple gap = hh.TupleSub(hp);					// hole edge�� peak�� ������
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// ���� ��� + (���� -> ��)

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
// Slope angle�� ����Ѵ�.
//
//	1. ���� ���� ����
//	2. ���� ������ ������� ���⺰ step height�� slope angle ���
//
//////////////////////////////////////////////////////////////////////////
/*
bool CInspection::measureSlopeAngleFromPeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights)
{
	int iSize = 4000;	// �̳��� �÷�� �ӽ� �Ķ����
	double dSizeMargin = 0.2;
	double dGrayDiffLimit = 2;
	bool bValid = false;

	// ������ ����
	int iShape = m_maskInfo.shape;

	// ������ ũ��
	HTuple imageHeight, imageWidth;
	laserImage.GetImageSize( &imageWidth, &imageHeight );


	//////////////////////////////////////////////////////////////////////////
	// 0. ��ó�� - ������ ����
	//////////////////////////////////////////////////////////////////////////
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored").MeanImage(3,3);
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");


	//////////////////////////////////////////////////////////////////////////
	// 2. Hole edge region ã�� : Laser + dic ���� ����
	//////////////////////////////////////////////////////////////////////////
	// 1) threshold for hole
	HRegion holesRegion1 = smoothLaser.Threshold(0,30).OpeningCircle(3.5).ClosingCircle(4.5).DilationCircle(3);

	// 	HRegion holesRegion2 = dicImage.Threshold(0,40).ClosingCircle(4.5).DilationCircle(8);
	HRegion holesRegion2 = dicImage.Threshold(0,40).OpeningCircle(5.5).ErosionCircle(7.5).FillUp();

	HRegion holesRegion  = holesRegion1.Intersection( holesRegion2 );;

	DISPLAY(smoothLaser,holesRegion);

	// 2) ���� ������ �������� real holes ����
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
		paramShapeHole[2] = "rectangularity";	// ���� hole�� ���
		paramMinHole[2] = 0.8;
		paramMaxHole[2] = 1;
		break;

	case HOLE_SHAPE_DIAMOND:
		break;

	case HOLE_SHAPE_CIRCLE:
		paramShapeHole[2] = "circularity";	// ���� hole�� ���
		paramMinHole[2] = 0.5;
		paramMaxHole[2] = 1;
		break;
	}

	HRegion holeRegions = holesRegion.ClosingCircle(4.5).DilationCircle(9.5).Connection().SelectShape(paramShapeHole, "and", paramMinHole, paramMaxHole);
	DISPLAY(smoothLaser,holeRegions, "�˻� ��� ����");



	// find roi hole 1��
	HTuple rowRoiHole, colRoiHole, roiHoleIndex;
	if( !findNearRegionIndex(holeRegions, imageHeight/2, imageWidth/2, &roiHoleIndex ) )
	{
		return false;
	}

	HRegion roiHoleRegion = holeRegions.SelectObj(roiHoleIndex+1);

	DISPLAY(smoothLaser, roiHoleRegion, "�˻� ��� ����");



	//
	//
	// 2. ROI ������ �������� 8���� slope angle�� step heigh�� ����Ѵ�.
	//
	//
	HTuple hh;	// Hole edge�� ����
	HTuple hp;	// peak�� ����
	HTuple ptpDist;	// peak �� �Ÿ�
	HTuple angDist;	// peak �� �Ÿ�

 	for( int i=0 ; i<holeRegions.CountObj() ; i++ )
 	{
 		HRegion tmpRegion = holeRegions.SelectObj(i+1);
 
 		if( tmpRegion.Column() > m_maskInfo.iPitchX && tmpRegion.Column() < imageWidth - m_maskInfo.iPitchX &&
 			tmpRegion.Row() > m_maskInfo.iPitchY && tmpRegion.Row() < imageHeight - m_maskInfo.iPitchY)
		{
			measureSlopeAngles(	laserImage,		// ������ ����
								smoothHeight,	// ���� ����
				//				roiHoleRegion,		// ���� ����
 								tmpRegion,
								holeRegions.DilationCircle(2.5),	// ��ȿ����(���ɱ��� �̿� ���� ���� )
								&hh, 
								&hp, 
								&angDist, 
								&ptpDist );
		}
 	}
 
//  	for( int i=0 ; i<24 ; i++ )
	for( int i=0 ; i<getMeasurePointNum() ; i++ )
	{
 		hh.Append(0.0);			// Hole edge�� ����
 		hp.Append(0.0);			// peak�� ����
 		ptpDist.Append(0.0);	// peak �� �Ÿ�
 		angDist.Append(0.0);	// peak �� �Ÿ�
 	}

	HTuple gap = hh.TupleSub(hp);					// hole edge�� peak�� ������
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// ���� ��� + (���� -> ��)

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
int iSize = 4000;	// �̳��� �÷�� �ӽ� �Ķ����
double dSizeMargin = 0.2;
double dGrayDiffLimit = 2;
bool bValid = false;

// ������ ����
int iShape = m_maskInfo.shape;

// ������ ũ��
HTuple imageHeight, imageWidth;
laserImage.GetImageSize( &imageWidth, &imageHeight );


//////////////////////////////////////////////////////////////////////////
// 0. ��ó��
//////////////////////////////////////////////////////////////////////////
HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored").MeanImage(5,5);
HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");


//////////////////////////////////////////////////////////////////////////
// 1. Peak ���� ã��
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
case HOLE_SHAPE_RECTANGLE:	// �� ������� ������
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

case HOLE_SHAPE_CIRCLE:	// Diamond ������� ������
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

// 3-1) ���� ����
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

// 5) Peak �� �߽� ���
rowPeak = roiPeakRegion.Row();
colPeak = roiPeakRegion.Column();


// 6) Peak�ǳ��̰��
// case 1
/////	heightPeak = smoothHeight.GetGrayval(rowPeak,colPeak);

// case 2
HTuple rowsPeak, colsPeak;
roiPeakRegion.GetRegionPoints( &rowsPeak,  &colsPeak );
HTuple peakHeights = smoothHeight.GetGrayval( rowsPeak, colsPeak );
heightPeak = peakHeights.TupleMean();


// * ǥ��
HRegion rgnPeak( rowPeak, colPeak, HTuple(3) );
DISPLAY(smoothLaser,rgnPeak);


//////////////////////////////////////////////////////////////////////////
// 2. Hole edge region ã��
//////////////////////////////////////////////////////////////////////////
// 1) threshold for hole
HRegion holesRegion1 = smoothLaser.Threshold(0,30).OpeningCircle(3.5).ClosingCircle(4.5).DilationCircle(3);
HRegion holesRegion2 = dicImage.Threshold(0,50).ClosingCircle(4.5).DilationCircle(8);

HRegion holesRegion  = holesRegion1.Intersection( holesRegion2 );;

DISPLAY(smoothLaser,holesRegion);

// 2) ���� ������ �������� real holes ����
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
paramShapeHole[2] = "rectangularity";	// ���� hole�� ���
paramMinHole[2] = 0.8;
paramMaxHole[2] = 1;
break;

case HOLE_SHAPE_DIAMOND:
break;

case HOLE_SHAPE_CIRCLE:
paramShapeHole[2] = "circularity";	// ���� hole�� ���
paramMinHole[2] = 0.5;
paramMaxHole[2] = 1;
break;
}

HRegion holeRegions = holesRegion.Connection().SelectShape(paramShapeHole, "and", paramMinHole, paramMaxHole);
DISPLAY(smoothLaser,holeRegions);

// 3) hole edge ���� ����
HRegion holeEdgeRegions = holeRegions.DilationCircle(3).Difference(holeRegions);
DISPLAY(smoothLaser,holeEdgeRegions);



//////////////////////////////////////////////////////////////////////////
// 3. �� ���⿡ ���� slope angle ���
//////////////////////////////////////////////////////////////////////////

// 1) Ž�� ����
double seedAngle[2] = { 45*PI/180, 135*PI/180 };	// 


// 2) 
for( int i=0 ; i<2 ; i++ )
{
HRegion region1, region2;
region1.GenEmptyRegion();
region2.GenEmptyRegion();


// a. hole edge ���� ����
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


// b. hole edge ������ ���̸� �̿��Ͽ� slope angle�� ����Ѵ�.
HTuple rows, cols, heights, dist_x, dist_z;

// b-1. 1��° �������� slope angle ���
region1.GetRegionPoints(&rows, &cols);
heights = heightImage.GetGrayval(rows, cols);

if( heights.Length()>0 )
{
DistancePp( rowPeak, colPeak, rows.TupleMean(), cols.TupleMean(), &dist_x );	// dx
dist_z = heights.TupleMean() - heightPeak;		// dz

pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// ���� ���
pHoleHeights->Append( heights.TupleMean() );	// Hole edge�� ��� ���� ���
pPeakHeights->Append( heightPeak );
} else 
{	// ���ܻ���
pAngles->Append( -1.0 );
pHoleHeights->Append( -1.0 );
pPeakHeights->Append( heightPeak );
}

DISPLAY(smoothLaser, region1);


// b-2. 2��° �������� slope angle ���
region2.GetRegionPoints(&rows, &cols);
heights = heightImage.GetGrayval(rows, cols);

if( heights.Length()>0 )
{
DistancePp( rowPeak, colPeak, rows.TupleMean(), cols.TupleMean(), &dist_x );	// dx
dist_z = heights.TupleMean() - heightPeak;		// dz

pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// ���� ���
pHoleHeights->Append( heights.TupleMean() );	// Hole edge�� ��� ���� ���
pPeakHeights->Append( heightPeak );
} else 
{	// ���ܻ���
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
// Slope angle�� ����Ѵ�.
//	- ������ ã�� 
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleFromOnePeak(const HalconCpp::HImage& laserImage, const HalconCpp::HImage& heightImage, const HalconCpp::HImage& dicImage, HTuple *pAngles, HTuple *pHoleHeights, HTuple *pPeakHeights)
{
	double dSizeMargin = 0.2;
	double dGrayDiffLimit = 2;
	bool bValid = false;

	// ������ ����
	int iShape = m_maskInfo.shape;

	// ������ ũ��
	HTuple imageHeight, imageWidth;
	laserImage.GetImageSize( &imageWidth, &imageHeight );



	//////////////////////////////////////////////////////////////////////////
	// 0. ��ó��
	//////////////////////////////////////////////////////////////////////////
	// Median + Mean
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored").MeanImage(5,5);

	// Median
	HImage smoothHeight = heightImage.MedianImage("circle", 4, "mirrored");



	//////////////////////////////////////////////////////////////////////////
	//
	// 1. Hole ���� ã��
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
	case HOLE_SHAPE_RECTANGLE:	// �� ������� ������
		break;

	case HOLE_SHAPE_DIAMOND:
		break;

	case HOLE_SHAPE_CIRCLE:	// Diamond ������� ������
		break;

	case HOLE_SHAPE_ELIPSE:	// ������ ������� ������
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
	// 3-1) ���� ����
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


	// * ǥ��
	DISPLAY(smoothLaser,roiHoleRegion);


	//
	// 5) Hole edge region ã��
	//
	HRegion holeEdgeRegion = roiHoleRegion.DilationCircle(3.5).Difference( roiHoleRegion.DilationCircle(0.5) );

	DISPLAY(smoothLaser,holeEdgeRegion);


	//
	// 3. Peak ���� ���ϱ�
	//
	//	- �¿���� ���� ���
	//

	// 3.1 ���� peak ���� ã��
	HRegion peakSearchRegion;
	HTuple rowSeed = roiHoleRegion.Row();
	HTuple colSeed = roiHoleRegion.Column() - m_maskInfo.iPitchX/2;

	// �˻� ���� ����
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(30), colSeed-HTuple(20), rowSeed+HTuple(30), colSeed+HTuple(20) );
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak ��ġ ���
	double rowLeftPeak, colLeftPeak, heightLeftPeak;
	findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowLeftPeak, &colLeftPeak, &heightLeftPeak);


	// 3.2 ������ peak ���� ã��
	rowSeed = roiHoleRegion.Row();
	colSeed = roiHoleRegion.Column() + m_maskInfo.iPitchX/2;

	// �˻� ���� ����
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(30), colSeed-HTuple(20), rowSeed+HTuple(30), colSeed+HTuple(20) );
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak ��ġ ���
	double rowRightPeak, colRightPeak, heightRightPeak;
	findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowRightPeak, &colRightPeak, &heightRightPeak);


	// 3.3 ���� peak ���� ã��
	rowSeed = roiHoleRegion.Row() - m_maskInfo.iPitchY/4;
	colSeed = roiHoleRegion.Column();

	// �˻� ���� ����
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(40), colSeed-HTuple(40), rowSeed+HTuple(40), colSeed+HTuple(40) );
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak ��ġ/���� ���
	double rowUpPeak, colUpPeak, heightUpPeak;
	HRegion rgnUpPeak;
	if( !findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowUpPeak, &colUpPeak, &heightUpPeak, rgnUpPeak) )
	{
		return false;
	}
	DISPLAY(smoothLaser,rgnUpPeak);

	// Peak�� edge ���
	HRegion rgnEdgeOfUpPeak = rgnUpPeak.Difference(rgnUpPeak.ErosionCircle(1.5));

	// *region�� ������ return false;
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


	// 3.4 �Ʒ��� peak ���� ã��
	rowSeed = roiHoleRegion.Row() + m_maskInfo.iPitchY/4;
	colSeed = roiHoleRegion.Column();

	// �˻� ���� ����
	peakSearchRegion.GenRectangle1( rowSeed-HTuple(40), colSeed-HTuple(40), rowSeed+HTuple(40), colSeed+HTuple(40) );	
	DISPLAY(smoothLaser,peakSearchRegion);

	// Peak ��ġ/���� ���
	double rowBottomPeak, colBottomPeak, heightBottomPeak;
	HRegion rgnDownPeak;
	if( !findPeakRegion(smoothHeight, peakSearchRegion, 2, 2, &rowBottomPeak, &colBottomPeak, &heightBottomPeak, rgnDownPeak) )
	{
		return false;
	}
	DISPLAY(smoothLaser,rgnDownPeak);

	//
	HRegion rgnEdgeOfDownPeak = rgnDownPeak.Difference(rgnDownPeak.ErosionCircle(1.5));

	// * region�� ������ return false;
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
	// 3. �� ���⿡ ���� slope angle ���
	//////////////////////////////////////////////////////////////////////////

	// 1) Ž�� ����
	// 	double seedAngle[2] = { 0*PI/180, 90*PI/180 };	// 0, 180, 90, 270 �� 


	//
	// 4.  
	// 

	// ���Ϲ��⿡ ���� angle & height
	{
		// a. Search bar ����
		HRegion searchBarRegion;

		searchBarRegion.GenRectangle2(	roiHoleRegion.Row(), roiHoleRegion.Column(), 
			HTuple(0),	//HTuple(seedAngle[i]), 
			HTuple( m_maskInfo.iPitchX ), 
			HTuple(10) );

		DISPLAY(smoothLaser,searchBarRegion);


		// c. Hole�� ddge ���� ����
		HRegion tmpHoleEdgeRegions = searchBarRegion.Intersection(holeEdgeRegion).Connection();

		DISPLAY(smoothLaser, tmpHoleEdgeRegions);

		if( tmpHoleEdgeRegions.CountObj()!=2 )
		{
			return false;
		}


		// ��
		{
			HTuple rowHole, colHole, heightHole;
			HTuple rowPeak, colPeak, heightPeak;

			// Hole ���� ���� �� ����
			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(1);
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);

			// Peak ���� ���� �� ����
			rowPeak		= rowHole;		// ���۰� ���� ��ġ
			colPeak		= colLeftPeak;	//
			heightPeak	= heightLeftPeak;

			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// ���� ���
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// ���� ���

				// ���� ������ ����
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge�� ��� ���� ��
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge�� ��� ���� ��
			} 
			else 
			{	// ���ܻ���
				pAngles->Append( -1.0 );
				pHoleHeights->Append( -1.0 );
				pPeakHeights->Append( heightPeak.TupleMean() );
			}

		}


		// �� 
		{
			HTuple rowHole, colHole, heightHole;
			HTuple rowPeak, colPeak, heightPeak;

			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(2);

			// Hole ���� �� ����
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);

			// Peak ���� �� ����
			rowPeak		= rowHole;		// ���۰� ���� ��ġ
			colPeak		= colRightPeak;	//
			heightPeak	= heightRightPeak;

			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// ���� ���
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// ���� ���

				// ���� ������ ����
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge�� ��� ���� ��
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge�� ��� ���� ��
			} 
			else 
			{	// ���ܻ���
				pAngles->Append( -1.0 );
				pHoleHeights->Append( -1.0 );
				pPeakHeights->Append( heightPeak.TupleMean() );
			}

		}
	}


	// ���Ϲ��⿡ ���� angle & height
	{
		// a. Search bar ����
		HRegion searchBarRegion;

		searchBarRegion.GenRectangle2(	roiHoleRegion.Row(), roiHoleRegion.Column(), 
			HTuple(90*PI/180), //HTuple(seedAngle[i]), 
			HTuple( m_maskInfo.iPitchX ), 
			HTuple(20) );

		DISPLAY(smoothLaser,searchBarRegion);


		// c. Hole�� ddge ���� ����
		HRegion tmpHoleEdgeRegions = searchBarRegion.Intersection(holeEdgeRegion).Connection();

		DISPLAY(smoothLaser, tmpHoleEdgeRegions);

		if( tmpHoleEdgeRegions.CountObj()!=2 )
		{
			return false;
		}


		// ��
		HTuple rowHole, colHole, heightHole;
		HTuple rowPeak, colPeak, heightPeak;
		{
			// 1) Hole ���� ���� �� ����
			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(1);
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);


			// 2) Peak ���� ����
			// 2.1) ���� edge
			HRegion tmpPeakEdgeRegions = searchBarRegion.Intersection(rgnEdgeOfUpPeak).Connection();
			if( tmpPeakEdgeRegions.CountObj()<1 )
			{
				return false;
			}
			DISPLAY(smoothLaser, tmpPeakEdgeRegions);

			// 2.2) ��ȿ edge
			HTuple index;
			findNearRegionIndex(tmpPeakEdgeRegions, roiHoleRegion.Row(), roiHoleRegion.Column(), &index );

			HRegion peakRegion = tmpPeakEdgeRegions.SelectObj(index+1);
			peakRegion.GetRegionPoints(&rowPeak, &colPeak);

			DISPLAY(smoothLaser, peakRegion);

			// 2.3) ��ȿ ����
			HTuple tmpRow, tmpCol;
			searchBarRegion.Intersection(rgnUpPeak.ErosionCircle(1.5)).GetRegionPoints(&tmpRow, &tmpCol);
			heightPeak = smoothHeight.GetGrayval(tmpRow, tmpCol);


			// 3) ���
			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// ���� ���
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// ���� ���

				// ���� ������ ����
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge�� ��� ���� ��
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge�� ��� ���� ��
			} 
			else 
			{	// ���ܻ���
				pAngles->Append( -1.0 );
				pHoleHeights->Append( -1.0 );
				pPeakHeights->Append( heightPeak.TupleMean() );
			}

		}


		// ��
		{
			// 1) Hole ���� �� ����
			HRegion holeRegion = tmpHoleEdgeRegions.SelectObj(2);
			holeRegion.GetRegionPoints(&rowHole, &colHole);
			heightHole = smoothHeight.GetGrayval(rowHole, colHole);


			// b. Peak ���� ����
			HRegion tmpPeakEdgeRegions = searchBarRegion.Intersection(rgnEdgeOfDownPeak).Connection();

			DISPLAY(smoothLaser, tmpPeakEdgeRegions);

			if( tmpPeakEdgeRegions.CountObj()<1 )
			{
				return false;
			}

			// 2.2) ��ȿ edge
			HTuple index;
			findNearRegionIndex(tmpPeakEdgeRegions, roiHoleRegion.Row(), roiHoleRegion.Column(), &index );

			HRegion peakRegion = tmpPeakEdgeRegions.SelectObj(index+1);
			peakRegion.GetRegionPoints(&rowPeak, &colPeak);
			// 			heightPeak = smoothHeight.GetGrayval(rowPeak, colPeak);

			DISPLAY(smoothLaser, peakRegion);

			// 2.3) ��ȿ ����
			HTuple tmpRow, tmpCol;
			searchBarRegion.Intersection(rgnDownPeak.ErosionCircle(1.5)).GetRegionPoints(&tmpRow, &tmpCol);
			heightPeak = smoothHeight.GetGrayval(tmpRow, tmpCol);


			// 3) 
			if( rowPeak.Length()>0 && rowHole.Length()>0 )
			{
				// ���� ���
				HTuple dist_x, dist_z;
				DistancePp( rowPeak.TupleMean(), colPeak.TupleMean(), rowHole.TupleMean(), colHole.TupleMean(), &dist_x );	// dx
				dist_z = heightHole.TupleMean() - heightPeak.TupleMean();		// dz

				pAngles->Append( dist_z.TupleAtan2(dist_x*m_dResolution)*180.0/PI );	// ���� ���

				// ���� ������ ����
				pHoleHeights->Append( heightHole.TupleMean() );		// Hole edge�� ��� ���� ��
				pPeakHeights->Append( heightPeak.TupleMean() );		// Peak edge�� ��� ���� ��
			} 
			else 
			{	// ���ܻ���
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
// ������ ã�� ������ ������ ���̿� ���۰� ���츮�� ���̸� ����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureSlopeAngleGreen(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	bool bValid = false;


	//
	// ��ó��
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


	// 1.2 Opening : ������ ����
	HRegion openRegion = thresholdRegion.OpeningCircle(3.5);

	// 	DISPLAY(openRegion);


	// 1.3 Closing : ������ ����
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
	// 1.5 �������� ����Ͽ� �������� ���� ���� ����
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
	// 1.6 ���� �߽ɰ� �Ÿ��� �������� �����Ͽ� ���� ����� ���� 4���� ã�´�.
	//
	// ������ �߽� ��ġ ���
	HTuple imageWidth, imageHeight;
	laserImage.GetImageSize( &imageWidth, &imageHeight );

	HTuple indexRoiHole;
	findNearRegionIndex(rgValidHoles, imageHeight/2, imageWidth/2, &indexRoiHole );

	HRegion rgRoiHole = rgValidHoles.SelectObj(indexRoiHole+1);
	DISPLAY(rgRoiHole);



	//
	// Hole �Ѱ��� �������� slope angle �� ���
	//
	HTuple hh;	// Hole edge�� ����
	HTuple hp;	// peak�� ����
	HTuple ptpDist;	// peak �� �Ÿ�
	HTuple angDist;	// peak �� �Ÿ�
	measureSlopeAngles(	laserImage, 
						smoothHeight, 
						rgRoiHole, 
						rgValidHoles,
						&hh, 
						&hp, 
						&angDist, 
						&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge�� peak�� ������
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// ���� ��� + (���� -> ��)

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
// ������ ã�� ������ ������ ���̿� ���۰� ���츮�� ���̸� ����Ѵ�.
//
//	1. Island �ܰ����� ã�´�.
//	2. �˻��� ������ ã�´�.
//	3. �˻��� ���ۿ� ���� ����� island �ܰ����� �����Ѵ�.
//	4. ���� ������ �������� ���⺰�� ���� step height�� slope angle�� ����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleAutoRG(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	double dIslandRange = 10.5;	// r�˻� ����


	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);


	//
	// ��ó��
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island ������ ã�´�.
	//

	// Island ���� ã��
	HRegion rgIsland;
	if( !findIslandRegion( smoothLaser, smoothHeight, rgIsland) )
	{
		return false;
	}

	// Island �ܰ��� ã��
	HRegion rgEdgeOfIslandCandidate = rgIsland.Difference( rgIsland.ErosionCircle(1.5 )).Connection();

	DISPLAY(laserImage, rgEdgeOfIslandCandidate, "Island �ܰ� ���� ã�� > �ĺ�����");


	// ��ȿ island �ܰ��� ����
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


	DISPLAY(rgEdgeOfIsland, "Island �ܰ� ���� ã�� > ���� ���� ����");



	//
	// 2. ���� ������ ã�´�.
	//

	// 2.1
	HRegion rgHoles;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles) )
	{
		return false;
	}

	// 2.2 ��� ���� ����
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "���� ã�� > ���� ���� ����");



	//
	// 3. ��ȿ island edge ����
	//

	// ���� ���ۿ��� ���� ����� ���� ���� ����
	HTuple indexRoiIsland;
	findNearRegionIndex( rgEdgeOfIsland, rgRoiHole.Row(), rgRoiHole.Column(), &indexRoiIsland);
	HRegion rgRoiEdgeOfIsland = rgEdgeOfIsland.SelectObj(indexRoiIsland+1);
	DISPLAY(rgRoiEdgeOfIsland, "Island ���� ã�� > island edge > ���� edge");


	// ������ �β��� �����.
	//	���õ� ������ �β��� ����� ���� island ������ ��ġ�� �κ��� �츰��.
	HRegion rgRoiEdgeOfIsland2 = rgIsland.Difference( rgIsland.ErosionCircle(dIslandRange )).Intersection(rgRoiEdgeOfIsland.DilationCircle(dIslandRange)).Connection();

	DISPLAY(rgRoiEdgeOfIsland2, "Island ���� ã�� > island > Connection");


	//
	if( rgRoiEdgeOfIsland2.CountObj()<1 )
	{
		return false;
	}



	//
	//
	// 2. ROI ������ �������� 8���� slope angle�� step heigh�� ����Ѵ�.
	//
	//
	HTuple hh;	// Hole edge�� ����
	HTuple hp;	// peak�� ����
	HTuple ptpDist;	// peak �� �Ÿ�
	HTuple angDist;	// peak �� �Ÿ�

	if( !measureOneLargeHoleFullIsland(	laserImage,		// ������ ����
										smoothHeight,	// ���� ����
										rgRoiHole,	//rgnRoiHole,		// ���� ����
										rgRoiEdgeOfIsland2,	//rgValidEdgesOfIsland,	// Island ����
										&hh, 
										&hp, 
										&angDist, 
										&ptpDist ) )
	{
		return false;
	}

	HTuple gap = hh.TupleSub(hp);					// hole edge�� peak�� ������
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// ���� ��� + (���� -> ��)

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
// ������ ã�� ������ ������ ���̿� ���۰� ���츮�� ���̸� ����Ѵ�.
//
//	1. ���� ���� 1���� ã�´�.
//	2. ���� ������ �������� ���⺰�� ���� step height�� slope angle�� ����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleAutoB(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	double dIslandRange = 10.5;


	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	//
	// ��ó��
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island ������ ã�´�.
	//

	// Island ���� ã��
	HRegion rgIsland1;
	if( !findIslandRegion( smoothLaser, smoothHeight, rgIsland1) )
	{
		return false;
	}
	HRegion rgIsland = rgIsland1.FillUp();

	// Island �ܰ��� ã��
	HRegion rgEdgeOfIslandCandidate = rgIsland.Difference( rgIsland.ErosionCircle(dIslandRange )).Connection();

	DISPLAY(laserImage, rgEdgeOfIslandCandidate, "Island �ܰ� ���� ã�� > �ĺ�����");


	// ��ȿ island �ܰ��� ����
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


	DISPLAY(rgEdgeOfIsland, "Island �ܰ� ���� ã�� > ���� ���� ����");



	//
	// 2. ���� ������ ã�´�.
	//

	// 2.1
	HRegion rgHoles;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles) )
	{
		return false;
	}

	// 2.2 ��� ���� ����
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "���� ã�� > ���� ���� ����");



	//
	//
	// 2. ROI ������ �������� 8���� slope angle�� step heigh�� ����Ѵ�.
	//
	//
	HTuple hh;	// Hole edge�� ����
	HTuple hp;	// peak�� ����
	HTuple ptpDist;	// peak �� �Ÿ�
	HTuple angDist;	// peak �� �Ÿ�

	measureOneLargeHoleHalfIsland(	laserImage,		// ������ ����
									smoothHeight,	// ���� ����
									rgRoiHole,		//rgnRoiHole,		// ���� ����
									rgEdgeOfIsland,	//rgValindIsland,	// Island ����
									rgHoles,		// ��ȿ ����
									&hh, 
									&hp, 
									&angDist, 
									&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge�� peak�� ������
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// ���� ��� + (���� -> ��)

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
// ������ ã�� ������ ������ ���̿� ���۰� ���츮�� ���̸� ����Ѵ�.
//
//	1. Island ������ ã�´�.
//		Laser ���󿡼� ���� �κа� ���� ���󿡼� ���̰� ���� �κ��� ���������� �����Ѵ�.
//
//	2. ���� ������ ã�´�.
//		Laser ���󿡼� ��ο� �κа� ���� ���󿡼� ���̰� ���� �κ��� ���������� �����Ѵ�.
//
//	3. ���ۿ� ���ؼ� �������� step height�� slope angle�� ����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureLargeHoleAuto(const HImage& laserImage, const HImage& heightImage, const HTuple& Row, const HTuple& Column, HTuple *pAngle, HTuple *pHeightHole, HTuple *pHeightPeak, HTuple *pPeakToPeak )
{
	double dIslandRange = 10.5;


	bool bValid = false;
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);


	//
	// ��ó��
	//
	HImage smoothLaser = laserImage.MedianImage("circle", 2, "mirrored");
	HImage smoothHeight = heightImage.MedianImage("circle", 2, "mirrored");



	//
	// 1. Island ������ ã�´�.
	//

	// Island ���� ã��
	HRegion rgIsland;
	if( !findIslandRegion( smoothLaser, smoothHeight, rgIsland) )
	{
		return false;
	}

	// Island �ܰ��� ã��
	HRegion rgEdgeOfIslandCandidate = rgIsland.Difference( rgIsland.ErosionCircle(dIslandRange )).Connection();

	DISPLAY(laserImage, rgEdgeOfIslandCandidate, "Island �ܰ� ���� ã�� > �ĺ�����");


	// ��ȿ island �ܰ��� ����
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


	DISPLAY(rgEdgeOfIsland, "Island �ܰ� ���� ã�� > ���� ���� ����");



	//
	// 2. ���� ������ ã�´�.
	//

	// 2.1
	HRegion rgHoles;
	if( !findHoleRegion( smoothLaser, smoothHeight, rgIsland, rgHoles) )
	{
		return false;
	}

	// 2.2 ��� ���� ����
	HTuple indexRoiHole;
	findNearRegionIndex( rgHoles, imageHeight/2, imageWidth/2, &indexRoiHole);
	HRegion rgRoiHole = rgHoles.SelectObj(indexRoiHole+1);

	DISPLAY(rgRoiHole, "���� ã�� > ���� ���� ����");



	//
	//
	// 2. ROI ������ �������� 8���� slope angle�� step heigh�� ����Ѵ�.
	//
	//
	HTuple hh;	// Hole edge�� ����
	HTuple hp;	// peak�� ����
	HTuple ptpDist;	// peak �� �Ÿ�
	HTuple angDist;	// peak �� �Ÿ�

	measureOneLargeHoleHalfIsland(	laserImage,		// ������ ����
		smoothHeight,	// ���� ����
		rgRoiHole,		//rgnRoiHole,		// ���� ����
		rgEdgeOfIsland,	//rgValindIsland,	// Island ����
		rgHoles,		// ��ȿ ����
		&hh, 
		&hp, 
		&angDist, 
		&ptpDist );

	HTuple gap = hh.TupleSub(hp);					// hole edge�� peak�� ������
	HTuple ptpDist_um = ptpDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple angDist_um = angDist.TupleMult(m_dResolution);	// um ������ ��ȯ
	HTuple ang = gap.TupleAtan2(angDist_um).TupleMult(180/3.145);	// ���� ��� + (���� -> ��)

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
// Step height ��� 
//
//	- Threshod DIC image
//	- Find hole of interest
//	- ��, ��, ��, ��, ���, ����, �»�, ���� ������ step height�� ����Ѵ�.( 8�� )
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::measureStepHeightAtSmallHoleRegion(const HImage& laserImage, const HImage& heightImage, const HImage& dicImage, const HTuple& height, const HTuple& width, HTuple *pStepHeight )
{
	HTuple imageWidth, imageHeight;
	laserImage.GetImageSize( &imageWidth, &imageHeight );


	CString str;

	HImage image(dicImage);
	HImage hHeightMapSmooth = heightImage.MedianImage(HTuple("circle"), 3, HTuple("mirrored"));	// �������� ������ ���� ���ſ�


	//////////////////////////////////////////////////////////////////////////
	//
	// 1. ���� ������ ã�´�.
	//
	// 1) hole ���� ����
// 	HRegion rgnThreshold = laserImage.Threshold(0, 50);
	HRegion rgnThreshold = dicImage.Threshold(0, 50);

	DISPLAY(hHeightMapSmooth, rgnThreshold, "���� ���� > Threshold");


	// 2) hole ���� �ĺ� ����
	HRegion rgnHoles = rgnThreshold.ErosionCircle(3.5);
	rgnHoles = rgnHoles.DilationCircle(3.5);
	rgnHoles = rgnHoles.ClosingCircle(25.5);
	rgnHoles = rgnHoles.DilationCircle(1.5);

	DISPLAY(hHeightMapSmooth, rgnHoles, "���� ���� > Threshold > ��ó��");


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

	DISPLAY(hHeightMapSmooth, rgnHoleROI, "���� ���� > ����");


	// ���õ� hole�� �������� �߰�Ȯ�� �ʿ�
	// ����, ��, ����, ��ġ ��...

	//
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	//
	// 2. Step height�� ����Ѵ�.
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
// �β� �� ��� 
//
//	- 3D data�� �Ÿ��� �̿�
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
// Pixel ������ x pitch ���� ��ȯ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
double CInspection::getPitchXPixel()
{
	return m_maskInfo.iPitchX;
}

//////////////////////////////////////////////////////////////////////////
//
// Pixel ������ y pitch ���� ��ȯ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
double CInspection::getPitchYPixel()
{
	return m_maskInfo.iPitchY;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixel���� um ������ �ٲ㼭 ��ȯ�Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
double CInspection::getPixelToMirco(double dPixel)
{
	return dPixel*m_dResolution;
}



//////////////////////////////////////////////////////////////////////////
//
// ������ �߽ɿ� mask�� ������ ��ġ���� �ʱ� ���� mask �̵����� ����Ѵ�.
//
//	- ccd ������ �Է¹޾� ����Ѵ�.
//	
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::checkMaskMovement( HImage& ccdImage, HTuple *dxMask, HTuple *dyMask )
{
	// Parameter
	int thresholdHoleRegion = 100;


	//
	// ó�� ���� ����
	//
	// ���� ũ��
	Hlong imageWidth, imageHeight;
	ccdImage.GetImageSize(&imageWidth, &imageHeight);

	// Red ���� �̹��� ����
	HImage imageR, imageG, imageB;
	imageB = ccdImage.Decompose3(&imageG, &imageR );

	DISPLAY(imageR);


	//
	// Hole ã��
	//
	// ���� ������ ����
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
		OutputDebugString("checkMaskMovement : ����1");
		return false;
	}

	DISPLAY(imageR, rgHoles);


	// ���� ��� hole ã��
	HTuple rowHoles, colHoles;
	rgHoles.AreaCenter( &rowHoles, &colHoles );	// Hole���� �߽� ��ǥ ȹ��

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
			// �ּҰŸ� ����
			minDistance = tempDistance;	

			// ��� hole ��ġ ����
			rowHole = rowHoles[i];
			colHole = colHoles[i];

			// 
			index = i;
			bValid = true;
		}
	}

	if( !bValid )
	{
		OutputDebugString("checkMaskMovement : ����2");
		return false;
	}

	DISPLAY(rgHoles.SelectObj(index+1));


	//
	// Mask �̵��� ����ϱ�
	//
	double rowTarget;
	double colTarget;
	double dRow = abs( rowImageCenter - rowHole );
	double dCol = abs( colImageCenter - colHole );


	int iArrage = m_maskInfo.arrangement;
	switch( iArrage )
	{
	case DIAMOND_ARRANGEMENT:
		// ������ ��ǥ��ġ�� �������� �¿����� �����Ͽ� ó���Ѵ�.
		if( dRow > dCol )	// ���� hole�� ���
		{
			// Row
			if( rowHole>rowImageCenter )
			{
				rowTarget = rowImageCenter + getPitchYPixel()/2;
			} else {
				rowTarget = rowImageCenter - getPitchYPixel()/2;
			}

			// Col : X��ǥ�� �̹��� �߽�
			colTarget = colImageCenter;
		} else {	// �¿� hole�� ���
			// Row : Y ��ǥ�� �̹��� �߽�
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
		OutputDebugString("checkMaskMovement : ����3");
		return false;
	}


	// ����� ���
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
// ������ �߽ɿ� mask�� ������ ��ġ���� �ʱ� ���� mask �̵����� ����Ѵ�.
//
//	- DIC ������ �Է¹޾� ����Ѵ�.
//	
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::checkMaskMovementDic( HImage& dicImage, HTuple *dxMask, HTuple *dyMask )
{
	// Parameter
	int thresholdHoleRegion = 50;	// @�̳���


	int iShape = m_maskInfo.shape;


	//
	// ó�� ���� ����
	//
	// ���� ũ��
	Hlong imageWidth, imageHeight;
	dicImage.GetImageSize(&imageWidth, &imageHeight);


	//
	// Hole ã��
	//
	// ���� ������ ����
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
		OutputDebugString("checkMaskMovement : ����1");
		return false;
	}

	DISPLAY(dicImage, rgHoles);


	// ���� ��� hole ã��
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
	// 	rgHoles.AreaCenter( &rowHoles, &colHoles );	// Hole���� �߽� ��ǥ ȹ��
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
	// 			// �ּҰŸ� ����
	// 			minDistance = tempDistance;	
	// 
	// 			// ��� hole ��ġ ����
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
	// 		OutputDebugString("checkMaskMovement : ����2");
	// 		return false;
	// 	}

	// 	DISPLAY(rgHoles.SelectObj(index+1));


	//
	// Mask �̵��� ����ϱ�
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
			// ������ ��ǥ��ġ�� �������� �¿����� �����Ͽ� ó���Ѵ�.
			if( dRow > dCol )	// ���� hole�� ���
			{
				// 			// Row
				// 			if( rowHole>rowImageCenter )
				// 			{
				// 				rowTarget = rowImageCenter + getPitchYPixel()/2;
				// 			} else {
				// 				rowTarget = rowImageCenter - getPitchYPixel()/2;
				// 			}
				// 
				// 			// Col : X��ǥ�� �̹��� �߽�
				// 			colTarget = colImageCenter;
				// Row
				if( roiHoleRegion.Row()>dTargetPosY )
				{
					rowTarget = dTargetPosY + getPitchYPixel()/2;
				} else {
					rowTarget = dTargetPosY - getPitchYPixel()/2;
				}

				// Col : X��ǥ�� �̹��� �߽�
				colTarget = dTargetPosX;
			} else {	// �¿� hole�� ���
				// 			// Row : Y ��ǥ�� �̹��� �߽�
				// 			rowTarget = rowImageCenter;
				// 
				// 			// Col
				// 			if( colHole>colImageCenter )
				// 			{
				// 				colTarget = colImageCenter + getPitchXPixel()/2;
				// 			} else {
				// 				colTarget = colImageCenter - getPitchXPixel()/2;
				// 			}

				// Row : Y ��ǥ�� �̹��� �߽�
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
			OutputDebugString("checkMaskMovement : ����3");
			return false;
		}
		break;

	case HOLE_SHAPE_ELIPSE:
		rowTarget = 340;
		colTarget = 670;
		break;

	}


	// ����� ���
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
//	Mask ������ width ����
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


	// �߽ɿ� ���� ����� ���� 1�� ����
	HTuple roiHoleIndex;
	findNearRegionIndex(holeRegions, imageHeight/2, imageWidth/2, &roiHoleIndex );

	HRegion roiHoleRegion = holeRegions.SelectObj(roiHoleIndex+1);
	DISPLAY(laserImage, roiHoleRegion);


	// ������ �� ���� ����
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
//	Mask ������ width ����
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


	// �߽ɿ� ���� ����� ���� 1�� ����
	HTuple roiHoleIndex;
	findNearRegionIndex(holeRegions, imageHeight/2, imageWidth/2, &roiHoleIndex );

	HRegion roiHoleRegion = holeRegions.SelectObj(roiHoleIndex+1);
	DISPLAY(laserImage, roiHoleRegion);


	// ������ �� ���� ����
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
// Hole edge ������ ���̿� ��ġ�� ã�´�.
//
//	- ������ hole ����(�Ѱ�) ������ �Է� �޴´�.
//	- Hole ������ edge �������� direction(��/��/��/��) ���⿡�� edge�� ã��, �ش� ��ġ�� 3D(����) ���� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findHoleEdge( HImage& laserImage, HImage& heightImage, HImage& dicImage, HRegion& holeRegion, HTuple width, HTuple height, HTuple *result_height, HTuple *result_row, HTuple *result_col, int direction )
{
	// Parameter
	int halfMargin = 10;	// �𼭸��� �������� 2N + 1���� edge data�� �����Ѵ�.


	// 1. Edge ������ ã�´�.
	// - Morphology�� �̿��Ͽ� edge ���� ����
	HRegion rgDilation = holeRegion.DilationCircle(1.5);
	HRegion rgHoleEdge = rgDilation.Difference(holeRegion);

	DISPLAY(laserImage, rgHoleEdge);


	// 2. Edge �߿��� �𼭸� ������ ã�´�.
	// - ���� : ������ ������ ����
	HTuple tpEdgeRows, tpEdgeCols;
	rgHoleEdge.GetRegionPoints( &tpEdgeRows, &tpEdgeCols );

	if( direction==0 )	// ���� ����
	{
		// ���� corner point�� ã�´�.
		HTuple tpLeftCornerRows;
		HTuple tpLeftCornerCol = tpEdgeCols.TupleMin();	// ���� ���� x
		HTuple tpLeftCornerIndices = tpEdgeCols.TupleFind(tpLeftCornerCol);	// 
		for( int i=0 ; i<tpLeftCornerIndices.Length() ; i++ )
		{
			tpLeftCornerRows.Append(tpEdgeRows.TupleSelect(tpLeftCornerIndices[i]));
		}
		double tpLeftCornerRow = tpLeftCornerRows.TupleMean();


		// corner point�� �������� �ֺ� edge�� ����
		HRegion searchRegion = HRegion( double(tpLeftCornerRow-halfMargin), double(tpLeftCornerCol-halfMargin), double(tpLeftCornerRow+halfMargin), double(tpLeftCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge �������� ���� ���� ȹ��
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge ������ ��� ����

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpLeftCornerCol;
		*result_row = tpLeftCornerRow;
	} 
	else if( direction==1 )	// ������ ����
	{
		// ������ corner point�� ã�´�.
		HTuple tpRightCornerRows;
		HTuple tpRightCornerCol = tpEdgeCols.TupleMax();	// ���� ���� x
		HTuple tpRightCornerIndices = tpEdgeCols.TupleFind(tpRightCornerCol);	// 
		for( int i=0 ; i<tpRightCornerIndices.Length() ; i++ )
		{
			tpRightCornerRows.Append(tpEdgeRows.TupleSelect(tpRightCornerIndices[i]));
		}
		double tpRightCornerRow = tpRightCornerRows.TupleMean();


		// corner point�� �������� �ֺ� edge�� ����
		HRegion searchRegion = HRegion( double(tpRightCornerRow-halfMargin), double(tpRightCornerCol-halfMargin), double(tpRightCornerRow+halfMargin), double(tpRightCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge �������� ���� ���� ȹ��
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge ������ ��� ����

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpRightCornerCol;
		*result_row = tpRightCornerRow;
	}
	else if( direction==2 )	// ���� ����
	{
		// ���� corner point�� ã�´�.
		HTuple tpTopCornerRow = tpEdgeRows.TupleMin();
		HTuple tpTopCornerCols;	// ���� ���� x
		HTuple tpTopCornerIndices = tpEdgeRows.TupleFind(tpTopCornerRow);	// 
		for( int i=0 ; i<tpTopCornerIndices.Length() ; i++ )
		{
			tpTopCornerCols.Append(tpEdgeCols.TupleSelect(tpTopCornerIndices[i]));
		}
		double tpTopCornerCol = tpTopCornerCols.TupleMean();


		// corner point�� �������� �ֺ� edge�� ����
		HRegion searchRegion = HRegion( double(tpTopCornerRow-halfMargin), double(tpTopCornerCol-halfMargin), double(tpTopCornerRow+halfMargin), double(tpTopCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge �������� ���� ���� ȹ��
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge ������ ��� ����

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpTopCornerCol;
		*result_row = tpTopCornerRow;
	}
	else if( direction==3 )	// �Ʒ��� ����
	{
		// ���� corner point�� ã�´�.
		HTuple tpBottomCornerRow = tpEdgeRows.TupleMax();
		HTuple tpBottomCornerCols;	// ���� ���� x
		HTuple tpBottomCornerIndices = tpEdgeRows.TupleFind(tpBottomCornerRow);	// 
		for( int i=0 ; i<tpBottomCornerIndices.Length() ; i++ )
		{
			tpBottomCornerCols.Append(tpEdgeCols.TupleSelect(tpBottomCornerIndices[i]));
		}
		double tpBottomCornerCol = tpBottomCornerCols.TupleMean();


		// corner point�� �������� �ֺ� edge�� ����
		HRegion searchRegion = HRegion( double(tpBottomCornerRow-halfMargin), double(tpBottomCornerCol-halfMargin), double(tpBottomCornerRow+halfMargin), double(tpBottomCornerCol+halfMargin) );
		HRegion rgRoiEdgeRegion = searchRegion.Intersection(rgHoleEdge);

		HTuple tpRoiEdgeRows, tpRoiEdgeCols;
		rgRoiEdgeRegion.GetRegionPoints(&tpRoiEdgeRows, &tpRoiEdgeCols);
		HTuple tpEdgeHeights = heightImage.GetGrayval( tpRoiEdgeRows, tpRoiEdgeCols );	// Edge �������� ���� ���� ȹ��
		double dEdgeHeight = tpEdgeHeights.TupleMean();					// Edge ������ ��� ����

		DISPLAY(laserImage, rgRoiEdgeRegion);

		*result_height = dEdgeHeight;
		*result_col = tpBottomCornerCol;
		*result_row = tpBottomCornerRow;
	}



	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// �־��� ROI ���������� island(bridge) ������ ã�´�.
//
//	- Island�� ���̰� ���� ���ٶ�� �����ϰ�, ���̰� ���� ���� ������ �����Ѵ�.
//	- 
//
// �־��� ���� ������ peak(���� ���� ����)�� ã��, �ش� ���̸� �������� threshod �� ���� ū blob�� ã�´�.
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


	// ������ gray �� 
	double minHeight, maxHeight, rangeHeight;
// 	heightImage.MinMaxGray(rgRoi, 0, &minHeight, &maxHeight, &rangeHeight);

	HImage roiImageSmooth = heightImage.MedianImage("circle", 2, "mirrored");	// ������ ����
	roiImageSmooth.MinMaxGray(rgRoi, 0, &minHeight, &maxHeight, &rangeHeight);


	// ROI ����
	HImage roiImage = heightImage.ReduceDomain(rgRoi);


	// Peak(���� ���� ��ġ) ���� �������� threshold
	HRegion vallyRegion = roiImage.Threshold( minHeight - dLowOffset, minHeight + dHighOffset );

	// 	DISPLAY(heightImage, vallyRegion);


	// Blob
	HRegion vallyRegions = vallyRegion.Connection();

	if( vallyRegions.CountObj()<1 )
	{
		return false;
	}

	// ���� ū region(peak) ����
	HTuple vallAreas = vallyRegions.Area();
	HTuple maxIndex = vallAreas.TupleSortIndex();
	HRegion realPeakRegion = vallyRegions.SelectObj( maxIndex[vallAreas.Length()-1] + 1 );

	DISPLAY(heightImage, realPeakRegion);


	// Island ������ ��ǥ�� ȹ��
	HTuple realPeakRow, realPeakCol;
	realPeakRegion.GetRegionPoints( &realPeakRow, &realPeakCol );

	// Island�� �߽���
	realPeakRegion.AreaCenter(peakPosRow, peakPosCol);

	// Island ������ ���̰� ȹ��
	HTuple grayValues = heightImage.GetGrayval(realPeakRow, realPeakCol );

	*dPeakHeight = grayValues.TupleMean();

	// �߰� : slope angle ���� ��ǥ
	double dCol_IslandEdge = *peakPosCol;		// �����߽��� col
	int iRow_IslandEdge = (int)(*peakPosRow);	// ���� �߽��� row
	for( int i=0 ; i<realPeakRow.Length() ; i++ )	// ��� ���� point�� ���ؼ�...
	{
		double dColTemp = realPeakCol[i];	// col ��ǥ
		int iRowTemp = realPeakRow[i];	// row ��ǥ
		if( iRowTemp == iRow_IslandEdge )	// Row ��ǥ�� �����߽ɰ� �����ϸ�...
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
// �־��� ���� ������ peak(���� ���� ����)�� ã��, �ش� ���̸� �������� threshod �� ���� ū blob�� ã�´�.
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


	// ������ gray �� 
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgRoi, 0, &minHeight, &maxHeight, &rangeHeight);


	// ROI ����
	HImage roiImage = heightImage.ReduceDomain(rgRoi);


	// Peak(���� ���� ��ġ) ���� �������� threshold
	HRegion vallyRegion = roiImage.Threshold( minHeight - dLowOffset, minHeight + dHighOffset );

	// 	DISPLAY(heightImage, vallyRegion);


	// Blob
	HRegion vallyRegions = vallyRegion.Connection();

	if( vallyRegions.CountObj()<1 )
	{
		return false;
	}

	// ���� ū region(peak) ����
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


	// ��� region ����
	resultRegion = realPeakRegion;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Target ��ġ�� ���� ����� ������ ���� index�� �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findNearRegionIndex( HRegion regions, HTuple rowTarget, HTuple colTarget, HTuple *index )
{
	// Region���� �߽���ǥ ȹ��
	HTuple rowsRegions, colsRegions;
	regions.AreaCenter( &rowsRegions, &colsRegions );	


	double rowHole=0, colHole=0;
	double minDistance = DBL_MAX;

	bool bValid = false;

	for( int i=0 ; i<regions.CountObj() ; i++ )
	{
		// Target ��ġ�� �Ÿ� ���
		HTuple distance;
		DistancePp(rowsRegions[i], colsRegions[i], rowTarget, colTarget, &distance );


		// �ּ� �Ÿ����� Ȯ��
		if( distance<minDistance )
		{
			// �ּҰŸ� ����
			minDistance = distance;	

			// index ����
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
// Hole ������ ã�´�.
//
//	1) Laser ���󿡼� ������ ã�´�.
//		. ������ ��Ӵ�.
//
//	2) Height ���󿡼� ������ ã�´�.
//		. ������ island ���� ����.
//
//	3) 1)�� 2)�� �������� ���Ѵ�.
//		. ���� �ĺ������� �����Ѵ�.
//
//	4) ���� ������ ������ �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::findHoleRegion( HImage& laserImage, HImage& heightImage, HRegion& rgnIsland, HRegion& resultRegion)
{
	HTuple imageWidth, imageHeight;
	heightImage.GetImageSize(&imageWidth, &imageHeight);

	HRegion rgAll;
	rgAll.GenRectangle1(HTuple(0), HTuple(0), imageHeight-1, imageWidth -1 );


	//
	// Laser ���󿡼� ���� ������ ã�´�.
	//

	// 1. Threshold : Laser image
	double minLaser, maxLaser, rangeLaser;
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// ��Ⱚ ������ ��´�.
	double thresholdLaser = minLaser+10;							// ���� ��⸦ �������� threshold���� �����Ѵ�.

	HRegion rgHoleCandidate1 = laserImage.Threshold( 0, thresholdLaser ).OpeningCircle(3.5).ClosingCircle(3.5);

	DISPLAY(laserImage, rgHoleCandidate1, "findHoleRegion > threshold laser image");


// 	// 2. ������ ����
// 	HRegion rgHoleCandidates = rgThresholdHole.OpeningCircle(3.5).ClosingCircle(3.5).Connection();
// 
// 	DISPLAY(laserImage, rgHoleCandidates, "findHoleRegion > �ĺ� ���� @ laser image");



	//
	// Height ���󿡼� ���� ������ ã�´�.
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

	DISPLAY(laserImage, rgHolesCandidate, "findHoleRegion > ���� �ĺ�");


	//
	// ��ȿ ���� ����
	//
	HTuple hv_Feature, hv_min, hv_max;

	// �� ���� ����
	hv_Feature[0] = "width";
	hv_min[0] = m_maskInfo.iWidth*0.8;
	hv_max[0] = m_maskInfo.iWidth*1.3;

	// ���� ���� ����
	hv_Feature[1] = "height";
	hv_min[1] = m_maskInfo.iHeight*0.8;
	hv_max[1] = m_maskInfo.iHeight*1.3;

	// ���� ���� ����
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

	// ���� ���� ����
	//	������ ���ۿ� ���� ���ܻ���
	if( m_maskInfo.shape == HOLE_SHAPE_ELIPSE_45 )
	{
		hv_Feature[3] = "rect2_phi";
		hv_min[3] = -3.14/2.0 * (3.0/4.0);	// -90�� * (3/4)
		hv_max[3] = -3.14/2.0 * (1.0/4.0);	// -90�� * (1/4)
	}


	// filtering
	resultRegion = rgHolesCandidate.SelectShape( hv_Feature, "and", hv_min, hv_max );

	if( resultRegion.CountObj()<1 )
	{
		return false;
	}


	DISPLAY(resultRegion, "findHoleRegion > ��ȿ�� ���۵�");


	// 
// 	HTuple indexRoiHole;
// 	findNearRegionIndex( rgHolesValid, imageHeight/2, imageWidth/2, &indexRoiHole);
// 	resultRegion = rgHolesValid.SelectObj(indexRoiHole+1);
// 
// 	DISPLAY(resultRegion, "findHoleRegion > ���� ���� ����");


	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Island ������ ã�´�.
//
//	1) Laser image���� ���� ������ ã�´�.
//		. Island�� ��ź�ؼ� laser ���󿡼� ���� ��� ��Ÿ����.
//
//	2) Height image���� ���� ������ ã�´�.
//		. Island�� �÷ῡ�� ���� ��ġ�� ����.
//
//	3) 1)�� 2)�� �������� island �������� �����Ѵ�.
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
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// ��Ⱚ ������ ��´�.

	double thresholdLaser = (maxLaser+2*minLaser)/3;							// ���� ��⸦ �������� threshold���� �����Ѵ�.
	HRegion rgThresholdLaser = laserImage.Threshold( thresholdLaser, 255 ).ClosingCircle(3.5);	// island ������ ã������ threshold�� �Ѵ�.

	DISPLAY(laserImage, rgThresholdLaser, "findIslandRegion > threshold laser image"); 


	//////////////////////////////////////////////////////////////////////////
	// 1.2 Threshold : height image
	//////////////////////////////////////////////////////////////////////////
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgThresholdLaser, 10, &minHeight, &maxHeight, &rangeHeight);	// ���̰� ���� ȹ��

	HRegion rgThresholdHeight = heightImage.Threshold( minHeight-5, minHeight+5 ).ClosingCircle(3.5);	// island ������ ã������ threshold�� �Ѵ�.(3D�������� ���� ���� �۴�)

	DISPLAY(laserImage, rgThresholdHeight, "findIslandRegion > threshold height map");


	//////////////////////////////////////////////////////////////////////////
	// 1.3 intersection : Laser & height
	//////////////////////////////////////////////////////////////////////////
	//resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight).FillUp();	// ���� ������ ä��� island�� ���� �̾��� �ִ� ��� ������ ����
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
// Island ������ ã�´�.
//
//	1) Laser image���� ���� ������ ã�´�.
//		. Island�� ��ź�ؼ� laser ���󿡼� ���� ��� ��Ÿ����.
//
//	2) Height image���� ���� ������ ã�´�.
//		. Island�� �÷ῡ�� ���� ��ġ�� ����.
//
//	3) 1)�� 2)�� �������� island �������� �����Ѵ�.
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
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// ��Ⱚ ������ ��´�.

	double thresholdLaser = (maxLaser+2*minLaser)/3;							// ���� ��⸦ �������� threshold���� �����Ѵ�.
	HRegion rgThresholdLaser = laserImage.Threshold( thresholdLaser, 255 ).ClosingCircle(3.5);	// island ������ ã������ threshold�� �Ѵ�.

	DISPLAY(laserImage, rgThresholdLaser, "findIslandRegion > threshold laser image"); 


	//////////////////////////////////////////////////////////////////////////
	// 1.2 Threshold : height image
	//////////////////////////////////////////////////////////////////////////
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgAll, 0, &minHeight, &maxHeight, &rangeHeight);	// ���̰� ���� ȹ��

	HRegion rgThresholdHeight = heightImage.Threshold( minHeight-5, minHeight+5 ).ClosingCircle(3.5);	// island ������ ã������ threshold�� �Ѵ�.(3D�������� ���� ���� �۴�)

	DISPLAY(laserImage, rgThresholdHeight, "findIslandRegion > threshold height map");


	//////////////////////////////////////////////////////////////////////////
	// 1.3 intersection : Laser & height
	//////////////////////////////////////////////////////////////////////////
	//resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight).FillUp();	// ���� ������ ä��� island�� ���� �̾��� �ִ� ��� ������ ����
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
// Island ������ ã�´�.
//
//	1) Laser image���� ���� ������ ã�´�.
//		. Island�� ��ź�ؼ� laser ���󿡼� ���� ��� ��Ÿ����.
//
//	2) Height image���� ���� ������ ã�´�.
//		. Island�� �÷ῡ�� ���� ��ġ�� ����.
//
//	3) 1)�� 2)�� �������� island �������� �����Ѵ�.
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
	laserImage.MinMaxGray(rgAll, 5, &minLaser, &maxLaser, &rangeLaser);	// ��Ⱚ ������ ��´�.

	double thresholdLaser = (maxLaser+2*minLaser)/3;							// ���� ��⸦ �������� threshold���� �����Ѵ�.
	HRegion rgThresholdLaser = laserImage.Threshold( thresholdLaser, 255 ).ClosingCircle(1.5);	// island ������ ã������ threshold�� �Ѵ�.

	DISPLAY(laserImage, rgThresholdLaser, "findIslandRegion > threshold laser image"); 


	//////////////////////////////////////////////////////////////////////////
	// 1.2 Threshold : height image
	//////////////////////////////////////////////////////////////////////////
	double minHeight, maxHeight, rangeHeight;
	heightImage.MinMaxGray(rgThresholdLaser, 5, &minHeight, &maxHeight, &rangeHeight);	// ���̰� ���� ȹ��

	HRegion rgThresholdHeight = heightImage.Threshold( minHeight-30, minHeight+10 ).ClosingCircle(1.5);	// island ������ ã������ threshold�� �Ѵ�.(3D�������� ���� ���� �۴�)

	DISPLAY(laserImage, rgThresholdHeight, "findIslandRegion > threshold height map");


	//////////////////////////////////////////////////////////////////////////
	// 1.3 intersection : Laser & height
	//////////////////////////////////////////////////////////////////////////
	//resultRegion = rgThresholdLaser.Intersection(rgThresholdHeight).FillUp();	// ���� ������ ä��� island�� ���� �̾��� �ִ� ��� ������ ����
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
// Hole ������ ������ �����ϰ�, �����Ѵ�.
//
//////////////////////////////////////////////////////////////////////////
bool CInspection::extractHoleInfo( HRegion holeRegion, double *pWidth )
{
	HTuple rowHole, colHole, radiusHole ;
	holeRegion.SmallestCircle(&rowHole, &colHole, &radiusHole);


	for( int i=0 ; i<4 ; i++ )
	{
		// �˻� region ����
		HTuple searchAngle = i*PI/4;	// ����

		HRegion searchRegion;	// �˻� region
		searchRegion.GenRectangle2(rowHole, colHole, searchAngle, radiusHole*HTuple(2), HTuple(1) );

		DISPLAY(holeRegion);
		DISPLAY(searchRegion);


		// ROI
		HRegion roiRegion = searchRegion.Intersection(holeRegion);


		// �������� ���� ����
		HTuple rowsRoi, colsRoi;
		roiRegion.GetRegionPoints(&rowsRoi, &colsRoi);
		HTuple nRoi = rowsRoi.Length();

		HTuple rowStart = rowsRoi[0];
		HTuple colStart = colsRoi[0];
		HTuple rowEnd = rowsRoi[nRoi-1];
		HTuple colEnd = colsRoi[nRoi-1];


		// �� ���
		HTuple distance;
		DistancePp( rowStart, colStart, rowEnd, colEnd, &distance );
		double dDist = distance;

		// ��� ����
		pWidth[i]		= dDist;
		// 		m_holeInfo.iWidthMask[i]		= dDist;
		// 		m_holeInfo.dRealWidthMask[i]	= dDist*m_dResolution;
	}


	return true;
}