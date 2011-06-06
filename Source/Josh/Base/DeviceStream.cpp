#include "stdafx.h"

// Include the main project
#include "Devices_Base.h"
//This will enable some useful info if the queue's get congested
#undef __DumpQueueSizes__

//for debug test purposes only
#undef	__WriteWavBeforeConversion__
#undef	__WriteWavAfterConversion__
#undef  __WriteWavAddToTBCList__

#if	(defined __WriteWavBeforeConversion__ || defined __WriteWavAfterConversion__ || defined __WriteWavAddToTBCList__)
#include "../../../../User/James Killian/AudioTesting/Wave/Wave.h"
#endif

using namespace std;
using namespace FrameWork::Base;
using namespace FrameWork::Threads;
using namespace Devices;
using namespace Devices::Streams;

#define _DS_ Devices::Streams

  /***************************************************************************************************/
 /*											OutputStream											*/
/***************************************************************************************************/

OutputStream::OutputStream (OutStream_Interface *pParent) :
m_pParent(pParent)
{
	m_AbortThread=false;

	assert(pParent);

	// Start the thread that processes queue frames
	m_pThread = new thread<OutputStream>(this);

}

OutputStream::~OutputStream()
{
	// Stop the thread that processes queue frames
	m_AbortThread = true;
	m_QueueActivitySignal.set();

	delete m_pThread;

	// We free any buffers that might be on the list
	m_ListOfBuffers_Lock.lock();

	TimeStamp TimeReleased;
	QueryPerformanceCounter((LARGE_INTEGER *)TimeReleased);

	/*
	// Release all frames
	for( size_t i=0 ; i<m_ListOfBuffers.size() ; i++ )
		// Release the frame data with an error
		m_ListOfBuffers[i]->ReleaseBufferData(TimeReleased, L"Device is being deleted." );

	// Delete all frames on the list
	m_ListOfBuffers.clear();
	*/
	while (!(m_ListOfBuffers.empty()))
	{
		BufferInterface *Val=(BufferInterface *)m_ListOfBuffers.front();
		if (Val)
			Val->ReleaseBufferData(&TimeReleased, L"Device is being deleted." );
		m_ListOfBuffers.pop();
	}

	m_ListOfBuffers_Lock.unlock();
}


const wchar_t *OutputStream::pGetDeviceName( void ) const 
{	
	return m_pParent->pGetDeviceName();
}

const bool OutputStream::GetAbortThread( void ) const 
{	
	return m_AbortThread; 
}



//From Device Interface
bool OutputStream::AddBufferToQueue( Devices::BufferInterface *pBuffer )
{
	//let parent know of this notification (before taking any action)
	bool success=m_pParent->AddBufferToQueue(pBuffer);

	BufferInterface	*pBI = dynamic_cast<BufferInterface*>( pBuffer );
	if (!pBI) return false;

	// Lock the list of buffers
	m_ListOfBuffers_Lock.lock();
	//m_ListOfBuffers.insert( m_ListOfBuffers.end() , pAudBI );
	m_ListOfBuffers.push( pBI );
	m_ListOfBuffers_Lock.unlock();

	//Signal the change (only if the parent is happy)
	if (success) m_QueueActivitySignal.set();

	// Success
	return true;
}

void OutputStream::FlushAllBuffers( void )
{
	// We cannot flush the queue at this point !
	auto_lock FlushingLock( &m_FlushQueue_Lock );
	auto_lock QueueLock( &m_ListOfBuffers_Lock );

	TimeStamp TimeReleased;
	QueryPerformanceCounter((LARGE_INTEGER *)TimeReleased);

	// Flush the queue
	/*
	for( size_t i=0 ; i<m_ListOfBuffers.size() ; i++ )
		m_ListOfBuffers[i]->ReleaseBufferData(TimeReleased, L"Flush Queue." );

	m_ListOfBuffers.clear();
	*/
	while (!(m_ListOfBuffers.empty()))
	{
		BufferInterface *Val=(BufferInterface *)m_ListOfBuffers.front();
		if (Val)
			Val->ReleaseBufferData(&TimeReleased, L"Flush Queue." );
		m_ListOfBuffers.pop();
	}

	m_AbortThread=true;
}

