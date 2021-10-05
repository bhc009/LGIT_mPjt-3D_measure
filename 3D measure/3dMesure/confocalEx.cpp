#include "StdAfx.h"
#include "confocalEx.h"


#ifndef _countof
#define _countof(x) (sizeof (x)/sizeof (x[0]))
#endif // _countof

#define CHECK_ERROR(errCode) \
	if ((errCode)!=ERR_NOERROR) \
	return Error (errCode, sensor);

/******************************************************************************
                              C type definitions
Following type definitions can be used, when the MEDAQLib driver is dynamically
loaded (LoadLibrary) and the function pointers retrieved by GetProcAddress must
be assigned to functions, e.g.: 

CREATESENSORINSTANCE pCreateSensorInstance= (CREATESENSORINSTANCE)
  GetProcAddress (LoadLibrary ("MEDAQLib.dll"), "CreateSensorInstance");
******************************************************************************/
typedef DWORD    (WINAPI *CREATESENSORINSTANCE)   (ME_SENSOR sensor);
typedef DWORD    (WINAPI *CREATESENSORINSTBYNAME) (LPCTSTR sensorName); // Suitable for both CreateSensorInstByName(U)
typedef ERR_CODE (WINAPI *RELEASESENSORINSTANCE)  (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *SETPPARAMETERINT)       (DWORD instanceHandle, LPCTSTR paramName, int         paramValue); // Suitable for both SetParameterInt(U)
typedef ERR_CODE (WINAPI *SETPPARAMETERDWORD_PTR) (DWORD instanceHandle, LPCTSTR paramName, DWORD_PTR   paramValue); // Suitable for both SetParameterDWORD_PTR(U)
typedef ERR_CODE (WINAPI *SETPPARAMETERDOUBLE)    (DWORD instanceHandle, LPCTSTR paramName, double      paramValue); // Suitable for both SetParameterDouble(U)
typedef ERR_CODE (WINAPI *SETPPARAMETERSTRING)    (DWORD instanceHandle, LPCTSTR paramName, LPCTSTR     paramValue); // Suitable for both SetParameterString(U)
typedef ERR_CODE (WINAPI *SETPPARAMETERBINARY)    (DWORD instanceHandle, LPCTSTR paramName, const char *paramValue, DWORD len); // Suitable for both SetParameterBinary(U)
typedef ERR_CODE (WINAPI *SETPPARAMETERS)         (DWORD instanceHandle, LPCTSTR parameterList); // Suitable for both SetParameters(U)
typedef ERR_CODE (WINAPI *GETPPARAMETERINT)       (DWORD instanceHandle, LPCTSTR paramName, int       *paramValue); // Suitable for both GetParameterInt(U)
typedef ERR_CODE (WINAPI *GETPPARAMETERDWORD_PTR) (DWORD instanceHandle, LPCTSTR paramName, DWORD_PTR *paramValue); // Suitable for both GetParameterDWORD_PTR(U)
typedef ERR_CODE (WINAPI *GETPPARAMETERDOUBLE)    (DWORD instanceHandle, LPCTSTR paramName, double    *paramValue); // Suitable for both GetParameterDouble(U)
typedef ERR_CODE (WINAPI *GETPPARAMETERSTRING)    (DWORD instanceHandle, LPCTSTR paramName, LPTSTR     paramValue, DWORD *maxLen); // Suitable for both GetParameterString(U)
typedef ERR_CODE (WINAPI *GETPPARAMETERBINARY)    (DWORD instanceHandle, LPCTSTR paramName, const char *paramValue, DWORD *maxLen); // Suitable for both GetParameterBinary(U)
typedef ERR_CODE (WINAPI *GETPPARAMETERS)         (DWORD instanceHandle, LPTSTR parameterList, DWORD *maxLen); // Suitable for both GetParameters(U)

