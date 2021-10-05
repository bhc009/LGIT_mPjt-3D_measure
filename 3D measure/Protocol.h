#pragma once


#define BHPROTOCOL_FAIL	-1
#define BHPROTOCOL_STATE	0
#define BHPROTOCOL_RECIPE	1
#define BHPROTOCOL_CALIBRATION	2
#define BHPROTOCOL_MESURE_LARGE	3
#define BHPROTOCOL_MESURE_SMALL	4
#define BHPROTOCOL_LENS10X	5
#define BHPROTOCOL_AUTOFOCUS	6
#define BHPROTOCOL_THICKNESS	7
#define BHPROTOCOL_STOP	8


#define BHPROTOCOL_LGD	0	
#define BHPROTOCOL_LGIT	1	


class CProtocol
{
public:
	CProtocol(void);
	~CProtocol(void);

	void setSite( int i );

	int Decode(unsigned char *pChar, int nLength);

	int EncodeState();
	int EncodeRecipe(bool bSuccess);
	int EncodeCalibration(bool bSuccess);
	int EncodeMeasureLarge( bool bSuccess, double dStepHeight, double dSlopeAngle );
	int EncodeMeasureSmall( bool bSuccess, double dStepHeight, double dSlopeAngle );
	int EncodeLens10x(bool bSuccess);
	int EncodeAutofocus( bool bSuccess, double dDx, double dDy );

	//
	double m_dPitchX;
	double m_dPitchY;
	double m_dSizeX;
	double m_dSizeY;
	int m_iShape;
	int m_iArrage;
	double m_dThickness;

	// thickness
	BOOL m_bCAL;
	int m_thicknessIndex;

	//
	CString m_strRecipe;
	CString m_strQRCode;

	BOOL m_bUseThickness;
	double m_d3dRangeUp;
	double m_d3dRangDown;

	int m_iSite;

	CString m_strMessage;
};