void OutputStream::operator() ( const void* )
{
	bool RenderedBuffer=false;

	// We get a frame off the queue
	BufferInterface *pBufferToFill = NULL;

	//halt this thread (active output will halt to its signaling mechanism, passive output may return immediately)
	//I put this up here so that the thread does not do anything until the Device client has something to stream
	m_pParent->WaitForNextBuffer();

	//TODO- This works fine as-is for a passive output stream; however for an active stream this may indeed need to increase
	//the queue depth.  At the moment this can work for now as long as the InputStream has made enough buffers.

	while ((!pBufferToFill)&&(!m_AbortThread))
	{	
		m_FlushQueue_Lock.lock();
		// Lock the list
		m_ListOfBuffers_Lock.lock();

		//DebugOutput(L"TBC OutStream Depth %d\n",m_ListOfBuffers.size());

		// If there are buffers, then get one
		if (m_ListOfBuffers.size())
		{	
			pBufferToFill = (BufferInterface *)m_ListOfBuffers.front();
			m_ListOfBuffers.pop();
		}
		// Unlock the list
		m_ListOfBuffers_Lock.unlock();
		m_FlushQueue_Lock.unlock();
		if (pBufferToFill) 
			break;
		m_QueueActivitySignal.wait(1000); //This could be infinite, but no harm to set for 1000
	}

	m_FlushQueue_Lock.lock();

	if ((pBufferToFill) && (!m_AbortThread))
	{	// Fill in the buffer ...
		TimeStamp TimeReleased;
		QueryPerformanceCounter((LARGE_INTEGER *)TimeReleased);

		// Check the buffer
		BufferInterface *pBI = dynamic_cast<BufferInterface*>( pBufferToFill );

		// No implementation ?
		if (!pBI)
		{	
			pBufferToFill->pGetBufferData(NULL);
			pBufferToFill->ReleaseBufferData(&TimeReleased, L"No BufferInterface found for source." );
			goto ExitBufferFill;
		}

		// We check that the format is correct
		if (!DoFormatsMatch(pBI))
		{
			// A bad format
			pBufferToFill->ReleaseBufferData(&TimeReleased, L"Buffer format does not match device." );
		}
		else
		{	// We now lock the buffer
			size_t BufferSize=m_pParent->GetBufferSize(); //Note: the preinitialized size should only apply to raw devices!
			unsigned char *pBuffer  = (unsigned char *)pBI->pGetBufferData( &BufferSize );
			if (pBuffer)
			{
				bool success=m_pParent->FillBuffer(pBuffer,BufferSize,TimeReleased);

				// The buffer is filled in !
				// Release it with no error.				
				pBufferToFill->ReleaseBufferData(&TimeReleased,success?NULL:L"OutStream_Interface write error");

				//printf("Sent %d field %d\n",CurrentFramePosition,m_AlternateFields);
				RenderedBuffer=success;
			}
			else
			{
				// A bad format
				pBufferToFill->ReleaseBufferData(&TimeReleased, L"Could not lock buffer." );
			}
		}						
	}	

	m_FlushQueue_Lock.unlock();

ExitBufferFill:
	// Do not use the CPU to hard if we are negotiating buffer changes
	if (!RenderedBuffer)
	{
		//printf("Sleep 10 <-!RenderBuffer\n");
		Sleep(10);
	}

	m_AbortThread = false;

}
  /***************************************************************************************************/
 /*										OutStream_NotThreaded										*/
/***************************************************************************************************/

OutStream_NotThreaded::OutStream_NotThreaded(const wchar_t *pDeviceName) :
m_pDeviceName(pDeviceName),
m_AbortThread(false){
	assert(pDeviceName);
}

