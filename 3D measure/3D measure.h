
// 3D measure.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CMy3DmeasureApp:
// �� Ŭ������ ������ ���ؼ��� 3D measure.cpp�� �����Ͻʽÿ�.
//

class CMy3DmeasureApp : public CWinApp
{
public:
	CMy3DmeasureApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CMy3DmeasureApp theApp;