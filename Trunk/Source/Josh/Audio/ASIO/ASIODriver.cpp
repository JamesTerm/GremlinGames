// ASIODriver.cpp: implementation of the ASIODriver class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Devices_Audio_ASIO.h"

//#include "ASIOFormatConverter.h"

//Easy access to common globals
using namespace FrameWork::Base;
using namespace Devices::Audio::ASIO;
using namespace Devices::Audio::Base::Streams;
using namespace Devices::Audio::Base::Host;
using namespace Devices::Audio::Base::aud_convert;

typedef void (*BUFFERSWITCH_CALLBACK) (long doubleBufferIndex, ASIOBool directProcess);
typedef void (*SAMPLERATEDIDCHANGE_CALLBACK) (ASIOSampleRate sRate);
typedef long (*ASIOMESSAGE_CALLBACK) (long selector, long value, void* message, double* opt);
typedef ASIOTime* (*BUFFERSWITCHTIMEINFO_CALLBACK) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);

const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)

/*
 * the ASIO SDK callback system is very poor. There is no "user" pointer passed
 * in parameter (like any Windows callback). So to be able to initiate multiple
 * drivers, we must create multiple callback functions.
 * using Macro, it can be easy to do.
 * MAX_DRIVERS_CALLBACK is set to 20, so this is the maximum, hardcoded, limitation
 * of the API.
 * ----------------------------------------------------------------------------------
 * mécanisme pour pallier à la pauvretée du mécanisme de callback du SDK ASIO
 *
 * les callback ne prévoient pas un paramètre utilisateur, dans lequel on aurait
 * pu passer l'objet ASIODriver représentant le driver.
 *
 * on fait donc un mapping en dur, entre une fonction callback et un objet ASIODriver
 *
 * un driver ASIO utilise 5 callback différentes
 * on gèrera au max 20 drivers.
 * pour que le code soit lisible, on utilisera les macro VC++ pour créer 20*5 callback
 *
 */
#define MAX_DRIVERS_CALLBACK 20

static BUFFERSWITCH_CALLBACK g_bufferSwitch[MAX_DRIVERS_CALLBACK];
static SAMPLERATEDIDCHANGE_CALLBACK g_sampleRateDidChange[MAX_DRIVERS_CALLBACK];
static ASIOMESSAGE_CALLBACK g_asioMessage[MAX_DRIVERS_CALLBACK];
static BUFFERSWITCHTIMEINFO_CALLBACK g_bufferSwitchTimeInfo[MAX_DRIVERS_CALLBACK];
static ASIODriver *g_asioDriver[MAX_DRIVERS_CALLBACK];

#define DECLARE_ASIO_CALLBACK(IDX) \
void g_bufferSwitch_##IDX (long doubleBufferIndex, ASIOBool directProcess)\
{\
	g_asioDriver[IDX]->OnBufferSwitch(doubleBufferIndex,directProcess);\
}\
void g_sampleRateDidChange_##IDX (ASIOSampleRate sRate)\
{\
	g_asioDriver[IDX]->OnSampleRateDidChange(sRate);\
}\
long g_asioMessage_##IDX (long selector, long value, void* message, double* opt)\
{\
	return g_asioDriver[IDX]->OnAsioMessage(selector,value,message,opt);\
}\
ASIOTime* g_bufferSwitchTimeInfo_##IDX(ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess)\
{\
	return g_asioDriver[IDX]->OnBufferSwitchTimeInfo(params,doubleBufferIndex,directProcess);\
}

#define DECLARE_ASIO_CALLBACK_DEF(IDX) \
			case IDX :\
			g_asioDriver[m_id] = this;\
			m_asioCallbacks.bufferSwitch = g_bufferSwitch[m_id] = g_bufferSwitch_##IDX;\
			m_asioCallbacks.sampleRateDidChange = g_sampleRateDidChange[m_id] = g_sampleRateDidChange_##IDX;\
			m_asioCallbacks.asioMessage = g_asioMessage[m_id] = g_asioMessage_##IDX;\
			m_asioCallbacks.bufferSwitchTimeInfo = g_bufferSwitchTimeInfo[m_id] =g_bufferSwitchTimeInfo_##IDX;\
			break;

DECLARE_ASIO_CALLBACK(0)
DECLARE_ASIO_CALLBACK(1)
DECLARE_ASIO_CALLBACK(2)
DECLARE_ASIO_CALLBACK(3)
DECLARE_ASIO_CALLBACK(4)
DECLARE_ASIO_CALLBACK(5)
DECLARE_ASIO_CALLBACK(6)
DECLARE_ASIO_CALLBACK(7)
DECLARE_ASIO_CALLBACK(8)
DECLARE_ASIO_CALLBACK(9)
DECLARE_ASIO_CALLBACK(10)
DECLARE_ASIO_CALLBACK(11)
DECLARE_ASIO_CALLBACK(12)
DECLARE_ASIO_CALLBACK(13)
DECLARE_ASIO_CALLBACK(14)
DECLARE_ASIO_CALLBACK(15)
DECLARE_ASIO_CALLBACK(16)
DECLARE_ASIO_CALLBACK(17)
DECLARE_ASIO_CALLBACK(18)
DECLARE_ASIO_CALLBACK(19)