OutStream_NotThreaded::~OutStream_NotThreaded()
{
	m_AbortThread=true;
	FlushAllBuffers();
}

size_t OutStream_NotThreaded::GetDepth() const
{
	auto_lock ThreadSafe(m_ListOfBuffers_Lock);
	return m_ListOfBuffers.size();
}

FrameWork::Threads::event &OutStream_NotThreaded::GetEventHandle() const
{
	return m_EventHandle;
}



//From Device Interface
bool OutStream_NotThreaded::AddBufferToQueue( Devices::BufferInterface *pBuffer )
{
	// Lock the list of buffers
	m_ListOfBuffers_Lock.lock();
	//m_ListOfBuffers.insert(m_ListOfBuffers.end(), pBuffer );
	m_ListOfBuffers.push(pBuffer);
	#ifdef __DumpQueueSizes__
	DebugOutput(L"--- D-%s queue=%d\n",m_pDeviceName,m_ListOfBuffers.size());
	#endif
	m_ListOfBuffers_Lock.unlock();

	// Success
	m_EventHandle.set();
	return true;
}

void OutStream_NotThreaded::FlushAllBuffers( void )
{
	// We cannot flush the queue at this point !
	auto_lock FlushingLock( &m_FlushQueue_Lock );
	auto_lock QueueLock( &m_ListOfBuffers_Lock );

	TimeStamp TimeReleased;
	QueryPerformanceCounter((LARGE_INTEGER *)TimeReleased);

	// Flush the queue
	/*
	for( size_t i=0 ; i<m_ListOfBuffers.size() ; i++ )
		m_ListOfBuffers[i]->ReleaseBufferData(TimeReleased, L"Flush Queue." );
	m_ListOfBuffers.clear();
	*/

	while (!(m_ListOfBuffers.empty()))
	{
		BufferInterface *Val=(BufferInterface *)m_ListOfBuffers.front();
		if (Val)
			Val->ReleaseBufferData(&TimeReleased, L"Flush Queue." );
		m_ListOfBuffers.pop();
	}
}


void OutStream_NotThreaded::TransferBuffer(const byte *SourceBuffer,Devices::BufferInterface *pBufferToFill,size_t BufferSizeInBytes,TimeStamp Time)
{
	m_FlushQueue_Lock.lock();

	if ((pBufferToFill) && (!m_AbortThread))
	{	// Fill in the buffer ...
		// Check that there is implementation
		BufferInterface *pBI = dynamic_cast<BufferInterface*>( pBufferToFill );

		TimeStamp TimeReleased=Time;
		if ((__int64)Time==-1)
			QueryPerformanceCounter((LARGE_INTEGER *)TimeReleased);

		// No implementation ?
		if (!pBI)
		{	
			pBufferToFill->pGetBufferData(NULL);
			pBufferToFill->ReleaseBufferData(&TimeReleased, L"No BufferInterface found for source." );
			//DebugOutput(L"No BufferInterface found for source.\n");
		}

		// We check that the format is correct
		if (!DoFormatsMatch(pBI))
		{	
			// A bad format
			pBufferToFill->ReleaseBufferData(&TimeReleased, L"Buffer format does not match device." );
			//DebugOutput(L"Buffer format does not match device.\n");
		}
		else 
		{	// We now lock the buffer
			size_t BufferSize=BufferSizeInBytes; //set to known buffer size (applies to raw buffer rules)
			unsigned char *pBuffer  = (unsigned char *)pBI->pGetBufferData( &BufferSize );
			if (pBuffer)
			{
				//Note: It may be possible that the source buffer and pBuffer are identical
				if (pBuffer!=SourceBuffer)
					OutStream_MemCopy(pBuffer,SourceBuffer,min(BufferSize,BufferSizeInBytes),pBufferToFill);
				// The buffer is filled in !
				// Release it with no error.				
				pBufferToFill->ReleaseBufferData(&TimeReleased,NULL);
				//DebugOutput(L"Sent buffer %p\n",pBI);
			}
			else
			{
				// A bad format
				pBufferToFill->ReleaseBufferData(&TimeReleased, L"Could not lock buffer." );
				//DebugOutput(L"Could not lock buffer.\n");
			}
		}						
	}	

	m_FlushQueue_Lock.unlock();
}

