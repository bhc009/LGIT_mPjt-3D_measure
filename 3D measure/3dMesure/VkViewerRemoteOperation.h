#pragma once

#ifndef VKAPI
#define VKAPI	__stdcall
#endif

/**
	@brief generic return code
*/
enum VKResult
{
	VKResult_OK = 0,								
	VKResult_UnknownError = -1,						
	VKResult_AlreadyInitialized = -2,				
	VKResult_NotInitialized = -3,					
	VKResult_NotAccepted = -5,						/**< Failed to connect the application */
	VKResult_InvalidArgument = -6,					
	VKResult_ReachedLimit = -7,						/**< Reached Upper or lower limit */
	VKResult_ShortageDistance = -9,					/**< The Z measure distance is too short  */
	VKResult_ExcessDistance = -10,					/**< The Z measure distance is too large  */
	VKResult_NotSupportFaceFilmThickness = -11,		/**< Top surface measurement cannot be performed */
	VKResult_ExcessPitch = -13,						/**< The pitch is too large */
	VKResult_OutOfMemory = -14,						
	VKResult_AutoProcessCancel = -15,				/**< Cancel auto processing */
	VKResult_AutoProcessChangedRevolver = -16,		/**< The revolver was rotated during autofocus */
	VKResult_NotSupportThicknessParam = -17,		/**< Film thickness measurement cannot be performed */
	VKResult_NotSupportTopLayer = -20,				/**< Unused */
	VKResult_PmtProtectionDetected = -22,			/**< The photoreceptor element protection function went into operation and processing was stopped */
	VKResult_ConnectionLost = -23,					/**< The controller was disconnected while processing */
	VKResult_LaserFailureAbort = -24,				/**< A laser output error has been detected and processing was stopped */
	VKResult_ComputerSuspended = -25,				/**< The computer went into sleep mode and processing was stopped */
	VKResult_AutoFocusFail = -26,					/**< Failed to perform autofocus */
	VKResult_StillZMoving = -27,					/**< Cannot move while moving the lens */
	VKResult_ControllerIncompatible = -28,			/**< The application version and the controller version do not match */
	VKResult_NotSupportedZoomAndSize = -29,			/**< Cannot use combination of zoom and size */
	VKResult_InvalidMeasureRange = -30,				/**< The current measurement range exceeds the operation range */
};

/**
	@brief Error code
*/
enum VKError
{
	VKError_NoError = 0,				
	VKError_Unknown = -1,				
	VKError_Cancel = -2,				/**< Cancel measurement */
	VKError_ChangedRevolver = -4,		/**< The revolver changed */
	VKError_ConnectionLost = -6,		/**< The controller was disconnected */
	VKError_PmtProtection = -7,			/**< The photoreceptor element protection function went into operation and processing was stopped */
	VKError_LaserFailure = -8,			/**< A laser output error has been detected and processing was stopped */
	VKError_ComputerSuspended = -9,		/**< The computer went into sleep mode and processing was stopped */
	VKError_ReachedZAxisLimit = -10,	/**< Reached Upper or lower limit and processing was stopped */
};

/**
	@brief Initialization
	@retval VKResult_OK 
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
	@note  At first, call this function when you want to use remote mode
*/
long VKAPI VK_Initialize();

/**
	@brief finalization
	@retval VKResult_OK
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_UnknownError
	@note  Call this function after remote mode was finished
*/
long VKAPI VK_Uninitialize();

/**
	@brief Get and set measurement mode
	@param mode, *pMode FaceMeasurement：Standard
						LineMeasurement：1 Line
						FaceFilmThicknessMeasurement：Film thickness(standard)
						LineFilmThicknessMeasurement：Film thickness(1 Line)
						FaceTopLayerMeasurement : Top surface(standard)
						LineTopLayerMeasurement : Top surface(1 Line)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_NotSupportThicknessParam  //Top surface measurement cannot be performed
	@retval VKResult_UnknownError
*/
typedef enum {
	Face,
	Line,
	FaceFilmThickness,
	LineFilmThickness,
	FaceTopLayer,
	LineTopLayer
} MeasurementMode;
long VKAPI VK_SetMeasurementMode(MeasurementMode mode);
long VKAPI VK_GetMeasurementMode(MeasurementMode* pMode);

