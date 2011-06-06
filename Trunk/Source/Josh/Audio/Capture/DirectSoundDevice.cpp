#include "stdafx.h"
#include "Devices_Audio_Capture.h"
#pragma comment (lib, "dsound")

//*************************************************************************************************************************************************

namespace Devices { namespace Audio { namespace Capture {
	
// DirectSound default device for voice playback {DEF00002-9C6D-47ED-AAF1-4DDA8F2B5C03}
const GUID DSDEVID_DefaultVoicePlayback = { 0xdef00002, 0x9c6d, 0x47ed, { 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03 } };

// DirectSound default device for voice capture {DEF00003-9C6D-47ED-AAF1-4DDA8F2B5C03}
const GUID DSDEVID_DefaultVoiceCapture = { 0xdef00003, 0x9c6d, 0x47ed, { 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03 } };

// DEFINE_GUID(IID_IDirectSoundNotify, 0xb0210783, 0x89cd, 0x11d0, 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16);
const GUID IID_IDirectSoundNotify = { 0xb0210783, 0x89cd, 0x11d0, { 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16 } };

} } }

using namespace Devices::Audio::Capture;

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

DirectSoundDevice::DirectSoundDevice( void ) : m_pThread(NULL), m_hCapBufferPosnNotifyEvent(NULL)
, m_dwCaptureBufferSize(0), m_dwNotifySize(0), m_dwNextCaptureOffset(0)
{
	/*
	HRESULT hr = ::CoInitialize(NULL);
	assert(SUCCEEDED(hr));
	*/
	HRESULT hr = ::DirectSoundCaptureCreate8(&DSDEVID_DefaultVoiceCapture/*default device*/, &m_pDSCap, NULL/*outer unknown*/);
	assert(SUCCEEDED(hr));

	// Create the capture buffer
	WAVEFORMATEX waveFormat = { WAVE_FORMAT_PCM, 2/*channels*/, 48000, 192000/*ave bytes per sec*/, 4/*nBlockAlign*/, 16/*bits per sample*/, 0 }; 
	DSCBUFFERDESC buffDesc = { 0 };
	buffDesc.dwSize = sizeof(buffDesc);
	buffDesc.dwBufferBytes = waveFormat.nAvgBytesPerSec * ms_cSecCaptureBuff;
	buffDesc.lpwfxFormat = &waveFormat;
	
	hr = m_pDSCap->CreateCaptureBuffer(&buffDesc, &m_pDSCapBuffer, NULL/*outer unknown*/);
	assert(SUCCEEDED(hr));

	// Setup the notification pons
	hr = m_pDSCapBuffer->QueryInterface(IID_IDirectSoundNotify, (void**)&m_pDSCapBufferNotify);	// TODO
	assert(SUCCEEDED(hr));
	m_hCapBufferPosnNotifyEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);	// Auto reset event!!!
	{
		const size_t cNotifyPosns = ms_cSecCaptureBuff * 4;	// 4 per sec
		DSBPOSITIONNOTIFY *pNotifyPosns = new DSBPOSITIONNOTIFY[cNotifyPosns];

		 // Setup the notification positions
		const size_t cbNotifyInterval = waveFormat.nAvgBytesPerSec / 4;	// 4 per sec
		m_dwCaptureBufferSize = buffDesc.dwBufferBytes;
		m_dwNotifySize = static_cast<DWORD>(cbNotifyInterval);
		for (size_t i=0; i<cNotifyPosns; i++)
		{
			pNotifyPosns[i].dwOffset = static_cast<DWORD>( (cbNotifyInterval*i) + cbNotifyInterval - 1 );
			assert(pNotifyPosns[i].dwOffset < buffDesc.dwBufferBytes);
			pNotifyPosns[i].hEventNotify = m_hCapBufferPosnNotifyEvent;             
		}
	    
		// Tell DirectSound when to notify us. 
		// The notification will come in the from of signaled events.
		hr = m_pDSCapBufferNotify->SetNotificationPositions(cNotifyPosns, pNotifyPosns);
		assert(SUCCEEDED(hr));

		delete [] pNotifyPosns;
	}
}

DirectSoundDevice::~DirectSoundDevice( void )
{
	// Stop my thread shit
	this->StopCapture();
	::CloseHandle(m_hCapBufferPosnNotifyEvent);	// TODO: Creation could be moved to StartCapture, and Deletion could be moved to StopCapture right before thread destruction

	// Dbl duty that all buffers are given back to upstream device so they have a chance at dealloc
	this->FlushAllBuffers();
	//::CoUninitialize();		// This is making the assumption that the constructor call suceeded!!!
}

bool DirectSoundDevice::AddBufferToQueue(Devices::BufferInterface *pBuffer)
{
	Devices::Audio::BufferInterface *pAudioBuffer = dynamic_cast<Devices::Audio::BufferInterface*>(pBuffer);
	if (!pAudioBuffer)
	{
		assert(false);
		return false;
	}

	if (Devices::Audio::Base::operator != (*this, *pAudioBuffer))
	{
		assert(false);	// Wrong format...you must respect my format f*@$ers!!!
		return false;
	}

	FrameWork::Threads::auto_lock threadSafe(m_BufferQueueLock);
	m_BufferQueue.push(pAudioBuffer);
	return true;
}

void DirectSoundDevice::FlushAllBuffers( void )
{
	FrameWork::Threads::auto_lock threadSafe(m_BufferQueueLock);
	while (!m_BufferQueue.empty())
	{	
		Devices::Audio::BufferInterface *pFront = m_BufferQueue.front();
		assert(pFront);
		m_BufferQueue.pop();
		pFront->ReleaseBufferData();
	}
}

