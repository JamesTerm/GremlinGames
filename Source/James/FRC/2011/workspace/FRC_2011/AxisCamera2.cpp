//WPI Camera Fix------------------------------------------------------------------------------------------------------------------------

/***********************************************************************************************************/
/*												AxisCameraParams2											*/
/***********************************************************************************************************/

//#include "AxisCameraParams2.h"
#include "AxisCamera2.h"

#include <inetLib.h>
#include "pcre.h"
#include <sockLib.h>
#include <string.h>
#include "Synchronized.h"
#include "Timer.h"
#include <string.h>
#include "Synchronized.h"
#include "AxisCamera2.h"
#include "Vision/PCVideoServer.h"

static const char *const kRotationChoices[] = {"0", "180"};
static const char *const kResolutionChoices[] = {"640x480", "640x360", "320x240", "160x120"};
static const char *const kExposureControlChoices[] = { "automatic", "hold", "flickerfree50", "flickerfree60" };
static const char *const kWhiteBalanceChoices[] = { "auto", "holdwb", "fixed_outdoor1","fixed_outdoor2", "fixed_indoor", "fixed_fluor1", "fixed_fluor2" };

AxisCameraParams2::AxisCameraParams2(const char* ipAddress)
: m_paramTask("paramTask", (FUNCPTR) s_ParamTaskFunction, Task::kDefaultPriority, 64000)
, m_ipAddress (inet_addr((char*)ipAddress))
, m_paramChangedSem (NULL)
, m_socketPossessionSem (NULL)
{
	m_brightnessParam = new IntCameraParameter("ImageSource.I0.Sensor.Brightness=%i",
		"root.ImageSource.I0.Sensor.Brightness=(.*)", false);
	m_parameters.push_back(m_brightnessParam);
	m_colorLevelParam = new IntCameraParameter("ImageSource.I0.Sensor.ColorLevel=%i",
		"root.ImageSource.I0.Sensor.ColorLevel=(.*)", false);
	m_parameters.push_back(m_colorLevelParam);
	m_exposurePriorityParam = new IntCameraParameter("ImageSource.I0.Sensor.exposurePriority=%i",
		"root.ImageSource.I0.Sensor.ExposurePriority=(.*)", false);
	m_parameters.push_back(m_exposurePriorityParam);
	m_compressionParam = new IntCameraParameter("Image.I0.Appearance.Compression=%i",
		"root.Image.I0.Appearance.Compression=(.*)", true);
	m_parameters.push_back(m_compressionParam);
	m_maxFPSParam = new IntCameraParameter("Image.I0.Stream.FPS=%i",
		"root.Image.I0.Stream.FPS=(.*)", false);
	m_parameters.push_back(m_maxFPSParam);
	m_rotationParam = new EnumCameraParameter("Image.I0.Appearance.Rotation=%s",
		"root.Image.I0.Appearance.Rotation=(.*)", true, kRotationChoices, sizeof(kRotationChoices)/sizeof(kRotationChoices[0]));
	m_parameters.push_back(m_rotationParam);
	m_resolutionParam = new EnumCameraParameter("Image.I0.Appearance.Resolution=%s",
		"root.Image.I0.Appearance.Resolution=(.*)", true, kResolutionChoices, sizeof(kResolutionChoices)/sizeof(kResolutionChoices[0]));
	m_parameters.push_back(m_resolutionParam);
	m_exposureControlParam = new EnumCameraParameter("ImageSource.I0.Sensor.Exposure=%s",
		"root.ImageSource.I0.Sensor.Exposure=(.*)", false, kExposureControlChoices, sizeof(kExposureControlChoices)/sizeof(kExposureControlChoices[0]));
	m_parameters.push_back(m_exposureControlParam);
	m_whiteBalanceParam = new EnumCameraParameter("ImageSource.IO.Sensor.WhiteBalance=%s",
		"root.ImageSource.I0.Sensor.WhiteBalance=(.*)", false, kWhiteBalanceChoices, sizeof(kWhiteBalanceChoices)/sizeof(kWhiteBalanceChoices[0]));
	m_parameters.push_back(m_whiteBalanceParam);

	m_paramChangedSem = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
	m_socketPossessionSem = semBCreate (SEM_Q_PRIORITY, SEM_FULL);

	m_paramTask.Start((int)this);
}

