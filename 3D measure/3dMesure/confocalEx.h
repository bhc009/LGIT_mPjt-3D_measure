#include <windows.h>
#include <tchar.h> // _T, _tprintf
// MEDAQLib.h must be included to know it's functions
#include "MeDaqLib.h"

#pragma once

class confocalEx
{
public:
	confocalEx(void);
	~confocalEx(void);
	int LoadDLLFunctions();
	int PrintError (LPCTSTR err);
	int Error (ERR_CODE err, int sensor);
	int Error (LPCTSTR err, int sensor);
	int Open (DWORD sensor);
	int GetInfo (DWORD sensor);
	int Process (int sensor);
	void Cleanup (int sensor);
	CString ConvertMultibyteToUnicode(char* pMultibyte);
	DWORD m_sensor;
	int m_err;
	CString m_errStr;
	CString m_infoRangeStr;
	CString m_infoSamplerateStr;
	double m_data[1024];
	//CString m_errStr;
};

