#pragma once
#define _DS_ Devices::Streams

//This is a generic audio buffer used that works with a constant buffer size.  This is this ideal buffer type used
//for audio drivers which allocate [hardware] buffers to be filled in, and these work with a constant size
class DEVICES_AUDIO_BASE_API AudioBuffer :	public AudioBuffer_Base
{
	public:
		AudioBuffer(AudioInputStream *pParent,const FormatInterface *pFormat);
		virtual ~AudioBuffer();

		//implementation of the buffer interface
		virtual void *pGetBufferData( size_t *pMemorySize );
		virtual void ReleaseBufferData(const TimeStamp *pFrameTime , const wchar_t *pErrorMessage = NULL ); 

		virtual const TimeStamp& GetTimeStamp( void ) const
			{	return m_TimeReleased;
			}
		size_t GetNoSamples( void ) const 
			{	return m_NoSamples;
			}
	protected:
		byte *m_Memory; //actual buffer
		size_t m_BufferSize;
		size_t m_NoSamples; //cache this (it will be used often)
		TimeStamp m_TimeReleased;
};

//This is a fuse between RawBuffer and AudioBuffer... The buffer sizes are not constant and are determined dynamically by the device
//For now this will follow the same conventions are the RawBuffer... however this will retain all of the other pertinent audio
//formatting information
struct DEVICES_AUDIO_BASE_API AudioDynamicBuffer : public AudioBuffer_Base
{
	public:
		AudioDynamicBuffer(AudioInputStream *pParent,const FormatInterface *pFormat);
		virtual void *pGetBufferData( size_t *pMemorySize );
		virtual void ReleaseBufferData( const TimeStamp *pFrameTime = NULL , const wchar_t *pErrorMessage = NULL );
		virtual const TimeStamp& GetTimeStamp( void ) const
			{	return m_TimeReleased;
			}
	private:
		RawBuffer m_RawBuffer;
		TimeStamp m_TimeReleased;
};



class DEVICES_AUDIO_BASE_API Audio_QueueControlDeviceClient :	public Audio_OutStream_QueueSource_Interface,
																	public AudioInStream_Interface,
																	public Audio::Base::aud_convert::ChannelMixer_Interface
{
	public:
		//for now the device to be used will use the raw data convention where the device will set the size and the buffer
		//will reallocate the memory as needed
		Audio_QueueControlDeviceClient(Audio::DeviceInterface *AudioDeviceInput,bool BypassFormatCheck=true);
		virtual ~Audio_QueueControlDeviceClient();

		Audio::DeviceInterface *GetAudioDevice() {return m_OutStream.GetAudioDevice();}

	protected:
		//derived from Audio_OutStream_QueueSource_Interface
		virtual bool Audio_OutStream_QueueSource_FillBuffer(PBYTE DestBuffer,size_t NoSamples,FormatInterface *OutFormat);
		//By default we can accommodate any buffer format!
		virtual bool Audio_OutStream_QueueSource_BypassFormatCheck()
			{	return m_BypassFormatCheck;
			}

		//derived from input stream interface
		virtual void InStreamClosing(void *ThisChild);
		virtual void AddBufferToTBCList( Devices::BufferInterface *pBuffer , const wchar_t *pErrorMessage );
		virtual void In_FormatChangeDetected();

		//From AudioInStream_Interface (except for SetBufferSampleSize)
		//The parent will need to set what type of audio buffer to use
		virtual Devices::BufferInterface *InStream_Interface_GetNewBuffer(_DS_::InputStream *pInputStream);
		virtual void InStream_Interface_FreeBuffer(Devices::BufferInterface *buffer);
		//Try to set this if possible so that the TBC can start streaming
		void SetBufferSampleSize(size_t SampleSize) {m_SampleSize=SampleSize;}
		size_t GetBufferSampleSize() {return m_SampleSize;}

		Audio::DeviceInterface * const m_AudioDeviceInput;
	protected:
		//From Channel Mixer
		virtual size_t ChannelMixer_GetSourceNoChannels(void *UserData) const
			{	return m_ChannelMixerSourceNoChannels;
			}
		virtual size_t ChannelMixer_GetDestNoChannels(void *UserData) const
			{	return m_ChannelMixerDestNoChannels;
			}
		virtual float *ChannelMixer_GetVolumeMatrix(void *UserData) const
			{ return m_VolumeMatrix;
			}

	private:
		//We will convert the buffers to floating point (preferred native) to work with the audio TBC
		virtual Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const { return Audio::FormatInterface::eSampleFormat_FP_32; }
		//we'll also deinterleave if necessary
		virtual bool GetIsBufferInterleaved( void ) const { return false; }

		//These simply wrap to the device
		virtual size_t GetSampleRate( void ) const	{return m_AudioDeviceInput->GetSampleRate();}
		virtual size_t GetNoChannels( void ) const	{return m_AudioDeviceInput->GetNoChannels();}

		AudioInputStream m_InputStreamer;	//The actual workhorse of the device client
		Audio_OutStream_QueueSource m_OutStream;  //Does all the things I need to work like a device

		Audio::Base::aud_convert::FloatingQueue m_FloatingQueue; //used to help align sample buffer sizes
		Audio::Base::aud_convert::ChannelMixer m_ChannelMixer; //used in case the device client needs to change the number of channels
		float *m_VolumeMatrix;
		//If I cache both of these I'll not need to create the volume matrix each time
		size_t m_ChannelMixerDestNoChannels;  //I have to access the buffer for the mixer to access this variable
		size_t m_ChannelMixerSourceNoChannels;
		//Cache the sample size so since it is constant and can provide a value even when no buffers are in queue
		size_t m_SampleSize;
		bool m_BypassFormatCheck;
};

