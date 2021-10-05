/* Copyright (C) Micro-Epsilon Messtechnik GmbH & Co. KG
 *
 * This file is part of MEDAQLib Software Development Kit.
 * All Rights Reserved.
 *
 * THIS SOURCE FILE IS THE PROPERTY OF MICRO-EPSILON MESSTECHNIK AND
 * IS NOT TO BE RE-DISTRIBUTED BY ANY MEANS WHATSOEVER WITHOUT THE
 * EXPRESSED WRITTEN CONSENT OF MICRO-EPSILON MESSTECHNIK.
 *
 * Contact information:
 *    mailto:info@micro-epsilon.de
 *    http://www.micro-epsilon.de
 *
 */

#pragma once
#include <windows.h>

#ifdef MEDAQLIB_EXPORTS
#define MEDAQLIB_API
#else
#define MEDAQLIB_API __declspec(dllimport)
#endif

/******************************************************************************
                            Data type declaration
BOOL      if a four byte integer type which can be
          FALSE (0) or TRUE (!=0)                                (int)
DWORD     is a four byte unsigned integer type                   (unsigned long)
DWORD_PTR is a four byte unsigned integer type on Win32 systems  (unsigned long)
          or a eight byte unsigned integer type on Win64 systems (unsigned __int64)
WORD      is a two byte unsigned integer type                    (unsigned short)
BYTE      is an one byte unsigned integer type                   (unsigned char)
LPCSTR    is a pointer to a constant character string            (const char *)
LPSTR     is a pointer to a character string                     (char *)
LPCWSTR   is a pointer to a constant unicode string              (const short *)
LPWSTR    is a pointer to a unicode string                       (short *)
LPCTSTR   is a pointer to a constant ANSI string                 (const char *)
          or unicode string (depending on defined UNICODE)       (const short *)
LPTSTR    is a pointer to a ANSI string                          (char *)
          or unicode string (depending on defined UNICODE)       (short *)
ERR_CODE  is an enumeration of error codes                       (int)
int       is a four byte signed integer type
double    is an eight byte floating point type
float     is a four byte floating point type
*         function parameters with * are pointers to the type. They are used to 
          return information from the function. In some languages it is called
          "call by reference"

FALSE     Defined as zero (0)
TRUE      Defined as nonzero (1)

MEDAQLIB_API is defined as __declspec(dllimport) when using the
driver. When linking with MEDAQLib.lib all funktions with this
specifier are found in the library.

WINAPI is defined as __stdcall, which defines the calling convention.
The arguments are submitted from right to left to the function. 
They are written as value (call by value) onto the stack (unless they 
are referenced as a pointer). The called function reads the arguments 
from the stack.
******************************************************************************/

/******************************************************************************
                       Resolve windows specific defines
For some parsers (like LabView import DLL tool), this defines might be helpful
******************************************************************************/
#if defined (MEDAQLIB_WINDOWS_SPECIFIC)
#define WINAPI    __stdcall
#define DWORD     unsigned long
#ifdef _WIN64
#define DWORD_PTR unsigned __int64
#else
#define DWORD_PTR unsigned long
#endif
#define BOOL      int
#define LPCSTR    const char *
#define LPCWSTR   const short *
#define LPSTR     char *
#define LPWSTR    short *

#ifndef _UNICODE
#define LPTSTR    LPSTR
#define LPCTSTR   LPCSTR
#else
#define LPTSTR    LPWSTR
#define LPCTSTR   LPCWSTR
#endif // _UNICODE
#endif

/******************************************************************************
                              Enumerations
In function calls enumerations can be used textual or as number
******************************************************************************/
typedef enum
	{
	NO_SENSOR=            0, // Dummy, only for internal use
	SENSOR_ILR110x_115x= 19, // optoNCDT ILR
	SENSOR_ILR118x=      20, // optoNCDT ILR
	SENSOR_ILR1191=      21, // optoNCDT ILR
	SENSOR_ILD1302=      24, // optoNCDT
	SENSOR_ILD1401=       1, // optoNCDT
	CONTROLLER_ILD1401=  14, // Demo case for ILD1401
	SENSOR_ILD1402=      23, // optoNCDT
	SENSOR_ILD1700=       2, // optoNCDT
	SENSOR_ILD1800=       3, // optoNCDT
	SENSOR_ILD2000=       4, // optoNCDT
	SENSOR_ILD2200=       5, // optoNCDT
	SENSOR_ILD2300=      29, // optoNCDT
	SENSOR_IFD2400=       6, // confocalDT
	SENSOR_IFD2401=      12, // confocalDT
	SENSOR_IFD2430=       7, // confocalDT
	SENSOR_IFD2431=      13, // confocalDT
	SENSOR_IFD2445=      39, // confocalDT
	SENSOR_IFD2451=      30, // confocalDT
	SENSOR_IFD2471=      26, // confocalDT
	SENSOR_ODC1202=      25, // optoCONTROL
	SENSOR_ODC2500=       8, // optoCONTROL
	SENSOR_ODC2520=      37, // optoCONTROL
	SENSOR_ODC2600=       9, // optoCONTROL
	CONTROLLER_DT3909=   27, // eddyNCDT
	CONTROLLER_ESC4912=  17, // eddyNCDT
	SENSOR_DT3100=       28, // eddyNCDT
	SENSOR_DT6100=       16, // capaNCDT
	CONTROLLER_DT6200=   33, // capaNCDT
	CONTROLLER_KSS6380=  18, // capaNCDT
	CONTROLLER_DT6500=   15, // capaNCDT
	ENCODER_IF2004=      10, // deprecated, use PCI_CARD_IF2004 instead
	PCI_CARD_IF2004=     10, // PCI card IF2004
	PCI_CARD_IF2008=     22, // PCI card IF2008
	SENSOR_LLT27xx=      31, // scanCONTROL+gapCONTROL, only for SensorFinder functionality, OpenSensor will fail
	CONTROLLER_CSP2008=  32, // Universal controller
	ETH_IF1032=          34, // Interface module Ethernet/EtherCAT
	USB_ADAPTER_IF2004=  36, // IF2004 USB adapter (4 x RS422)
	CONTROLLER_CBOX=     38, // External C-Box controller
	SENSOR_ACS7000=      35, // colorCONTROL
	NUMBER_OF_SENSORS=   39,
	} ME_SENSOR;

