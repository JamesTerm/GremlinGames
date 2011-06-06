#include "stdafx.h"
#include "Interfaces.h"
#include "Devices_Audio_PeakLevels.h"

//*************************************************************************************************************************************************

using namespace Devices::Audio::PeakLevels;

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

// The c'tor called only by class clients
PeakLevelComputer::PeakLevelComputer(Devices::Audio::DeviceInterface *pUpstreamDevice, IPeakLevelListener *pFanOfMan, 
									 Devices::Audio::FormatInterface *pFormatSpec, bool bInline, bool blow_priority)
: m_pUpstreamDevice(pUpstreamDevice), m_pPLListener(pFanOfMan),  
m_SampleRate(pFormatSpec ? pFormatSpec->GetSampleRate() : 48000),
m_cChannels(pFormatSpec ? pFormatSpec->GetNoChannels() : 4),
m_SampleFormat(pFormatSpec ? pFormatSpec->GetSampleFormat() : Devices::Audio::BufferInterface::eSampleFormat_FP_32),
m_bInterleaved(true),		// TODO: I'm only supporting interleaved buffers here!!!
m_pThread(NULL), m_bInline(bInline), m_bRunning(false), m_pPeaksAndValleys(NULL)
{
	assert(pUpstreamDevice && pFanOfMan);	// Sanity check. Don't create me w/out this BS
	// TODO: I'm only supporting FP or PCM16 audio samples
	assert(Devices::Audio::BufferInterface::eSampleFormat_FP_32==m_SampleFormat ||
		Devices::Audio::BufferInterface::eSampleFormat_I_16==m_SampleFormat);

	m_pPeaksAndValleys = new float[m_cChannels];
	for (size_t j=0; j<m_cChannels; j++)
		m_pPeaksAndValleys[j] = 0.0f;

	// For this C'tor, I'm the most derived class so give it a go!!!
	this->Startup();
}

// The C'tor called by derived classes
PeakLevelComputer::PeakLevelComputer(IPeakLevelListener *pFanOfMan, Devices::Audio::DeviceInterface *pUpstreamDevice,
									 Devices::Audio::FormatInterface *pFormatSpec, bool bInline)
: m_pUpstreamDevice(pUpstreamDevice), m_pPLListener(pFanOfMan),  
m_SampleRate(pFormatSpec ? pFormatSpec->GetSampleRate() : 48000),
m_cChannels(pFormatSpec ? pFormatSpec->GetNoChannels() : 4),
m_SampleFormat(pFormatSpec ? pFormatSpec->GetSampleFormat() : Devices::Audio::BufferInterface::eSampleFormat_FP_32),
m_bInterleaved(true),		// TODO: I'm only supporting interleaved buffers here!!!
m_pThread(NULL), m_bInline(bInline), m_bRunning(false), m_pPeaksAndValleys(NULL)
{
	assert(pUpstreamDevice && pFanOfMan);	// Sanity check. Don't create me w/out this BS
	// TODO: I'm only supporting FP or PCM16 audio samples
	assert(Devices::Audio::BufferInterface::eSampleFormat_FP_32==m_SampleFormat ||
		Devices::Audio::BufferInterface::eSampleFormat_I_16==m_SampleFormat);

	m_pPeaksAndValleys = new float[m_cChannels];
	for (size_t j=0; j<m_cChannels; j++)
		m_pPeaksAndValleys[j] = 0.0f;

	// NOTE: I DO NOT call Startup here!!!  Derived class must do it!!!
}

PeakLevelComputer::~PeakLevelComputer( void )
{
	this->Shutdown();
	delete [] m_pPeaksAndValleys;
}

// Should be called in most derived class C'tor
void PeakLevelComputer::Startup(bool blow_priority)
{
	if (m_bRunning)
	{
		assert(false);
		return;
	}

	// Sanity check
	assert(m_pUpstreamDevice);

	// Create a few of these bad boy buffers and pass them upwind
	const size_t c_NumBuffers = 3;
	for (size_t i=0; i<c_NumBuffers; i++)
	{
		Buffer *pNewBuff = new Buffer(this);
		m_ListOfAllBuffers.push_back(pNewBuff);
		m_pUpstreamDevice->AddBufferToQueue(pNewBuff);
	}

	// Do what is requested of me.  e.g. be async if requested to be async
	if (!m_bInline)
	{
		m_pThread = new FrameWork::Threads::thread<PeakLevelComputer>(this);
		if (blow_priority)
			m_pThread->low_priority();
	}

	// Set my state
	m_bRunning = true;
}