Devices::BufferInterface *OutStream_NotThreaded::GetBufferToFill()
{
	if (m_AbortThread)
		return NULL;

	// We get a frame off the queue
	Devices::BufferInterface *pBufferToFill = NULL;

	m_FlushQueue_Lock.lock();
	// Lock the list
	m_ListOfBuffers_Lock.lock();

	// If there are buffers, then get one
	if (!(m_ListOfBuffers.empty()))
	{	
		//pBufferToFill = m_ListOfBuffers[0];
		pBufferToFill=(BufferInterface *)m_ListOfBuffers.front();
		m_ListOfBuffers.pop();
	}
	else 
		pBufferToFill=NULL;

	// Unlock the list
	m_ListOfBuffers_Lock.unlock();
	m_FlushQueue_Lock.unlock();
	return pBufferToFill;
}

void OutStream_NotThreaded::TransferBuffer(const byte *Buffer,size_t BufferSizeInBytes,TimeStamp Time)
{
	Devices::BufferInterface *pBufferToFill = GetBufferToFill();
	if (pBufferToFill)
		TransferBuffer(Buffer,pBufferToFill,BufferSizeInBytes,Time);
}


  /***************************************************************************************************/
 /*												InputStream											*/
/***************************************************************************************************/

InputStream::InputStream(InStream_Interface *pParent,DeviceInterface *pDevice,size_t DefaultTBCQueueDepth,const wchar_t *pDeviceClientName) :
m_pParent(pParent),m_pDevice(pDevice),m_DefaultTBCQueueDepth(DefaultTBCQueueDepth),m_DeviceClientName(pDeviceClientName),
m_FlushingQueue(false)
{
	// Checking
	assert (pParent && pDevice);
	//pParent->SetCallbackInterface(this);
	m_BufferCount=0;

	//Note: The parent (owning class) will need to call StartTBCQueue();
}

InputStream::~InputStream() 
{
	//Notify my parent that I am closing
	m_pParent->InStreamClosing(this);
	//m_pParent->SetCallbackInterface(NULL); //release the link
	FinishTBCQueue();
} 

void InputStream::AdvanceTBC( void )
{
		// Get the next frames if the switcher is requesting them
	if (m_TBCQueue.size() && m_TBCQueue[0])
	{	// Lock the TBC queue
		m_TBCQueue_Lock.lock();

		// For now, we always look and smell like a full frame-rate device although
		// we might change that in the future.

		// If there is more than one field in the output queue, then we 
		// move on to the next frame
		if (m_TBCQueue.size()>0)
		{	// Add this field back for filling in
			AddBufferToDeviceQueue (m_TBCQueue[0]);

			// Remove it from the TBC queue
			m_TBCQueue.erase(m_TBCQueue.begin());
		}
		else
		{	

			//This is handy in the case where the TBC queue may be too lean and may need to have more buffers
			if (m_BufferCount<m_MaxTBCQueueDepth)
			{
				AddBufferToDeviceQueue (NULL);
				// Debugging
				DebugOutput(L"InputStream::AdvanceTBC Insert Frame %p\n",this);
				printf("InputStream::AdvanceTBC Insert Frame\n",this);
			}
		}

		// Unlock the TBC queue
		m_TBCQueue_Lock.unlock();
	}

	// Reset the buffer values
	//TODO
}