typedef ERR_CODE (WINAPI *CLEARALLPARAMETERS)     (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *OPENSENSOR)             (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *CLOSESENSOR)            (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *SENSORCOMMAND)          (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *DATAAVAIL)              (DWORD instanceHandle, int *avail);
typedef ERR_CODE (WINAPI *TRANSFERDATA)           (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues, int *read);
typedef ERR_CODE (WINAPI *TRANSFERDATATS)         (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues, int *read, double *timestamp);
typedef ERR_CODE (WINAPI *POLL)                   (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues);
typedef ERR_CODE (WINAPI *GETERROR)               (DWORD instanceHandle, LPCTSTR errText, DWORD maxLen); // Suitable for both GetError(U)
typedef ERR_CODE (WINAPI *GETDLLVERSION)          (LPCTSTR versionStr, DWORD maxLen); // Suitable for both GetDLLVersion(U)
typedef ERR_CODE (WINAPI *ENABLELOGGING)          (DWORD instanceHandle, BOOL enableLogging, int logType, int logLevel, LPCTSTR logFile, BOOL logAppend, BOOL logFlush, int logSplitSize); // Suitable for both EnableLogging(U)
typedef ERR_CODE (WINAPI *OPENSENSORRS232)        (DWORD instanceHandle, LPCTSTR port); // Suitable for both OpenSensorRS232(U)
typedef ERR_CODE (WINAPI *OPENSENSORIF2004)       (DWORD instanceHandle, int cardInstance, int channelNumber);
typedef ERR_CODE (WINAPI *OPENSENSORIF2004_USB)   (DWORD instanceHandle, int deviceInstance, LPCTSTR serialNumber, LPCTSTR port, int channelNumber); // Suitable for both OpenSensorIF2004_USB(U)
typedef ERR_CODE (WINAPI *OPENSENSORIF2008)       (DWORD instanceHandle, int cardInstance, int channelNumber);
typedef ERR_CODE (WINAPI *OPENSENSORTCPIP)        (DWORD instanceHandle, LPCTSTR remoteAddr); // Suitable for both OpenSensorTCPIP(U)
typedef ERR_CODE (WINAPI *OPENSENSORWINUSB)       (DWORD instanceHandle, int deviceInstance);
typedef ERR_CODE (WINAPI *EXECSCMD)               (DWORD instanceHandle, LPCTSTR sensorCommand); // Suitable for both ExecSCmd(U)
typedef ERR_CODE (WINAPI *SETINTEXECSCMD)         (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, int     paramValue); // Suitable for both SetIntExecSCmd(U)
typedef ERR_CODE (WINAPI *SETDOUBLEEXECSCMD)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, double  paramValue); // Suitable for both SetDoubleExecSCmd(U)
typedef ERR_CODE (WINAPI *SETSTRINGEXECSCMD)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, LPCTSTR paramValue); // Suitable for both SetStringExecSCmd(U)
typedef ERR_CODE (WINAPI *EXECSCMDGETINT)         (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, int    *paramValue); // Suitable for both ExecSCmdGetInt(U)
typedef ERR_CODE (WINAPI *EXECSCMDGETDOUBLE)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, double *paramValue); // Suitable for both ExecSCmdGetDouble(U)
typedef ERR_CODE (WINAPI *EXECSCMDGETSTRING)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, LPTSTR  paramValue, DWORD *maxLen); // Suitable for both ExecSCmdGetString(U)