typedef enum
	{
	ERR_NOERROR= 0,
	ERR_FUNCTION_NOT_SUPPORTED= -1,
	ERR_CANNOT_OPEN= -2,
	ERR_NOT_OPEN= -3,
	ERR_APPLYING_PARAMS= -4,
	ERR_SEND_CMD_TO_SENSOR= -5,
	ERR_CLEARUNG_BUFFER= -6,
	ERR_HW_COMMUNICATION= -7,
	ERR_TIMEOUT_READING_FROM_SENSOR= -8,
	ERR_READING_SENSOR_DATA= -9,
	ERR_INTERFACE_NOT_SUPPORTED= -10,
	ERR_ALREADY_OPEN= -11,
	ERR_CANNOT_CREATE_INTERFACE= -12,
	ERR_NO_SENSORDATA_AVAILABLE= -13,
	ERR_UNKNOWN_SENSOR_COMMAND= -14,
	ERR_UNKNOWN_SENSOR_ANSWER= -15,
	ERR_SENSOR_ANSWER_ERROR= -16,
	ERR_SENSOR_ANSWER_TOO_SHORT= -17,
	ERR_WRONG_PARAMETER= -18,
	ERR_NOMEMORY= -19,
	ERR_NO_ANSWER_RECEIVED= -20,
	ERR_SENSOR_ANSWER_DOES_NOT_MATCH_COMMAND= -21,
	ERR_BAUDRATE_TOO_LOW= -22,
	ERR_OVERFLOW= -23,
	ERR_INSTANCE_NOT_EXIST= -24,
	ERR_NOT_FOUND= -25,
	ERR_WARNING= -26,
	ERR_SENSOR_ANSWER_WARNING= -27,
	} ERR_CODE;

