#pragma once
#include "VkViewerRemoteOperation.h"

/**
	@brief Get height data (VK-8700/9700 Compatible)
	@param *lHeightData
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted //VK Viewer is not launched by remote mode or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note Preserve data area ( size:width * height * sizeof(long)), 
	      but height changes in accordance with measurement area
*/
long VKAPI VK_GetHeightData(long* plHeightData);

/**
	@brief Get line height data (VK-8700/9700 Compatible)
	@param lLineNumber  //Line number (Only 1)
	@param *plHeightData  //Height data
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted //VK Viewer is not launched by remote mode or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note Preserve data area ( size:1024(width) * 1(height) * sizeof(long) )
*/
long VKAPI VK_GetLineHeightData(long lLineNumber, long* plHeightData);

/**
	@brief Get and set gain (VK-8700/9700 Compatible)
	@param lGain, *plGain  //Gain(Minimum value -4095)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
	@note  Scaling a gain with VK_GetLaserGain1/VK_SetLaserGain1
*/
long VKAPI VK_SetGain(long lGain);
long VKAPI VK_GetGain(long* plGain);

/**
	@brief Get and set line position (VK-8700/9700 Compatible)
	@param lLineNumber  //Line Number(1-3)
	@param lLinePosition, *plLinePosition  //Line position(0-767)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted //VK Viewer is not launched by remote mode or condition is invalid
	@retval VKResult_ConnectionLost //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetLinePosition(long lLineNumber, long lLinePosition);
long VKAPI VK_GetLinePosition(long lLineNumber, long* plLinePosition);

/**
	@brief Get and set line count (VK-8700/9700 Compatible)
	@param lNumberOfLine, *plNumberOfLine  //Line count(Only 1)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
	@note  It is not necessary to use this function (This function is prepared to keep VK-8700/VK-9700 Compatible)
*/
long VKAPI VK_SetNumberOfLine(long lNumberOfLine);
long VKAPI VK_GetNumberOfLine(long* plNumberOfLine);

/**
	@brief Get measurement condition (VK-8700/9700 Compatible)
	@param *pParameter //Struct data that has measurement condition
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError 
*/
typedef struct
{
	MeasurementMode mode;				// Mode
	MeasurementQuality quality;			// Quality
	long lUpperPosition;				// Upper limit of lens position(nm)
	long lLowerPosition;				// Lower limit of lens position(nm)
	long lPitch;						// Pitch(nm)
	long lDistance;						// Measurement distance(nm)
	long lNd;							// ND filter 1
	long lGain;							// Gain1 (VK-8700/VK-9700 Compatible, Minimum value is -4095)
	long lShutterSpeed;					// Shutter speed (VK-8700/VK-9700 Compatible 0-255)
	BOOL bIsShutterSpeedAuto;			// Shutter speed(Auto)
	long lLinePosition[3];				// Line position(pixel)
	long lLineNumber;					// Line count
	BOOL bIsRpd;						// RPD
	RpdPreference rpdPreference;		// Z axis mode
	ZoomIndex zoom;						// Zoom
	double dXYCalibration;				// XY calibration(É m/pixel)
	double dZCalibration;				// Z calibration(É m, VK-8700/VK-9700 Compatible, GetHeightData/GetLineHeightData)
	BOOL bEnableColorAcquisition;		// Enable color acquisition
	BOOL bThicknessParameterAuto;		// Auto film thickness setting
	LayerAdjustFilter filter;			// Layer adjustment filter
	PeakDetectParam peakDetectParam;	// Eliminate bad peaks
	long lThickness;					// Film thickness(nm)
} MeasurementParameter;
long VKAPI VK_GetMeasurementParameter(MeasurementParameter* pParameter);


/**
	@brief  Get intensity data (VK-8700/9700 Compatible)
	@param *psLightData  // Intensity data (14bit)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note	Preserve data area ( size : width * height * sizeof(short)), 
			but data area changes in accordance with measurement area
*/
long VKAPI VK_GetLightData(short* psLightData);

/**
	@brief  Get intensity data of line (VK-8700/9700 Compatible)
	@param lLineNumber  // Line Numbers. (Only 1)
	@param *psLightData  // Intensity data (14bit)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note 	Preserve data area ( size : 1024(width) * 1(height) * sizeof(short))
*/
long VKAPI VK_GetLineLightData(long lLineNumber, short* psLightData);

/**
	@brief  Get intensity data of measurement result (film thickness) (VK-8700/9700 Compatible)
	@param *psLightData  // Intensity data(14bit)
	@param lLayer  // Layer Numbers. (1?3)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note 	Preserve data area ( size : 1024(width) * 1(height) * sizeof(short))
*/
long VKAPI VK_GetLightDataEx(short* psLightData, long lLayer);

/**
	@brief  Get height data of measurement result ( film thickness ) (VK-8700/9700 Compatible)
	@param *lHeightData  // Height data
	@param lLayer  // Layer Numbers. (1?3)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note	Preserve data area ( size : width * height * sizeof(long)), 
			but data area changes in accordance with measurement area
*/
long VKAPI VK_GetHeightDataEx(long* plHeightData, long lLayer);

/**
	@brief  Get intensity data of measurement result ( film thickness of line ) (VK-8700/9700 Compatible)
	@param *psLightData  // Intensity data of measurement result (14bit)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note At first, get data size (width and height) by VK_GetThicknessHeightDataSize. 
	      Preserve data area ( size : width * height * sizeof(short) )
*/
long VKAPI VK_GetThicknessLightData(short* psLightData);

/**
	@brief  Get height data of measurement result ( film thickness of line ) (VK-8700/9700 Compatible)
	@param *plHeightData  // height data of measurement result
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note At first, get data size (width and height) by VK_GetThicknessHeightDataSize. 
	      Preserve data area ( size : width * height * sizeof(long) )
*/
long VKAPI VK_GetThicknessHeightData(long* plHeightData);

/**
	@brief  Get and set shutterspeed settings (VK-8700/9700 Compatible)
	@param lShutterSpeed, *plShutterSpeed  // shutterspeed (0-255)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetShutterSpeed(long lShutterSpeed);
long VKAPI VK_GetShutterSpeed(long* plShutterSpeed);

/**
	@brief  Set measurement pitch by integer (VK-8700/9700 Copatible)
	@param lPitch  // pitch
	@retval VKResult_OK 
	@retval VKResult_NotInitialized 
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
	@note   You could be set fixed value as pitch ( Fixed value are defined by interface )
*/
long VKAPI VK_SetPitchValue(long lPitch);

/**
	@brief  Get measurement condition of measurement result (VK-8700/9700 Compatible)
	@param *pParameter  // Struct data of measurement condition
	@retval VKResult_OK 
	@retval VKResult_NotInitialized 
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode, or condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError 
*/
long VKAPI VK_GetMeasurementResultParameter(MeasurementParameter* pParameter);