/**
	@brief  Get and set measurement quality
	@param quality, *pQuality HighSpeed：Standard and Ultra high-speed
							  HighResolution：Standard and high accuracy
							  HighDensity：Super fine and high accuracy
							  Part1of12Accuracy：Part 1/12 and high accuracy
							  Part1of12SuperHighSpeed：Part 1/12 and ultra high speed
							  StandardHighSpeed：Sandard and high speed
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
typedef enum {
	HighSpeed,
	HighResolution,
	HighDensity,
	Part1Of12Accuracy,
	Part1Of12SuperHighSpeed,
	StandardHighSpeed,
} MeasurementQuality;
long VKAPI VK_SetMeasurementQuality(MeasurementQuality quality);
long VKAPI VK_GetMeasurementQuality(MeasurementQuality* pQuality);

/**
	@brief Get and set view type
	@param type, *pType	Camera
						Laser
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
typedef enum {
	Camera,
	Laser,
} ViewType;
long VKAPI VK_SetViewType(ViewType type);
long VKAPI VK_GetViewType(ViewType* pType);

/**
	@brief Move lens
	@param lMove  Move amount(nm) [When move amount is positive number, the revolver move lower direction
									When move amount is negative number, the revolver move upper direction]
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_ReachedLimit  //Reached Upper or lower limit and cannot move
	@retval VKResult_UnknownError
*/
long VKAPI VK_MoveLens(long lMove);

/**
	@brief Move to the Z origin
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
	@note  When moving the lens, execute after lowering the stage as it may collide with the sample
*/
long VKAPI VK_MoveLensToOrigin();

/**
	@brief Get lens position
	@param *plPosition  //Lens position(nm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_GetLensPosition(long* plPosition);

/**
	@brief  Get and set upper limit
	@param lPosition, *plPosition  //Lens position(nm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
	@note   If VK_SrtUpperPosition is called without argument, current lens position is set in upper limit
*/
long VKAPI VK_SetUpperPosition(long lPosition = -1);
long VKAPI VK_GetUpperPosition(long* plPosition);

/**
	@brief  Get and set upper limit
	@param lPosition, *plPosition   //Lens position(nm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
	@note   If VK_SrtUpperPosition is called without argument, current lens position is set in lower limit
*/
long VKAPI VK_SetLowerPosition(long lPosition = -1);
long VKAPI VK_GetLowerPosition(long* plPosition);

/**
	@brief Set pitch
	@param pitch Pitch_001um：0.01μm
				 Pitch_002um：0.02μm
				 Pitch_005um：0.05μm
				 Pitch_01um：0.1μm
				 Pitch_02um：0.2μm
				 Pitch_05um：0.5μm
				 Pitch_1um：1μm
				 Pitch_2um：2μm
				 Pitch_5um：5μm
				 Pitch_10um：10μm
				 Pitch_20um：20μm
				 Pitch_50um：50μm
				 Pitch_100um：50μm
				 Pitch_200um：50μm
				 Pitch_500um：50μm
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
typedef enum {
	Pitch_001um,
	Pitch_002um,
	Pitch_005um,
	Pitch_01um,
	Pitch_02um,
	Pitch_05um,
	Pitch_1um,
	Pitch_2um,
	Pitch_5um,
	Pitch_10um,
	Pitch_20um,
	Pitch_50um,
	Pitch_100um,
	Pitch_200um,
	Pitch_500um,
} PitchIndex;
long VKAPI VK_SetPitch(PitchIndex pitch);

/**
	@brief  Get pitch
	@param *plPitch  //Pitch(nm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument 
	@retval VKResult_UnknownError
*/
long VKAPI VK_GetPitch(long* plPitch);

/**
	@brief  Get and set ND filter
	@param lNd, *plNd   //ND filter(0-4)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetNd(long lNd);
long VKAPI VK_GetNd(long* plNd);

/**
	@brief  Get and set gain
	@param lGain, *plGain   //Gain (Minimum value -16000)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetLaserGain1(long lGain);
long VKAPI VK_GetLaserGain1(long* plGain);

/**
	@brief  Get and set line position
	@param lLinePosition, *plLinePosition  //Line position(0-767)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError 
*/
long VKAPI VK_SetMeasureLinePosition(long lLinePosition);
long VKAPI VK_GetMeasureLinePosition(long* plLinePosition);

