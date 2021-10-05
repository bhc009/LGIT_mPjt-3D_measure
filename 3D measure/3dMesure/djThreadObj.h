/*
저작권 : LG전자 생산기술원
*/

/*
[20161207-최동진]
*/

//
#pragma once

//
// #include "stdafx.h"
#include <afxmt.h>

//
typedef DWORD(*_DJ_THREAD_OBJECT_FUNC_)(LPVOID pUserObj);

//
//class AFX_EXT_CLASS CdjThreadObj
class  CdjThreadObj
{
protected:
	//
	HANDLE m_hThread;
	BOOL m_bInitThread;
	BOOL m_bExitThread;

	//
	DWORD m_nSeqLoopSleepTime;

	//
	CCriticalSection m_Lock;

	//
	_DJ_THREAD_OBJECT_FUNC_ m_pOnThreadFunc;
	_DJ_THREAD_OBJECT_FUNC_ m_pOnThreadStartFunc;
	_DJ_THREAD_OBJECT_FUNC_ m_pOnThreadEndFunc;

	LPVOID m_pUserObj;

public: //
	CdjThreadObj();
	CdjThreadObj(const CdjThreadObj &);
	virtual ~CdjThreadObj();

	CdjThreadObj & operator=(const CdjThreadObj &);

public:
	INT Initialize(_DJ_THREAD_OBJECT_FUNC_ pOnThreadFunc, _DJ_THREAD_OBJECT_FUNC_ pOnThreadStartFunc, _DJ_THREAD_OBJECT_FUNC_ pOnThreadEndFunc, LPVOID pUserObj, DWORD nSeqLoopSleepTime);
	INT Finalize();
	INT Finalize(BOOL bWaitThreadKill);

	BOOL IsInitialized() const;

public:
	VOID SetSeqLoopSleepTime(DWORD nSeqLoopSleepTimer) { m_nSeqLoopSleepTime = nSeqLoopSleepTimer; }
	DWORD GetSeqLoopSleepTime() const { return m_nSeqLoopSleepTime; }

protected:
	static DWORD WINAPI OnThreadProcWrapper(LPVOID pObject);
};