static CREATESENSORINSTANCE   pCreateSensorInstance  = NULL;
static CREATESENSORINSTBYNAME pCreateSensorInstByName= NULL;
static RELEASESENSORINSTANCE  pReleaseSensorInstance = NULL;
static SETPPARAMETERINT       pSetParameterInt       = NULL;
static SETPPARAMETERDWORD_PTR pSetParameterDWORD_PTR = NULL;
static SETPPARAMETERDOUBLE    pSetParameterDouble    = NULL;
static SETPPARAMETERSTRING    pSetParameterString    = NULL;
static SETPPARAMETERBINARY    pSetParameterBinary    = NULL;
static SETPPARAMETERS         pSetParameters         = NULL;
static GETPPARAMETERINT       pGetParameterInt       = NULL;
static GETPPARAMETERDWORD_PTR pGetParameterDWORD_PTR = NULL;
static GETPPARAMETERDOUBLE    pGetParameterDouble    = NULL;
static GETPPARAMETERSTRING    pGetParameterString    = NULL;
static CLEARALLPARAMETERS     pClearAllParameters    = NULL;
static OPENSENSOR             pOpenSensor            = NULL;
static CLOSESENSOR            pCloseSensor           = NULL;
static SENSORCOMMAND          pSensorCommand         = NULL;
static DATAAVAIL              pDataAvail             = NULL;
static TRANSFERDATA           pTransferData          = NULL;
static POLL                   pPoll                  = NULL;
static GETERROR               pGetError              = NULL;
static GETDLLVERSION          pGetDLLVersion         = NULL;
static ENABLELOGGING          pEnableLogging         = NULL;
static OPENSENSORRS232        pOpenSensorRS232       = NULL;
static OPENSENSORIF2004       pOpenSensorIF2004      = NULL;
static OPENSENSORIF2008       pOpenSensorIF2008      = NULL;
static OPENSENSORTCPIP        pOpenSensorTCPIP       = NULL;
static OPENSENSORWINUSB       pOpenSensorWinUSB      = NULL;
static EXECSCMD               pExecSCmd              = NULL;
static SETINTEXECSCMD         pSetIntExecSCmd        = NULL;
static SETDOUBLEEXECSCMD      pSetDoubleExecSCmd     = NULL;
static SETSTRINGEXECSCMD      pSetStringExecSCmd     = NULL;
static EXECSCMDGETINT         pExecSCmdGetInt        = NULL;
static EXECSCMDGETDOUBLE      pExecSCmdGetDouble     = NULL;
static EXECSCMDGETSTRING      pExecSCmdGetString     = NULL;

confocalEx::confocalEx(void)
{
	m_err = 0;
	int err= 0;
	if ((err= LoadDLLFunctions())<0)
		m_err = err;
	//	PrintError(_T("Cannot load MEDAQLib.dll and it's functions!"));

	m_sensor= pCreateSensorInstance (SENSOR_ILD2200);
	if (!m_sensor)
		m_err = -100;
	//	PrintError(_T("Cannot create driver instance!"));

//	if ((err= Open (m_sensor))<0)
//		Cleanup (m_sensor);
//
//	if ((err= GetInfo (m_sensor))<0)
//		Cleanup (m_sensor);
//
//	if ((err= Process (m_sensor))<0)
//		goto end;
//
//end:
//	Cleanup (m_sensor);

}


confocalEx::~confocalEx(void)
{	
	Cleanup(m_sensor);
}

CString confocalEx::ConvertMultibyteToUnicode( char* pMultibyte )
{
	int nLen = strlen(pMultibyte);

	WCHAR *pWideChar = new WCHAR[nLen];
	memset(pWideChar, 0x00, (nLen)*sizeof(WCHAR));

	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pMultibyte, -1, pWideChar, nLen);

	CString strUnicode;
	strUnicode.Format(_T("%s"), pWideChar);

	delete [] pWideChar;

	return strUnicode;
}