/**
	@brief  Get and set RPD
	@param bIsRpd, *pbIsRpd  //Whether RPD is set or not
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetRpd(BOOL bIsRpd);
long VKAPI VK_IsRpd(BOOL* pbIsRpd);

/**
	@brief  Get and set Z axis mode for RPD
	@param rpdPreference, *pRpdPreference
			Quality		//Accuracy priority
			Speed		//Speed priority
			Automatic	//Auto( Z axis mode depends on lens magnification )
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
typedef enum {
	Quality,
	Speed,
	Automatic,
} RpdPreference;
long VKAPI VK_SetRpdPreference(RpdPreference rpdPreference);
long VKAPI VK_GetRpdPreference(RpdPreference* pRpdPreference);

/**
	@brief  Get and set zoom
	@param zoom, pZoom Zoom_10x   1.0x
					   Zoom_15x   1.5x
					   Zoom_20x   2.0x
					   Zoom_30x   3.0x
					   Zoom_50x   5.0x
					   Zoom_80x   8.0x
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
typedef enum {
	Zoom_10x,
	Zoom_15x,
	Zoom_20x,
	Zoom_30x,
	Obsoleted_Zoom_40x,
	Obsoleted_Zoom_60x,
	Zoom_50x,
	Zoom_80x,
} ZoomIndex;
long VKAPI VK_SetZoom(ZoomIndex zoom);
long VKAPI VK_GetZoom(ZoomIndex* pZoom);

/**
	@brief Get and set color acquisition
	@param bEnableColorAcquisition, *pbEnableColorAcquisition   //Whether color information are obtained or not
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetEnableColorAcquisition(BOOL bEnableColorAcquisition);
long VKAPI VK_GetEnableColorAcquisition(BOOL* pbEnableColorAcquisition);

typedef enum {
	None,
    Weak,
    Middle,
    Strong,
} LayerAdjustFilter;

typedef enum {
    PeakDetect_Standard,
    PeakDetect_Strong,
    PeakDetect_SpecialStrong,
} PeakDetectParam;

/**
	@brief  Autofocus
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_DoAutofocus();

/**
	@brief  Start measurement
	@retval VKResult
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //Measuring, VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_ShortageDistance   //The Z measure distance is too short
	@retval VKResult_ExcessDistance   //The Z measure distance is too long
	@retval VKResult_NotSupportFaceFilmThickness   //Film thickness measurement cannot be performed with this lens
	@retval VKResult_NotSupportTopLayer   //Top surface measurement cannot be performed with this lens.
	@retval VKResult_UnknownError
*/
long VKAPI VK_StartMeasurement();

/**
	@brief   Stop measuring
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   // Not measuring, VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_StopMeasurement();

/**
	@brief  Whether vk4 is measuring or not
	@param pbIsMeasuring  //Whether vk4 is measuring or not
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_IsMeasuring(BOOL* pbIsMeasuring);

/**
	@brief  Measurement result
	@param plLastError   //Reference VKError
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_GetLastError(long* plLastError);

/**
	@brief Acquisition laser+color image
	@param *pbyColorData  //Color data
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note  Preserve data area ( size : width * height * 3(RGB) * sizeof(byte) ),
			but height changes in accordance measurement area
*/
long VKAPI VK_GetColorData(BYTE* pbyColorData);

/**
	@brief  Get height data
	@param *lHeightData   //Height data
	@param dataType    HeightDataType_Unit1Nanometer   //By 1nm (VK-8700/9700 Compatible)
					   HeightDataType_Unit100Picometer  //By 0.1nm(=100pm)
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note   Preserve data area ( size : width * height * sizeof(long) ),
			but height changes in accordance measurement area
*/
typedef enum {
    HeightDataType_Unit1Nanometer,
    HeightDataType_Unit100Picometer,
} HeightDataType;
long VKAPI VK_GetHeightData2(long* plHeightData, HeightDataType dataType);