void PeakLevelComputer::Shutdown( void )
{
	if (!m_bRunning)
		return;			// Already shutdown

	// Stop thread processing (if I was doing any)
	delete m_pThread;
	m_pThread = NULL;

	// Flush upstream device so that it don't ref my buffs any mas.  They are about to be given their freedom.  
	// Additionally, don't allow AddBufferToQueue to be called on m_pUpstreamDevice any mas neither.
	Devices::Audio::DeviceInterface *pTempUpstreamDevice = m_pUpstreamDevice;	// This is complete paranoia!!!
	m_pUpstreamDevice = NULL;													// This is complete paranoia!!!
	if (pTempUpstreamDevice)
		pTempUpstreamDevice->FlushAllBuffers();
	m_pUpstreamDevice = pTempUpstreamDevice;									// This is complete paranoia!!!

	// Set my state
	m_bRunning = false;
	
	// Delete all my buffers
	for (AllBuffersIter iter = m_ListOfAllBuffers.begin(); iter < m_ListOfAllBuffers.end(); iter++)
		delete (*iter);
	// Clear my list and queue which contained pointers to my buffers
	m_ListOfAllBuffers.clear();
	while (!m_Queue.empty())
		m_Queue.pop();
}

size_t PeakLevelComputer::GetSampleRate( void ) const
{
	return m_SampleRate;
}

size_t PeakLevelComputer::GetNoChannels( void ) const
{
	return m_cChannels;
}

Devices::Audio::FormatInterface::eSampleFormat PeakLevelComputer::GetSampleFormat( void ) const
{
	return m_SampleFormat;
}

bool PeakLevelComputer::GetIsBufferInterleaved( void ) const
{
	return m_bInterleaved;
}

void PeakLevelComputer::operator() ( const void* )
{
	// I'm just going to do 1 at a time here
	Buffer *pFilledBuffer = NULL;
	m_QueueLock.lock();
	if (!m_Queue.empty())
	{
		pFilledBuffer = m_Queue.front();
		m_Queue.pop();
	}
	m_QueueLock.unlock();

	if (pFilledBuffer)
	{
		this->DoProcess(pFilledBuffer);
	}
	else
	{
		::Sleep(5);	// Be nice to the CPU.  Give it some time to give me another buffer!!!
	}
}

// Called to give me back a buffer
	// Either a buffer is filled OR
	// Flush is going down
void PeakLevelComputer::ReleaseBufferData_CB(Devices::Audio::BufferInterface *pBuffer, const wchar_t *pErrorMessage)
{
	// Must be my kinda buffer
	Buffer *pRealDealBuff = dynamic_cast<Buffer*>(pBuffer);
	if (!pRealDealBuff)
	{
		assert(!pBuffer);		// Otherwise wtf???	THIS SHOULD NEVER HAPPEN!!!
		return;
	}

	// If async, push buff on queue that thread will pop from asyncly to process
	if (!m_bInline)	// This is a const so threadsafe!!!
	{
		FrameWork::Threads::auto_lock threadSafe(m_QueueLock);
		m_Queue.push(pRealDealBuff);
	}
	// Otherwise do my sync inline boogie
	else
	{
		this->DoProcess(pRealDealBuff);
	}
}

// These next 2 functions are called either on my worker thread (if not doing inline processing) OR
//								called on the upstream devices release buffer thread (if DOING inline processing)
// Either way the important thing is that if I flush upstream, the buffer MUST NOT be added back to the upstream device
// I am guaranteed this b/c m_pUpstreamDevice is set to NULL before I call flush and it is not reset until flush is complete
void PeakLevelComputer::DoProcess(Buffer *pFilledBuffer)
{	assert(pFilledBuffer);	// Precondition

	// Analyze the buff contents for peak levels and tell our listener about such
	size_t cbData = 0;
	void *pData = pFilledBuffer->pGetBufferData(&cbData);
	if (pData && cbData)
	{
		this->AnalizeAudioSamples(pData, cbData);
		m_pPLListener->PealLevelsChanged(m_pPeaksAndValleys, this->GetNoChannels());
	}

	// I have finished w/ the buffer, so give back to the upstream device for reuse as long as we aren't shutting down
	if (m_pUpstreamDevice)
		m_pUpstreamDevice->AddBufferToQueue(pFilledBuffer);
}