const wchar_t* DirectSoundDevice::pGetDeviceName( void ) const
{
	return L"Direct Sound Capture Device";
}

size_t DirectSoundDevice::GetSampleRate( void ) const
{
	return 48000;
}

size_t DirectSoundDevice::GetNoChannels( void ) const
{
	return 2;
}

Devices::Audio::FormatInterface::eSampleFormat DirectSoundDevice::GetSampleFormat( void ) const
{
	return Devices::Audio::FormatInterface::eSampleFormat_I_16;
}

bool DirectSoundDevice::GetIsBufferInterleaved( void ) const
{
	return true;
}

bool DirectSoundDevice::StartCapture( void )
{
	// Already running???
	if (m_pThread)
	{
		assert(false);
		return true;
	}
	// Do I have what I need???
	if (!m_pDSCapBuffer || !m_pDSCapBufferNotify)
	{
		assert(false);
		return false;
	}

	// Start my thread
	m_pThread = new FrameWork::Threads::thread<DirectSoundDevice>(this);

	// Start the capture
	m_dwNextCaptureOffset = 0;
	HRESULT hr = m_pDSCapBuffer->Start(DSCBSTART_LOOPING);
	assert(SUCCEEDED(hr));

	return SUCCEEDED(hr);
}

bool DirectSoundDevice::StopCapture( void )
{
	// Alreayd stopped?
	if (!m_pThread)
		return true;
	// Do I have what I need???
	if (!m_pDSCapBuffer)
	{
		assert(false);
		return true;
	}

	// Stop Capture
	HRESULT hr = m_pDSCapBuffer->Stop();
	assert(SUCCEEDED(hr));

	// Stop My thread
	delete m_pThread;
	m_pThread = NULL;

	// TODO: Flush my buffers???

	return true;
}

double DirectSoundDevice::PollPosition(bool &bDone)
{
	bDone = false;
	return -DBL_MAX;
}

void DirectSoundDevice::operator() ( const void* )
{	assert(m_pDSCapBuffer);

	// Wait for event signal
	// If timeout
		// Return
	// Else
		// If queue empty
			// Check for drop of samples
			// Return
		// Else
			// Pop off queue
			// Get the size and starting posn of new data
			// Lock sound card buff
			// Copy into memory popped off queue
			// UnLock sound card buff
			// Release memory popped off queue
	////////////////////////////////////////////////////////////////

	DWORD dwSuccess = ::WaitForSingleObject(m_hCapBufferPosnNotifyEvent, 375);	// 3/8 second
	if (WAIT_OBJECT_0 != dwSuccess)
		return;	// Not signaled...timeout elapsed.

	// First find out how much new data we've got and return if by chance it's 0
	DWORD dwCapturePos=0, dwReadPos=0;
	HRESULT hr =  m_pDSCapBuffer->GetCurrentPosition(&dwCapturePos, &dwReadPos);
	assert(SUCCEEDED(hr));

	LONG lLockSize = dwReadPos - m_dwNextCaptureOffset;
    if (0L > lLockSize)
        lLockSize += m_dwCaptureBufferSize;
    // Block align lock size so that we are always write on a boundary
    lLockSize -= (lLockSize % m_dwNotifySize);

    if (0L == lLockSize)
	{
		assert(false);	// To be or not to be, that is the question...never thought I'd get Hamlet up in this hizzy
		return;
	}

	//*********************************************************************************************************************************
	// Get a buffer to fill and send out to the world

	Devices::Audio::BufferInterface *pBufferToFill = NULL;
	{
		FrameWork::Threads::auto_lock threadSafe(m_BufferQueueLock);
		if (m_BufferQueue.empty())
		{
			// TODO: Output debug string
			// TODO: Check that the sound card isn't going to write over memory that I haven't retrieved yet out of the circular buffer
			return;
		}
		pBufferToFill = m_BufferQueue.front();
		m_BufferQueue.pop();
	}
	assert(pBufferToFill);

	//*********************************************************************************************************************************
  	
	size_t cbCopyToBuff = static_cast<size_t>(lLockSize);
	byte *pCopyToBuff = reinterpret_cast<byte*>(pBufferToFill->pGetBufferData(&cbCopyToBuff));
	assert(pCopyToBuff && static_cast<size_t>(lLockSize)==cbCopyToBuff);

    // Lock the capture buffer down
	DWORD dwCaptureLength=0, dwCaptureLength2=0;
	VOID *pbCaptureData=NULL, *pbCaptureData2=NULL;
	hr = m_pDSCapBuffer->Lock(m_dwNextCaptureOffset, lLockSize, &pbCaptureData, &dwCaptureLength, &pbCaptureData2, &dwCaptureLength2, 0L);
    assert(SUCCEEDED(hr));

    // Write the data into the wav file
	::memcpy(pCopyToBuff, pbCaptureData, dwCaptureLength);

    // Move the capture offset along
    m_dwNextCaptureOffset += dwCaptureLength; 
    m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer

    if (NULL != pbCaptureData2)
    {
        // Write the data into the wav file
		::memcpy(pCopyToBuff+dwCaptureLength, pbCaptureData2, dwCaptureLength2);
        
        // Move the capture offset along
        m_dwNextCaptureOffset += dwCaptureLength2; 
        m_dwNextCaptureOffset %= m_dwCaptureBufferSize; // Circular buffer
    }

    // Unlock the capture buffer
    hr = m_pDSCapBuffer->Unlock(pbCaptureData, dwCaptureLength, pbCaptureData2, dwCaptureLength2);
	assert(SUCCEEDED(hr));

	// Release the copy to buff
	pBufferToFill->ReleaseBufferData();

	//*********************************************************************************************************************************
}

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************