AxisCameraParams2::~AxisCameraParams2()
{
	m_paramTask.Stop();
}

/**
* Static function to start the parameter updating task
*/
int AxisCameraParams2::s_ParamTaskFunction(AxisCameraParams2* thisPtr)
{
	return thisPtr->ParamTaskFunction();
}

/**
* Main loop of the parameter task.
* This loop runs continuously checking parameters from the camera for
* posted changes and updating them if necessary.
*/
// TODO: need to synchronize the actual setting of parameters (the assignment statement)
int AxisCameraParams2::ParamTaskFunction()
{
	static bool		firstTime = true;

	while (true)
	{
		semTake(m_socketPossessionSem, WAIT_FOREVER);
		if (firstTime)
		{
			while (ReadCamParams() == 0) ;
			firstTime = false;
		}
		bool restartRequired = false;

		ParameterVector_t::iterator it = m_parameters.begin();
		ParameterVector_t::iterator end = m_parameters.end();
		for(; it != end; it++)
		{
			bool changed = false;
			char param[150];
			restartRequired |= (*it)->CheckChanged(changed, param);
			if (changed)
			{
				UpdateCamParam(param);
			}
		}
		if (restartRequired)
		{
			RestartCameraTask();
		}
		semGive(m_socketPossessionSem);
	}
	return 0;
}

/**
* Write the brightness value to the camera.
* @param brightness valid values 0 .. 100
*/
void AxisCameraParams2::WriteBrightness(int brightness)
{
	m_brightnessParam->SetValue(brightness);
	semGive(m_paramChangedSem);
}

/**
* Get the brightness value.
* @return Brightness value from the camera.
*/
int AxisCameraParams2::GetBrightness()
{
	return m_brightnessParam->GetValue();
}

/**
* Set the white balance value.
* @param whiteBalance Valid values from the WhiteBalance_t enum.
*/
void AxisCameraParams2::WriteWhiteBalance(WhiteBalance_t whiteBalance)
{
	m_whiteBalanceParam->SetValue(whiteBalance);
	semGive(m_paramChangedSem);
}

/**
* Retrieve the current white balance parameter.
* @return The white balance value.
*/
AxisCameraParams2::WhiteBalance_t AxisCameraParams2::GetWhiteBalance()
{
	return (WhiteBalance_t) m_whiteBalanceParam->GetValue();
}

/**
* Write the color level to the camera.
* @param colorLevel valid values are 0 .. 100
*/
void AxisCameraParams2::WriteColorLevel(int colorLevel)
{
	m_colorLevelParam->SetValue(colorLevel);
	semGive(m_paramChangedSem);
}

/**
* Retrieve the color level from the camera.
* @Returns the camera color level.
*/
int AxisCameraParams2::GetColorLevel()
{
	return m_colorLevelParam->GetValue();
}

/**
* Write the exposure control value to the camera.
* @param exposureControl A mode to write in the Exposure_t enum.
*/
void AxisCameraParams2::WriteExposureControl(Exposure_t exposureControl)
{
	m_exposureControlParam->SetValue(exposureControl);
	semGive(m_paramChangedSem);
}

/**
* Get the exposure value from the camera.
* @returns the exposure value from the camera.
*/
AxisCameraParams2::Exposure_t AxisCameraParams2::GetExposureControl()
{
	return (Exposure_t) m_exposureControlParam->GetValue();
}

/**
* Write resolution value to camera.
* @param resolution The camera resolution value to write to the camera.  Use the Resolution_t enum.
*/
void AxisCameraParams2::WriteResolution(Resolution_t resolution)
{
	m_resolutionParam->SetValue(resolution);
	semGive(m_paramChangedSem);
}

/**
* Get the resolution value from the camera.
* @returns resultion value for the camera.
*/
AxisCameraParams2::Resolution_t AxisCameraParams2::GetResolution()
{
	return (Resolution_t) m_resolutionParam->GetValue();
}