bool InputStream::AddBufferToDeviceQueue( BufferInterface *pBufferToAdd )
{
	if (!pBufferToAdd)
	{
		// Create a new buffer
		pBufferToAdd = m_pParent->InStream_Interface_GetNewBuffer(this);
		m_BufferCount++;
		//TODO (maybe) Check for a failure to allocate the buffer

		// Store that this frame was allocated
		//RenderTreeGPU::auto_lock LockList( &m_AllFrames_Queue_Lock );
		//m_AllFrames_Queue.Add( pBufferToAdd );
	}

	// Add it to the input device
	return m_pDevice->AddBufferToQueue( pBufferToAdd );
}

void InputStream::AddBufferToTBCList( BufferInterface *pBuffer , const wchar_t *pErrorMessage )
{
		// Lock the TBC queue while this is added
	auto_lock LockQueue( &m_TBCQueue_Lock );

	//DebugOutput(L"AddBufferToTBCList %p %d\n",this,m_TBCQueue.size());
	// If the queue is being flushed, then we just return immediately (deleting these buffers!)
	if ( m_FlushingQueue )
	{
		//delete pBuffer
		m_pParent->InStream_Interface_FreeBuffer(pBuffer);
		m_BufferCount--;
		return;
	}

	// Handle the frame delivery
	if (pErrorMessage)
	{	
		// We try to add the buffer back onto the device.
		//we'll check to see if it is because it is an error mismatch
		if (!DoFormatsMatch(m_pDevice,pBuffer))
		{
			delete pBuffer;
			m_BufferCount--;
			AddBufferToDeviceQueue(NULL);
			m_pParent->In_FormatChangeDetected(); //let parent know of this
		}
		else
			AddBufferToDeviceQueue( pBuffer );

		// Debugging
		#ifdef __ShowInfo__
		{	OutputDebugString("Audio Packet Return Error : ");
			OutputDebugString(pErrorMessage);
			OutputDebugString("\n");
			printf("Audio Packet Return Error : %s\n",pErrorMessage);			
		}
		#endif __ShowInfo__
	}
	else
	{
		if ((int)m_TBCQueue.size()>m_MaxTBCQueueDepth)
		{	
			//This should not occur unless someone has added some buffers externally and did not check the threshold
			//Note: there is currently no public access to this member variable, but there is access to the TBC list
			assert(false);
			// Debugging
			DebugOutput(L"TBC : Dropped packet\n");
			printf("TBC : Dropped packet\n");

			delete pBuffer;
			m_BufferCount--;
			//AddBufferToDeviceQueue( pBuffer );
		}
		else
		{	// This is a valid frame, so add it to the queue
			//let the parent know of this change
			m_pParent->AddBufferToTBCList(pBuffer,pErrorMessage);
			m_TBCQueue.insert(m_TBCQueue.end(), pBuffer );
			#ifdef __DumpQueueSizes__
			DebugOutput(L"+++ DC-%s queue=%d\n",m_DeviceClientName.c_str(),m_TBCQueue.size());
			#endif
		}
	}	
}

bool InputStream::StartTBCQueue( size_t NoBuffersToUse,size_t NoBuffersToStart)
{
	if (NoBuffersToUse==-1)
		NoBuffersToUse=m_DefaultTBCQueueDepth;
	if (NoBuffersToStart==-1)
		NoBuffersToStart=NoBuffersToUse;

	m_MaxTBCQueueDepth=NoBuffersToUse;

	// Stop the queue if it is running
	FinishTBCQueue();

	for( size_t i=0 ; i<NoBuffersToStart ; i++ )
		// Try adding this buffer to the queue
		if (!AddBufferToDeviceQueue(NULL)) goto Error;

	// Success
	return true;

Error:
	// Stop the queue if it is running
	FinishTBCQueue();
	return false;
}