/**
	@brief  Get height data of line
	@param *plHeightData   //Height data
	@param dataType    HeightDataType_Unit1Nanometer   //By 1nm (VK-8700/9700 Compatible)
					   HeightDataType_Unit100Picometer   //By 0.1nm(=100pm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note   Preserve data area ( size : 1024(width) * 1(height) * sizeof(long) )
*/
long VKAPI VK_GetLineHeightData2(long* plHeightData, HeightDataType dataType);

/**
	@brief  Get measurement condition
	@param *pParameter   //Struct data that has measurement condition
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
	@note   Substitute "sizeof(MeasurementParameter2) or sizeof(MeasurementParameter3)" for nSize
*/
typedef struct
{
	size_t nSize;						// Struct size
	MeasurementMode mode;				// Mode
	MeasurementQuality quality;			// Quality
	long lUpperPosition;				// Upper limit of lens position(nm)
	long lLowerPosition;				// Lower lomit of lens position(nm)
	long lPitch;						// Pitch(nm)
	long lDistance;						// Measurement distance(nm)
	long lNd;							// ND filter 1
	long lGain;							// Gain 1 (VK-8700/VK-9700 Compatible minimum value -4095)
	long lShutterSpeed;					// Shutter speed (VK-8700/VK-9700 Compatible 0-255)
	BOOL bIsShutterSpeedAuto;			// Shutter speed(auto)
	long lLinePosition[3];				// Line position(pixel)
	long lLineNumber;					// Line count
	BOOL bIsRpd;						// RPD
	RpdPreference rpdPreference;		// Z axis mode
	ZoomIndex zoom;						// Zoom
	double dXYCalibration;				// XY calibration(μm/pixel)
	double dZCalibrationFactor;			// Z calibration(μm, VK-8700/VK-9700 Compatible, GetHeightData/GetLineHeightData)
	BOOL bEnableColorAcquisition;		// Enable color acquisition
	BOOL bThicknessParameterAuto;		// Auto film thickness setting
	LayerAdjustFilter filter;			// Layer adjustment filter
	PeakDetectParam peakDetectParam;	// Eliminate bad peaks
	long lThickness;					// Film thickness(nm)
	long lMeasureLinePosition;			// Line measurement position
	long lLaserGain1;					// Laser brightness 1 (Minimum value -16000)
	long lCameraShutterSpeed;			// Shutter speed (0-789)
} MeasurementParameter2;
long VKAPI VK_GetMeasurementParameter2(void* pParameter);

/**
	@brief Save measurement result
	@param *pcFileName   //Save file name
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_SaveMeasurementResult(const char* szFileName);
long VKAPI VK_SaveMeasurementResultW(const wchar_t* wszFileName);

/**
	@brief  Save snapshot
	@param *pcFileName   //Save file name
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost   //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_SaveSnapshot(const char* szFileName);
long VKAPI VK_SaveSnapshotW(const wchar_t* wszFileName);

/**
	@brief  Get and set RPD pitch
	@param dataType    RpdPitch_Normal  //Accuracy priority
					   RpdPitch_HighSpeed  //Speed priority
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted   //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
	@note   Film thickness and Top surface measurements cannot be performed with speed priority of RPD pitch. 
	 		If you try setting speed priority of RPD pitch for them, the argument is regarded as invalid. 
*/
typedef enum
{
	RpdPitch_Normal = 1,		/**< Accuracy priority */
	RpdPitch_HighSpeed = 3,		/**< Speed priority */
} RpdPitch;
long VKAPI VK_GetRpdPitch(RpdPitch* value);
long VKAPI VK_SetRpdPitch(RpdPitch value);

/**
	@brief  The event ID is caused by the controler
*/
typedef enum 
{
	PmtProtectionDetected = 1,	/**< Detected the photoreceptor element protection function */
} VKHardwareEventId;
/**
	@brief   //The callback function which communicates event is caused by controler
	@param eventId
	@param pvUserParam   //User's parameter
*/
typedef void (__stdcall *VKHardwareEventCallback)(VKHardwareEventId eventId, void* pvUserParam);