/******************************************************************************
                              Function declaration
******************************************************************************/
#ifdef __cplusplus
extern "C"	// all functions are exported with "C" linkage (C decorated)
	{
#endif // __cplusplus
/* CreateSensorInstance
   Create an instance of the specified sensor driver and returns an index 
   greater 0. If the function fails, 0 is returned.                          */
	MEDAQLIB_API DWORD WINAPI CreateSensorInstance    (ME_SENSOR sensor);
	MEDAQLIB_API DWORD WINAPI CreateSensorInstByName  (LPCSTR sensorName);
	MEDAQLIB_API DWORD WINAPI CreateSensorInstByNameU (LPCWSTR sensorName);

/* ReleaseSensorInstance
   Frees the sensor driver instance previously created by CreateSensorInstance.
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI ReleaseSensorInstance (DWORD instanceHandle);

/* SetParameter
   Many different parameters can be specified for the sensors and interfaces.
	 For normalisation purpose they can be set and buffered with SetParameter 
	 functions. All other functions use the parameters to operate. 
	 If binary data containing binary zero "\0" should be written, the
	 function SetParameterBinary must be used and the complete length
	 of the binary data (in bytes) must be set.
	 See the manual for a list of parameters used in which function and for which 
	 sensor or interface.
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI SetParameterInt        (DWORD instanceHandle, LPCSTR paramName, int          paramValue);
	// SetParameterDWORD is deprecated, use SetParameterDWORD_PTR instead
	MEDAQLIB_API ERR_CODE WINAPI SetParameterDWORD      (DWORD instanceHandle, LPCSTR paramName, DWORD        paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterDWORD_PTR  (DWORD instanceHandle, LPCSTR paramName, DWORD_PTR    paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterDouble     (DWORD instanceHandle, LPCSTR paramName, double       paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterString     (DWORD instanceHandle, LPCSTR paramName, LPCSTR       paramValue);
	// SetParameterStringL is deprecated, use SetParameterBinary instead
	MEDAQLIB_API ERR_CODE WINAPI SetParameterStringL    (DWORD instanceHandle, LPCSTR paramName, LPCSTR       paramValue, DWORD len);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterBinary     (DWORD instanceHandle, LPCSTR paramName, const char  *paramValue, DWORD len);
	MEDAQLIB_API ERR_CODE WINAPI SetParameters          (DWORD instanceHandle, LPCSTR parameterList);

	// Unicode versions
	MEDAQLIB_API ERR_CODE WINAPI SetParameterIntU       (DWORD instanceHandle, LPCWSTR paramName, int         paramValue);
	// SetParameterDWORDU is deprecated, use SetParameterDWORD_PTRU instead
	MEDAQLIB_API ERR_CODE WINAPI SetParameterDWORDU     (DWORD instanceHandle, LPCWSTR paramName, DWORD       paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterDWORD_PTRU (DWORD instanceHandle, LPCWSTR paramName, DWORD_PTR   paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterDoubleU    (DWORD instanceHandle, LPCWSTR paramName, double      paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterStringU    (DWORD instanceHandle, LPCWSTR paramName, LPCWSTR     paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetParameterBinaryU    (DWORD instanceHandle, LPCWSTR paramName, const char *paramValue, DWORD len);
	MEDAQLIB_API ERR_CODE WINAPI SetParametersU         (DWORD instanceHandle, LPCWSTR parameterList);


/* GetParameter
   Results from any functions are stored and can be retrieved with GetParameter.
	 Values are stored in the second parameter (call by reference). When 
	 retrieving a string value, a string with enough space to hold the result 
	 must be passed. The length of the string must be specified in the length
	 parameter. The length of the result string is returned in the length 
	 parameter too. If the string parameter is not specified (null pointer) the
	 required length is returned in the parameter length.
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI GetParameterInt        (DWORD instanceHandle, LPCSTR paramName, int         *paramValue);
	// GetParameterDWORD is deprecated, use GetParameterDWORD_PTR instead
	MEDAQLIB_API ERR_CODE WINAPI GetParameterDWORD      (DWORD instanceHandle, LPCSTR paramName, DWORD       *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterDWORD_PTR  (DWORD instanceHandle, LPCSTR paramName, DWORD_PTR   *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterDouble     (DWORD instanceHandle, LPCSTR paramName, double      *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterString     (DWORD instanceHandle, LPCSTR paramName, LPSTR        paramValue, DWORD *maxLen);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterBinary     (DWORD instanceHandle, LPCSTR paramName, char        *paramValue, DWORD *maxLen);

	// Unicode versions
	MEDAQLIB_API ERR_CODE WINAPI GetParameterIntU       (DWORD instanceHandle, LPCWSTR paramName, int        *paramValue);
	// GetParameterDWORDU is deprecated, use GetParameterDWORD_PTRU instead
	MEDAQLIB_API ERR_CODE WINAPI GetParameterDWORDU     (DWORD instanceHandle, LPCWSTR paramName, DWORD      *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterDWORD_PTRU (DWORD instanceHandle, LPCWSTR paramName, DWORD_PTR  *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterDoubleU    (DWORD instanceHandle, LPCWSTR paramName, double     *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterStringU    (DWORD instanceHandle, LPCWSTR paramName, LPWSTR      paramValue, DWORD *maxLen);
	MEDAQLIB_API ERR_CODE WINAPI GetParameterBinaryU    (DWORD instanceHandle, LPCWSTR paramName, char       *paramValue, DWORD *maxLen);

/* ClearAllParameters
   Before building a new sensor command the internal buffer can be cleared to ensure
	 that no old parameters affects the new command.
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI ClearAllParameters (DWORD instanceHandle);

/* OpenSensor
   Establish a connection to the sensor using the interface and parameters 
	 specified at SetParameter. 
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI OpenSensor (DWORD instanceHandle);

/* CloseSensor
   Close the connection to the connected sensor. 
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI CloseSensor (DWORD instanceHandle);

/* SensorCommand
   Send a command to the sensor and retrievs the answer. Both, command and
	 answer can be set/read with Set- and GetParameter.
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI SensorCommand (DWORD instanceHandle);

/* DataAvail
   Check if data from Sensor is available and returns the number of values.
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI DataAvail (DWORD instanceHandle, int *avail);

/* TransferData
   Retrievs specified amount of values from sensor and return the raw data 
	 in rawData (if not null) and the scaled data in scaledData (if not null). 
	 The first data in the buffer is returned and then removed from the buffer.
	 The actual data read is stored in variable read (if not null).
	 The second version stores the a timestamp of the first (oldest) value in the
	 array (if not null). It is in milli seconds starting at 01.01.1970 01:00.
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI TransferData (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues, int *read);
	MEDAQLIB_API ERR_CODE WINAPI TransferDataTS (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues, int *read, double *timestamp);

/* Poll
   Retrievs specified amount of values from sensor (max. one frame) and 
	 return the raw data in rawData (if not null) and the scaled data in 
	 scaledData (if not null). The latest values are returned, but no data is 
	 discarded in the sensor instance class (TransferData can be called 
	 independently from this).
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI Poll (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues);

/* GetError
   If an error had occured, the error text can be retrieved with GetError. The
	 text is stored in errText limited to length of maxLen. 
	 Returns the number of the error returned in errText.                      */
	MEDAQLIB_API ERR_CODE WINAPI GetError (DWORD instanceHandle, LPSTR errText, DWORD maxLen);
	// Unicode version
	MEDAQLIB_API ERR_CODE WINAPI GetErrorU (DWORD instanceHandle, LPWSTR errText, DWORD maxLen);

/* GetDLLVersion
   Retrievs the version of the MEDAQLib dll. The version is stored in 
	 versionStr and is limited to length of maxLen (should be at least 64 bytes). 
	 Returns ERR_NOERROR on success, otherwise an error return value.          */
	MEDAQLIB_API ERR_CODE WINAPI GetDLLVersion (LPSTR versionStr, DWORD maxLen);
	// Unicode version
	MEDAQLIB_API ERR_CODE WINAPI GetDLLVersionU (LPWSTR versionStr, DWORD maxLen);

/* EnableLogging
	 Wrapper functions for a set of SetParameterXXX to enable logging.
	 This usage of this functions makes the code shorter and more readable.    */
	MEDAQLIB_API ERR_CODE WINAPI EnableLogging (DWORD instanceHandle, BOOL enableLogging, int logType, int logLevel, LPCSTR logFile, BOOL logAppend, BOOL logFlush, int logSplitSize);
	// Unicode version
	MEDAQLIB_API ERR_CODE WINAPI EnableLoggingU (DWORD instanceHandle, BOOL enableLogging, int logType, int logLevel, LPCWSTR logFile, BOOL logAppend, BOOL logFlush, int logSplitSize);

/* LogToFile
	 User function to allow an application to add lines to MEDAQLib log file.  */
	MEDAQLIB_API ERR_CODE WINAPI LogToFile (DWORD instanceHandle, int logLevel, LPCSTR location, LPCSTR message, ...);
	// Unicode version
	MEDAQLIB_API ERR_CODE WINAPI LogToFileU (DWORD instanceHandle, int logLevel, LPCWSTR location, LPCWSTR message, ...);

/* OpenSensorXXX
	 Wrapper functions for a set of SetParameterXXX and OpenSensor.
	 This usage of this functions makes the code shorter and more readable.    */
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorRS232       (DWORD instanceHandle, LPCSTR port);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorIF2004      (DWORD instanceHandle, int cardInstance, int channelNumber);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorIF2004_USB  (DWORD instanceHandle, int deviceInstance, LPCSTR serialNumber, LPCSTR port, int channelNumber);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorIF2008      (DWORD instanceHandle, int cardInstance, int channelNumber);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorTCPIP       (DWORD instanceHandle, LPCSTR remoteAddr);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorDriverX_USB (DWORD instanceHandle, int deviceInstance);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorUSBIO       (DWORD instanceHandle, int deviceInstance);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorWinUSB      (DWORD instanceHandle, int deviceInstance);
	// Unicode version
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorRS232U      (DWORD instanceHandle, LPCWSTR port);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorIF2004_USBU (DWORD instanceHandle, int deviceInstance, LPCWSTR serialNumber, LPCWSTR port, int channelNumber);
	MEDAQLIB_API ERR_CODE WINAPI OpenSensorTCPIPU      (DWORD instanceHandle, LPCWSTR remoteAddr);

/* ExecSCmd
	 Wrapper functions for a set of Set/GetParameterXXX and SensorCommand.
	 This usage of this functions makes the code shorter and more readable.    */
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmd          (DWORD instanceHandle, LPCSTR sensorCommand);
	MEDAQLIB_API ERR_CODE WINAPI SetIntExecSCmd    (DWORD instanceHandle, LPCSTR sensorCommand, LPCSTR paramName, int     paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetDoubleExecSCmd (DWORD instanceHandle, LPCSTR sensorCommand, LPCSTR paramName, double  paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetStringExecSCmd (DWORD instanceHandle, LPCSTR sensorCommand, LPCSTR paramName, LPCSTR  paramValue);
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmdGetInt    (DWORD instanceHandle, LPCSTR sensorCommand, LPCSTR paramName, int    *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmdGetDouble (DWORD instanceHandle, LPCSTR sensorCommand, LPCSTR paramName, double *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmdGetString (DWORD instanceHandle, LPCSTR sensorCommand, LPCSTR paramName, LPSTR   paramValue, DWORD *maxLen);
	// Unicode version
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmdU          (DWORD instanceHandle, LPCWSTR sensorCommand);
	MEDAQLIB_API ERR_CODE WINAPI SetIntExecSCmdU    (DWORD instanceHandle, LPCWSTR sensorCommand, LPCWSTR paramName, int     paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetDoubleExecSCmdU (DWORD instanceHandle, LPCWSTR sensorCommand, LPCWSTR paramName, double  paramValue);
	MEDAQLIB_API ERR_CODE WINAPI SetStringExecSCmdU (DWORD instanceHandle, LPCWSTR sensorCommand, LPCWSTR paramName, LPCWSTR paramValue);
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmdGetIntU    (DWORD instanceHandle, LPCWSTR sensorCommand, LPCWSTR paramName, int    *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmdGetDoubleU (DWORD instanceHandle, LPCWSTR sensorCommand, LPCWSTR paramName, double *paramValue);
	MEDAQLIB_API ERR_CODE WINAPI ExecSCmdGetStringU (DWORD instanceHandle, LPCWSTR sensorCommand, LPCWSTR paramName, LPWSTR  paramValue, DWORD *maxLen);

#ifdef __cplusplus
	}
#endif // __cplusplus

/******************************************************************************
                              C type definitions
Following type definitions can be used, when the MEDAQLib driver is dynamically
loaded (LoadLibrary) and the function pointers retrieved by GetProcAddress must
be assigned to functions, e.g.: 

CREATESENSORINSTANCE pCreateSensorInstance= (CREATESENSORINSTANCE)
  GetProcAddress (LoadLibrary ("MEDAQLib.dll"), "CreateSensorInstance");
******************************************************************************/
typedef DWORD    (WINAPI *CREATESENSORINSTANCE)   (ME_SENSOR sensor);
typedef DWORD    (WINAPI *CREATESENSORINSTBYNAME) (LPCTSTR sensorName);
typedef ERR_CODE (WINAPI *RELEASESENSORINSTANCE)  (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *SETPPARAMETERINT)       (DWORD instanceHandle, LPCTSTR paramName, int        paramValue);
typedef ERR_CODE (WINAPI *SETPPARAMETERDWORD)     (DWORD instanceHandle, LPCTSTR paramName, DWORD      paramValue);
typedef ERR_CODE (WINAPI *SETPPARAMETERDWORD_PTR) (DWORD instanceHandle, LPCTSTR paramName, DWORD_PTR  paramValue);
typedef ERR_CODE (WINAPI *SETPPARAMETERDOUBLE)    (DWORD instanceHandle, LPCTSTR paramName, double     paramValue);
typedef ERR_CODE (WINAPI *SETPPARAMETERSTRING)    (DWORD instanceHandle, LPCTSTR paramName, LPCTSTR    paramValue);
typedef ERR_CODE (WINAPI *SETPPARAMETERSTRINGL)   (DWORD instanceHandle, LPCTSTR paramName, LPCTSTR    paramValue, DWORD len);
typedef ERR_CODE (WINAPI *SETPPARAMETERS)         (DWORD instanceHandle, LPCTSTR paramList);
typedef ERR_CODE (WINAPI *GETPPARAMETERINT)       (DWORD instanceHandle, LPCTSTR paramName, int       *paramValue);
typedef ERR_CODE (WINAPI *GETPPARAMETERDWORD)     (DWORD instanceHandle, LPCTSTR paramName, DWORD     *paramValue);
typedef ERR_CODE (WINAPI *GETPPARAMETERDWORD_PTR) (DWORD instanceHandle, LPCTSTR paramName, DWORD_PTR *paramValue);
typedef ERR_CODE (WINAPI *GETPPARAMETERDOUBLE)    (DWORD instanceHandle, LPCTSTR paramName, double    *paramValue);
typedef ERR_CODE (WINAPI *GETPPARAMETERSTRING)    (DWORD instanceHandle, LPCTSTR paramName, LPTSTR     paramValue, DWORD *maxLen);
typedef ERR_CODE (WINAPI *CLEARALLPARAMETERS)     (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *OPENSENSOR)             (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *CLOSESENSOR)            (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *SENSORCOMMAND)          (DWORD instanceHandle);
typedef ERR_CODE (WINAPI *DATAAVAIL)              (DWORD instanceHandle, int *avail);
typedef ERR_CODE (WINAPI *TRANSFERDATA)           (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues, int *read);
typedef ERR_CODE (WINAPI *TRANSFERDATATS)         (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues, int *read, double *timestamp);
typedef ERR_CODE (WINAPI *POLL)                   (DWORD instanceHandle, int *rawData, double *scaledData, int maxValues);
//typedef ERR_CODE (WINAPI *GETERROR)               (DWORD instanceHandle, LPCTSTR errText, DWORD maxLen);
typedef ERR_CODE (WINAPI *GETDLLVERSION)          (LPCTSTR versionStr, DWORD maxLen);
typedef ERR_CODE (WINAPI *ENABLELOGGING)          (DWORD instanceHandle, BOOL enableLogging, int logType, int logLevel, LPCTSTR logFile, BOOL logAppend, BOOL logFlush, int logSplitSize);
typedef ERR_CODE (WINAPI *OPENSENSORRS232)        (DWORD instanceHandle, LPCTSTR port);
typedef ERR_CODE (WINAPI *OPENSENSORIF2004)       (DWORD instanceHandle, int cardInstance, int channelNumber);
typedef ERR_CODE (WINAPI *OPENSENSORIF2004_USB)   (DWORD instanceHandle, int deviceInstance, LPCTSTR serialNumber, LPCTSTR port, int channelNumber);
typedef ERR_CODE (WINAPI *OPENSENSORIF2008)       (DWORD instanceHandle, int cardInstance, int channelNumber);
typedef ERR_CODE (WINAPI *OPENSENSORTCPIP)        (DWORD instanceHandle, LPCTSTR remoteAddr);
typedef ERR_CODE (WINAPI *OPENSENSORDRIVERX_USB)  (DWORD instanceHandle, int deviceInstance);
typedef ERR_CODE (WINAPI *OPENSENSORUSBIO)        (DWORD instanceHandle, int deviceInstance);
typedef ERR_CODE (WINAPI *OPENSENSORWINUSB)       (DWORD instanceHandle, int deviceInstance);
typedef ERR_CODE (WINAPI *EXECSCMD)               (DWORD instanceHandle, LPCTSTR sensorCommand);
typedef ERR_CODE (WINAPI *SETINTEXECSCMD)         (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, int     paramValue);
typedef ERR_CODE (WINAPI *SETDOUBLEEXECSCMD)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, double  paramValue);
typedef ERR_CODE (WINAPI *SETSTRINGEXECSCMD)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, LPCTSTR paramValue);
typedef ERR_CODE (WINAPI *EXECSCMDGETINT)         (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, int    *paramValue);
typedef ERR_CODE (WINAPI *EXECSCMDGETDOUBLE)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, double *paramValue);
typedef ERR_CODE (WINAPI *EXECSCMDGETSTRING)      (DWORD instanceHandle, LPCTSTR sensorCommand, LPCTSTR paramName, LPTSTR  paramValue, DWORD *maxLen);

/******************************************************************************
                        Visual Basic type definitions
Following type definitions can be used in Visual Basic to declare the 
functions of MEDAQLib driver.

Private Declare Function CreateSensorInstance   Lib "MEDAQLib.dll" (ByVal sensor As Long)                                                                                                                                                                                           As Long
Private Declare Function CreateSensorInstByName Lib "MEDAQLib.dll" (ByVal sensorName As String)                                                                                                                                                                                     As Long
Private Declare Function ReleaseSensorInstance  Lib "MEDAQLib.dll" (ByVal instanceHandle As Long)                                                                                                                                                                                   As Long
Private Declare Function SetParameterInt        Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByVal paramValue As Long)                                                                                                                              As Long
Private Declare Function SetParameterDWORD      Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByVal paramValue As Long)                                                                                                                              As Long
Private Declare Function SetParameterDouble     Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByVal paramValue As Double)                                                                                                                            As Long
Private Declare Function SetParameterString     Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByVal paramValue As String)                                                                                                                            As Long
Private Declare Function SetParameterStringL    Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByVal paramValue As String, ByVal len As Long)                                                                                                         As Long
Private Declare Function SetParameters          Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal parameterList As String)                                                                                                                                                    As Long
Private Declare Function GetParameterInt        Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByRef paramValue As Long)                                                                                                                              As Long
Private Declare Function GetParameterDWORD      Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByRef paramValue As Long)                                                                                                                              As Long
Private Declare Function GetParameterDouble     Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByRef paramValue As Double)                                                                                                                            As Long
Private Declare Function GetParameterString     Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal paramName As String, ByVal paramValue As String, ByRef maxLen As Long)                                                                                                      As Long
Private Declare Function ClearAllParameters     Lib "MEDAQLib.dll" (ByVal instanceHandle As Long)                                                                                                                                                                                   As Long
Private Declare Function OpenSensor             Lib "MEDAQLib.dll" (ByVal instanceHandle As Long)                                                                                                                                                                                   As Long
Private Declare Function CloseSensor            Lib "MEDAQLib.dll" (ByVal instanceHandle As Long)                                                                                                                                                                                   As Long
Private Declare Function SensorCommand          Lib "MEDAQLib.dll" (ByVal instanceHandle As Long)                                                                                                                                                                                   As Long
Private Declare Function DataAvail              Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByRef avail As Long)                                                                                                                                                              As Long
Private Declare Function TransferData           Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByRef rawData As Long, ByRef scaledData As Double, ByVal maxValues As Long, ByRef read As Long)                                                                                   As Long
Private Declare Function TransferDataTS         Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByRef rawData As Long, ByRef scaledData As Double, ByVal maxValues As Long, ByRef read As Long, ByRef timestamp As Double)                                                        As Long
Private Declare Function Poll                   Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByRef rawData As Long, ByRef scaledData As Double, ByVal maxValues As Long)                                                                                                       As Long
Private Declare Function GetError               Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal errText As String, ByVal maxLen As Long)                                                                                                                                    As Long
Private Declare Function GetDLLVersion          Lib "MEDAQLib.dll" (ByVal versionStr As String,   ByVal maxLen As Long)                                                                                                                                                             As Long
Private Declare Function EnableLogging          Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal enableLogging As Long, ByVal logType As Long, ByVal logLevel As Long, ByVal logFile As String, ByVal logAppend As Long, ByVal logFlush As Long, ByVal logSplitSize As Long) As Long
Private Declare Function OpenSensorRS232        Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal port As String)                                                                                                                                                             As Long
Private Declare Function OpenSensorIF2004       Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal cardInstance As Long, ByVal channelNumber As Long)                                                                                                                          As Long
Private Declare Function OpenSensorIF2004_USB   Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal deviceInstance as Long, ByVal serialNumber As String, ByVal port As String, ByVal channelNumber As Long)                                                                    As Long
Private Declare Function OpenSensorIF2008       Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal cardInstance As Long, ByVal channelNumber As Long)                                                                                                                          As Long
Private Declare Function OpenSensorTCPIP        Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal remoteAddr As String)                                                                                                                                                       As Long
Private Declare Function OpenSensorDriverX_USB  Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal deviceInstance As Long)                                                                                                                                                     As Long
Private Declare Function OpenSensorUSBIO        Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal deviceInstance As Long)                                                                                                                                                     As Long
Private Declare Function OpenSensorWINUSB       Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal deviceInstance As Long)                                                                                                                                                     As Long
Private Declare Function ExecSCmd               Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal sensorCommand As String)                                                                                                                                                    As Long
Private Declare Function SetIntExecSCmd         Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal sensorCommand As String, ByVal paramName As String, ByVal paramValue As Long)                                                                                               As Long
Private Declare Function SetDoubleExecSCmd      Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal sensorCommand As String, ByVal paramName As String, ByVal paramValue As Double)                                                                                             As Long
Private Declare Function StringExecSCmd         Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal sensorCommand As String, ByVal paramName As String, ByVal paramValue As String)                                                                                             As Long
Private Declare Function ExecSCmdGetInt         Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal sensorCommand As String, ByVal paramName As String, ByRef paramValue As Long)                                                                                               As Long
Private Declare Function ExecSCmdGetDouble      Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal sensorCommand As String, ByVal paramName As String, ByRef paramValue As Double)                                                                                             As Long
Private Declare Function ExecSCmdGetString      Lib "MEDAQLib.dll" (ByVal instanceHandle As Long, ByVal sensorCommand As String, ByVal paramName As String, ByVal paramValue As String, ByRef maxLen As Long)                                                                       As Long
******************************************************************************/

/******************************************************************************
                        C# type definitions
Following type definitions can be used in C# to declare the 
functions of MEDAQLib driver.

[DllImport ("MEDAQLib.dll")] public static extern UInt32   CreateSensorInstance   (ME_SENSOR sensor);
[DllImport ("MEDAQLib.dll")] public static extern UInt32   CreateSensorInstByName (String sensorName);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE ReleaseSensorInstance  (UInt32 instanceHandle);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetParameterInt        (UInt32 instanceHandle, String paramName,     Int32     paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetParameterDWORD      (UInt32 instanceHandle, String paramName,     UInt32    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetParameterDouble     (UInt32 instanceHandle, String paramName,     Double    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetParameterString     (UInt32 instanceHandle, String paramName,     String    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetParameterStringL    (UInt32 instanceHandle, String paramName,     String    paramValue, UInt32 len);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetParameters          (UInt32 instanceHandle, String paramList,);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE GetParameterInt        (UInt32 instanceHandle, String paramName, ref Int32     paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE GetParameterDWORD      (UInt32 instanceHandle, String paramName, ref UInt32    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE GetParameterDouble     (UInt32 instanceHandle, String paramName, ref Double    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE GetParameterString     (UInt32 instanceHandle, String paramName, StringBuilder paramValue, ref UInt32 maxLen);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE ClearAllParameters     (UInt32 instanceHandle);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensor             (UInt32 instanceHandle);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE CloseSensor            (UInt32 instanceHandle);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SensorCommand          (UInt32 instanceHandle);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE DataAvail              (UInt32 instanceHandle, ref Int32 avail);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE TransferData           (UInt32 instanceHandle, [MarshalAs (UnmanagedType.LPArray)] Int32[] rawData, [MarshalAs (UnmanagedType.LPArray)] Double[] scaledData, Int32 maxValues, ref Int32 read);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE TransferDataTS         (UInt32 instanceHandle, [MarshalAs (UnmanagedType.LPArray)] Int32[] rawData, [MarshalAs (UnmanagedType.LPArray)] Double[] scaledData, Int32 maxValues, ref Int32 read, ref Double timestamp);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE Poll                   (UInt32 instanceHandle, [MarshalAs (UnmanagedType.LPArray)] Int32[] rawData, [MarshalAs (UnmanagedType.LPArray)] Double[] scaledData, Int32 maxValues);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE GetError               (UInt32 instanceHandle, StringBuilder errText, UInt32 maxLen);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE GetDLLVersion          (StringBuilder versionStr, UInt32 maxLen);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE EnableLogging          (UInt32 instanceHandle, Int32 enableLogging, Int32 logType, Int32 logLevel, String logFile, Int32 logAppend, Int32 logFlush, Int32 logSplitSize);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorRS232        (UInt32 instanceHandle, String port);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorIF2004       (UInt32 instanceHandle, Int32 cardInstance, Int32 channelNumber);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorIF2004_USB   (UInt32 instanceHandle, Int32 deviceInstance, String serialNumber, String port, Int32 channelNumber);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorIF2008       (UInt32 instanceHandle, Int32 cardInstance, Int32 channelNumber);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorTCPIP        (UInt32 instanceHandle, String remoteAddr);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorDriverX_USB  (UInt32 instanceHandle, Int32 deviceInstance);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorUSBIO        (UInt32 instanceHandle, Int32 deviceInstance);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE OpenSensorWinUSB       (UInt32 instanceHandle, Int32 deviceInstance);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE ExecSCmd               (UInt32 instanceHandle, String sensorCommand);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetIntExecSCmd         (UInt32 instanceHandle, String sensorCommand, String paramName,     Int32     paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetDoubleExecSCmd      (UInt32 instanceHandle, String sensorCommand, String paramName,     Double    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE SetStringExecSCmd      (UInt32 instanceHandle, String sensorCommand, String paramName,     String    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE ExecSCmdGetInt         (UInt32 instanceHandle, String sensorCommand, String paramName, ref Int32     paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE ExecSCmdGetDouble      (UInt32 instanceHandle, String sensorCommand, String paramName, ref Double    paramValue);
[DllImport ("MEDAQLib.dll")] public static extern ERR_CODE ExecSCmdGetString      (UInt32 instanceHandle, String sensorCommand, String paramName, StringBuilder paramValue, ref UInt32 maxLen);
******************************************************************************/

/******************************************************************************
                        Delphi type definitions
Following type definitions can be used in Delphi to declare the 
functions of MEDAQLib driver.

function CreateSensorInstance   (sensor: Integer):                                                                                                                                                        Integer; stdcall; external 'MEDAQLib.dll';
function CreateSensorInstByName (sensorName: PAnsiChar):                                                                                                                                                  Integer; stdcall; external 'MEDAQLib.dll';
function ReleaseSensorInstance  (instanceHandle: Integer):                                                                                                                                                Integer; stdcall; external 'MEDAQLib.dll';
function SetParameterInt        (instanceHandle: Integer; paramName: PAnsiChar;     paramValue: Integer):                                                                                                 Integer; stdcall; external 'MEDAQLib.dll';
function SetParameterDWORD      (instanceHandle: Integer; paramName: PAnsiChar;     paramValue: DWORD):                                                                                                   Integer; stdcall; external 'MEDAQLib.dll';
function SetParameterDouble     (instanceHandle: Integer; paramName: PAnsiChar;     paramValue: Double):                                                                                                  Integer; stdcall; external 'MEDAQLib.dll';
function SetParameterString     (instanceHandle: Integer; paramName: PAnsiChar;     paramValue: PAnsiChar):                                                                                               Integer; stdcall; external 'MEDAQLib.dll';
function SetParameterStringL    (instanceHandle: Integer; paramName: PAnsiChar;     paramValue: PAnsiChar; maxLen: Integer):                                                                              Integer; stdcall; external 'MEDAQLib.dll';
function SetParameters          (instanceHandle: Integer; paramList: PAnsiChar):                                                                                                                          Integer; stdcall; external 'MEDAQLib.dll';
function GetParameterInt        (instanceHandle: Integer; paramName: PAnsiChar; var paramValue: Integer):                                                                                                 Integer; stdcall; external 'MEDAQLib.dll';
function GetParameterDWORD      (instanceHandle: Integer; paramName: PAnsiChar; var paramValue: DWORD):                                                                                                   Integer; stdcall; external 'MEDAQLib.dll';
function GetParameterDouble     (instanceHandle: Integer; paramName: PAnsiChar; var paramValue: Double):                                                                                                  Integer; stdcall; external 'MEDAQLib.dll';
function GetParameterString     (instanceHandle: Integer; paramName: PAnsiChar;     paramValue: PAnsiChar; var maxLen: Integer):                                                                          Integer; stdcall; external 'MEDAQLib.dll';
function ClearAllParameters     (instanceHandle: Integer):                                                                                                                                                Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensor             (instanceHandle: Integer):                                                                                                                                                Integer; stdcall; external 'MEDAQLib.dll';
function CloseSensor            (instanceHandle: Integer):                                                                                                                                                Integer; stdcall; external 'MEDAQLib.dll';
function SensorCommand          (instanceHandle: Integer):                                                                                                                                                Integer; stdcall; external 'MEDAQLib.dll';
function DataAvail              (instanceHandle: Integer; var avail: Integer):                                                                                                                            Integer; stdcall; external 'MEDAQLib.dll';
function TransferData           (instanceHandle: Integer; rawData: PInteger; scaledData: PDouble; maxValues: Integer; var read: Integer):                                                                 Integer; stdcall; external 'MEDAQLib.dll';
function TransferDataTS         (instanceHandle: Integer; rawData: PInteger; scaledData: PDouble; maxValues: Integer; var read: Integer; var timestamp: Double):                                          Integer; stdcall; external 'MEDAQLib.dll';
function Poll                   (instanceHandle: Integer; rawData: PInteger; scaledData: PDouble; maxValues: Integer):                                                                                    Integer; stdcall; external 'MEDAQLib.dll';
function GetError               (instanceHandle: Integer; errText: PAnsiChar; maxLen: Integer):                                                                                                           Integer; stdcall; external 'MEDAQLib.dll';
function GetDLLVersion          (versionStr: PAnsiChar; maxLen: Integer):                                                                                                                                 Integer; stdcall; external 'MEDAQLib.dll';
function EnableLogging          (instanceHandle: Integer; enableLogging: Integer; logType: Integer; logLevel: Integer; logFile: PAnsiChar; logAppend: Integer; logFlush: Integer; logSplitSize: Integer): Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorRS232        (instanceHandle: Integer; port: PAnsiChar):                                                                                                                               Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorIF2004       (instanceHandle: Integer; cardInstance: Integer; channelNumber: Integer):                                                                                                 Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorIF2004_USB   (instanceHandle: Integer; deviceInstance: Integer; serialNumber: PAnsiChar; port: PAnsiChar; channelNumber: Integer):                                                     Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorIF2008       (instanceHandle: Integer; cardInstance: Integer; channelNumber: Integer):                                                                                                 Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorTCPIP        (instanceHandle: Integer; remoteAddr: PAnsiChar):                                                                                                                         Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorDriverX_USB  (instanceHandle: Integer; deviceInstance: Integer):                                                                                                                       Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorUSBIO        (instanceHandle: Integer; deviceInstance: Integer):                                                                                                                       Integer; stdcall; external 'MEDAQLib.dll';
function OpenSensorWinUSB       (instanceHandle: Integer; deviceInstance: Integer):                                                                                                                       Integer; stdcall; external 'MEDAQLib.dll';
function ExecSCmd               (instanceHandle: Integer; sensorCommand: PAnsiChar):                                                                                                                      Integer; stdcall; external 'MEDAQLib.dll';
function SetIntExecSCmd         (instanceHandle: Integer; sensorCommand: PAnsiChar; paramName: PAnsiChar;     paramValue: Integer):                                                                       Integer; stdcall; external 'MEDAQLib.dll';
function SetDoubleExecSCmd      (instanceHandle: Integer; sensorCommand: PAnsiChar; paramName: PAnsiChar;     paramValue: Double):                                                                        Integer; stdcall; external 'MEDAQLib.dll';
function SetStringExecSCmd      (instanceHandle: Integer; sensorCommand: PAnsiChar; paramName: PAnsiChar;     paramValue: PAnsiChar):                                                                     Integer; stdcall; external 'MEDAQLib.dll';
function ExecSCmdGetInt         (instanceHandle: Integer; sensorCommand: PAnsiChar; paramName: PAnsiChar; var paramValue: Integer):                                                                       Integer; stdcall; external 'MEDAQLib.dll';
function ExecSCmdGetDouble      (instanceHandle: Integer; sensorCommand: PAnsiChar; paramName: PAnsiChar; var paramValue: Double):                                                                        Integer; stdcall; external 'MEDAQLib.dll';
function ExecSCmdGetString      (instanceHandle: Integer; sensorCommand: PAnsiChar; paramName: PAnsiChar;     paramValue: PAnsiChar; var maxLen: Integer):                                                Integer; stdcall; external 'MEDAQLib.dll';
******************************************************************************/
