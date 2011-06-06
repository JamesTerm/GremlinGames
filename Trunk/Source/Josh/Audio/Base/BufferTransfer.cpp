#include "Stdafx.h"
#include "Devices_Audio_Base.h"

using namespace FrameWork::Base;
using namespace Devices;
using namespace FrameWork::xml;
using namespace FrameWork::Communication;
using namespace FrameWork::Threads;
using namespace FrameWork::Audio;
using namespace Devices::Streams;

//This will enable some useful info if the queue's get congested
#undef __DumpQueueSizes__

namespace Devices
{	
	namespace Audio
	{
		namespace Base
		{
			namespace FramePipeTransfer
			{

  /***********************************************************************************************************/
 /*											AudioStreamBuffer_Base											*/
/***********************************************************************************************************/
const TimeStamp& AudioStreamBuffer_Base::GetTimeStamp( void ) const
{
	return m_TimeStamp;
}

size_t AudioStreamBuffer_Base::GetSampleRate( void ) const 
{	return m_SampleRate;
}
size_t AudioStreamBuffer_Base::GetNoChannels( void ) const 
{	return m_NoChannels;
}

Audio::FormatInterface::eSampleFormat AudioStreamBuffer_Base::GetSampleFormat( void ) const
{	return m_SampleFormat;
}
bool AudioStreamBuffer_Base::GetIsBufferInterleaved( void ) const
{	return m_bIsInterleaved;
}


  /***********************************************************************************************************/
 /*											AudioInputStreamBuffer											*/
/***********************************************************************************************************/

AudioInputStreamBuffer::AudioInputStreamBuffer(AudioBufferReceive *pParent) : m_pParent(pParent),m_Buffer(NULL)
{
}

AudioInputStreamBuffer::~AudioInputStreamBuffer()
{
}

void *AudioInputStreamBuffer::pGetBufferData( size_t *pMemorySize )
{
	assert(m_Buffer);
	if (pMemorySize)
		*pMemorySize=m_MemorySize;
	return m_pData;
}

void AudioInputStreamBuffer::ReleaseBufferData( const TimeStamp *pFrameTime, const wchar_t *pErrorMessage)
{

	//I most likely do not care about the frame time during the release, but I have included it in case the client wishes to add it
	if (pFrameTime)
		m_TimeStamp=*pFrameTime;

	//Note: it does not matter if I had an error or not... the device will just need to be flexable enough to handle the data given to it.
	//We simply give the buffer back to the process that is sending the frames

	//Give the format used, return the correct buffer type...
	switch (m_SampleFormat)
	{
		case Audio::BufferInterface::eSampleFormat_I_16:
			switch (m_NoChannels)
			{
				case 2:
					m_pParent->m_AudioReceive->return_buffer(*((aud_buf_stereo_s16 *)m_Buffer));
					break;
				case 8:
					m_pParent->m_AudioReceive->return_buffer(*((aud_buf_oct_s16 *)m_Buffer));
					break;
				default:
					assert(false);
					break;
			}
			break;
		case Audio::BufferInterface::eSampleFormat_FP_32:
			switch (m_NoChannels)
			{
				case 2:
					m_pParent->m_AudioReceive->return_buffer(*((aud_buf_stereo_f32 *)m_Buffer));
					break;
				case 4:
					m_pParent->m_AudioReceive->return_buffer(*((aud_buf_quad_f32 *)m_Buffer));
					break;
				case 8:
					m_pParent->m_AudioReceive->return_buffer(*((aud_buf_oct_f32 *)m_Buffer));
					break;
				default:
					assert(false);
					break;
			}
			break;
		default:
			assert(false);
			break;
	}

	m_Buffer=NULL;  //return_image deletes the image

	//Give my buffer container back to be reused
	m_pParent->AddBufferToDCQueue(this,pErrorMessage);
}

void AudioInputStreamBuffer::SetProperties(const void *buffer, const audio_input_stream_buffer_info &buffer_info)
{
	m_TimeStamp=buffer_info.m_time_stamp;

	m_bIsInterleaved=true;  //always true with these (can change later if necessary)
	const wchar_t *wsz_SampleType=buffer_info.m_SampleType.c_str();
	
	//Here is where we switch between various buffer types
	// Prioritize conditional string compares by commonality
	
	if (wcscmp(wsz_SampleType,L"aud_buf_oct_f32")==0)
	{
		m_NoChannels=8;
		m_SampleFormat=Audio::BufferInterface::eSampleFormat_FP_32;
		aud_buf_oct_f32 *buf=(aud_buf_oct_f32 *)buffer;
		m_pData=(*buf)();
		m_MemorySize=buf->size_in_bytes();
		m_SampleRate=buf->sample_rate();
	}
	else if (wcscmp(wsz_SampleType,L"aud_buf_quad_f32")==0)
	{
		m_NoChannels=4;
		m_SampleFormat=Audio::BufferInterface::eSampleFormat_FP_32;
		aud_buf_quad_f32 *buf=(aud_buf_quad_f32 *)buffer;
		m_pData=(*buf)();
		m_MemorySize=buf->size_in_bytes();
		m_SampleRate=buf->sample_rate();
	}
	else if (wcscmp(wsz_SampleType,L"aud_buf_stereo_f32")==0)
	{
		m_NoChannels=2;
		m_SampleFormat=Audio::BufferInterface::eSampleFormat_FP_32;
		aud_buf_stereo_f32 *buf=(aud_buf_stereo_f32 *)buffer;
		m_pData=(*buf)();
		m_MemorySize=buf->size_in_bytes();
		m_SampleRate=buf->sample_rate();
	}
	else if (wcscmp(wsz_SampleType,L"aud_buf_stereo_s16")==0)
	{
		m_NoChannels=2;
		m_SampleFormat=Audio::BufferInterface::eSampleFormat_I_16;
		aud_buf_stereo_s16 *buf=(aud_buf_stereo_s16 *)buffer;
		m_pData=(*buf)();
		m_MemorySize=buf->size_in_bytes();
		m_SampleRate=buf->sample_rate();
	}
	else if (wcscmp(wsz_SampleType,L"aud_buf_oct_s16")==0)
	{
		m_NoChannels=8;
		m_SampleFormat=Audio::BufferInterface::eSampleFormat_I_16;
		aud_buf_oct_s16 *buf=(aud_buf_oct_s16 *)buffer;
		m_pData=(*buf)();
		m_MemorySize=buf->size_in_bytes();
		m_SampleRate=buf->sample_rate();
	}
	else
		assert(false);

	m_Buffer=buffer;
}

