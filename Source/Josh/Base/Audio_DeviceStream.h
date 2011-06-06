#pragma once
#define _DS_ Devices::Streams

class AudioInStream_Interface : public _DS_::InStream_Interface
{
	public:
		///When providing this value, you must be able to successfully have the information available... you may wish to use an assert
		///to ensure all the environment needed to get this value has been set up by the time this gets called.
		/// \note this gets called when the StartTBCQueue instantiates the first AudioBuffer 
		virtual size_t GetBufferSampleSize()=0;
};

class DEVICES_BASE_API Audio_OutStream_QueueSource_Interface : public Audio::FormatInterface
{
public:
	virtual ~Audio_OutStream_QueueSource_Interface() {}
	///Here is the callback used to get the buffer processed.
	/// \note This callback will continue to be called as long as there are buffers to fill, so
	///for a queue'd source try to lower the queue depth as much as possible...
	/// \return true if successful
	virtual bool Audio_OutStream_QueueSource_FillBuffer(byte *DestBuffer,size_t NoSamples,Audio::FormatInterface *OutFormat)=0;
	///You can ignore this if the device client is managing its own buffer sizes.
	/// \return If this value is non-zero, this means the Device (The client who implements this interface) will control the size
	///in accordance to the raw rules.  The Device Client of this kind of Device should most likely use AudioDynamicBuffer
	///where it allocates the size that the device will give it.
	virtual size_t Audio_OutStream_QueueSource_GetBufferSizeInBytes(Audio::FormatInterface *OutFormat)
	{	return 0;
	}
	///Override with true if your device can accommodate all buffer formats
	virtual bool Audio_OutStream_QueueSource_BypassFormatCheck()
	{	return false;
	}
};

//******************************** Audio

class AudioStreamBase
{
	public:
		bool DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer);
		bool DoFormatsMatch(const Devices::Audio::FormatInterface *pFormat1,const Devices::Audio::FormatInterface *pFormat2);
};

class AudioStreamBaseDevice :	public AudioStreamBase,
								public Audio::DeviceInterface
{
	public:
		//refer to parent to wrap device methods
		//And pass in the format to also be wrapped
		AudioStreamBaseDevice(Devices::DeviceInterface * const Parent,Audio::FormatInterface * const Format) :
		  m_Parent(Parent),m_Format(Format) {}

		bool DoFormatsMatch(const Devices::BufferInterface *Buffer);

		//wrap everything
		//From device interface
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer ) {return m_Parent->AddBufferToQueue(pBuffer);}
		virtual void FlushAllBuffers( void ) {m_Parent->FlushAllBuffers();}
		virtual const wchar_t *pGetDeviceName( void ) const {return m_Parent->pGetDeviceName();}

		//From format interface
		virtual size_t GetSampleRate( void ) const {return m_Format->GetSampleRate();}
		virtual size_t GetNoChannels( void ) const {return m_Format->GetNoChannels();}
		virtual Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const {return m_Format->GetSampleFormat();}
		virtual bool GetIsBufferInterleaved( void ) const {return m_Format->GetIsBufferInterleaved();}

	private:
		Devices::DeviceInterface * const m_Parent;
		Audio::FormatInterface * const m_Format;
};

class DEVICES_BASE_API AudioOutputStream : public _DS_::OutputStream
{
	public:
		AudioOutputStream(_DS_::OutStream_Interface *pParent,Audio::FormatInterface * const Format) : 
		  _DS_::OutputStream(pParent),m_AudioStreamBase(this,Format) {}

		Audio::DeviceInterface *GetAudioDevice()
		  {	return &m_AudioStreamBase;
		  }

	protected:
		virtual bool DoFormatsMatch(const Devices::BufferInterface *Buffer);
	private:
		AudioStreamBaseDevice m_AudioStreamBase;
};

class DEVICES_BASE_API AudioOutStream_NotThreaded : public _DS_::OutStream_NotThreaded
{
	public:
		AudioOutStream_NotThreaded(Audio::FormatInterface * const Format,const wchar_t *pDeviceName = L"OutStream_NotThreaded") : 
		  _DS_::OutStream_NotThreaded(pDeviceName),m_AudioStreamBase(this,Format) {}

		Audio::DeviceInterface *GetAudioDevice()
			  {	return &m_AudioStreamBase;
			  }

	protected:
		virtual bool DoFormatsMatch(const Devices::BufferInterface *Buffer);
		//Override this if your Device can accommodate to any buffer format
		virtual bool BypassFormatCheck(); //default implementation returns false
	private:
		AudioStreamBaseDevice m_AudioStreamBase;
};

class DEVICES_BASE_API AudioInputStream : public _DS_::InputStream
{
	public:
		AudioInputStream(AudioInStream_Interface *pParent,Devices::DeviceInterface *pDevice,size_t DefaultTBCQueueDepth=_DS_::c_DefaultTBCQueueDepth,
			const wchar_t *pDeviceClientName = L"VideoInputStream") : InputStream(pParent, pDevice, DefaultTBCQueueDepth, pDeviceClientName) {}
		AudioInStream_Interface *GetParent() const;
	protected:
		virtual bool DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer);
	private:
		AudioStreamBase m_AudioStreamBase;
};

class DEVICES_BASE_API Audio_OutStream_QueueSource : public AudioOutStream_NotThreaded
{
	public:
		Audio_OutStream_QueueSource(Audio_OutStream_QueueSource_Interface *OutStreamClient,const wchar_t *pDeviceName = L"OutStream_QueueSource");
		//This will continue to iterate through all buffers available as long as the callbacks are providing data
		void TransferBuffers();
	protected:
		//defer to parent
		virtual bool BypassFormatCheck()
			{	return m_pParent->Audio_OutStream_QueueSource_BypassFormatCheck();
			}
	private:
		Audio_OutStream_QueueSource_Interface * const m_pParent;
};

class DEVICES_BASE_API RawBuffer : public Devices::BufferInterface
{
	public:
		RawBuffer(_DS_::InputStream *pParent);
		virtual ~RawBuffer();
		//Derived from Buffer interface
		virtual void *pGetBufferData( size_t *pMemorySize );
		virtual void ReleaseBufferData( const TimeStamp *pFrameTime = NULL , const wchar_t *pErrorMessage = NULL );
		virtual const TimeStamp& GetTimeStamp( void ) const;

	protected:
		_DS_::InputStream * const m_pParent;

		void *m_pBuff;
		size_t m_cbBuff;
		size_t m_cbData;
		Devices::TimeStamp m_Time;
};

//All this does is cache format information passed in the constructor (usually from the device)
//Note: we may wish to add a method to dynamically update the format
class DEVICES_BASE_API AudioBuffer_Base  :	public Devices::Audio::BufferInterface
{
	public:
		AudioBuffer_Base(AudioInputStream *pParent,const FormatInterface *pFormat);
		virtual ~AudioBuffer_Base();

		//implementation of the audio format interface
		virtual size_t GetSampleRate( void ) const 
			{	return m_SampleRate;
			}
		virtual size_t GetNoChannels( void ) const 
			{	return m_NoChannels;
			}

		virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const
			{	return m_SampleFormat;
			}
		virtual bool GetIsBufferInterleaved( void ) const
			{	return m_bIsInterleaved;
			}

	protected:
		AudioInputStream * const m_pParent;

		//Note: we cache the buffer specs (should be constant)
		size_t m_SampleRate;
		size_t m_NoChannels;
		Devices::Audio::FormatInterface::eSampleFormat m_SampleFormat;
		bool m_bIsInterleaved;
};

#undef _DS_