/**
	@brief  Setting the callback function which is called and caused by hardware event
	@param pCallback   //Pointer of callback function ( When the pointer is null, setting is canceled )
	@param pvUserParam   //User's parameter
	@retval VKResult_NotInitialized
	@note  Only one function could be registered. 
	       In the case that you have alreafy registered some function, 
	       if you registered another function, old function is disapered. 
*/
long VKAPI VK_SetHardwareEventCallback(VKHardwareEventCallback pCallback, void* pvUserParam);

/**
	@brief Camera autofocus
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
	@retval VKResult_AutoFocusFail //Failed to do camera autofocus
*/
long VKAPI VK_DoCameraAutofocus();

/**
	@brief Do auto gain (Supported all measurement area)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_DoAutoGain();

/**
	@brief Do auto gain (Depended on current measurement area)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_DoAutoGain2();

/**
	@brief  Do set auto up/low pos. 
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_DoAutoUpperAndLowerPositions();

/**
	@brief Get color information
	@param *pbyCcdData Color information
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
	@note Preserve data area ( size : width * height * 3(RGB) ),
		  but data area changes in accordance measurement area
*/
long VKAPI VK_GetCcdData(BYTE* pbyCcdData);

/**
	@brief Get C-Laser DIC image
	@param *pbyDifferentiationData C-Laser DIC image data
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
	@note Preserve data area ( size : width * height * 3(RGB) ),
		  but data area changes in accordance measurement area
*/
long VKAPI VK_GetDifferentiationData(BYTE* pbyDifferentiationData);

/**
	@brief  Save measurement condition
	@param *szFileName File name
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SaveMeasurementParameter(const char* szFileName);
long VKAPI VK_SaveMeasurementParameterW(const wchar_t* wszFileName);

/**
	@brief  Load measurement condition
	@param *szFileName  File name
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_LoadMeasurementParameter(const char* szFileName);
long VKAPI VK_LoadMeasurementParameterW(const wchar_t* wszFileName);

/**
	@brief  Get light data (16bit)
	@param *pusLightData light data(16bit)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note Preserve data area ( size : width * height * sizeof(unsigned short) ),
		  but data area changes in accordance measurement area
*/
long VKAPI VK_GetLightData2(unsigned short* pusLightData);

/**
	@brief  Get light data of line (16bit)
	@param *pusLightData light data (16bit)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note Preserve data area ( size : 1024(width) * 1(height) * sizeof(unsigned short) )
*/
long VKAPI VK_GetLineLightData2(unsigned short* pusLightData);

/**
	@brief  Get light data of measurement result (film thickness of standard)
	@param *psLightData light data(16bit)
	@param lLayer  layer Numbers (1?3)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note Preserve data area ( size : 1024(width) * 1(height) * sizeof(unsigned short) )
*/
long VKAPI VK_GetLightDataEx2(unsigned short* pusLightData, long lLayer);

/**
	@brief  Get height data of measurement result (film thickness of standard)
	@param *lHeightData  height data
	@param lLayer  layer Numbers (1?3)
	@param dataType    HeightDataType_Unit1Nanometer  // By 1nm (VK-8700/9700 Compatible)
					   HeightDataType_Unit100Picometer  // By 0.1nm(=100pm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note Preserve data area ( size : width * height * sizeof(long) ),
		  but data area changes in accordance measurement area
*/
long VKAPI VK_GetHeightDataEx2(long* plHeightData, long lLayer, HeightDataType dataType);

/**
	@brief  Get light data of measurement result (film thickness of line)
	@param *plWidth  // width
	@param *plHeight  // height
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
*/
long VKAPI VK_GetThicknessLightDataSize(long* plWidth, long* plHeight);

/**
	@brief  Get light data of measurement data(16bit) (film thickness of line)
	@param *psLightData  light data of measurement result (16bit)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note At first, get data size (width and height) by VK_GetThicknessLightDataSize. 
	      Preserve data area ( size : width * height * sizeof(unsigned short) )
*/
long VKAPI VK_GetThicknessLightData2(unsigned short* pusLightData);

/**
	@brief  Get data size (height) of measurement result (film thickness of line)
	@param *plWidth  // width
	@param *plHeight  // height
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
*/
long VKAPI VK_GetThicknessHeightDataSize(long* plWidth, long* plHeight);