int confocalEx::LoadDLLFunctions()
{
	HMODULE h= LoadLibrary (_T("D:\\MEDAQLib.dll"));
	if (h==NULL)
		return -1;
	pCreateSensorInstance  = (CREATESENSORINSTANCE)   GetProcAddress (h, "CreateSensorInstance");
	pCreateSensorInstByName= (CREATESENSORINSTBYNAME) GetProcAddress (h, "CreateSensorInstByName");
	pReleaseSensorInstance = (RELEASESENSORINSTANCE)  GetProcAddress (h, "ReleaseSensorInstance");
	pSetParameterInt       = (SETPPARAMETERINT)       GetProcAddress (h, "SetParameterInt");
	pSetParameterDWORD_PTR = (SETPPARAMETERDWORD_PTR) GetProcAddress (h, "SetParameterDWORD_PTR");
	pSetParameterDouble    = (SETPPARAMETERDOUBLE)    GetProcAddress (h, "SetParameterDouble");
	pSetParameterString    = (SETPPARAMETERSTRING)    GetProcAddress (h, "SetParameterString");
	pSetParameterBinary    = (SETPPARAMETERBINARY)    GetProcAddress (h, "SetParameterBinary");
	pSetParameters         = (SETPPARAMETERS)         GetProcAddress (h, "SetParameters");
	pGetParameterInt       = (GETPPARAMETERINT)       GetProcAddress (h, "GetParameterInt");
	pGetParameterDWORD_PTR = (GETPPARAMETERDWORD_PTR) GetProcAddress (h, "GetParameterDWORD_PTR");
	pGetParameterDouble    = (GETPPARAMETERDOUBLE)    GetProcAddress (h, "GetParameterDouble");
	pGetParameterString    = (GETPPARAMETERSTRING)    GetProcAddress (h, "GetParameterString");
	pClearAllParameters    = (CLEARALLPARAMETERS)     GetProcAddress (h, "ClearAllParameters");
	pOpenSensor            = (OPENSENSOR)             GetProcAddress (h, "OpenSensor");
	pCloseSensor           = (CLOSESENSOR)            GetProcAddress (h, "CloseSensor");
	pSensorCommand         = (SENSORCOMMAND)          GetProcAddress (h, "SensorCommand");
	pDataAvail             = (DATAAVAIL)              GetProcAddress (h, "DataAvail");
	pTransferData          = (TRANSFERDATA)           GetProcAddress (h, "TransferData");
	pPoll                  = (POLL)                   GetProcAddress (h, "Poll");
	pGetError              = (GETERROR)               GetProcAddress (h, "GetError");
	pGetDLLVersion         = (GETDLLVERSION)          GetProcAddress (h, "GetDLLVersion");
	pEnableLogging         = (ENABLELOGGING)          GetProcAddress (h, "EnableLogging");
	pOpenSensorRS232       = (OPENSENSORRS232)        GetProcAddress (h, "OpenSensorRS232");
	pOpenSensorIF2004      = (OPENSENSORIF2004)       GetProcAddress (h, "OpenSensorIF2004");
	pOpenSensorIF2008      = (OPENSENSORIF2008)       GetProcAddress (h, "OpenSensorIF2008");
	pOpenSensorTCPIP       = (OPENSENSORTCPIP)        GetProcAddress (h, "OpenSensorTCPIP");
	pOpenSensorWinUSB      = (OPENSENSORWINUSB)       GetProcAddress (h, "OpenSensorWinUSB");
	pExecSCmd              = (EXECSCMD)               GetProcAddress (h, "ExecSCmd");
	pSetIntExecSCmd        = (SETINTEXECSCMD)         GetProcAddress (h, "SetIntExecSCmd");
	pSetDoubleExecSCmd     = (SETDOUBLEEXECSCMD)      GetProcAddress (h, "SetDoubleExecSCmd");
	pSetStringExecSCmd     = (SETSTRINGEXECSCMD)      GetProcAddress (h, "SetStringExecSCmd");
	pExecSCmdGetInt        = (EXECSCMDGETINT)         GetProcAddress (h, "ExecSCmdGetInt");
	pExecSCmdGetDouble     = (EXECSCMDGETDOUBLE)      GetProcAddress (h, "ExecSCmdGetDouble");
	pExecSCmdGetString     = (EXECSCMDGETSTRING)      GetProcAddress (h, "ExecSCmdGetString");
	if (!pCreateSensorInstance || !pCreateSensorInstByName || !pReleaseSensorInstance 
		|| !pSetParameterInt     || !pSetParameterDWORD_PTR  || !pSetParameterDouble || !pSetParameterString || !pSetParameterBinary || !pSetParameters
		|| !pGetParameterInt     || !pGetParameterDWORD_PTR  || !pGetParameterDouble || !pGetParameterString
		|| !pClearAllParameters  || !pOpenSensor             || !pCloseSensor        || !pSensorCommand
		|| !pDataAvail           || !pTransferData           || !pPoll
		|| !pGetError            || !pGetDLLVersion          || !pEnableLogging
		|| !pOpenSensorRS232     || !pOpenSensorIF2004       || !pOpenSensorIF2008   || !pOpenSensorTCPIP    || !pOpenSensorWinUSB
		|| !pExecSCmd            || !pSetIntExecSCmd         || !pSetDoubleExecSCmd  || !pSetStringExecSCmd
		|| !pExecSCmdGetInt      || !pExecSCmdGetDouble      || !pExecSCmdGetString)
		return -2;
	return 0;
}

int confocalEx::PrintError( LPCTSTR err )
{
	_tprintf (_T("Error!\n%s"), err);
	m_errStr.Format(_T("Error Msg : %s"), err);
	return -1;
}