void InputStream::FinishTBCQueue( void )
{
	// Lock the queue and internally mark us as being in flushing mode
	m_TBCQueue_Lock.lock();
	m_FlushingQueue = true;
	m_TBCQueue_Lock.unlock();

	// FLush the device of all buffers
	m_pDevice->FlushAllBuffers();

	// We now free all allocated buffers
	//m_AllFrames_Queue_Lock.lock();

	//for( size_t i=0 ; i<m_AllFrames_Queue.NoItems() ; i++ )
	//	delete m_AllFrames_Queue[i];

	//m_AllFrames_Queue.DeleteAll();

	//m_AllFrames_Queue_Lock.unlock();

	// No longer flushing
	m_FlushingQueue = false;
}

InStream_Interface *InputStream::GetParent() const
{	return m_pParent;
}

DeviceInterface *InputStream::GetDevice() const
{	return m_pDevice;
}

FrameWork::Threads::critical_section &InputStream::GetTBCQueue_Lock()
{	return m_TBCQueue_Lock;
}

std::vector<BufferInterface *> &InputStream::GetTBCQueue()
{	return m_TBCQueue;
}

bool InputStream::GetFlushingQueue() const
{	return m_FlushingQueue;
}


  /***************************************************************************************************/
 /*											VideoStream												*/
/***************************************************************************************************/
using namespace Devices::Video::Base::Streams;

bool VideoInputStream::DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer)
{
	return m_VideoStreamBase.DoFormatsMatch(Device,Buffer);
}

bool operator== ( const Video::FormatInterface &Op1 , const Video::FormatInterface &Op2 )
{	
	return ( (Op1.GetXResolution()==Op2.GetXResolution()) &&
		(Op1.GetYResolution()==Op2.GetYResolution()) &&
		(Op1.GetFrameFormat()==Op2.GetFrameFormat()) &&
		(!stricmp(Op1.pGetFourCC(),Op2.pGetFourCC())) ) ? true : false;
}

bool operator!= ( const Video::FormatInterface &Op1 , const Video::FormatInterface &Op2 )
{	
	return !(Op1 == Op2);
}

bool VideoStreamBase::DoFormatsMatch(const Video::FormatInterface *pFormat1,const Video::FormatInterface *pFormat2)
{
	return ( (*pFormat1) == (*pFormat2) );

}
bool VideoStreamBase::DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer)
{
	return DoFormatsMatch(dynamic_cast<const Video::FormatInterface *>(Device),dynamic_cast<const Video::FormatInterface *>(Buffer));
}

bool VideoStreamBaseDevice::DoFormatsMatch(const Devices::BufferInterface *Buffer)
{
	return VideoStreamBase::DoFormatsMatch(this,dynamic_cast<const Video::FormatInterface *>(Buffer));
}

bool VideoOutputStream::DoFormatsMatch(const Devices::BufferInterface *Buffer)
{
	return m_VideoStreamBase.DoFormatsMatch(Buffer);
}


  /***************************************************************************************************/
 /*											AudioStream												*/
/***************************************************************************************************/
using namespace Devices::Audio::Base::Streams;


bool AudioInputStream::DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer)
{
	return m_AudioStreamBase.DoFormatsMatch(Device,Buffer);
}

bool operator== ( const Audio::FormatInterface &Op1 , const Audio::FormatInterface &Op2 )
{	
	return ( (Op1.GetSampleRate()==Op2.GetSampleRate()) &&
		(Op1.GetSampleFormat()==Op2.GetSampleFormat()) &&
		(Op1.GetIsBufferInterleaved()==Op2.GetIsBufferInterleaved()) &&
		(Op1.GetNoChannels()==Op2.GetNoChannels()) ) ? true : false;

}

bool operator!= ( const Audio::FormatInterface &Op1 , const Audio::FormatInterface &Op2 )
{	
	return !(Op1 == Op2);
}

bool AudioStreamBase::DoFormatsMatch(const Audio::FormatInterface *pFormat1,const Audio::FormatInterface *pFormat2)
{
	return ( (*pFormat1) == (*pFormat2) );

}
bool AudioStreamBase::DoFormatsMatch(const Devices::DeviceInterface *Device,const Devices::BufferInterface *Buffer)
{
	return DoFormatsMatch(dynamic_cast<const Audio::FormatInterface *>(Device),dynamic_cast<const Audio::FormatInterface *>(Buffer));
}