void ASIODriver::ClearCallbackTables()
{
	for (int i=0;i<MAX_DRIVERS_CALLBACK;i++)
	{
		g_bufferSwitch[i] = NULL;
		g_sampleRateDidChange[i] = NULL;
		g_asioMessage[i] = NULL;
		g_bufferSwitchTimeInfo[i] = NULL;
	}
}
bool ASIODriver::SetId(int id)
{
	if (id >= MAX_DRIVERS_CALLBACK)
	{
		return false;
	}

	m_id = id;

	switch (m_id)
	{
	 DECLARE_ASIO_CALLBACK_DEF(0)
	 DECLARE_ASIO_CALLBACK_DEF(1)
	 DECLARE_ASIO_CALLBACK_DEF(2)
	 DECLARE_ASIO_CALLBACK_DEF(3)
	 DECLARE_ASIO_CALLBACK_DEF(4)
	 DECLARE_ASIO_CALLBACK_DEF(5)
	 DECLARE_ASIO_CALLBACK_DEF(6)
	 DECLARE_ASIO_CALLBACK_DEF(7)
	 DECLARE_ASIO_CALLBACK_DEF(8)
	 DECLARE_ASIO_CALLBACK_DEF(9)
	 DECLARE_ASIO_CALLBACK_DEF(10)
	 DECLARE_ASIO_CALLBACK_DEF(11)
	 DECLARE_ASIO_CALLBACK_DEF(12)
	 DECLARE_ASIO_CALLBACK_DEF(13)
	 DECLARE_ASIO_CALLBACK_DEF(14)
	 DECLARE_ASIO_CALLBACK_DEF(15)
	 DECLARE_ASIO_CALLBACK_DEF(16)
	 DECLARE_ASIO_CALLBACK_DEF(17)
	 DECLARE_ASIO_CALLBACK_DEF(18)
	 DECLARE_ASIO_CALLBACK_DEF(19)
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//Don't want to hear about 'this' being used in base member warning
#pragma warning (disable : 4355)
ASIODriver::ASIODriver() :
m_AudioHostInterface(this)
{
	m_iasio = NULL;
	m_bufferInfos = NULL;
	m_channelInfos = NULL;
	m_id = -1;
	m_state = ASIO_DRIVER_STOPPED;
	m_nbInputChannels = 0;
	m_nbOutputChannels = 0;
	m_bufferSize = 0;
	m_samplerate = 0;
}
#pragma warning (default : 4355)

ASIODriver::~ASIODriver()
{
	CloseDriver();
}

IASIO * ASIODriver::OpenDriver(HWND hwnd)
{
	DebugOutput(L"Init: %s\n",m_sclsid);
	if (CoCreateInstance(m_clsid,0,CLSCTX_INPROC_SERVER,m_clsid,(void**)&m_iasio)==S_OK)
	{
		if(!m_iasio->init(hwnd))
		{
			CloseDriver();
			return NULL;
		}
		
		if(!m_iasio->init(hwnd))
		{
			CloseDriver();
			return NULL;
		}
		if (m_iasio->getSampleRate(&m_samplerate)!=ASE_OK)
		{
			CloseDriver();
			return NULL;
		}
		if (!CreateBuffer())
		{
			CloseDriver();
			return NULL;
		}
		else
		{
			return m_iasio;
		}
	}
	else
	{
		return NULL;
	}
}

bool ASIODriver::CreateBuffer()
{
	if (m_iasio==NULL)
		return false;

	if(m_iasio->getChannels(&m_nbInputChannels,&m_nbOutputChannels)!=ASE_OK)
		return false;

	long minBufferSize;
	long maxBufferSize;
	long granularity;
	if (m_iasio->getBufferSize(&minBufferSize,&maxBufferSize,&m_bufferSize,&granularity)!=ASE_OK)
		return false;
	m_bufferInfos = new ASIOBufferInfo[m_nbInputChannels + m_nbOutputChannels];
	
	int b=0;
	int c;
	for (c=0;c<m_nbInputChannels;c++)
	{
		m_bufferInfos[b].isInput = ASIOTrue;
		m_bufferInfos[b].channelNum = c;
		m_bufferInfos[b].buffers[0] = NULL;
		m_bufferInfos[b].buffers[1] = NULL;
		b++;
	}
	for (c=0;c<m_nbOutputChannels;c++)
	{
		m_bufferInfos[b].isInput = ASIOFalse;
		m_bufferInfos[b].channelNum = c;
		m_bufferInfos[b].buffers[0] = NULL;
		m_bufferInfos[b].buffers[1] = NULL;
		b++;
	}	
	if (m_iasio->createBuffers(	m_bufferInfos, 
								m_nbInputChannels + m_nbOutputChannels,
								m_bufferSize, 
								&m_asioCallbacks)==ASE_OK)
	{
		// now get all the buffer details, sample word length, name, word clock group and activation
		assert (!m_channelInfos);
		m_channelInfos=new ASIOChannelInfo[m_nbInputChannels+m_nbOutputChannels];
		for (size_t i=0;i<(size_t)(m_nbInputChannels+m_nbOutputChannels);i++)
		{
			m_channelInfos[i].channel = m_bufferInfos[i].channelNum;
			m_channelInfos[i].isInput = m_bufferInfos[i].isInput;
			if (GetIASIOInterface()->getChannelInfo(&m_channelInfos[i]) != ASE_OK)
				return false;
		}

	}
	else
		return false;
		
	return true;
}


// ----------------------------------------------------------------------------------------
// CALLBACK
// ----------------------------------------------------------------------------------------
void ASIODriver::OnSampleRateDidChange(ASIOSampleRate sRate)
{
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
	m_samplerate = sRate;
}
long ASIODriver::OnAsioMessage(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	DebugOutput(L"OnAsioMessage selector:%ld value: %ld\n",selector,value);
	long ret = 0;
	switch(selector)
	{
		case kAsioBufferSizeChange:
			DebugOutput(L"value: %l\n",value);
			break;
		case kAsioSelectorSupported:
			if(value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
			//asioDriverInfo.stopped;  // In this sample the processing will just stop
			ret = 1L;
			break;
		case kAsioResyncRequest:
			// This informs the application, that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media.
			// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
			// Windows Multimedia system, which could loose data because the Mutex was hold too long
			// by another thread.
			// However a driver can issue it in other situations, too.
			ret = 1L;
			break;
		case kAsioLatenciesChanged:
			// This will inform the host application that the drivers were latencies changed.
			// Beware, it this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.
			ret = 1L;
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
	}
	return ret;
}
void ASIODriver::OnBufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
{
	//just divert to host interface callback
	m_AudioHostInterface.OnBufferSwitch(doubleBufferIndex,directProcess);
	return;

	// the actual processing callback.
	// Beware that this is normally in a separate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	memset (&timeInfo, 0, sizeof (timeInfo));

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	
	if(m_iasio->getSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	OnBufferSwitchTimeInfo (&timeInfo, doubleBufferIndex, directProcess);
}
ASIOTime* ASIODriver::OnBufferSwitchTimeInfo(ASIOTime* timeInfo, long doubleBufferIndex, ASIOBool directProcess)
{
	if (timeInfo->timeInfo.flags & kSamplePositionValid)
		m_CurrentSamplePosition = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
	else
		m_CurrentSamplePosition = 0;

	DebugOutput(L"SamplePos: %f\n",m_CurrentSamplePosition);

	return 0L;
}

void ASIODriver::CloseDriver()
{
	if (m_iasio && m_bufferInfos)
	{
		Stop();
		m_iasio->disposeBuffers();
	}

	if (m_bufferInfos)
	{
		delete m_bufferInfos;
		m_bufferInfos = NULL;
	}
	if (m_channelInfos)
	{
		delete m_channelInfos;
		m_channelInfos=NULL;
	}
	if (m_iasio)
	{
		m_iasio->Release();
		m_iasio = NULL;
	}
	if (m_id!=-1)
	{
		g_bufferSwitch[m_id] = NULL;
		g_sampleRateDidChange[m_id] = NULL;
		g_asioMessage[m_id] = NULL;
		g_bufferSwitchTimeInfo[m_id] = NULL;
	}
}

IASIO * ASIODriver::GetIASIOInterface()
{
	return m_iasio;
}

bool ASIODriver::Start()
{
	if (m_state == ASIO_DRIVER_STARTED)
		return true;

	if (m_iasio==NULL)
		return false;
	if (m_iasio->start() != ASE_OK)
		return false;
	m_state = ASIO_DRIVER_STARTED;
	DebugOutput(L"STARTED\n");
	return true;
}

bool ASIODriver::Stop()
{
	if (m_state == ASIO_DRIVER_STOPPED)
		return true;

	if (m_iasio->stop() != ASE_OK)
		return false;
	if (m_iasio==NULL)
		return false;
	m_state = ASIO_DRIVER_STOPPED;
	DebugOutput(L"STOPED\n");
	return true;
}

int ASIODriver::GetState()
{
	return m_state;
}

const char * ASIODriver::getCLSID()
{
	return m_sclsid;
}

int ASIODriver::getBufferSize()
{
	return m_bufferSize;
}

int ASIODriver::getNbInput()
{
	return m_nbInputChannels;
}

int ASIODriver::getNbOutput()
{
	return m_nbOutputChannels;
}

const char* ASIODriver::getName()
{
	return m_drvname;
}

const char * ASIODriver::getPath()
{
	return m_dllpath;
}

ASIOBufferInfo *ASIODriver::GetBufferInfo(size_t channel)
{
	return &m_bufferInfos[channel];
}

ASIOChannelInfo *ASIODriver::GetChannelInfo(size_t channel)
{
	assert(m_channelInfos);
	return &m_channelInfos[channel];
}

void ASIODriver::setCLSID(CLSID clsid)
{
	m_clsid = clsid;
}

void ASIODriver::setCLSID(char *clsid)
{
	strncpy(m_sclsid,clsid,CLSID_STRING_LEN);
}

void ASIODriver::setPath(char *path)
{
	strncpy(m_dllpath,path,MAXPATHLEN);
}

ASIOSampleRate ASIODriver::getSampleRate()
{
	return m_samplerate;
}

Devices::Audio::Base::Host::AudioHost_Interface *ASIODriver::GetAudioHost_Interface() 
{
	return &m_AudioHostInterface;
}


  /*******************************************************************************************************/
 /*										ASIODriver::ASIOAudioHost										*/
/*******************************************************************************************************/

ASIODriver::ASIOAudioHost::ASIOAudioHost(ASIODriver *pParent) : 
m_pParent(pParent),m_pAudioHostCallback(NULL)
{
}

ASIODriver::ASIOAudioHost::~ASIOAudioHost()
{
}

size_t ASIODriver::ASIOAudioHost::GetSampleRate( void ) const
{
	return (size_t)m_pParent->getSampleRate();
}

Devices::Audio::FormatInterface::eSampleFormat ASIODriver::ASIOAudioHost::GetSampleFormat( void ) const 
{
	Devices::Audio::FormatInterface::eSampleFormat ret=Devices::Audio::FormatInterface::eSampleFormat_U_8; //TODO make an unknown one
	//TODO figure out which channel to use

	ASIOSampleType ast=m_pParent->GetChannelInfo(2)->type;
	switch(ast)
	{
	case ASIOSTInt16LSB:
		ret=Devices::Audio::FormatInterface::eSampleFormat_I_16;
		break;

	case ASIOSTInt32LSB:
	case ASIOSTInt32LSB16:
	case ASIOSTInt32LSB18:
	case ASIOSTInt32LSB20:
	case ASIOSTInt32LSB24:
		ret=Devices::Audio::FormatInterface::eSampleFormat_I_32;
		break;

	case ASIOSTFloat32LSB:
		ret=Devices::Audio::FormatInterface::eSampleFormat_FP_32;
		break;
	}
	return ret;
}

size_t ASIODriver::ASIOAudioHost::GetNoChannels( void ) const
{
	//TODO figure out how to set this (most likely we'll work with stereo pairs)
	//Note: currently ASIODriver's accessor will return the total number of inputs (which is not what we want)
	return 2;
}

void ASIODriver::ASIOAudioHost::OnBufferSwitch(long doubleBufferIndex, ASIOBool directProcess)
{	// For information (GetFormatBitDepth)
	using namespace Base;

	size_t NoOutChannels=GetNoChannels();
	size_t BytesPerChannel=GetFormatBitDepth(GetSampleFormat());
	//size_t BytesPerSample=BytesPerChannel * NoOutChannels;
	size_t BufferSampleSize=GetBufferSampleSize();


	AudioHostCallback_Interface *ahi=GetAudioHostCallback();
	if (ahi) 
	{
		//borrow the stack memory
		size_t BufferChannelSizeInBytes=BufferSampleSize*BytesPerChannel;
		size_t BufferTotalSizeInBytes=BufferChannelSizeInBytes * NoOutChannels;
		byte *TempBuffer=(byte *)_alloca(BufferTotalSizeInBytes + 16);
		byte *AlignedTestBuf=(byte *)(((size_t)TempBuffer+0x0f)&~0x0f); //We need this aligned to a 16 byte boundary

		//This if statement helps to debug the de-interleaved code
		if (ahi->FillBuffer(TempBuffer,BufferSampleSize))
		{
			for (size_t i=0;i<BufferSampleSize;i++)
			{
				for (size_t j=0;j<NoOutChannels;j++)
					memcpy((m_pParent->GetBufferInfo(2+j)->buffers[doubleBufferIndex]),AlignedTestBuf+(j*BufferChannelSizeInBytes),BufferChannelSizeInBytes);
			}
		}
	}
}