class DEVICES_AUDIO_BASE_API Audio_FrameStreamer :	public Audio_QueueControlDeviceClient
{
	public:
			Audio_FrameStreamer(Audio::DeviceInterface *AudioDeviceInput,double FrameRate);
			virtual ~Audio_FrameStreamer( void );
	protected:
			//This will compute the frame size of samples
			//For precision this number will vary + - 1 sample... so the client will need a dynamic type of buffer
			virtual size_t Audio_OutStream_QueueSource_GetBufferSizeInBytes(Audio::FormatInterface *OutFormat);
	private:
			//A circular frame rate tracker
			const double m_FrameRate;
			double m_rho;
};

//Just like the AudioInputStream... except for the following:
//1. it only gets the sample rate from the buffer format pointer... everything else gets its format from the audio device
//2. This will construct a channel volume matrix which works with the Audio Host Mixer
class DEVICES_AUDIO_BASE_API AudioHostInputStream :		public AudioInputStream,
														public FormatInterface
{
	public:
		//Note for a InputStream which works with a passive Audio Device (e.g. the AudioTBC) use a low value like 2
		AudioHostInputStream(AudioInStream_Interface *pParent,Devices::Audio::DeviceInterface *pAudioDevice,FormatInterface *pBufferFormat,size_t DefaultTBCQueueDepth=_DS_::c_DefaultTBCQueueDepth);
		virtual ~AudioHostInputStream();

		virtual size_t GetSampleRate( void ) const 
			{	return m_pBufferFormat->GetSampleRate();
			}
		virtual size_t GetNoChannels( void ) const 
			{	return m_pAudioDevice->GetNoChannels();
			}

		virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const
			{	return m_pAudioDevice->GetSampleFormat();
			}

		virtual bool GetIsBufferInterleaved( void ) const
			{	return m_pAudioDevice->GetIsBufferInterleaved();
			}

		float *GetVolumeMatrix(); 

	private:
		FormatInterface * const m_pBufferFormat;	//Destination format
		Audio::DeviceInterface * const m_pAudioDevice;		//Source format
		//Cache the number of channels to ensure the volume matrix is the correct size
		size_t m_SourceNoChannels,m_DestNoChannels;
		float *m_pVolumeMatrix; //This is set in the constructor (since we have const pointers for both dimensions)
};

///This will passively convert a DeviceClient into a Device.  The way it works is that you sending device client point to this, and the
///destination device client uses GetAudioDevice() the same way other types of devices work.  One way to think of this is a female to female
///phono adaptor where device clients are the male phono jacks.  The source device client would be the one which has its own driving force
///To send buffers to get processed.  A good example of this would be frames which are sent to a audio buffer receive device client.  In that
///example you would have that point to "this", and now it works like a device.  You can then hook up this device like you would other devices
///to some device client such as Audio_QueueControlDeviceClient.  Where Audio_QueueControlDeviceClient would be the destination device client.
/// \see VideoDeviceClientToDevice
class DEVICES_AUDIO_BASE_API AudioDeviceClientToDevice : public Audio::DeviceInterface
{
	public:
		AudioDeviceClientToDevice(
			const wchar_t *pDeviceName = L"ConvertedDevice",				///<The name of the converted device for destination client
			const wchar_t *InputDeviceClientName=L"InputFromDeviceClient"	///<The name of the source device client that is being converted
			);

	  ///This device is the actual device that we have converted to so the destination device client connects to this.  (e.g. Audio_QueueControlDeviceClient)
	  Audio::DeviceInterface *GetAudioDevice()
		  {	return m_OutStream.GetAudioDevice();
		  }
	protected: //From device interface
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer );
		virtual void FlushAllBuffers( void );
		virtual const wchar_t *pGetDeviceName( void ) const;

	protected: //From Audio Format Interface
		virtual size_t GetSampleRate( void ) const;
		virtual size_t GetNoChannels( void ) const;
		virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;
		virtual bool GetIsBufferInterleaved( void ) const;

	//private:
		AudioOutStream_NotThreaded m_OutStream;
		std::wstring m_InputDeviceClientName;

		//Note: we cache the buffer specs (should be constant)
		size_t m_SampleRate;
		size_t m_NoChannels;
		Devices::Audio::FormatInterface::eSampleFormat m_SampleFormat;
		bool m_bIsInterleaved;
};

#ifndef _WIN64
class DEVICES_AUDIO_BASE_API ResamplingDevice : public AudioDeviceClientToDevice
{
public:
			
		ResamplingDevice(const wchar_t *pDeviceName = L"ConvertedDevice",				///<The name of the converted device for destination client
			const wchar_t *InputDeviceClientName=L"InputFromDeviceClient"	///<The name of the source device client that is being converted
			):AudioDeviceClientToDevice(pDeviceName, InputDeviceClientName)
		{
			size_t src_rates[] = 			{
												8000,
												11025,
												16000,
												22050,
												32000,
												44100											
											};
		
			for (size_t i=0; i<sizeof(src_rates)/sizeof(size_t); i++)
				m_hq_resamplers[src_rates[i]] = new FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_stereo_f32>(48000, src_rates[i]);
		}
		~ResamplingDevice()
		{
			size_t src_rates[] = 	{
												8000,
												11025,
												16000,
												22050,
												32000,
												44100,
									};
		
			for (size_t i=0; i<sizeof(src_rates)/sizeof(size_t); i++)
				delete m_hq_resamplers[src_rates[i]];
		}

protected: //From device interface
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer );

		std::map<size_t, FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_stereo_f32> *> m_hq_resamplers;
};
#endif


#undef _DS_