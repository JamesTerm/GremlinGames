#pragma once

#include "Vision/EnumCameraParameter.h"
#include "ErrorBase.h"
#include "Vision/IntCameraParameter.h"
#include "Task.h"

#include <vector>
#include <taskLib.h>
#include <vxWorks.h> 
#include <sockLib.h> 
#include <inetLib.h>
#include <set>

//#include "AxisCameraParams2.h"
#include "Vision/ColorImage.h"
#include "Vision/HSLImage.h"
#include "nivision.h"
#include "Task.h"

/**
* AxisCameraParams2 class.
* This class handles parameter configuration the Axis 206 Ethernet Camera.
* It starts up a tasks with an independent connection to the camera that monitors
* for changes to parameters and updates the camera.
* It is only separate from AxisCamera2 to isolate the parameter code from the image streaming code.
*/
class AxisCameraParams2: public ErrorBase
{
public:
	typedef enum Exposure_t {kExposure_Automatic, kExposure_Hold, kExposure_FlickerFree50Hz, kExposure_FlickerFree60Hz};
	typedef enum WhiteBalance_t {kWhiteBalance_Automatic, kWhiteBalance_Hold, kWhiteBalance_FixedOutdoor1, kWhiteBalance_FixedOutdoor2, kWhiteBalance_FixedIndoor, kWhiteBalance_FixedFlourescent1, kWhiteBalance_FixedFlourescent2};
	typedef enum Resolution_t {kResolution_640x480, kResolution_640x360, kResolution_320x240, kResolution_160x120};
	typedef enum Rotation_t {kRotation_0, kRotation_180};

protected:
	AxisCameraParams2(const char* ipAddress);
	virtual ~AxisCameraParams2();

public:
	// Mid-stream gets & writes
	void WriteBrightness(int);
	int GetBrightness();
	void WriteWhiteBalance(WhiteBalance_t whiteBalance);
	WhiteBalance_t GetWhiteBalance();
	void WriteColorLevel(int);
	int GetColorLevel();
	void WriteExposureControl(Exposure_t);
	Exposure_t GetExposureControl();
	void WriteExposurePriority(int);
	int GetExposurePriority();
	void WriteMaxFPS(int);
	int GetMaxFPS();

	// New-Stream gets & writes (i.e. require restart)
	void WriteResolution(Resolution_t);
	Resolution_t GetResolution();
	void WriteCompression(int);
	int GetCompression();
	void WriteRotation(Rotation_t);
	Rotation_t GetRotation();

protected:
	virtual void RestartCameraTask() = 0;
	int CreateCameraSocket(const char *requestString);

	static int s_ParamTaskFunction(AxisCameraParams2* thisPtr);
	int ParamTaskFunction();

	int UpdateCamParam(const char *param);
	int ReadCamParams();

	Task m_paramTask;
	UINT32 m_ipAddress; // IPv4
	SEM_ID m_paramChangedSem;
	SEM_ID m_socketPossessionSem;

	//Camera Properties
	IntCameraParameter *m_brightnessParam;
	IntCameraParameter *m_compressionParam;
	IntCameraParameter *m_exposurePriorityParam;
	IntCameraParameter *m_colorLevelParam;
	IntCameraParameter *m_maxFPSParam;
	EnumCameraParameter *m_rotationParam;
	EnumCameraParameter *m_resolutionParam;
	EnumCameraParameter *m_exposureControlParam;
	EnumCameraParameter *m_whiteBalanceParam;

	// A vector to access all properties simply.
	typedef std::vector<IntCameraParameter*> ParameterVector_t;
	ParameterVector_t m_parameters;
};



/**
* AxisCamera2 class.
* This class handles everything about the Axis 206 FRC Camera.
* It starts up 2 tasks each using a different connection to the camera:
* - image reading task that reads images repeatedly from the camera
* - parameter handler task in the base class that monitors for changes to
*     parameters and updates the camera
*/
class AxisCamera2: public AxisCameraParams2
{
	AxisCamera2(const char *cameraIP = "192.168.0.90");
public:
	virtual ~AxisCamera2();
	static AxisCamera2& GetInstance();
	void DeleteInstance();

	bool IsFreshImage();
	SEM_ID GetNewImageSem();

	int GetImage(Image *imaqImage);
	int GetImage(ColorImage *image);
	HSLImage *GetImage();

	int CopyJPEG(char **destImage, int &destImageSize, int &destImageBufferSize);

private:
	static int s_ImageStreamTaskFunction(AxisCamera2 *thisPtr);
	int ImageStreamTaskFunction();

	int ReadImagesFromCamera();
	void UpdatePublicImageFromCamera(char *imgBuffer, int imgSize);

	virtual void RestartCameraTask();

	static AxisCamera2 *m_instance;
	int m_cameraSocket;
	typedef std::set<SEM_ID> SemSet_t;
	SemSet_t m_newImageSemSet;

	char* m_protectedImageBuffer;
	int m_protectedImageBufferLength;
	int m_protectedImageSize;
	SEM_ID m_protectedImageSem;
	bool m_freshImage;

	Task m_imageStreamTask;
};

extern "C" {
	void AxisCameraStart();
	int AxisCameraGetImage(Image *image);
	void AxisCameraDeleteInstance();
	int AxisCameraFreshImage();

	// Mid-stream gets & writes
	void AxisCameraWriteBrightness2(int brightness);
	int AxisCameraGetBrightness2();
	void AxisCameraWriteWhiteBalance2(AxisCameraParams2::WhiteBalance_t whiteBalance);
	AxisCameraParams2::WhiteBalance_t AxisCameraGetWhiteBalance2();
	void AxisCameraWriteColorLevel2(int colorLevel);
	int AxisCameraGetColorLevel2();
	void AxisCameraWriteExposureControl2(AxisCameraParams2::Exposure_t exposure);
	AxisCameraParams2::Exposure_t AxisCameraGetExposureControl2();
	void AxisCameraWriteExposurePriority2(int exposurePriority);
	int AxisCameraGetExposurePriority2();
	void AxisCameraWriteMaxFPS2(int maxFPS);
	int AxisCameraGetMaxFPS2();

	// New-Stream gets & writes
	void AxisCameraWriteResolution2(AxisCameraParams2::Resolution_t resolution);
	AxisCameraParams2::Resolution_t AxisCameraGetResolution2();
	void AxisCameraWriteCompression2(int compression);
	int AxisCameraGetCompression2();
	void AxisCameraWriteRotation2(AxisCameraParams2::Rotation_t rotation);
	AxisCameraParams2::Rotation_t AxisCameraGetRotation2();
}
