#include "stdafx.h"

// Include the main project
#include "Devices_Audio_Base.h"


using namespace std;
using namespace FrameWork::Base;
using namespace FrameWork::Threads;
using namespace Devices;
using namespace Devices::Audio;
using namespace Devices::Audio::Base::aud_convert;
using namespace Devices::Audio::Base::Streams;


  /*******************************************************************************************************/
 /*												FloatingQueue											*/
/*******************************************************************************************************/


FloatingQueue::FloatingQueue()
{
	m_CurrentSizeInBytes=0;
	m_SpliceOffset=0;
}


size_t FloatingQueue::GetSizeInBytes() 
{	return m_CurrentSizeInBytes;
}


void FloatingQueue::ResetQueue()
{
	auto_lock FunctionBlock(m_FlushQueue_Lock);
	while (!(m_AudioFloatingQueue_Memory.empty()))
	{
		byte *buffer=(byte *)m_AudioFloatingQueue_Memory.front();
		_aligned_free(buffer);
		m_AudioFloatingQueue_Memory.pop();
	}

	while (!(m_AudioFloatingQueue_Size.empty()))
		m_AudioFloatingQueue_Size.pop();

	m_CurrentSizeInBytes=0;
	m_SpliceOffset=0;
}

void FloatingQueue::AddToQueue(byte *SampleSource,size_t SampleSizeInBytes)
{
	auto_lock FunctionBlock(m_FlushQueue_Lock);
	byte *NewSampleBuffer=(byte *)_aligned_malloc(SampleSizeInBytes,16);
	memcpy(NewSampleBuffer,SampleSource,SampleSizeInBytes);

	m_AudioFloatingQueue_Memory.push(NewSampleBuffer);
	m_AudioFloatingQueue_Size.push(SampleSizeInBytes);
	m_CurrentSizeInBytes+=SampleSizeInBytes;
}

bool FloatingQueue::TakeFromQueue(byte *DestBuffer,size_t SizeInBytes)
{
	auto_lock FunctionBlock(m_FlushQueue_Lock);
	if (SizeInBytes>m_CurrentSizeInBytes)
		return false;
	size_t NumberOfSamplesLeft=SizeInBytes;
	size_t DestIndex=0;
	size_t SpliceOffset,PrevSpliceOffset=m_SpliceOffset;

	while (NumberOfSamplesLeft)
	{
		byte *memory=(byte *)m_AudioFloatingQueue_Memory.front();
		size_t size=(size_t)m_AudioFloatingQueue_Size.front()-PrevSpliceOffset;
		size_t NumberOfSamplesToCopy=min(size,NumberOfSamplesLeft);

		//DestBuffer may be NULL... in which case we still want
		//to go through the motions of a successful transfer to empty the queue
		if (DestBuffer) 
			memcpy (DestBuffer+DestIndex,memory+PrevSpliceOffset,NumberOfSamplesToCopy);
		SpliceOffset=NumberOfSamplesToCopy;
		if (PrevSpliceOffset+NumberOfSamplesToCopy>=(size_t)m_AudioFloatingQueue_Size.front())
		{
			//Now we can free this memory
			_aligned_free(memory);
			m_AudioFloatingQueue_Memory.pop();
			m_AudioFloatingQueue_Size.pop();
			m_SpliceOffset=SpliceOffset=0;
		}
		DestIndex+=NumberOfSamplesToCopy;
		NumberOfSamplesLeft-=NumberOfSamplesToCopy;
		PrevSpliceOffset=0;
	}
	m_SpliceOffset+=SpliceOffset;
	m_CurrentSizeInBytes-=SizeInBytes;
	//DebugOutput("TakeFromQueue needed->%d LeftOver->%d\n",SizeInBytes>>3,m_CurrentSizeInBytes>>3);
	return true;
}


  /*******************************************************************************************************/
 /*											FixedQueue_Buffer											*/
/*******************************************************************************************************/


FixedQueue_Buffer::FixedQueue_Buffer(FixedQueue *pParent,FormatInterface *pFormat,size_t NoSamples) : m_pParent(pParent)
{	
	//lets get the buffer format
	m_NoChannels=pFormat->GetNoChannels();
	m_SampleRate=pFormat->GetSampleRate();
	m_bInterleaved=pFormat->GetIsBufferInterleaved();
	m_SampleFormat=pFormat->GetSampleFormat();  //No longer supported
	size_t BytesPerSample=GetFormatBitDepth(m_SampleFormat)*m_NoChannels;
	//size_t BytesPerSample=sizeof(float)*m_NoChannels;	//we can assume float now

	m_NoSamples=NoSamples;
	assert(m_NoSamples); //ensure my parent can safely obtain this information
	m_BufferSize=m_NoSamples*BytesPerSample;
	m_Memory=(byte *)_aligned_malloc(m_BufferSize,16);
}

FixedQueue_Buffer::~FixedQueue_Buffer() 
{
	_aligned_free(m_Memory);
}

