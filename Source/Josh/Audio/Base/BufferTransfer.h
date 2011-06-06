#pragma once

///This is some general purpose grunt work that works for both VideoOutputStreamBuffer and VideoInputStreamBuffer
class DEVICES_AUDIO_BASE_API AudioStreamBuffer_Base : public BufferInterface
{
	protected: //from BufferInterface
		virtual const TimeStamp& GetTimeStamp( void ) const;

	protected: //From Audio Format Interface
		virtual size_t GetSampleRate( void ) const;
		virtual size_t GetNoChannels( void ) const;
		virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;
		virtual bool GetIsBufferInterleaved( void ) const;

	protected:
		TimeStamp m_TimeStamp;

		//Note: we cache the buffer specs (should be constant)
		size_t m_SampleRate;
		size_t m_NoChannels;
		FormatInterface::eSampleFormat m_SampleFormat;
		bool m_bIsInterleaved;
};


//--------------------------------------------Receiving Side

class AudioBufferReceive;
/// These are special kind of buffers that are not allocated, but instead accesses the memory map to obtain the pointer of the buffers
///which were already created (and filled in) from the send side
class DEVICES_AUDIO_BASE_API AudioInputStreamBuffer : public AudioStreamBuffer_Base
{
	public:
		AudioInputStreamBuffer(AudioBufferReceive *pParent);
		virtual ~AudioInputStreamBuffer();
		void SetProperties(const void *buffer, const FrameWork::Communication::audio_input_stream_buffer_info &buffer_info);
	protected: //from BufferInterface
		virtual void *pGetBufferData( size_t *pMemorySize );
		virtual void ReleaseBufferData( const TimeStamp *pFrameTime = NULL , const wchar_t *pErrorMessage = NULL );
	private:
		AudioBufferReceive * const m_pParent;
		const void *m_Buffer;

		//cache these to make life easy on pGetBufferData
		size_t m_MemorySize;  
		void *m_pData;
};

///This is a special kind of device client that works in reverse from our regular convention.  Like other device clients it owns the buffer; however
///the device client relationship is different in that the buffers are already filled with valid audio buffers.  So the device will not fill in the
///buffers, but rather process the data, and return them back (just like how devices work in that regard).  Since this is a reverse kind of 
///convention it is great in that we need not have any memory copies, but the trade off is that we will need to memcpy for Device Clients that we 
///want to connect to such as AudioDeviceClientToDevice.  In that case use AudioDeviceClientToDevice to connect to these
class DEVICES_AUDIO_BASE_API AudioBufferReceive : public FrameWork::Communication::audio_input_stream_callback_interface
{
	public:
		AudioBufferReceive(DeviceInterface *pDevice,		///< The name of the device which processes the already received filled buffers
			const wchar_t source_name[],						///< This would typically be the name of the parent application
			FrameWork::Communication::FC3_Bridge *pParent   ///< This is needed for fc3 integration
			);

		~AudioBufferReceive();
		///The client can choose what sender to process buffer from (i.e. mixer choosing an audio source)
		/// \param SenderToProcess Enter the name of one sender or empty string (default) to listen to any incoming frames (ideal for testing case).
		/// \note under the audio device mixing paradigm... each device may be committed to a certain "channel" and then the devices can be added
		/// individually to the audio host where they get mixed
		void SelectSenderToProcess(const wchar_t SenderToProcess[]=L"");

		///You'll want your application to use this node for all "SendFrame" messages
		FrameWork::xml::node &GetXMLNode();

	protected: //from audio input stream callback interface
		virtual void audio_input_stream_receive_buffer_from_input(const void *buffer, const FrameWork::Communication::audio_input_stream_buffer_info &buffer_info);
		virtual void audio_input_stream_flush_all_images( void );

		bool AddBufferToDeviceQueue( AudioInputStreamBuffer *pBufferToAdd,const void *buffer, const FrameWork::Communication::audio_input_stream_buffer_info &buffer_info);
		void FinishQueue();

	private:
		friend AudioInputStreamBuffer;
		void AddBufferToDCQueue( AudioInputStreamBuffer *pBuffer , const wchar_t *pErrorMessage );

		boost::scoped_ptr<FrameWork::Communication::audio_input_stream> m_AudioReceive; ///< This is a scoped pointer to ensure it gets deleted first
		DeviceInterface * const m_pDevice;

		///We manage our own queue here since this is a reverse convention of how devices work, and no base code has been written to work this way
		FrameWork::Threads::critical_section m_Queue_Lock;
		std::queue<AudioInputStreamBuffer *> m_Queue;

		///<This is used to filter out unwanted "channels" from other senders
		std::wstring m_senderSelectionName;
		size_t m_BufferCount; ///<used to manage how many buffers to add in an insert situation
		bool m_FlushingQueue;
};