bool AudioStreamBaseDevice::DoFormatsMatch(const Devices::BufferInterface *Buffer)
{
	return AudioStreamBase::DoFormatsMatch(this,dynamic_cast<const Audio::FormatInterface *>(Buffer));
}

bool AudioOutputStream::DoFormatsMatch(const Devices::BufferInterface *Buffer)
{
	return m_AudioStreamBase.DoFormatsMatch(Buffer);
}

bool AudioOutStream_NotThreaded::BypassFormatCheck()
{
	//check formats by default
	return false;
}

bool AudioOutStream_NotThreaded::DoFormatsMatch(const Devices::BufferInterface *Buffer)
{
	if (BypassFormatCheck()) return true;
	return m_AudioStreamBase.DoFormatsMatch(Buffer);
}


AudioInStream_Interface *AudioInputStream::GetParent() const
{	return dynamic_cast<AudioInStream_Interface *>(m_pParent);
}

  /***************************************************************************************************/
 /*									Audio_OutStream_QueueSource										*/
/***************************************************************************************************/


Audio_OutStream_QueueSource::Audio_OutStream_QueueSource(Audio_OutStream_QueueSource_Interface *OutStreamClient,const wchar_t *pDeviceName) :
AudioOutStream_NotThreaded(OutStreamClient,pDeviceName),m_pParent(OutStreamClient)
{
}

void Audio_OutStream_QueueSource::TransferBuffers()
{
	if (m_AbortThread)
		return;

	// We get a frame off the queue
	Devices::BufferInterface *pBufferToFill = NULL;

	do 
	{
		m_FlushQueue_Lock.lock();
		// Lock the list
		m_ListOfBuffers_Lock.lock();

		// If there are buffers, then get one
		if (!(m_ListOfBuffers.empty()))
		{	
			pBufferToFill=(BufferInterface *)m_ListOfBuffers.front();
			//Note I cannot pop here yet until I know that I can take it in my callback
			//m_ListOfBuffers.pop();
		}
		else 
			pBufferToFill=NULL;

		// Unlock the list
		m_ListOfBuffers_Lock.unlock();
		m_FlushQueue_Lock.unlock();

		m_FlushQueue_Lock.lock();

		if ((pBufferToFill) && (!m_AbortThread))
		{
			// Fill in the buffer ...
			// Check that there is a audio implementation
			Audio::BufferInterface *pABI = dynamic_cast<Audio::BufferInterface*>( pBufferToFill );

			TimeStamp TimeReleased;
			QueryPerformanceCounter((LARGE_INTEGER *)TimeReleased);

			// No audio implementation ?
			if (!pABI)
			{	
				pBufferToFill->pGetBufferData(NULL);
				pBufferToFill->ReleaseBufferData(&TimeReleased, L"No BufferInterface found for audio source." );
				//DebugOutput(L"No BufferInterface found for audio source.\n");
			}

			// We check that the audio format is correct
			else if ( !DoFormatsMatch(pABI) )
			//else if ( (*this) != (*pABI) )
			{	
				// A bad format
				pBufferToFill->ReleaseBufferData(&TimeReleased, L"Buffer audio format does not match device." );
				//DebugOutput(L"Buffer audio format does not match device.\n");
			}

			else {	// We now lock the buffer
				//The Client will decide whether we read the buffer size of the buffer or
				//Set it himself.  If this is zero... this means the device client knows what size to set his buffers
				size_t BufferSize=m_pParent->Audio_OutStream_QueueSource_GetBufferSizeInBytes(pABI);
				unsigned char *pBuffer  = (unsigned char *)pABI->pGetBufferData( &BufferSize );
				if (pBuffer)
				{
					//Now to figure out how many samples there are
					//Note: we are using only the float format at this point.
					size_t BytesPerSample=sizeof(float) * pABI->GetNoChannels();
					size_t No_Samples = (BufferSize / BytesPerSample);

					//Fill the buffer... and break out the loop if not successful
					if (!(m_pParent->Audio_OutStream_QueueSource_FillBuffer(pBuffer,No_Samples,pABI)))
					{
						m_FlushQueue_Lock.unlock();
						break;
					}


					// The buffer is filled in !
					// Release it with no error.				
					pBufferToFill->ReleaseBufferData(&TimeReleased,NULL);
					//DebugOutput(L"Sent buffer %p\n",pABI);
				}
				else
				{
					// A bad format
					pBufferToFill->ReleaseBufferData(&TimeReleased, L"Could not lock buffer." );
					//DebugOutput(L"Could not lock buffer.\n");
				}
			}
			//All branches in this scope will release the buffer... except for the case where the client's fill buffer call is unsuccessful
			//for this case it will break out of the loop and this buffer will remain in queue
			m_ListOfBuffers_Lock.lock();
			m_ListOfBuffers.pop();
			m_ListOfBuffers_Lock.unlock();
		}	

		m_FlushQueue_Lock.unlock();

	} while(pBufferToFill);
}


  /***********************************************************************************************************/
 /*														RawBuffer											*/