size_t FixedQueue_Buffer::GetSampleRate( void ) const 
{	return m_SampleRate;
}

size_t FixedQueue_Buffer::GetNoChannels( void ) const 
{	return m_NoChannels;
}

size_t FixedQueue_Buffer::GetNoSamples( void ) const 
{	return m_NoSamples;
}

Devices::Audio::FormatInterface::eSampleFormat FixedQueue_Buffer::GetSampleFormat( void ) const
{
	return m_SampleFormat;
}

bool FixedQueue_Buffer::GetIsBufferInterleaved( void ) const
{
	return m_bInterleaved;
}


void *FixedQueue_Buffer::pGetBufferData( size_t *pMemorySize ) 
{
	if (pMemorySize)
		*pMemorySize=m_BufferSize;
	return m_Memory;
}

void FixedQueue_Buffer::ReleaseBufferData(const TimeStamp *pFrameTime, const wchar_t *pErrorMessage) 
{	assert(pFrameTime);
	if (pFrameTime)
		m_TimeReleased=*pFrameTime;
	m_pParent->ReturnBufferToAvailableList( this , pErrorMessage );
	//if (pErrorMessage)
	//	printf("**Buffer Error-%s\n",pErrorMessage);
}

const TimeStamp& FixedQueue_Buffer::GetTimeStamp( void ) const
{
	return m_TimeReleased;
}

  /*******************************************************************************************************/
 /*													FixedQueue											*/
/*******************************************************************************************************/

FixedQueue::FixedQueue(size_t QueueDepth)
{
	m_QueueDepth=QueueDepth;
}

FixedQueue::~FixedQueue()
{
	ResetQueue();
}

void FixedQueue::AddToQueue(byte *Buffer,size_t NoSamples,FormatInterface *Format)
{
	//DebugOutput(L"Add=%d ",m_ListToAdd.size());

	FixedQueue_Buffer *BufferToUse;

	m_ListToAdd_Lock.lock();

	if (!(m_ListToAdd.empty()))
	{
		//Get it
		BufferToUse=(FixedQueue_Buffer *)m_ListToAdd.front();
		m_ListToAdd.pop();
	}
	else  //no empty buffers available
	{
		//Note while it is true that there may be one or two buffers being used... it is not critical to count them for audio buffers
		//determine whether to add new buffers or take the oldest from my list to take
		if (m_ListToTake.size()<m_QueueDepth)
			BufferToUse=new FixedQueue_Buffer(this,Format,NoSamples);
		else
		{
			//DebugOutput(L"FixedQueue::AddToQueue Adding more than taking\n");
			BufferToUse=(FixedQueue_Buffer *)m_ListToTake.front();
			m_ListToTake.pop();
		}
	}

	m_ListToAdd_Lock.unlock();

	//check format
	if ((*BufferToUse!=*Format)||(BufferToUse->GetNoSamples()!=NoSamples))
	{
		//fix it up
		delete BufferToUse;
		BufferToUse=new FixedQueue_Buffer(this,Format,NoSamples);
	}

	size_t SizeInBytes;
	byte *DestBuffer=(byte *)BufferToUse->pGetBufferData(&SizeInBytes);
	memcpy(DestBuffer,Buffer,SizeInBytes);
	m_ListToTake.push(BufferToUse);
}


FixedQueue_Buffer *FixedQueue::TakeFromQueue(FormatInterface *Format,size_t NoSamples)
{
	FixedQueue_Buffer *BufferToUse=NULL;
	//straight forward...
	//DebugOutput(L"Add=%d ",m_ListToAdd.size());
	//DebugOutput(L"Take=%d\n",m_ListToTake.size());
	do
	{
		if (!(m_ListToTake.empty()))
		{
			BufferToUse=(FixedQueue_Buffer *)m_ListToTake.front();
			m_ListToTake.pop();

			//check format
			if ((*BufferToUse!=*Format)||(BufferToUse->GetNoSamples()!=NoSamples))
			{
				delete BufferToUse;
				BufferToUse=NULL;
			}
		}
		else 
			break;
	} while(!BufferToUse);

	return BufferToUse;
}

void FixedQueue::ResetQueue()
{
	while (!(m_ListToTake.empty()))
	{
		FixedQueue_Buffer *buffer=(FixedQueue_Buffer *)m_ListToTake.front();
		delete buffer;
		m_ListToTake.pop();
	}

	m_ListToAdd_Lock.lock();
	while (!(m_ListToAdd.empty()))
	{
		FixedQueue_Buffer *buffer=(FixedQueue_Buffer *)m_ListToAdd.front();
		delete buffer;
		m_ListToAdd.pop();
	}
	m_ListToAdd_Lock.unlock();
}

void FixedQueue::ReturnBufferToAvailableList(FixedQueue_Buffer *pBuffer , const wchar_t *pErrorMessage)
{
	m_ListToAdd_Lock.lock();
	m_ListToAdd.push(pBuffer);
	m_ListToAdd_Lock.unlock();
	//DebugOutput(L"returned=%d ",m_ListToAdd.size());
}