/**
	@brief  Get height data of measurement result (film thickness of line)
	@param *plHeightData  // height data of measurement result
	@param dataType    HeightDataType_Unit1Nanometer  // By 1nm (VK-8700/9700 Compatible)
					   HeightDataType_Unit100Picometer  // By 0.1nm(=100pm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_OutOfMemory
	@retval VKResult_UnknownError
	@note At first, get data size (width and height) by VK_GetThicknessHeightDataSize. 
	      Preserve data area ( size : width * height * sizeof(long) )
*/
long VKAPI VK_GetThicknessHeightData2(long* plHeightData, HeightDataType dataType);

/**
	@brief  Reset total illumination time of halogen lamp
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_ResetTotalHalogenLightingSeconds();

/**
	@brief  Get total illumination time of halogen lamp
	@param *plSeconds  // total illumination time (by seconds)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_GetTotalHalogenLightingSeconds(long* plSeconds);

/**
	@brief  Get and set measurement distance
	@param lDistance, *plDistance  // measurement distance (by nm)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetDistance(long lDistance);
long VKAPI VK_GetDistance(long* plDistance);

/**
	@brief  Get and set shutterspeed
	@param lShutterSpeed, *plShutterSpeed  // shutterspeed (0-789)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetCameraShutterSpeed(long lShutterSpeed);
long VKAPI VK_GetCameraShutterSpeed(long* plShutterSpeed);

/**
	@brief  Get and set auto shutterspeed mode
	@param bIsShutterSpeedAuto, *pbIsShutterSpeedAuto  // whether shutterspeed mode is auto or not
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetShutterSpeedAuto(BOOL bIsShutterSpeedAuto);
long VKAPI VK_IsShutterSpeedAuto(BOOL* pbIsShutterSpeedAuto);

/**
	@brief  Get and set target shutterspeed (auto)
	@param lAutoReferenceData, *plAutoReferenceData  // target shutterspeed (auto) (0-255)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetShutterSpeedAutoReferenceData(long lAutoReferenceData);
long VKAPI VK_GetShutterSpeedAutoReferenceData(long* plAutoReferenceData);

/**
	@brief  Get and set dim lamp light
	@param bEnabled, *pbEnabled  // whether dim lamp light is valid or mot
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetHalogenFilterEnabled(BOOL bEnabled);
long VKAPI VK_IsHalogenFilterEnabled(BOOL* pbEnabled);

/**
	@brief  Get and set double scan setting
	@param value, *pvalue DoubleScanState_Off
                          DoubleScanState_OnManualBrightness  // ON (Brightness is manual setting)
                          DoubleScanState_AutoJudgement Auto setting
                          DoubleScanState_OnAutoBrightness  // ON (Brightness is auto setting)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
typedef enum
{
	DoubleScanState_Off = 0,
	DoubleScanState_OnManualBrightness = 1,
	DoubleScanState_AutoJudgement = 2,
	DoubleScanState_OnAutoBrightness = 3,
} DoubleScanState;
long VKAPI VK_SetDoubleScanEnabled(DoubleScanState value);
long VKAPI VK_IsDoubleScanEnabled(DoubleScanState* pvalue);

/**
	@brief  Get and set brightness 2
	@param lGain, *plGain  // brightness 2 ( minimum value is -16000)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetLaserGain2(long lGain);
long VKAPI VK_GetLaserGain2(long* plGain);

/**
	@brief  Get and set ND filtere 2
	@param lNd, *plNd  // ND filter 2 (0-4)
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetNd2(long lNd);
long VKAPI VK_GetNd2(long* plNd);


typedef enum
{
	FilterState_Off = 0,
	FilterState_On = 1,
	FilterState_Auto = 2,
} FilterState;

/**
	@brief  Get and set AI noise reduction setting
	@param value, *pvalue FilterState_Off  // OFF
                          FilterState_On  // ON
                          FilterState_Auto  // Auto
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetAINoiseEliminationEnabled(FilterState value);
long VKAPI VK_IsAINoiseEliminationEnabled(FilterState* pvalue);

/**
	@brief  Get and set angled surface noise filter
	@param value, *pvalue FilterState_Off  // OFF
                          FilterState_On  // ON
                          FilterState_Auto  // Auto
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetAngledSurfaceNoiseFilterEnabled(FilterState value);
long VKAPI VK_IsAngledSurfaceNoiseFilterEnabled(FilterState* pvalue);

/**
	@brief  Get and set Z calibration
	@param dFactor, *pdFactor  // Z calibration
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetZCalibrationFactor(double dFactor);
long VKAPI VK_GetZCalibrationFactor(double* pdFactor);

typedef struct
{
	size_t nSize;						// Struct size
	MeasurementMode mode;				// Mode
	MeasurementQuality quality;			// Quality
	long lUpperPosition;				// Upper limit of lens position(nm)
	long lLowerPosition;				// Lower limit of lens position(nm)
	long lPitch;						// Pitch (nm)
	long lDistance;						// Measurement distance(nm)
	long lNd;							// ND filter 1
	long lGain;							// Gain 1 (VK-8700/VK-9700 Compatible minimum value -4095)
	long lShutterSpeed;					// Shutter speed (VK-8700/VK-9700 Compatible 0-255)
	BOOL bIsShutterSpeedAuto;			// Shutter speed(auto)
	long lLinePosition[3];				// Line position(pixel)
	long lLineNumber;					// Line count
	BOOL bIsRpd;						// RPD
	RpdPreference rpdPreference;		// Z axis mode
	ZoomIndex zoom;						// Zoom
	double dXYCalibration;				// XY calibration(μm/pixel)
	double dZCalibrationFactor;			// Z calibration(μm, VK-8700/VK-9700 Compatible, GetHeightData/GetLineHeightData)
	BOOL bEnableColorAcquisition;		// Enable color acquisition
	BOOL bThicknessParameterAuto;		// Auto film thickness setting
	LayerAdjustFilter filter;			// Layer adjustment filter
	PeakDetectParam peakDetectParam;	// Eliminate bad peaks
	long lThickness;					// Film thickness(nm)
	long lMeasureLinePosition;			// Line measurement position
	long lLaserGain1;					// Laser brightness 1 (Minimum value -16000)
	long lCameraShutterSpeed;			// Shutter speed (0-789)
	DoubleScanState doubleScanEnabled;	// Double scan settings value
	BOOL bDoubleScanApplied;			// Whther double scan was applied or not
	long lLaserGain2;					// Laser brightness (minimum value -16000)
	long lNd2;							// ND filter 2
	double dZeroHeight;					// Abusolute position of height 0 (By um) 
	BOOL bHalogenFilterEnabled;			// Dim lamp light
	FilterState aiNoiseEliminationEnabled;	// AI noise reduction settings value
	BOOL bAINoiseEliminationApplied;	// Whether AI noise reduction was applied or not
	FilterState angledSurfaceNoiseFilterEnabled;	// Angled surface noise filter settings value
	BOOL bAngledSurfaceNoiseFilterApplied;	//  Whether angled surface noise filter was applied or not
	RpdPreference rpdPreferenceApplied;	// Z axis mode which are used
	long lAutoReferenceData;			// Target value of shutter speed(0-255)
} MeasurementParameter3;


/**
	@brief  // Get measurement condition of measurement result
	@param *pParameter  // Struct data of measurement condition
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_ConnectionLost  //The controller is disconnected or has been disconnected
	@retval VKResult_UnknownError
	@note   Substitute "sizeof(MeasurementParameter2) or sizeof(MeasurementParameter3)" for nSize
*/
long VKAPI VK_GetMeasurementResultParameter2(void* pParameter);

/**
	@brief  // Set halogen lamp state
	@param bLampState true:on false:off
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_InvalidArgument
	@retval VKResult_UnknownError
*/
long VKAPI VK_SetHalogenLampState(BOOL bLampState );

/**
	@brief  // Get halogen lamp state
	@retval VKResult_OK
	@retval VKResult_NotInitialized
	@retval VKResult_NotAccepted  //VK Viewer is not launched by remote mode or Condition is invalid
	@retval VKResult_UnknownError
*/
long VKAPI VK_GetHalogenLampState(BOOL* bLampState );