/***********************************************************************************************************/

RawBuffer::RawBuffer(InputStream *pParent) :
m_pParent(pParent), m_pBuff(NULL), m_cbBuff(0), m_cbData(0), m_Time(0.0)
{
}

RawBuffer::~RawBuffer()
{
	if (m_pBuff)
		::free(m_pBuff);
}

void *RawBuffer::pGetBufferData( size_t *pMemorySize )
{
	assert(pMemorySize);

	if (0 == *pMemorySize)	// Is somebody just trying to read me???
	{
		*pMemorySize = m_cbData;
		return m_pBuff;
	}
	else					// Gonna be a write???
	{
		if (*pMemorySize > m_cbBuff)	// Requested size is larger that what I've already got???
		{
			if (m_pBuff)
				::free(m_pBuff);
			m_pBuff = ::malloc(*pMemorySize);
			m_cbBuff = m_cbData = *pMemorySize;
		}
		else
		{
			m_cbData = *pMemorySize;
		}

		return m_pBuff;
	}
}

void RawBuffer::ReleaseBufferData( const TimeStamp *pFrameTime, const wchar_t *pErrorMessage)
{
	/*
	//Note I do not need to do this... I pass the error along and the parent will handle the error

	if (pErrorMessage)
	{
		//	printf("**Buffer Error-%s\n",pErrorMessage);
		// My data is BS b/c of an error
		m_cbData = 0;
		m_Time = 0;
	}
	else 
	*/
	if (pFrameTime)
		m_Time = *pFrameTime;

	m_pParent->AddBufferToTBCList( this , pErrorMessage );
}

const TimeStamp& RawBuffer::GetTimeStamp( void ) const
{
	return m_Time;
}


  /***************************************************************************************************/
 /*										AudioBuffer_Base											*/
/***************************************************************************************************/


AudioBuffer_Base::AudioBuffer_Base(AudioInputStream *pParent,const FormatInterface *pFormat) : m_pParent(pParent)
{	
	//lets get the buffer format
	//if we don't have it yet...make the best guess
	m_NoChannels=pFormat->GetNoChannels();
	if (m_NoChannels==0)
		m_NoChannels=2;
	m_SampleRate=pFormat->GetSampleRate();
	if (m_SampleRate==0)
		m_SampleRate=48000;
	m_bIsInterleaved=pFormat->GetIsBufferInterleaved();
	m_SampleFormat=pFormat->GetSampleFormat();
	if (m_SampleFormat==Audio::FormatInterface::eSampleFormat_Unknown)
		m_SampleFormat=Audio::FormatInterface::eSampleFormat_I_16;
}

AudioBuffer_Base::~AudioBuffer_Base()
{
}