/**
* Write the exposre priority value to the camera.
* @param exposurePriority Valid values are 0, 50, 100.
* 0 = Prioritize image quality
* 50 = None
* 100 = Prioritize frame rate
*/
void AxisCameraParams2::WriteExposurePriority(int exposurePriority)
{
	m_exposurePriorityParam->SetValue(exposurePriority);
	semGive(m_paramChangedSem);
}

int AxisCameraParams2::GetExposurePriority()
{
	return m_exposurePriorityParam->GetValue();
}

/**
* Write the rotation value to the camera.
* If you mount your camera upside down, use this to adjust the image for you.
* @param rotation The image from the Rotation_t enum in AxisCameraParams2 (kRotation_0 or kRotation_180)
*/
void AxisCameraParams2::WriteRotation(Rotation_t rotation)
{
	m_rotationParam->SetValue(rotation);
	semGive(m_paramChangedSem);
}

/**
* Get the rotation value from the camera.
* @return The rotation value from the camera (Rotation_t).
*/
AxisCameraParams2::Rotation_t AxisCameraParams2::GetRotation()
{
	return (Rotation_t) m_rotationParam->GetValue();
}

/**
* Write the compression value to the camera.
* @param compression Values between 0 and 100.
*/

void AxisCameraParams2::WriteCompression(int compression)
{
	m_compressionParam->SetValue(compression);
	semGive(m_paramChangedSem);
}

/**
* Get the compression value from the camera.
* @return The cached compression value from the camera.
*/
int AxisCameraParams2::GetCompression()
{
	return m_compressionParam->GetValue();
}

/**
* Write the maximum frames per second that the camera should send
* Write 0 to send as many as possible.
* @param maxFPS The number of frames the camera should send in a second, exposure permitting.
*/
void AxisCameraParams2::WriteMaxFPS(int maxFPS)
{
	m_maxFPSParam->SetValue(maxFPS);
	semGive(m_paramChangedSem);
}

/**
* Get the max number of frames per second that the camera will send
* @return Maximum frames per second.
*/
int AxisCameraParams2::GetMaxFPS()
{
	return m_maxFPSParam->GetValue();
}

/**
* Update a camera parameter.
* Write a camera parameter to the camera when it has bene changed.
* @param param the string to insert into the http request.
* @returns 0 if it failed, otherwise nonzero.
*/
int AxisCameraParams2::UpdateCamParam(const char* param)
{
	char * requestString =
		"GET /axis-cgi/admin/param.cgi?action=update&%s HTTP/1.1\n\
		User-Agent: HTTPStreamClient\n\
		Connection: Keep-Alive\n\
		Cache-Control: no-cache\n\
		Authorization: Basic RlJDOkZSQw==\n\n";
	char completedRequest[1024];
	sprintf(completedRequest, requestString, param);
	// Send request
	int camSocket = CreateCameraSocket(completedRequest);
	if (camSocket == 0)
	{
		printf("UpdateCamParam failed: %s\n", param);
		return 0;
	}
	close(camSocket);
	return 1;
}

/**
* Read the full param list from camera, use regular expressions to find the bits we care about
* assign values to member variables.
*/
int AxisCameraParams2::ReadCamParams()
{
	char * requestString =
		"GET /axis-cgi/admin/param.cgi?action=list HTTP/1.1\n\
		User-Agent: HTTPStreamClient\n\
		Connection: Keep-Alive\n\
		Cache-Control: no-cache\n\
		Authorization: Basic RlJDOkZSQw==\n\n";

	int camSocket = CreateCameraSocket(requestString);
	if (camSocket == 0)
	{
		return 0;
	}
	char readBuffer[27000];
	int totalRead = 0;
	while (1)
	{
		wpi_assert(totalRead < 26000);
		int bytesRead = recv(camSocket, &readBuffer[totalRead], 1000, 0);
		if (bytesRead == ERROR)
		{
			perror("AxisCameraParams2: Failed to read image header");
			close(camSocket);
			return 0;
		}
		else if (bytesRead <= 0)
		{
			break;
		}
		totalRead += bytesRead;
	}
	readBuffer[totalRead] = '\0';

	ParameterVector_t::iterator it = m_parameters.begin();
	ParameterVector_t::iterator end = m_parameters.end();
	for(; it != end; it++)
	{
		(*it)->GetParamFromString(readBuffer, totalRead);
	}
	close(camSocket);
	return 1;
}