  /***********************************************************************************************************/
 /*												AudioBufferReceive											*/
/***********************************************************************************************************/
const size_t c_NoBuffersToStart=5;
const size_t c_MaxQueueDepth=10;

bool AudioBufferReceive::AddBufferToDeviceQueue( AudioInputStreamBuffer *pBufferToAdd,const void *buffer, const audio_input_stream_buffer_info &buffer_info)
{
	if (!pBufferToAdd)
	{
		// Create a new buffer
		pBufferToAdd = new AudioInputStreamBuffer(this);
		m_BufferCount++;
		//TODO (maybe) Check for a failure to allocate the buffer
	}

	assert(pBufferToAdd);
	pBufferToAdd->SetProperties(buffer,buffer_info);

	// Add it to the input device
	return m_pDevice->AddBufferToQueue( pBufferToAdd );
}


void AudioBufferReceive::FinishQueue()
{
	// Lock the queue and internally mark us as being in flushing mode
	m_Queue_Lock.lock();
	m_FlushingQueue = true;
	m_Queue_Lock.unlock();

	// FLush the device of all buffers
	m_pDevice->FlushAllBuffers();

	// No longer flushing
	m_FlushingQueue = false;
}

AudioBufferReceive::AudioBufferReceive(DeviceInterface *pDevice, const wchar_t source_name[],FC3_Bridge *pParent) :
	m_AudioReceive(new audio_input_stream(this,source_name,pParent)),m_pDevice(pDevice),m_BufferCount(0),m_FlushingQueue(false)
{
	assert (pDevice);
}

AudioBufferReceive::~AudioBufferReceive()
{
	m_AudioReceive.reset();  //ensure this gets delete before any of the queue variables
}

node &AudioBufferReceive::GetXMLNode()
{
	return m_AudioReceive->get_send_buffer_parser();
}


void AudioBufferReceive::SelectSenderToProcess(const wchar_t SenderToProcess[])
{
	m_senderSelectionName=SenderToProcess;
}

void AudioBufferReceive::audio_input_stream_receive_buffer_from_input(const void *buffer, const audio_input_stream_buffer_info &buffer_info)
{
	//Check if we are filtering for a certain sender, if so... other senders will return the frames back immediately
	if ((m_senderSelectionName[0])&&(m_senderSelectionName!=buffer_info.m_sender))
	{
		// Prioritize conditional string compares by commonality
		if (wcscmp(buffer_info.m_SampleType.c_str(),L"aud_buf_oct_f32")==0)
			m_AudioReceive->return_buffer(*((aud_buf_oct_f32 *)buffer));
		else if (wcscmp(buffer_info.m_SampleType.c_str(),L"aud_buf_quad_f32")==0)
			m_AudioReceive->return_buffer(*((aud_buf_quad_f32 *)buffer));
		else if (wcscmp(buffer_info.m_SampleType.c_str(),L"aud_buf_stereo_f32")==0)
			m_AudioReceive->return_buffer(*((aud_buf_stereo_f32 *)buffer));
		else if (wcscmp(buffer_info.m_SampleType.c_str(),L"aud_buf_stereo_s16")==0)
			m_AudioReceive->return_buffer(*((aud_buf_stereo_s16 *)buffer));
		else if (wcscmp(buffer_info.m_SampleType.c_str(),L"aud_buf_oct_s16")==0)
			m_AudioReceive->return_buffer(*((aud_buf_oct_s16 *)buffer));
		else
			assert(false);

		return;
	}

	m_Queue_Lock.lock();

	// For now, we always look and smell like a full frame-rate device although
	// we might change that in the future.

	// If there is more than one field in the output queue, then we 
	// move on to the next frame
	if (m_Queue.size()>0)
	{	// Add this field back for filling in
		AddBufferToDeviceQueue (m_Queue.front(),buffer,buffer_info);

		// Remove it from the  queue
		m_Queue.pop();
	}
	else
	{	

		//This is handy in the case where the  queue may be too lean and may need to have more buffers
		if (m_BufferCount<c_MaxQueueDepth)
		{
			AddBufferToDeviceQueue (NULL,buffer,buffer_info);
			// Debugging
			DebugOutput(L"AudioBufferReceive::Advance Insert Frame %p\n",this);
			printf("AudioBufferReceive::Advance Insert Frame\n",this);
		}
	}

	// Unlock the  queue
	m_Queue_Lock.unlock();
}

void AudioBufferReceive::AddBufferToDCQueue( AudioInputStreamBuffer *pBuffer , const wchar_t *pErrorMessage )
{
	// Lock the TBC queue while this is added
	auto_lock LockQueue( &m_Queue_Lock );

	//DebugOutput(L"AddBufferToTBCList %p %d\n",this,m_Queue.size());
	// If the queue is being flushed, then we just return immediately (deleting these buffers!)
	if ( m_FlushingQueue )
	{
		delete pBuffer;
		m_BufferCount--;
		return;
	}

	//Note in this version I do not care about error messages... the buffers will get completely written when they get recycled

	if ((int)m_Queue.size()>c_MaxQueueDepth)
	{	
		//This should not occur unless someone has added some buffers externally and did not check the threshold
		//Note: there is currently no public access to this member variable, but there is access to the  list
		assert(false);
		// Debugging
		DebugOutput(L" : Dropped packet\n");
		printf(" : Dropped packet\n");

		delete pBuffer;
		m_BufferCount--;
		//AddBufferToDeviceQueue( pBuffer );
	}
	else
	{
		m_Queue.push(pBuffer);
		#ifdef __DumpQueueSizes__
		DebugOutput(L"+++ AudioBufferReceive-%s queue=%d\n",m_AudioReceive->get_source_name(),m_Queue.size());
		#endif
	}

}

void AudioBufferReceive::audio_input_stream_flush_all_images( void )
{
	FinishQueue();
}

			}
		}
	}
}
