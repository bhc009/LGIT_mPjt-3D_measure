#pragma once

#include "Inspection.h"
#define DLG_RECIPE_MAX_SIZE	24

// CDialogRecipe 대화 상자입니다.

class CDialogRecipe : public CDialogEx
{
	DECLARE_DYNAMIC(CDialogRecipe)

public:
	CDialogRecipe(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDialogRecipe();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_RECIPE };

	BOOL init( PARAM_MASK_INFO *pInfo );

	int getSize(void);
	int getData( double *pAngle, BOOL *pIsIsland );

protected:
	int m_nData;

	BOOL *getIsUse(int i);
	BOOL *getIsIsland(int i);
	double *getAngle(int i);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bUse1;
	BOOL m_bUse2;
	BOOL m_bUse3;
	BOOL m_bUse4;
	BOOL m_bUse5;
	BOOL m_bUse6;
	BOOL m_bUse7;
	BOOL m_bUse8;
	BOOL m_bUse9;
	BOOL m_bUse10;
	BOOL m_bUse11;
	BOOL m_bUse12;
	BOOL m_bUse13;
	BOOL m_bUse14;
	BOOL m_bUse15;
	BOOL m_bUse16;
	BOOL m_bUse17;
	BOOL m_bUse18;
	BOOL m_bUse19;
	BOOL m_bUse20;
	BOOL m_bUse21;
	BOOL m_bUse22;
	BOOL m_bUse23;
	BOOL m_bUse24;
	BOOL m_bIsland1;
	BOOL m_bIsland2;
	BOOL m_bIsland3;
	BOOL m_bIsland4;
	BOOL m_bIsland5;
	BOOL m_bIsland6;
	BOOL m_bIsland7;
	BOOL m_bIsland8;
	BOOL m_bIsland9;
	BOOL m_bIsland10;
	BOOL m_bIsland11;
	BOOL m_bIsland12;
	BOOL m_bIsland13;
	BOOL m_bIsland14;
	BOOL m_bIsland15;
	BOOL m_bIsland16;
	BOOL m_bIsland17;
	BOOL m_bIsland18;
	BOOL m_bIsland19;
	BOOL m_bIsland20;
	BOOL m_bIsland21;
	BOOL m_bIsland22;
	BOOL m_bIsland23;
	BOOL m_bIsland24;
	double m_dAngle1;
	double m_dAngle2;
	double m_dAngle3;
	double m_dAngle4;
	double m_dAngle5;
	double m_dAngle6;
	double m_dAngle7;
	double m_dAngle8;
	double m_dAngle9;
	double m_dAngle10;
	double m_dAngle11;
	double m_dAngle12;
	double m_dAngle13;
	double m_dAngle14;
	double m_dAngle15;
	double m_dAngle16;
	double m_dAngle17;
	double m_dAngle18;
	double m_dAngle19;
	double m_dAngle20;
	double m_dAngle21;
	double m_dAngle22;
	double m_dAngle23;
	double m_dAngle24;
};
