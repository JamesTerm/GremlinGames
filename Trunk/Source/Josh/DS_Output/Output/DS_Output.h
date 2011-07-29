#pragma once

const size_t DS_Output_BufferParam = 2;	//not entirely sure what this does, but it's required as a parameter for creating a queue and it has something to do with latency

//TODO Move to input
class OUTPUT_API DS_Input
{
	public:
		DS_Input(int AudioInput=0,const wchar_t AudioName[]=L"DS_Input_Audio");
		~DS_Input();

		/// You may add additional destinations to send your frames.  Let me know if we need to add a remove method.
		/// \param destination_name This is the name used in the xml destination tag
		/// \note to maintain backward compatibility this name will be added to both video and audio destinations.
		virtual void add_dest(const wchar_t destination_name[]) {}

		//TODO put the format as second parameter (e.g. samplerate)
		bool ChangeAudioFormat(size_t AudioInput) {return true;}

		size_t GetAudioInputChannel() const {return 0;}
		void SetUseDCFilter(bool UseDCFilter) {}

		virtual void StartStreaming( void ) {}
		virtual void StopStreaming( void ) {}
	private:
};

typedef FrameWork::Communication3::audio::receive::client AudioReceiveClient; 
typedef const FC3::audio::message AudioMsg;

//import these first maybe?  i uno
typedef FrameWork::Audio2::buffer_s32 buffer_s32;
typedef FrameWork::Audio2::buffer_f32 buffer_f32;
typedef FrameWork::Audio2::utils::resampler resampler;
typedef std::pair<HD::Base::StreamType,size_t> ChannelInfoType;


class DS_Output_Internal;
/// Direct Sound output
class OUTPUT_API DS_Output : public AudioReceiveClient
{
	public:
		DS_Output(size_t AudioOutput=0,const wchar_t AudioName[]=L"DS_Output_Audio");
		~DS_Output();

		//TODO see if you want to throw... this depends on how recoverable it is 
		/// \param AudioOutput  Throws exception if out of bounds
		void SetAudioOutputChannel(size_t AudioOutput);
		size_t GetAudioOutputChannel() const;

		/// \param AudioOutput  Throws exception if out of bounds
		void AddOutputChannel(size_t AudioOutput);
		/// You may add additional destinations to send your frames.  Let me know if we need to add a remove method.
		virtual void add_thru_dest(const wchar_t *audio_destination_name);

		virtual void StartStreaming( void );
		virtual void StopStreaming( void );

		void SetUseDisplayedCallbacks(bool UseDisplayedCallbacks) {m_UseDisplayedCallbacks=UseDisplayedCallbacks;}
		WAVEFORMATEX *GetWaveFormatToUse( void );

	protected: //from comm3 receive client
		virtual void deliver_audio(	const FC3::audio::message* p_aud_msg );
		void StartServer();

		//These methods are separated out for backward compatibility
		virtual FC3::audio::message *AllocateFrame_audio( const int no_samples, const int no_channels);

		bool m_IsStreaming;


	private:
		//the file server, we use scoped pointer to delay instantiation until m_DS_Output_Internal has been initialized
		SCOPED_PTR<FrameWork::Communication3::audio::receive> m_Server_Audio;

		//Cached since this is only managed on construction
		size_t m_QueueDepthLevel;
		std::wstring m_AudioName;
		bool m_ServerStarted; //Clearly mark when the server has been started
		bool m_UseDisplayedCallbacks;

		size_t m_AudioOutput;
		//size_t m_CurrentFieldIndex_audio; // defined instead in DS_Output_Internal
		DS_Output_Internal * const m_DS; //encapsulate SDK specifics from public
		//HD::Base::AudioScope m_AudioScope;

		struct DestThru
		{
			std::wstring Audio;
		};

		std::vector<DestThru> m_Dest_Thru;

		//double m_AudioSampleRate;
		//std::queue <const AudioMsg *> m_IncomingBufferQueueFrame_audio;
		//FrameWork::Threads::critical_section m_BlockIncomingBufferQueue_audio;
};

class DirectSound_Globals;

class OUTPUT_API DirectSound_Initializer
{
public:
	DirectSound_Initializer() {m_DirectSoundInitStatus=StartUp_DirectSound();}
	~DirectSound_Initializer(){ ShutDown_DirectSound(); }
	operator bool (void) const {return m_DirectSoundInitStatus;}
	bool GetDirectSoundInitStatus() {return m_DirectSoundInitStatus;}
protected:
private:
	bool StartUp_DirectSound();
	void ShutDown_DirectSound();
	bool m_DirectSoundInitStatus;
	static long s_ref;
};
