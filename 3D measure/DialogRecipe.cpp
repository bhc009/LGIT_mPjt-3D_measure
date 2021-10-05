// DialogRecipe.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "3D measure.h"
#include "DialogRecipe.h"
#include "afxdialogex.h"


// CDialogRecipe 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDialogRecipe, CDialogEx)

CDialogRecipe::CDialogRecipe(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDialogRecipe::IDD, pParent)
	, m_bUse1(TRUE)
	, m_bUse2(TRUE)
	, m_bUse3(TRUE)
	, m_bUse4(TRUE)
	, m_bUse5(TRUE)
	, m_bUse6(TRUE)
	, m_bUse7(TRUE)
	, m_bUse8(TRUE)
	, m_bUse9(TRUE)
	, m_bUse10(TRUE)
	, m_bUse11(TRUE)
	, m_bUse12(TRUE)
	, m_bUse13(TRUE)
	, m_bUse14(TRUE)
	, m_bUse15(TRUE)
	, m_bUse16(TRUE)
	, m_bUse17(TRUE)
	, m_bUse18(TRUE)
	, m_bUse19(TRUE)
	, m_bUse20(TRUE)
	, m_bUse21(TRUE)
	, m_bUse22(TRUE)
	, m_bUse23(TRUE)
	, m_bUse24(TRUE)
	, m_bIsland1(FALSE)
	, m_bIsland2(FALSE)
	, m_bIsland3(FALSE)
	, m_bIsland4(FALSE)
	, m_bIsland5(FALSE)
	, m_bIsland6(FALSE)
	, m_bIsland7(FALSE)
	, m_bIsland8(FALSE)
	, m_bIsland9(FALSE)
	, m_bIsland10(FALSE)
	, m_bIsland11(FALSE)
	, m_bIsland12(FALSE)
	, m_bIsland13(FALSE)
	, m_bIsland14(FALSE)
	, m_bIsland15(FALSE)
	, m_bIsland16(FALSE)
	, m_bIsland17(FALSE)
	, m_bIsland18(FALSE)
	, m_bIsland19(FALSE)
	, m_bIsland20(FALSE)
	, m_bIsland21(FALSE)
	, m_bIsland22(FALSE)
	, m_bIsland23(FALSE)
	, m_bIsland24(FALSE)
	, m_dAngle1(0)
	, m_dAngle2(0)
	, m_dAngle3(0)
	, m_dAngle4(0)
	, m_dAngle5(0)
	, m_dAngle6(0)
	, m_dAngle7(0)
	, m_dAngle8(0)
	, m_dAngle9(0)
	, m_dAngle10(0)
	, m_dAngle11(0)
	, m_dAngle12(0)
	, m_dAngle13(0)
	, m_dAngle14(0)
	, m_dAngle15(0)
	, m_dAngle16(0)
	, m_dAngle17(0)
	, m_dAngle18(0)
	, m_dAngle19(0)
	, m_dAngle20(0)
	, m_dAngle21(0)
	, m_dAngle22(0)
	, m_dAngle23(0)
	, m_dAngle24(0)
{
	m_nData = 0;
}

CDialogRecipe::~CDialogRecipe()
{
}