/*
* Create a socket connected to camera
* Used to create a connection to the camera by both AxisCameraParams2 and AxisCamera2.
* @param requestString The initial request string to send upon successful connection.
* @return 0 if failed, socket handle if successful.
*/
int AxisCameraParams2::CreateCameraSocket(const char *requestString)
{
	int sockAddrSize;
	struct sockaddr_in serverAddr;
	int camSocket;
	/* create socket */
	if ((camSocket = socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{
		perror("AxisCameraParams2: socket");
		return 0;
	}

	sockAddrSize = sizeof(struct sockaddr_in);
	bzero((char *) &serverAddr, sockAddrSize);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_len = (u_char) sockAddrSize;
	serverAddr.sin_port = htons(80);

	if ((serverAddr.sin_addr.s_addr = m_ipAddress) == (u_long)ERROR)
	{
		perror("AxisCameraParams2: invalid IP");
		close(camSocket);
		return 0;
	}

	/* connect to server */
	if (connect(camSocket, (struct sockaddr *) &serverAddr, sockAddrSize) == ERROR)
	{
		perror("AxisCameraParams2: connect");
		close(camSocket);
		return 0;
	}
	int sent = send(camSocket, requestString, strlen(requestString), 0);
	if (sent == ERROR)
	{
		perror("AxisCameraParams2: send");
		close(camSocket);
		return 0;
	}
	return camSocket;
}


/***********************************************************************************************************/
/*													Axis Camera												*/
/***********************************************************************************************************/


/** Private NI function to decode JPEG */
IMAQ_FUNC int Priv_ReadJPEGString_C(Image* _image, const unsigned char* _string, UINT32 _stringLength);

// Max packet without jumbo frames is 1500... add 36 because??
#define kMaxPacketSize 1536
#define kImageBufferAllocationIncrement 1000

AxisCamera2* AxisCamera2::m_instance = NULL;

/**
* AxisCamera2 constructor
*/
AxisCamera2::AxisCamera2(const char *ipAddress)
: AxisCameraParams2(ipAddress)
, m_cameraSocket (0)
, m_protectedImageBuffer(NULL)
, m_protectedImageBufferLength (0)
, m_protectedImageSize (0)
, m_protectedImageSem (NULL)
, m_freshImage (false)
, m_imageStreamTask("cameraTask", (FUNCPTR)s_ImageStreamTaskFunction)
{
	m_protectedImageSem = semMCreate(SEM_Q_PRIORITY | SEM_INVERSION_SAFE | SEM_DELETE_SAFE);

	m_imageStreamTask.Start((int)this);
}

/**
* Destructor
*/
AxisCamera2::~AxisCamera2()
{
	m_imageStreamTask.Stop();
	close(m_cameraSocket);

	SemSet_t::iterator it = m_newImageSemSet.begin();
	SemSet_t::iterator end = m_newImageSemSet.end();
	for (;it != end; it++)
	{
		semDelete(*it);
	}
	m_newImageSemSet.clear();

	semDelete(m_protectedImageSem);
	m_instance = NULL;
}

/**
* Get a pointer to the AxisCamera2 object, if the object does not exist, create it
* @return reference to AxisCamera2 object
*/
AxisCamera2& AxisCamera2::GetInstance()
{
	if (NULL == m_instance) {
		// Since this is a singleton for now, just use the default IP address.
		m_instance = new AxisCamera2();
		// TODO: Keep track of this so it can be shut down!
		new PCVideoServer();
	}
	return *m_instance;
}

/**
* Called by Java to delete the camera... how thoughtful
*/
void AxisCamera2::DeleteInstance()
{
	delete m_instance;
}

/**
* Return true if the latest image from the camera has not been retrieved by calling GetImage() yet.
* @return true if the image has not been retrieved yet.
*/
bool AxisCamera2::IsFreshImage()
{
	return m_freshImage;
}

/**
* Get the semaphore to be used to synchronize image access with camera acquisition
*
* Call semTake on the returned semaphore to block until a new image is acquired.
*
* The semaphore is owned by the AxisCamera2 class and will be deleted when the class is destroyed.
* @return A semaphore to notify when new image is received
*/
SEM_ID AxisCamera2::GetNewImageSem()
{
	SEM_ID sem = semBCreate (SEM_Q_PRIORITY, SEM_EMPTY);
	m_newImageSemSet.insert(sem);
	return sem;
}

/**
* Get an image from the camera and store it in the provided image.
* @param image The imaq image to store the result in. This must be an HSL or RGB image
* This function is called by Java.
* @return 1 upon success, zero on a failure
*/
int AxisCamera2::GetImage(Image* imaqImage)
{
	if (m_protectedImageBuffer == NULL)
		return 0;
	Synchronized sync(m_protectedImageSem);
	Priv_ReadJPEGString_C(imaqImage,
		(unsigned char*)m_protectedImageBuffer, m_protectedImageSize);
	m_freshImage = false;
	return 1;
}

/**
* Get an image from the camera and store it in the provided image.
* @param image The image to store the result in. This must be an HSL or RGB image
* @return 1 upon success, zero on a failure
*/
int AxisCamera2::GetImage(ColorImage* image)
{
	return GetImage(image->GetImaqImage());
}

/**
* Instantiate a new image object and fill it with the latest image from the camera.
*
* The returned pointer is owned by the caller and is their responsibility to delete.
* @return a pointer to an HSLImage object
*/
HSLImage* AxisCamera2::GetImage()
{
	HSLImage *image = new HSLImage();
	GetImage(image);
	return image;
}

/**
* Copy an image into an existing buffer.
* This copies an image into an existing buffer rather than creating a new image
* in memory. That way a new image is only allocated when the image being copied is
* larger than the destination.
* This method is called by the PCVideoServer class.
* @param imageData The destination image.
* @param numBytes The size of the destination image.
* @return 0 if failed (no source image or no memory), 1 if success.
*/
int AxisCamera2::CopyJPEG(char **destImage, int &destImageSize, int &destImageBufferSize)
{
	Synchronized sync(m_protectedImageSem);
	wpi_assert(destImage != NULL);
	if (m_protectedImageBuffer == NULL) return 0; // if no source image
	if (destImageBufferSize < m_protectedImageSize) // if current destination buffer too small
	{
		if (*destImage != NULL) delete [] *destImage;
		destImageBufferSize = m_protectedImageSize + kImageBufferAllocationIncrement;
		*destImage = new char[destImageBufferSize];
		if (*destImage == NULL) return 0;
	}
	// copy this image into destination buffer
	wpi_assert(*destImage != NULL);
	wpi_assert(m_protectedImageBuffer != NULL);
	wpi_assert(m_protectedImageSize > 0);
	// TODO: Is this copy realy necessary... perhaps we can simply transmit while holding the protected buffer
	memcpy(*destImage, m_protectedImageBuffer, m_protectedImageSize);
	destImageSize = m_protectedImageSize;
	return 1;
}

/**
* Static interface that will cause an instantiation if necessary.
* This static stub is directly spawned as a task to read images from the camera.
*/
int AxisCamera2::s_ImageStreamTaskFunction(AxisCamera2 *thisPtr)
{
	return thisPtr->ImageStreamTaskFunction();
}

/**
* Task spawned by AxisCamera2 constructor to receive images from cam
* If setNewImageSem has been called, this function does a semGive on each new image
* Images can be accessed by calling getImage()
*/
int AxisCamera2::ImageStreamTaskFunction()
{
	// Loop on trying to setup the camera connection. This happens in a background
	// thread so it shouldn't effect the operation of user programs.
	while (1)
	{
		char * requestString = "GET /mjpg/video.mjpg HTTP/1.1\n\
							   User-Agent: HTTPStreamClient\n\
							   Connection: Keep-Alive\n\
							   Cache-Control: no-cache\n\
							   Authorization: Basic RlJDOkZSQw==\n\n";
		semTake(m_socketPossessionSem, WAIT_FOREVER);
		m_cameraSocket = CreateCameraSocket(requestString);
		if (m_cameraSocket == 0)
		{
			// Don't hammer the camera if it isn't ready.
			semGive(m_socketPossessionSem);
			taskDelay(1000);
		}
		else
		{
			ReadImagesFromCamera();
		}
	}
}

/**
* This function actually reads the images from the camera.
*/
int AxisCamera2::ReadImagesFromCamera()
{
	char *imgBuffer = NULL;
	int imgBufferLength = 0;
	//Infinite loop, task deletion handled by taskDeleteHook
	// Socket cleanup handled by destructor

	// TODO: these recv calls must be non-blocking. Otherwise if the camera
	// fails during a read, the code hangs and never retries when the camera comes
	// back up.

	int counter = 2;
	while (1)
	{
		char initialReadBuffer[kMaxPacketSize] = "";
		char intermediateBuffer[1];
		char *trailingPtr = initialReadBuffer;
		int trailingCounter = 0;
		while (counter)
		{
			// TODO: fix me... this cannot be the most efficient way to approach this, reading one byte at a time.
			if(recv(m_cameraSocket, intermediateBuffer, 1, 0) == ERROR)
			{
				perror ("AxisCamera2: Failed to read image header");
				close (m_cameraSocket);
				return (ERROR);
			}
			strncat(initialReadBuffer, intermediateBuffer, 1);
			// trailingCounter ensures that we start looking for the 4 byte string after
			// there is at least 4 bytes total. Kind of obscure.
			// look for 2 blank lines (\r\n)
			if (NULL != strstr(trailingPtr, "\r\n\r\n"))
			{
				--counter;
			}
			if (++trailingCounter >= 4)
			{
				trailingPtr++;
			}
		}
		counter = 1;
		char *contentLength = strstr(initialReadBuffer, "Content-Length: ");
		if (contentLength == NULL)
		{
			perror("AxisCamera2: No content-length token found in packet");
			close(m_cameraSocket);
			return(ERROR);
		}
		contentLength = contentLength + 16; // skip past "content length"
		int readLength = atol(contentLength); // get the image byte count

		// Make sure buffer is large enough
		if (imgBufferLength < readLength)
		{
			if (imgBuffer) delete[] imgBuffer;
			imgBufferLength = readLength + kImageBufferAllocationIncrement;
			imgBuffer = new char[imgBufferLength];
			if (imgBuffer == NULL)
			{
				imgBufferLength = 0;
				continue;
			}
		}

		// Read the image data for "Content-Length" bytes
		int bytesRead = 0;
		int remaining = readLength;
		while(bytesRead < readLength)
		{
			int bytesThisRecv = recv(m_cameraSocket, &imgBuffer[bytesRead], remaining, 0);
			bytesRead += bytesThisRecv;
			remaining -= bytesThisRecv;
		}
		// Update image
		UpdatePublicImageFromCamera(imgBuffer, readLength);
		if (semTake(m_paramChangedSem, NO_WAIT) == OK)
		{
			// params need to be updated: close the video stream; release the camera.
			close(m_cameraSocket);
			semGive(m_socketPossessionSem);
			return(0);
		}
	}
}

/**
* Copy the image from private buffer to shared buffer.
* @param imgBuffer The buffer containing the image
* @param bufLength The length of the image
*/
void AxisCamera2::UpdatePublicImageFromCamera(char *imgBuffer, int imgSize)
{
	{
		Synchronized sync(m_protectedImageSem);

		// Adjust the buffer size if current destination buffer is too small.
		if (m_protectedImageBufferLength < imgSize)
		{
			if (m_protectedImageBuffer != NULL) delete [] m_protectedImageBuffer;
			m_protectedImageBufferLength = imgSize + kImageBufferAllocationIncrement;
			m_protectedImageBuffer = new char[m_protectedImageBufferLength];
			if (m_protectedImageBuffer == NULL)
			{
				m_protectedImageBufferLength = 0;
				return;
			}
		}

		memcpy(m_protectedImageBuffer, imgBuffer, imgSize);
		m_protectedImageSize = imgSize;
	}

	m_freshImage = true;
	// Notify everyone who is interested.
	SemSet_t::iterator it = m_newImageSemSet.begin();
	SemSet_t::iterator end = m_newImageSemSet.end();
	for (;it != end; it++)
	{
		semGive(*it);
	}
}

/**
* Implement the pure virtual interface so that when parameter changes require a restart, the image task can be bounced.
*/
void AxisCamera2::RestartCameraTask()
{
	m_imageStreamTask.Stop();
	m_imageStreamTask.Start((int)this);
}


// C bindings used by Java
// These need to stay as is or Java has to change

void AxisCameraStart2()
{
	AxisCamera2::GetInstance();
}

int AxisCameraGetImage2 (Image* image)
{
	return AxisCamera2::GetInstance().GetImage(image);
}

void AxisCameraWriteBrightness2(int brightness)
{
	AxisCamera2::GetInstance().WriteBrightness(brightness);
}

int AxisCameraGetBrightness2()
{
	return AxisCamera2::GetInstance().GetBrightness();
}

void AxisCameraWriteWhiteBalance2(AxisCameraParams2::WhiteBalance_t whiteBalance)
{
	AxisCamera2::GetInstance().WriteWhiteBalance(whiteBalance);
}

AxisCameraParams2::WhiteBalance_t AxisCameraGetWhiteBalance2()
{
	return AxisCamera2::GetInstance().GetWhiteBalance();
}

void AxisCameraWriteColorLevel2(int colorLevel)
{
	AxisCamera2::GetInstance().WriteColorLevel(colorLevel);
}

int AxisCameraGetColorLevel2()
{
	return AxisCamera2::GetInstance().GetColorLevel();
}

void AxisCameraWriteExposureControl2(AxisCameraParams2::Exposure_t exposure)
{
	AxisCamera2::GetInstance().WriteExposureControl(exposure);
}

AxisCameraParams2::Exposure_t AxisCameraGetExposureControl2()
{
	return AxisCamera2::GetInstance().GetExposureControl();
}

void AxisCameraWriteExposurePriority2(int exposure)
{
	AxisCamera2::GetInstance().WriteExposurePriority(exposure);
}

int AxisCameraGetExposurePriority2()
{
	return AxisCamera2::GetInstance().GetExposurePriority();
}

void AxisCameraWriteMaxFPS2(int maxFPS)
{
	AxisCamera2::GetInstance().WriteMaxFPS(maxFPS);
}

int AxisCameraGetMaxFPS2()
{
	return AxisCamera2::GetInstance().GetMaxFPS();
}

void AxisCameraWriteResolution2(AxisCameraParams2::Resolution_t resolution)
{
	AxisCamera2::GetInstance().WriteResolution(resolution);
}

AxisCameraParams2::Resolution_t AxisCameraGetResolution2()
{
	return AxisCamera2::GetInstance().GetResolution();
}

void AxisCameraWriteCompression2(int compression)
{
	AxisCamera2::GetInstance().WriteCompression(compression);
}

int AxisCameraGetCompression2()
{
	return AxisCamera2::GetInstance().GetCompression();
}

void AxisCameraWriteRotation2(AxisCameraParams2::Rotation_t rotation)
{
	AxisCamera2::GetInstance().WriteRotation(rotation);
}

AxisCameraParams2::Rotation_t AxisCameraGetRotation2()
{
	return AxisCamera2::GetInstance().GetRotation();
}

void AxisCameraDeleteInstance2()
{
	AxisCamera2::GetInstance().DeleteInstance();
}

int AxisCameraFreshImage2()
{
	return AxisCamera2::GetInstance().IsFreshImage();
}


//--------------------------------------------------------------------------------------------------------------------------------------
