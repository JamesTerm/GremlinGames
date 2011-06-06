#pragma once

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

struct DEVICES_AUDIO_CAPTURE_API ICaptureSource
	: public Devices::Audio::DeviceInterface,
	  public Devices::AudioVideo::DSCapture::ICaptureSession,
	  public DeviceBase
{ };

class DEVICES_AUDIO_CAPTURE_API DirectSoundDevice : public ICaptureSource
{
	// Interface
public:

	// C'tor/D'tor
	DirectSoundDevice( void );
	virtual ~DirectSoundDevice( void );

	// Devices::Audio::DeviceInterface
	virtual bool AddBufferToQueue(Devices::BufferInterface *pBuffer);
	virtual void FlushAllBuffers( void );
	virtual const wchar_t *pGetDeviceName( void ) const;
	virtual size_t GetSampleRate( void ) const;
	virtual size_t GetNoChannels( void ) const;
	virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;
	virtual bool GetIsBufferInterleaved( void ) const;

	// Devices::AudioVideo::DSCapture::ICaptureSession
	virtual bool StartCapture( void );
	virtual bool StopCapture( void );
	virtual double PollPosition(bool &bDone);

	// Mine all mine...
	virtual void operator() ( const void* );	// My worker thread function that does something useful w/ a buffer after it's been filled

	// Implementation
protected:

	// Data
protected:

	static const byte ms_cSecCaptureBuff = 4;

	// The root direct sound object for capture
	CComPtr<IDirectSoundCapture8> m_pDSCap;
	// Enables you to retrieve props of the buffer, start and stop audio capture, and lock portions of the memory
	CComPtr<IDirectSoundCaptureBuffer> m_pDSCapBuffer;		// This buffer is conceptually circular
	CComPtr<IDirectSoundNotify> m_pDSCapBufferNotify;

	HANDLE m_hCapBufferPosnNotifyEvent;
	DWORD m_dwCaptureBufferSize;
	DWORD m_dwNotifySize;
	DWORD m_dwNextCaptureOffset;

	FrameWork::Threads::critical_section m_BufferQueueLock;
	std::queue<Devices::Audio::BufferInterface*> m_BufferQueue;

	// My worker thread that does something useful w/ a buffer after it's been filled
	FrameWork::Threads::thread<DirectSoundDevice> *m_pThread;

	// Illegal
private:

	DirectSoundDevice(const DirectSoundDevice&) { assert(false); }
	void operator = (const DirectSoundDevice&) { assert(false); }
};

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************