void CDialogRecipe::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_USE_1, m_bUse1);
	DDX_Check(pDX, IDC_CHECK_USE_2, m_bUse2);
	DDX_Check(pDX, IDC_CHECK_USE_3, m_bUse3);
	DDX_Check(pDX, IDC_CHECK_USE_4, m_bUse4);
	DDX_Check(pDX, IDC_CHECK_USE_5, m_bUse5);
	DDX_Check(pDX, IDC_CHECK_USE_6, m_bUse6);
	DDX_Check(pDX, IDC_CHECK_USE_7, m_bUse7);
	DDX_Check(pDX, IDC_CHECK_USE_8, m_bUse8);
	DDX_Check(pDX, IDC_CHECK_USE_9, m_bUse9);
	DDX_Check(pDX, IDC_CHECK_USE_10, m_bUse10);
	DDX_Check(pDX, IDC_CHECK_USE_11, m_bUse11);
	DDX_Check(pDX, IDC_CHECK_USE_12, m_bUse12);
	DDX_Check(pDX, IDC_CHECK_USE_13, m_bUse13);
	DDX_Check(pDX, IDC_CHECK_USE_14, m_bUse14);
	DDX_Check(pDX, IDC_CHECK_USE_15, m_bUse15);
	DDX_Check(pDX, IDC_CHECK_USE_16, m_bUse16);
	DDX_Check(pDX, IDC_CHECK_USE_17, m_bUse17);
	DDX_Check(pDX, IDC_CHECK_USE_18, m_bUse18);
	DDX_Check(pDX, IDC_CHECK_USE_19, m_bUse19);
	DDX_Check(pDX, IDC_CHECK_USE_20, m_bUse20);
	DDX_Check(pDX, IDC_CHECK_USE_21, m_bUse21);
	DDX_Check(pDX, IDC_CHECK_USE_22, m_bUse22);
	DDX_Check(pDX, IDC_CHECK_USE_23, m_bUse23);
	DDX_Check(pDX, IDC_CHECK_USE_24, m_bUse24);
	DDX_Check(pDX, IDC_CHECK_ISLAND_1, m_bIsland1);
	DDX_Check(pDX, IDC_CHECK_ISLAND_2, m_bIsland2);
	DDX_Check(pDX, IDC_CHECK_ISLAND_3, m_bIsland3);
	DDX_Check(pDX, IDC_CHECK_ISLAND_4, m_bIsland4);
	DDX_Check(pDX, IDC_CHECK_ISLAND_5, m_bIsland5);
	DDX_Check(pDX, IDC_CHECK_ISLAND_6, m_bIsland6);
	DDX_Check(pDX, IDC_CHECK_ISLAND_7, m_bIsland7);
	DDX_Check(pDX, IDC_CHECK_ISLAND_8, m_bIsland8);
	DDX_Check(pDX, IDC_CHECK_ISLAND_9, m_bIsland9);
	DDX_Check(pDX, IDC_CHECK_ISLAND_10, m_bIsland10);
	DDX_Check(pDX, IDC_CHECK_ISLAND_11, m_bIsland11);
	DDX_Check(pDX, IDC_CHECK_ISLAND_12, m_bIsland12);
	DDX_Check(pDX, IDC_CHECK_ISLAND_13, m_bIsland13);
	DDX_Check(pDX, IDC_CHECK_ISLAND_14, m_bIsland14);
	DDX_Check(pDX, IDC_CHECK_ISLAND_15, m_bIsland15);
	DDX_Check(pDX, IDC_CHECK_ISLAND_16, m_bIsland16);
	DDX_Check(pDX, IDC_CHECK_ISLAND_17, m_bIsland17);
	DDX_Check(pDX, IDC_CHECK_ISLAND_18, m_bIsland18);
	DDX_Check(pDX, IDC_CHECK_ISLAND_19, m_bIsland19);
	DDX_Check(pDX, IDC_CHECK_ISLAND_20, m_bIsland20);
	DDX_Check(pDX, IDC_CHECK_ISLAND_21, m_bIsland21);
	DDX_Check(pDX, IDC_CHECK_ISLAND_22, m_bIsland22);
	DDX_Check(pDX, IDC_CHECK_ISLAND_23, m_bIsland23);
	DDX_Check(pDX, IDC_CHECK_ISLAND_24, m_bIsland24);
	DDX_Text(pDX, IDC_EDIT_ANGLE_1, m_dAngle1);
	DDX_Text(pDX, IDC_EDIT_ANGLE_2, m_dAngle2);
	DDX_Text(pDX, IDC_EDIT_ANGLE_3, m_dAngle3);
	DDX_Text(pDX, IDC_EDIT_ANGLE_4, m_dAngle4);
	DDX_Text(pDX, IDC_EDIT_ANGLE_5, m_dAngle5);
	DDX_Text(pDX, IDC_EDIT_ANGLE_6, m_dAngle6);
	DDX_Text(pDX, IDC_EDIT_ANGLE_7, m_dAngle7);
	DDX_Text(pDX, IDC_EDIT_ANGLE_8, m_dAngle8);
	DDX_Text(pDX, IDC_EDIT_ANGLE_9, m_dAngle9);
	DDX_Text(pDX, IDC_EDIT_ANGLE_10, m_dAngle10);
	DDX_Text(pDX, IDC_EDIT_ANGLE_11, m_dAngle11);
	DDX_Text(pDX, IDC_EDIT_ANGLE_12, m_dAngle12);
	DDX_Text(pDX, IDC_EDIT_ANGLE_13, m_dAngle13);
	DDX_Text(pDX, IDC_EDIT_ANGLE_14, m_dAngle14);
	DDX_Text(pDX, IDC_EDIT_ANGLE_15, m_dAngle15);
	DDX_Text(pDX, IDC_EDIT_ANGLE_16, m_dAngle16);
	DDX_Text(pDX, IDC_EDIT_ANGLE_17, m_dAngle17);
	DDX_Text(pDX, IDC_EDIT_ANGLE_18, m_dAngle18);
	DDX_Text(pDX, IDC_EDIT_ANGLE_19, m_dAngle19);
	DDX_Text(pDX, IDC_EDIT_ANGLE_20, m_dAngle20);
	DDX_Text(pDX, IDC_EDIT_ANGLE_21, m_dAngle21);
	DDX_Text(pDX, IDC_EDIT_ANGLE_22, m_dAngle22);
	DDX_Text(pDX, IDC_EDIT_ANGLE_23, m_dAngle23);
	DDX_Text(pDX, IDC_EDIT_ANGLE_24, m_dAngle24);
}


BEGIN_MESSAGE_MAP(CDialogRecipe, CDialogEx)
END_MESSAGE_MAP()


// CDialogRecipe 메시지 처리기입니다.