void PeakLevelComputer::AnalizeAudioSamples(const void *pBuffer, size_t cbData)
{
	// Reset peak levels for every channel
	for (size_t k=0; k<this->GetNoChannels(); k++)
		m_pPeaksAndValleys[k] = 0.0f;

	if (Devices::Audio::BufferInterface::eSampleFormat_FP_32 == this->GetSampleFormat())
	{
		const float *pData = reinterpret_cast<const float*>(pBuffer);
		const size_t cSamples = cbData / (4/*sizeof(float)*/*this->GetNoChannels());
		// Cycle over all samples
		for (size_t i=0; i<cSamples; i++)
		{
			// All channels need to be handled seperately
			for (size_t j=0; j<this->GetNoChannels(); j++, pData++)
			{
				// Compute the peak level, since this is what
				// most PeakLevels seem to show
				m_pPeaksAndValleys[j] = std::max(pData[0], m_pPeaksAndValleys[j]);
			}
		}
	}
	else if (Devices::Audio::BufferInterface::eSampleFormat_I_16 == this->GetSampleFormat())
	{
		const INT16 *pData = reinterpret_cast<const INT16*>(pBuffer);
		const size_t cSamples = cbData / (2/*sizeof(INT16)*/*this->GetNoChannels());
		// Cycle over all samples
		for (size_t i=0; i<cSamples; i++)
		{
			// All channels need to be handled seperately
			for (size_t j=0; j<this->GetNoChannels(); j++, pData++)
			{
				// Compute the peak level, since this is what
				// most PeakLevels seem to show
				float fSample = (static_cast<float>(pData[0]))/32768.0f;
				m_pPeaksAndValleys[j] = std::max(fSample, m_pPeaksAndValleys[j]);
			}
		}
	}
	else
	{
		// assert(false);
	}
}

//*************************************************************************************************************************************************

PeakLevelComputer::Buffer::Buffer(PeakLevelComputer *pParent)
: m_pOwner(pParent), m_pBuff(NULL), m_cbBuff(0), m_cbData(0), m_Time(0.0)
{
	assert(pParent);	// Must be constructed w/ da non-NULL
}

PeakLevelComputer::Buffer::~Buffer( void )
{
	if (m_pBuff)
		::free(m_pBuff);
}

size_t PeakLevelComputer::Buffer::GetSampleRate( void ) const
{
	return m_pOwner->GetSampleRate();
}

size_t PeakLevelComputer::Buffer::GetNoChannels( void ) const
{
	return m_pOwner->GetNoChannels();
}

Devices::Audio::FormatInterface::eSampleFormat PeakLevelComputer::Buffer::GetSampleFormat( void ) const
{
	return m_pOwner->GetSampleFormat();
}

bool PeakLevelComputer::Buffer::GetIsBufferInterleaved( void ) const
{
	return m_pOwner->GetIsBufferInterleaved();
}

void* PeakLevelComputer::Buffer::pGetBufferData(size_t *pMemorySize)
{	assert(pMemorySize);

	if (0 == *pMemorySize)	// Is somebody just trying to read me???
	{
		*pMemorySize = m_cbData;
		return m_pBuff;
	}
	else					// Gonna be a write???
	{
		if (*pMemorySize > m_cbBuff)	// Requested size is larger that what I've already got???
		{
			if (m_pBuff)
				::free(m_pBuff);
			m_pBuff = ::malloc(*pMemorySize);
			m_cbBuff = m_cbData = *pMemorySize;
		}
		else
		{
			m_cbData = *pMemorySize;
		}

		return m_pBuff;
	}
}

void PeakLevelComputer::Buffer::ReleaseBufferData(const Devices::TimeStamp *pFrameTime, const wchar_t *pErrorMessage)
{
	if (pErrorMessage)
	{
		m_cbData = 0;
		m_Time = 0.0;
	}
	else if (pFrameTime)
	{
		m_Time = *pFrameTime;
	}

	assert(m_pOwner);
	m_pOwner->ReleaseBufferData_CB(this, pErrorMessage);
}

const Devices::TimeStamp& PeakLevelComputer::Buffer::GetTimeStamp( void ) const
{
	return m_Time;
}

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************