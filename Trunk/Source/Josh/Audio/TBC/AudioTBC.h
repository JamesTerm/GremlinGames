#pragma once

//The latency time is in reference to the latency set for the video... note: this does not take into account
//the latency time to sent to the audio card.
//This depth must be at least 3 to avoid fill buffer stalling
//I have set this to 5 so to provide a safe low latency... when using 512k buffer packets, this will keep the time
//under two frames (e.g. 48000 = 1600 * 2 = 3200 > 5 * 512 = 2560)
//I have tested 3 and 4... while these work they have a harder time staying locked on when the sample rates match
//Note: if resampling is used (e.g. FIR filter) this will add an addition packet, but this should still keep the latency low enough
const size_t DefaultLatency=5;

class AudioTBC_DLL AudioTBC_Internal :	public Devices::Audio::Base::Streams::AudioInStream_Interface, 
										public Devices::Streams::OutStream_Interface,
										public Devices::Audio::FormatInterface
{
	public:
		AudioTBC_Internal(DeviceInterface *pAudioDevice,FormatInterface *pBufferFormat,size_t LatencyQueueDepth=DefaultLatency);
		virtual ~AudioTBC_Internal();

		//both of these get the audio device by returning m_OutputStreamer
		operator DeviceInterface* ( void );
		DeviceInterface *GetAudioDevice();


	private: //From  OutStream_Interface
		//This will throw a timeout exception (should never happen)
		virtual bool AddBufferToQueue( Devices::BufferInterface *pBuffer );
		virtual bool FillBuffer(PBYTE DestBuffer,size_t BufferSizeInBytes,TimeStamp &DeviceTime);

		//returns "AudioTBC"
		virtual const wchar_t *pGetDeviceName( void ) const;
		//We can use this to check and ensure there are enough buffers in the queue
		virtual void WaitForNextBuffer();

	private: //From AudioInStream_Interface
		//Since I own it... I don't need to use this
		virtual void InStreamClosing(void *ThisChild) {}
		// This is called by the AudioBuffers when they are received
		virtual void AddBufferToTBCList(Devices::BufferInterface *pBuffer , const wchar_t *pErrorMessage );
		virtual void In_FormatChangeDetected(); //now I can update my sample reference point

		virtual Devices::BufferInterface *InStream_Interface_GetNewBuffer(Devices::Streams::InputStream *pInputStream);
		virtual void InStream_Interface_FreeBuffer(Devices::BufferInterface *buffer);

		///This will obtain the value from the first buffer passed in to FillBuffer()... will be successful as long as I set the
		///variable prior to starting the AudioInStream's StartTBCQueue
		///returns m_BufferSampleSize
		virtual size_t GetBufferSampleSize();

	private:
		//The out stream sample rate must always match the rate of client accessing the OutStream (ie. ASIO driver callbacks)
		//It is debatable whether the other formats should follow this convention.  The advantage
		//to doing this is that it will lighten the workload of the driver callback.  The only disadvantage
		//would be that these accessors will not be initialized until the first buffer is passed through
		//So I am making these private for now (I do not think anyone else should be calling these anyhow)
		//returns m_OutStream_SampleRate
		virtual size_t GetSampleRate( void ) const;
		virtual size_t GetNoChannels( void ) const;

		virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;
		virtual bool GetIsBufferInterleaved( void ) const;

		bool FillBuffer(byte *DestBuffer,size_t NoSamples,FormatInterface *Format,TimeStamp &DeviceTime);

		size_t m_OutStream_SampleRate;
		Devices::Audio::FormatInterface::eSampleFormat m_OutStream_SampleFormat;
		bool m_bInterleaved;
		//size_t m_OutStream_NoChannels;
		size_t m_BufferSampleSize;

		size_t m_LatencyQueueDepth; //A measurement of the queue depth for the source to maintain

		//Note: both of these are passive
		Devices::Audio::Base::Streams::AudioInputStream m_InputStreamer;
		  //Keep track of the device that is passed into the constructor
		const Devices::Audio::DeviceInterface * const m_InputDevice;
		Devices::Audio::Base::Streams::AudioOutputStream m_OutputStreamer;

		//Time Correction Variables
		TimeStamp m_LastInBufferSentTime,m_LastOutBufferSentTime;  //used to clock the sample rates
		double m_InSampleRate,m_OutSampleRate; //high precision time of the real sample rates
		double m_CurrentOutSampleTime,m_CurrentInSampleTime;  //We smooth the time intervals themselves for best results

		Devices::Audio::Base::Streams::AudioBuffer *m_pCurrentBuffer; //keep track of the current buffer to transfer
		double m_CurrentTransferRate; //the rate of transfer where 1.0 is the same rate (very importatnt varible!)
		double m_ReferencePoint; //This helps to compute a vector offset for the correction value
		double m_CurrentLocalPosition; //keeps track of where the sample cursor within the current buffer
		bool m_StartedTBC;		//since we wait for the first buffer to be delivered, we keep track of when to start the TBC
};

class AudioTBC_DLL AudioTBC : public Devices::Audio::Base::DeviceWrapper
{
	public:
		AudioTBC(DeviceInterface *pAudioDevice,size_t LatencyQueueDepth=DefaultLatency) :
		  m_AudioTBC(pAudioDevice,pAudioDevice,LatencyQueueDepth),DeviceWrapper(m_AudioTBC.GetAudioDevice()) {}

		//Usually the buffer format is the same as the input device, but it may be possible that they could be different
		//(e.g. could be different to test device converters)
		AudioTBC(DeviceInterface *pAudioDevice,FormatInterface *pBufferFormat,size_t LatencyQueueDepth=DefaultLatency) :
		  m_AudioTBC(pAudioDevice,pBufferFormat,LatencyQueueDepth),DeviceWrapper(m_AudioTBC.GetAudioDevice()) {}

	private:
		AudioTBC_Internal m_AudioTBC;
};
