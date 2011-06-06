#pragma once

//This allows you to put in chunks of memory at a time and take a new custom size amount of memory, when there is enough 
//source memory which has accumulated
class DEVICES_AUDIO_BASE_API FloatingQueue 
{
	public:
		FloatingQueue();
		virtual ~FloatingQueue() 
			{	ResetQueue();
			}
		
		virtual void AddToQueue(byte *SampleSource,size_t SampleSizeInBytes);
		//This will fill your destination buffer with the new memory and pull it out from the queue
		//There must be enough memory stored for this to return successful... otherwise this function will
		//not take anything out and return false
		bool TakeFromQueue(byte *DestBuffer,size_t SizeInBytes);
		//This computes how many samples are currently stored in queue
		//returns m_CurrentSizeInBytes
		size_t GetSizeInBytes();

		//! use this to clean out any existing memory and reset the vars to use all over again
		virtual void ResetQueue();
	protected:
		//This gets re-computed per each Add and take function call
		size_t m_CurrentSizeInBytes;
		//This is the heart of this class which stores all the sample clips
		std::queue<byte *> m_AudioFloatingQueue_Memory;
		std::queue<size_t> m_AudioFloatingQueue_Size;
		// This is a critical section only used for locking buffers during flushing
		FrameWork::Threads::critical_section	m_FlushQueue_Lock;
	private:
		//Instead of resizing the memory we'll assign a splice offset (This applies to the destination list of samples)
		size_t m_SpliceOffset;
};

const size_t c_Default_FixedQueue_Depth=8; //This value may change...


class FixedQueue;
struct DEVICES_AUDIO_BASE_API FixedQueue_Buffer :	public BufferInterface
{
	FixedQueue_Buffer(FixedQueue *pParent,FormatInterface *pFormat,size_t NoSamples);
	virtual ~FixedQueue_Buffer();

	//implementation of the buffer interface
	virtual void *pGetBufferData(size_t *pMemorySize);
	virtual void ReleaseBufferData(const TimeStamp *pFrameTime , const wchar_t *pErrorMessage = NULL );
	virtual const TimeStamp& GetTimeStamp( void ) const;

	//implementation of the audio format interface
	//returns m_SampleRate
	virtual size_t GetSampleRate( void ) const;
	//returns m_NoChannels
	virtual size_t GetNoChannels( void ) const;
	//returns m_NoSamples
	size_t GetNoSamples( void ) const;
	virtual Devices::Audio::FormatInterface::eSampleFormat GetSampleFormat( void ) const;
	virtual bool GetIsBufferInterleaved( void ) const;

	TimeStamp m_TimeReleased;
	FixedQueue * const m_pParent;
	byte *m_Memory; //actual buffer

	//Note: we cache the buffer specs (should be constant)
	size_t m_SampleRate;
	size_t m_NoChannels;
	Devices::Audio::FormatInterface::eSampleFormat m_SampleFormat;
	bool m_bInterleaved;
	size_t m_BufferSize;
	size_t m_NoSamples; //cache this (it will be used often)
};

//This queue works with buffer interfaces, and keeps a fixed number of buffers of the same type and size... it will release old data if it can't
//keep up with the workload.  The current design assumes the AddToQueue and TakeFromQueue are from the same thread, and that the buffer may
//may be released from another thread.
class DEVICES_AUDIO_BASE_API FixedQueue
{
	public:
		FixedQueue(size_t QueueDepth=c_Default_FixedQueue_Depth);
		virtual ~FixedQueue();

		//This will perform a memcpy to its own set of internal buffers... (assuming the client has temporary memory)
		void AddToQueue(byte *Buffer,size_t NoSamples,FormatInterface *Format);
		//Be sure to call ReleaseBufferData() when buffer is finished!
		//Note: this will return NULL when the queue is empty
		//This only returns a buffer that matches the format and sample size
		FixedQueue_Buffer *TakeFromQueue(FormatInterface *Format,size_t NoSamples);
		void ResetQueue();

	private:
		friend FixedQueue_Buffer;
		void ReturnBufferToAvailableList(FixedQueue_Buffer *pBuffer , const wchar_t *pErrorMessage);

        size_t m_QueueDepth;
		std::queue<FixedQueue_Buffer *> m_ListToTake;
		std::queue<FixedQueue_Buffer *> m_ListToAdd;
		FrameWork::Threads::critical_section m_ListToAdd_Lock;
};
