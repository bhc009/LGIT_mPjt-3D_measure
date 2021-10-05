/*
저작권 : LG전자 생산기술원
*/

/*
[20161207-최동진]
*/


//
#include "stdafx.h"
#include "djThreadObj.h"

CdjThreadObj::CdjThreadObj()
{
	m_hThread = NULL;
	m_bInitThread = FALSE;
	m_bExitThread = FALSE;

	//
	m_nSeqLoopSleepTime = 1;

	//
	m_pOnThreadFunc = NULL;
	m_pOnThreadStartFunc = NULL;
	m_pOnThreadEndFunc = NULL;
}

CdjThreadObj::~CdjThreadObj()
{
	if (IsInitialized() == TRUE)
		Finalize();
}

INT CdjThreadObj::Initialize(_DJ_THREAD_OBJECT_FUNC_ pOnThreadFunc, _DJ_THREAD_OBJECT_FUNC_ pOnThreadStartFunc, _DJ_THREAD_OBJECT_FUNC_ pOnThreadEndFunc, LPVOID pUserObj, DWORD nSeqLoopSleepTime)
{
	//
	CSingleLock lock(&m_Lock, TRUE);

	//
	if (pOnThreadFunc == NULL)
	{
		ASSERT(FALSE);

		return 2;
	}

	//
	if (IsInitialized() == TRUE)
	{
		ASSERT(FALSE);

		return 1;
	}

	//
	SetSeqLoopSleepTime(nSeqLoopSleepTime);

	//
	m_pOnThreadFunc = pOnThreadFunc;
	m_pOnThreadStartFunc = pOnThreadStartFunc;
	m_pOnThreadEndFunc = pOnThreadEndFunc;
	m_pUserObj = pUserObj;

	//
	m_hThread = NULL;
	m_bInitThread = FALSE;
	m_bExitThread = FALSE;

	m_hThread = ::CreateThread(NULL, 0, CdjThreadObj::OnThreadProcWrapper, this, 0, NULL);

	if (m_hThread != NULL)
		return 0;
	else
		return 1;
}

INT CdjThreadObj::Finalize()
{
	return Finalize(TRUE);
}

INT CdjThreadObj::Finalize(BOOL bWaitThreadKill)
{
	//
	if (IsInitialized() == FALSE)
		return 0;

	//
	m_bExitThread = TRUE;

	//
	if (bWaitThreadKill == TRUE)
	{
		while (m_hThread != NULL)
		{
			if (WaitForSingleObject(m_hThread, 100) == WAIT_OBJECT_0)
				m_hThread = NULL;
		}

		return 0;
	}
	else
	{
		return 0;
	}
}

BOOL CdjThreadObj::IsInitialized() const
{
	return m_bInitThread;
}

DWORD WINAPI CdjThreadObj::OnThreadProcWrapper(LPVOID pObject)
{
	//
	CdjThreadObj * pThread = (CdjThreadObj *)pObject;

	//
	pThread->m_bInitThread = TRUE;

	//
	if (pThread->m_pOnThreadStartFunc != NULL)
		pThread->m_pOnThreadStartFunc(pThread->m_pUserObj);

	while (pThread->m_bExitThread == FALSE)
	{
		if (pThread->m_nSeqLoopSleepTime > 0)
			::Sleep(pThread->m_nSeqLoopSleepTime);

		if (pThread->m_pOnThreadFunc != NULL)
			pThread->m_pOnThreadFunc(pThread->m_pUserObj);
	}

	//
	if (pThread->m_pOnThreadEndFunc != NULL)
		pThread->m_pOnThreadEndFunc(pThread->m_pUserObj);

	//
	pThread->m_bInitThread = FALSE;

	//
	return 0;
}