int confocalEx::Error( ERR_CODE err, int sensor )
{
	//char out[1024];//buf[1024];
	wchar_t out[1024];
	CString buf;
	pGetError (sensor, buf, 1024);
// 	_stprintf (out, _T("Error in %d\n%s"), err, buf);
	//CString str;
	//str.Format(_T("in %d\n%s"), err, buf);
// 	return PrintError (out);
	return -1;
}

int confocalEx::Error( LPCTSTR err, int sensor )
{
	//char out[1024], buf[1024];
	//pGetError (sensor, (LPTSTR)buf, _countof (buf));
	wchar_t out[1024];
	CString buf;
	pGetError (sensor, buf, 1024);
// 	_stprintf (out, _T("Error in %s\n%s"), err, buf);
	//_stprintf ((LPTSTR)out, _T("Error in %s\n%s"), err, buf);
// 	return PrintError (out);
	return -1;
}

int confocalEx::Open( DWORD sensor )
{
	// If logging should be enabled
	//pEnableLogging (sensor, TRUE, 31, 127, _T(".\\Log.txt"), TRUE, FALSE, 0);

	//CHECK_ERROR (pOpenSensorIF2004 (sensor, 0, 0));
	ERR_CODE ret = pOpenSensorIF2004 (sensor, 0, 0);
	if(ret!=ERR_NOERROR){
		Error(ret, m_sensor);
	}
	//CHECK_ERROR(ret);
	//return ERR_NOERROR;
	return ret;
}

int confocalEx::GetInfo( DWORD sensor )
{
	//CHECK_ERROR (pExecSCmd(sensor, _T("Get_Settings")));
	ERR_CODE ret = pExecSCmd(sensor, _T("Get_Settings"));
	if(ret!=ERR_NOERROR){
		Error(ret, m_sensor);
	}
	int iErr= 0;
	ERR_CODE err= pGetParameterInt (sensor, _T("SA_ErrorNumber"), &iErr);
	if (err!=ERR_NOERROR && err!=ERR_NOT_FOUND)
		return Error(_T("GetParameterInt (SA_ErrorNumber)"), sensor);
	if (iErr!=0)
	{
		char cErr[1024], buf[1024];
		DWORD len= sizeof (cErr);
		//CHECK_ERROR (pGetParameterString (sensor, _T("SA_ErrorText"), (LPTSTR)cErr, &len));
		ret = pGetParameterString (sensor, _T("SA_ErrorText"), (LPTSTR)cErr, &len);
		if(ret!=ERR_NOERROR){
			Error(ret, m_sensor);
		}
		_stprintf ((LPTSTR)buf, _T("Sensor returned error code after command Get_Settings\n%d: %s"), iErr, cErr);
		return PrintError ((LPTSTR)buf);
		//return -1;
	}

	double range;
	//CHECK_ERROR (pGetParameterDouble (sensor, _T("SA_Range"), &range));
	ret = pGetParameterDouble (sensor, _T("SA_Range"), &range);
	if(ret!=ERR_NOERROR){
		Error(ret, m_sensor);
	}
	_tprintf (_T("Sensor range: %.0f mm\n"), range);
	m_infoRangeStr.Format(_T("Sensor range: %.0f mm\n"), range);

	double samplerate;
	CHECK_ERROR (pExecSCmdGetDouble (sensor, _T("Get_Info"), _T("SA_Samplerate"), &samplerate));
	_tprintf (_T("Sensor samplerate: %.0f Hz\n"), samplerate);
	m_infoSamplerateStr.Format(_T("Sensor samplerate: %.0f Hz\n"), samplerate);

	return ERR_NOERROR;
}

int confocalEx::Process( int sensor )
{
	ERR_CODE ret = ERR_NOERROR;
	_tprintf (_T("Values:\r\n"));
	for (int i=0 ; i<20 ; i++)	// 20 cycles
	{
		double data;

		CHECK_ERROR (pPoll (sensor, NULL, &data, 1));
		_tprintf (_T("%.1f "), data);
		m_data[i] = data;

		Sleep (100);
	}
	return ret;
}

void confocalEx::Cleanup( int sensor )
{
	if (pCloseSensor (sensor)!=ERR_NOERROR)
		PrintError (_T("Cannot close sensor!"));
	if (pReleaseSensorInstance (sensor)!=ERR_NOERROR)
		PrintError (_T("Cannot release driver instance!"));
}
