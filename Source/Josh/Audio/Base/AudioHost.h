#pragma once
#define _DS_ Devices::Streams

class DEVICES_AUDIO_BASE_API AudioHostMixer :	public Streams::AudioInStream_Interface, 
												public FormatInterface,
												public AudioHostCallback_Interface,
												public aud_convert::ChannelMixer_Interface

{
	public:
		AudioHostMixer(AudioHost_Interface *pHost);
		~AudioHostMixer();

		//To use the Audio Host Mixer... add and remove these streams which in turn will give access to the audio device
		bool AddAudioStream(Streams::AudioHostInputStream *pAudioStream);
		bool RemoveAudioStream (Streams::AudioHostInputStream *pAudioStreamToRemove);

		virtual bool FillBuffer(byte *Buffer,size_t NoSamples,void *AudioInputStream_ptr=NULL);
		bool FillBuffer(byte *Buffer,size_t NoSamples,Streams::AudioHostInputStream *AudioInputStream_ptr,bool mix);

		//gets the stream's parameter of the volume matrix
		float *GetVolumeMatrix(Streams::AudioHostInputStream *stream) const;
		//gets the stream's parameter of the number of channels
		size_t GetSourceNoChannels(Streams::AudioHostInputStream *stream) const;

		//For these three (using a device), it may return NULL if it can't find the stream
		Streams::AudioHostInputStream *GetAudioHostInputStream(Devices::DeviceInterface *AudioDevice) const;
		float *GetVolumeMatrix(Devices::DeviceInterface *AudioDevice) const;
		size_t GetSourceNoChannels(Devices::DeviceInterface *AudioDevice) const;
		//Note: just use ChannelMixer_GetDestNoChannels() for the other diminsion

	protected:  //from AudioInStream_Interface
		//removes the audio stream in this child
		virtual void InStreamClosing(void *ThisChild);
		virtual void In_FormatChangeDetected( void ) {}
		virtual Devices::BufferInterface *InStream_Interface_GetNewBuffer(_DS_::InputStream *pInputStream);
		virtual void InStream_Interface_FreeBuffer(Devices::BufferInterface *buffer);

		//Nothing to do
		virtual void AddBufferToTBCList(Devices::BufferInterface *pBuffer , const wchar_t *pErrorMessage ) {} 
		//Gets the parent/host buffer sample size
		virtual size_t GetBufferSampleSize();
	private:

		//Implementation of the audio format interface

		//Gets the parent/host sample rate
		virtual size_t GetSampleRate( void ) const;
		//Gets the parent/host sample format
		virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;
		//Gets the parent/host number of channels
		virtual size_t GetNoChannels( void ) const;
		//Is the parent interleaved
		virtual bool GetIsBufferInterleaved( void ) const;


		//Implementation of the ChannelMixer_Interface
		//Note: this is thread safe since the number of channels is assigned only once within the constructor
		virtual size_t ChannelMixer_GetSourceNoChannels(void *UserData) const;
		//returns the parent/host number of channels
		virtual size_t ChannelMixer_GetDestNoChannels(void *UserData) const;

		virtual float *ChannelMixer_GetVolumeMatrix(void *UserData) const;

	private:
		AudioHost_Interface * const m_pHost;
		aud_convert::ChannelMixer m_ChannelMixer;

		//These is only for debugging info
		//__int64 m_LastOutBufferSentTime;  //used to clock the sample rates
		//double m_OutSampleTime;

		//**** TBC Variables
		mutable FrameWork::Threads::critical_section m_AudioStreams_Lock;
		std::vector<Streams::AudioHostInputStream *>	m_AudioStreams;
};



//This class neatly wraps the AudioDevice interface around the AudioHost.

//This class inherits the device capabilities and also 
//wraps up the way to connect multiple devices by managing all the input streams created internally
// Note: The buffers that it fills will have its own timestamps... instead of mixing some composite time from upstream.

class DEVICES_AUDIO_BASE_API  AudioHostDevice :		public Streams::AudioOutStream_NotThreaded,
													public AudioHostCallback_Interface
{
	public:
		AudioHostDevice(AudioHost_Interface *AudioHost,const wchar_t *pDeviceName = L"AudioHost");
		virtual ~AudioHostDevice();

		//This returns the AudioHostInputStream that it creates internally
		//This pointer may be useful to extract a particular stream in the host device
		Streams::AudioHostInputStream *AddAudioDevice(Devices::Audio::DeviceInterface *pDevice,size_t DefaultTBCQueueDepth=_DS_::c_DefaultTBCQueueDepth);
		bool RemoveAudioDevice(DeviceInterface *pDeviceToRemove);

		//May return NULL if it can't find the stream
		//Gets the m_Mixer's volume matrix
		float *GetVolumeMatrix(DeviceInterface *AudioDevice) const;
		//Gets the m_Mixer's source number of channels
		size_t GetSourceNoChannels(DeviceInterface *AudioDevice) const;
		//Note use GetNoChannels for the other dimension for the volume matrix

		//This will call Delegated_FillBuffer and then attempt to fill any buffers available
		virtual bool FillBuffer(
			byte *Buffer,		//The buffer to fill
			size_t NoSamples,	//The number of samples to fill
			void *AudioInputStream_ptr=NULL //If this is not NULL no mixing will occur and it will fill this buffer with only one stream
			//Otherwise if this is NULL... the buffer will have all the streams mixed into it.
			);
	private:
		AudioHostMixer m_Mixer;
		//Manage our input streams internally when client adds audio devices
		std::vector<Streams::AudioHostInputStream *> m_InputStreams;
};

#undef _DS_