BOOL CDialogRecipe::init( PARAM_MASK_INFO *pInfo )
{
	if( pInfo->iPositionNum > DLG_RECIPE_MAX_SIZE )
	{
		return FALSE;
	}

	m_nData = 0;

	for( int i=0 ; i<DLG_RECIPE_MAX_SIZE ; i++ )
	{
		*getIsUse(i) = FALSE;
		*getIsIsland(i) = FALSE;
		*getAngle(i) = 0.0;
	}


	for( int i=0 ; i<pInfo->iPositionNum ; i++ )
	{
		*getIsUse(i) = TRUE;
		*getIsIsland(i) = pInfo->bPositionPeak[i];
		*getAngle(i) = pInfo->dPositionAngle[i];
	}


	m_nData = pInfo->iPositionNum;

// 	UpdateData(FALSE);

	return TRUE;
}


int CDialogRecipe::getSize(void)
{
	return m_nData;
}


int CDialogRecipe::getData( double *pAngle, BOOL *pIsIsland )
{
	int nData = 0;

	for( int i=0 ; i<DLG_RECIPE_MAX_SIZE ; i++ )
	{
		if( *getIsUse(i) )
		{
			pAngle[nData] = *getAngle(i);
			pIsIsland[nData] = *getIsIsland(i);
			nData++;
		}
	}

	m_nData = nData;

	return m_nData;
}


BOOL *CDialogRecipe::getIsUse(int i)
{
	if( i==0 ) return &m_bUse1;
	if( i==1 ) return &m_bUse2;
	if( i==2 ) return &m_bUse3;
	if( i==3 ) return &m_bUse4;
	if( i==4 ) return &m_bUse5;
	if( i==5 ) return &m_bUse6;
	if( i==6 ) return &m_bUse7;
	if( i==7 ) return &m_bUse8;
	if( i==8 ) return &m_bUse9;
	if( i==9 ) return &m_bUse10;
	if( i==10 ) return &m_bUse11;
	if( i==11 ) return &m_bUse12;
	if( i==12 ) return &m_bUse13;
	if( i==13 ) return &m_bUse14;
	if( i==14 ) return &m_bUse15;
	if( i==15 ) return &m_bUse16;
	if( i==16 ) return &m_bUse17;
	if( i==17 ) return &m_bUse18;
	if( i==18 ) return &m_bUse19;
	if( i==19 ) return &m_bUse20;
	if( i==20 ) return &m_bUse21;
	if( i==21 ) return &m_bUse22;
	if( i==22 ) return &m_bUse23;
	if( i==23 ) return &m_bUse24;

	return FALSE;
}


BOOL *CDialogRecipe::getIsIsland(int i)
{
	if( i==0 ) return &m_bIsland1;
	if( i==1 ) return &m_bIsland2;
	if( i==2 ) return &m_bIsland3;
	if( i==3 ) return &m_bIsland4;
	if( i==4 ) return &m_bIsland5;
	if( i==5 ) return &m_bIsland6;
	if( i==6 ) return &m_bIsland7;
	if( i==7 ) return &m_bIsland8;
	if( i==8 ) return &m_bIsland9;
	if( i==9 ) return &m_bIsland10;
	if( i==10 ) return &m_bIsland11;
	if( i==11 ) return &m_bIsland12;
	if( i==12 ) return &m_bIsland13;
	if( i==13 ) return &m_bIsland14;
	if( i==14 ) return &m_bIsland15;
	if( i==15 ) return &m_bIsland16;
	if( i==16 ) return &m_bIsland17;
	if( i==17 ) return &m_bIsland18;
	if( i==18 ) return &m_bIsland19;
	if( i==19 ) return &m_bIsland20;
	if( i==20 ) return &m_bIsland21;
	if( i==21 ) return &m_bIsland22;
	if( i==22 ) return &m_bIsland23;
	if( i==23 ) return &m_bIsland24;

	return FALSE;
}


double *CDialogRecipe::getAngle(int i)
{
	if( i==0 ) return &m_dAngle1;
	if( i==1 ) return &m_dAngle2;
	if( i==2 ) return &m_dAngle3;
	if( i==3 ) return &m_dAngle4;
	if( i==4 ) return &m_dAngle5;
	if( i==5 ) return &m_dAngle6;
	if( i==6 ) return &m_dAngle7;
	if( i==7 ) return &m_dAngle8;
	if( i==8 ) return &m_dAngle9;
	if( i==9 ) return &m_dAngle10;
	if( i==10 ) return &m_dAngle11;
	if( i==11 ) return &m_dAngle12;
	if( i==12 ) return &m_dAngle13;
	if( i==13 ) return &m_dAngle14;
	if( i==14 ) return &m_dAngle15;
	if( i==15 ) return &m_dAngle16;
	if( i==16 ) return &m_dAngle17;
	if( i==17 ) return &m_dAngle18;
	if( i==18 ) return &m_dAngle19;
	if( i==19 ) return &m_dAngle20;
	if( i==20 ) return &m_dAngle21;
	if( i==21 ) return &m_dAngle22;
	if( i==22 ) return &m_dAngle23;
	if( i==23 ) return &m_dAngle24;

	return FALSE;
}
