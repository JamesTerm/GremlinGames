#pragma once

//This structure is used to append additional stream information 
struct AudioHostStreamData
{
	//Simply pass the AudioInputStream pointer to here when AudioHost_CreateNewStream is called
	//This will help the AudioHostMixer to find the stream immediately
	void *AudioInputStream_ptr;
	//You can make your own data in here when implementing AudioHost_CreateNewStream
	void *AudioHostData;
};

class DEVICES_AUDIO_DIRECTSOUND_API DirectSoundHost_Internal
{
	public:
		DirectSoundHost_Internal();
		virtual ~DirectSoundHost_Internal();
		
		//Throws exception if DirectShow fails
		void StartUp(HWND ParentWindow);

		void AudioHost_CreateNewStream(void *AudioInputStream_ptr);

		void AudioHost_RemoveStream(void *AudioInputStream_ToRemove);

		//returns m_pAudioHostCallback
		Devices::Audio::Base::Host::AudioHostCallback_Interface *GetAudioHostCallback_Interface( void );
		//sets  m_pAudioHostCallback
		void SetAudioHostCallback_Interface(Devices::Audio::Base::Host::AudioHostCallback_Interface *ahi);

		//returns m_FormatToUse.nSamplesPerSec
		size_t GetSampleRate( void ) const;

		Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;

		//returns m_FormatToUse.nChannels
		size_t GetNoChannels( void ) const;

		//Sample size Must be a power of 2 (so it can do some anding techniques)
		//1024 is too small for the notifications to be reliable
		//We want this small enough to minimize in latency
		size_t GetBufferSampleSize( void ) const;

		//returns m_FormatToUse
		WAVEFORMATEX *GetWaveFormatToUse( void );
		//returns m_lpDS
		LPDIRECTSOUND8 GetDirectSoundObject( void );

		//provide access to the current cursor's position
		//Note: This is a DirectSound specific call that really shouldn't be used... This could be temporary and may be replaced
		//with a clock interface so please don't use this unless you really need to.   - James
		double GetTimeElapsed(void *AudioInputStream_ptr);
	protected:
		//Note: we want to be able to dynamically change this
		Devices::Audio::Base::Host::AudioHostCallback_Interface * m_pAudioHostCallback;

	private:
		// This is the thread callback
		//virtual bool ThreadCallback( void );
		void operator() ( const void* );

		FrameWork::Threads::thread<DirectSoundHost_Internal>	*m_pThread;
		friend FrameWork::Threads::thread<DirectSoundHost_Internal>;

		void SetWaveFormat(WAVEFORMATEX &wfx,WORD wFormatTag,WORD nChannels,DWORD nSamplesPerSecond,WORD wBitsPerSample);
		void ShutDown(); //called by destructor

		class AudioStreamData
		{
			public:
				//Throws exception if it fails
				AudioStreamData(DirectSoundHost_Internal *pParent,void *AudioInputStreamHandle);
				virtual ~AudioStreamData();

				void FillBuffer();

				//provide access to the current cursor's position
				//Note: This is a DirectSound specific call that really shouldn't be used... This could be temporary and may be replaced
				//with a clock interface so please don't use this unless you really need to.   - James
				double GetTimeElapsed();

			private:
				DirectSoundHost_Internal * const m_pParent;
				//Going to need this to let the callback know which stream I want
				void * const m_AudioInputStreamHandle;

				LPDIRECTSOUNDBUFFER m_lpdsb;
				DWORD m_BufferSizeInBytes;
				DWORD m_FillPosition; //keeps track of where the buffer has been filled

				//This is for GetTimeElapsed
				long m_GTE_LastCalled;
		};

		LPDIRECTSOUND8 m_lpDS;
		FrameWork::Threads::critical_section	m_BlockDirectSound;

		WAVEFORMATEX m_FormatToUse;
		HANDLE m_NotificationTimer;			// high precision timer to update the buffer streaming

		#pragma warning (disable : 4251)
		//**** TBC Variables
		FrameWork::Threads::critical_section						m_AudioStreams_Lock;
		std::vector<AudioHostStreamData*>	m_AudioStreams;
		#pragma warning (default : 4251)

		//This safely unwinds the thread (used mainly for destructor)
		//Not yet needed//bool m_AbortThread;
};

class DEVICES_AUDIO_DIRECTSOUND_API DirectSoundHost :	public DirectSoundHost_Internal,
														public Devices::Audio::Base::Host::AudioHost_Interface
{
	public:
		DirectSoundHost(){}
		virtual ~DirectSoundHost() {}

		//wraps to internal... this usually will connect to something like the Audio Host Mixer
		virtual void SetAudioHostCallback_Interface(Devices::Audio::Base::Host::AudioHostCallback_Interface *ahi);
		//returns m_pAudioHostCallback
		Devices::Audio::Base::Host::AudioHostCallback_Interface *GetAudioHostCallback();

		//provide access to the current cursor's position
		//Note: This is a DirectSound specific call that really shouldn't be used... This could be temporary and may be replaced
		//with a clock interface so please don't use this unless you really need to.   - James
		double GetTimeElapsed(void *AudioInputStream_ptr);
	private:
		//sets the audio host callback interface to null
		virtual void AudioInStreamClosing(void *ThisChild);

		//Implementation of the audio format interface
		
		//wraps to internal
		virtual size_t GetSampleRate( void ) const;
		//wraps to internal
		virtual eSampleFormat GetSampleFormat( void ) const;
		//returns true... direct show is always interleaved
		virtual bool GetIsBufferInterleaved( void ) const;

		//wraps to internal
		virtual size_t GetNoChannels( void ) const; 

		//Implementation from Host Interface

		//wraps to internal
		virtual size_t GetBufferSampleSize();

		//wraps to internal
		virtual void AudioHost_CreateNewStream(void *AudioInputStream_ptr);
		//wraps to internal
		virtual void AudioHost_RemoveStream(void *AudioInputStream_ToRemove); 
};

class DEVICES_AUDIO_DIRECTSOUND_API DirectSoundOutputDevice :	public Devices::Audio::Base::DeviceWrapper,
																public Devices::Audio::Base::AudioHostDeviceWrapper
{
	public:
		//Unfortunately you must provide an hwnd to use Direct Sound :(
		DirectSoundOutputDevice(HWND hwnd,const wchar_t *pDeviceName = L"DirectSoundOutput");

		//provide access to the current cursor's position
		//Note: This is a DirectSound specific call that really shouldn't be used... This could be temporary and may be replaced
		//with a clock interface so please don't use this unless you really need to.   - James
		double GetTimeElapsed(void *AudioInputStream_ptr);
	private:
		DirectSoundHost m_AudioHost;
		Devices::Audio::Base::Host::AudioHostDevice m_AudioHostDevice;
};
