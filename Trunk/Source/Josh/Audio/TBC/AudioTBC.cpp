#include "stdafx.h"
// Include the main project
#include "Devices_Audio_TBC.h"
// Operators and information
#include "..\Base\Devices_Audio_Base.h"
// Local utilities
#include "Private_Utilities.h"

//This is temporary for debugging purposes
#undef	__WriteWavIncomingBuffers__
#undef	__WriteWavOutgoingBuffers__

#if	(defined __WriteWavIncomingBuffers__ ||defined __WriteWavOutgoingBuffers__)
#include "../../../../User/James Killian/AudioTesting/Wave/Wave.h"
#endif 

#undef __ShowInfo__

using namespace FrameWork::Base;
using namespace FrameWork::Threads;
using namespace Devices;
using namespace Devices::Streams;
using namespace Devices::Audio;
using namespace Devices::Audio::TBC;
using namespace Devices::Audio::Base;
using namespace Devices::Audio::Base::Streams;

const double c_SmoothingRate=0.01; //I can't imagine this needing to change

#ifndef CACHE_ALIGN
#define CACHE_LINE  32
#define CACHE_ALIGN __declspec(align(CACHE_LINE))
#endif CACHE_ALIGN

template<class L>
__forceinline void SubSample(L *BasePointer,L *BasePointer2,double Position,double Position2,float &Sample)
{
	size_t   iBasePosition=(size_t)((double)(Position));
	double cBasePosition=(double)(Position-iBasePosition);
	size_t   iBasePosition2=(size_t)((double)(Position2));
	Sample=(float)(double(BasePointer[iBasePosition])*(1.0f-cBasePosition)+double(BasePointer2[iBasePosition2])*cBasePosition);
}



  /***************************************************************************************************/
 /*											AudioTBC_Internal										*/
/***************************************************************************************************/


#pragma warning (disable : 4355)
AudioTBC_Internal::AudioTBC_Internal(Devices::Audio::DeviceInterface *pAudioDevice,FormatInterface *pBufferFormat,size_t LatencyQueueDepth) :
m_InputStreamer(this,pAudioDevice),m_OutputStreamer(this,this),
m_pCurrentBuffer(NULL),
m_LastInBufferSentTime(0.0),m_LastOutBufferSentTime(0.0),m_CurrentLocalPosition(0.0),
m_LatencyQueueDepth(LatencyQueueDepth),m_StartedTBC(false),
m_BufferSampleSize(0),m_InputDevice(pAudioDevice)
{
	//We'll go ahead and initialize these because the client may which to access the upstream device's format as its own
	//the upstream may not be ready... so we'll make a good guess as to what it is
	//m_OutStream_NoChannels=pAudioDevice->GetNoChannels();
	//if (m_OutStream_NoChannels==0)
	//	m_OutStream_NoChannels=2;
	m_OutStream_SampleFormat=pAudioDevice->GetSampleFormat();
	if (m_OutStream_SampleFormat==FormatInterface::eSampleFormat_Unknown)
		m_OutStream_SampleFormat=FormatInterface::eSampleFormat_FP_32;
	m_bInterleaved=pAudioDevice->GetIsBufferInterleaved();
	m_OutStream_SampleRate=pAudioDevice->GetSampleRate();
	if (m_OutStream_SampleRate==0)
		m_OutStream_SampleRate=48000;
}
#pragma warning (default : 4355)

AudioTBC_Internal::~AudioTBC_Internal() 
{
} 


AudioTBC_Internal::operator Devices::Audio::DeviceInterface* ( void ) 
{
	return m_OutputStreamer.GetAudioDevice();
}
Devices::Audio::DeviceInterface *AudioTBC_Internal::GetAudioDevice() 
{
	return m_OutputStreamer.GetAudioDevice();
}

size_t AudioTBC_Internal::GetBufferSampleSize() 
{
	return m_BufferSampleSize;
}
const wchar_t *AudioTBC_Internal::pGetDeviceName( void ) const 
{
	return L"AudioTBC";
}

size_t AudioTBC_Internal::GetSampleRate( void ) const 
{
	return m_OutStream_SampleRate;
}

size_t AudioTBC_Internal::GetNoChannels( void ) const 
{
	//return m_OutStream_NoChannels;

	//While the TBC can accommodate sample rate it doesn't accommodate number of channels.  The device client will need to handle supporting
	//the number of channels specified by the device this TBC is wrapping.  We must however, ensure that we return a valid count, so if the
	//device is not ready (e.g. returns 0) we'll return a typical number so that the client can get things started.  They should be able to
	//handle a change of count when it happens
	size_t NoChannels=m_InputDevice->GetNoChannels();
	if (NoChannels==0)
		NoChannels=2;
	return NoChannels;
}

Devices::Audio::FormatInterface::eSampleFormat AudioTBC_Internal::GetSampleFormat( void ) const
{
	return m_OutStream_SampleFormat;
}

bool AudioTBC_Internal::GetIsBufferInterleaved( void ) const
{
	return m_bInterleaved;
}

void AudioTBC_Internal::AddBufferToTBCList( Devices::BufferInterface *pBuffer , const wchar_t *pErrorMessage )
{
	AudioBuffer *pAudioBuffer=dynamic_cast<AudioBuffer *>(pBuffer);
	if ((__int64)m_LastInBufferSentTime)
	{
		// This is a valid frame, so add it to the queue
		//Compute the high precision sample rate here
		__int64 Frequency;
		QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
		double dif=(double)(pAudioBuffer->GetTimeStamp()-m_LastInBufferSentTime);
		//For best results we smooth the time intervals!
		m_CurrentInSampleTime=(1.0-c_SmoothingRate) * m_CurrentInSampleTime + ( c_SmoothingRate * (dif/(double)Frequency) );
		double CurrentSampleRate=(double)pAudioBuffer->GetNoSamples() / m_CurrentInSampleTime;
		//m_InSampleRate=(1.0-c_SmoothingRate) * m_InSampleRate + ( c_SmoothingRate * CurrentSampleRate );
		m_InSampleRate=CurrentSampleRate;
		//DebugOutput(L"Current time %f, Smoothed %f, Rate %f \n",(dif/(double)Frequency),m_CurrentInSampleTime,m_InSampleRate);
		//printf("In Samples Time %f\n",m_CurrentInSampleTime);
		//DebugOutput(L"In Samples %f\n",m_InSampleRate);
	}
	m_LastInBufferSentTime=pAudioBuffer->GetTimeStamp();
}

void AudioTBC_Internal::In_FormatChangeDetected()
{
	m_ReferencePoint=(double)m_InputDevice->GetSampleRate()/(double)m_OutputStreamer.GetAudioDevice()->GetSampleRate();
}

Devices::BufferInterface *AudioTBC_Internal::InStream_Interface_GetNewBuffer(InputStream *pInputStream)
{
	return new AudioBuffer(&m_InputStreamer,m_InputDevice);
}

void AudioTBC_Internal::InStream_Interface_FreeBuffer(Devices::BufferInterface *buffer)
{
	delete dynamic_cast<AudioBuffer *>(buffer);
}

bool AudioTBC_Internal::AddBufferToQueue( Devices::BufferInterface *pBuffer )
{
	if (!m_StartedTBC)
	{
		BufferInterface *pABI = dynamic_cast<BufferInterface*>( pBuffer );
		if (pABI)
		{	
			size_t BufferSize;
			unsigned char *pBuffer  = (unsigned char *)pABI->pGetBufferData( &BufferSize );
			if (pBuffer)
			{
				//We want our OutStream to have our sample rate match this buffer
				//As for the other format info... it is debatable where the pro is that we can do all the conversion here in a threaded environment
				//The con is that these accessors are not initialized until now
				//m_OutStream_NoChannels=pABI->GetNoChannels();
				m_OutStream_SampleFormat=pABI->GetSampleFormat();
				m_bInterleaved=pABI->GetIsBufferInterleaved();
				m_OutStream_SampleRate=pABI->GetSampleRate();

				//Now to figure out how many samples there are
				size_t BytesPerSample=Devices::Audio::Base::aud_convert::GetFormatBitDepth(GetSampleFormat()) * GetNoChannels();  //we can assume float now
				//size_t BytesPerSample=sizeof(float) * GetNoChannels();
				size_t NoSamples = (BufferSize / BytesPerSample);

				//It is important that this gets set before calling m_InputStreamer.StartTBCQueue(NoBuffers);
				m_BufferSampleSize=NoSamples;

				//make a good guess what the transfer rate will be
				m_OutSampleRate=(double)m_OutputStreamer.GetAudioDevice()->GetSampleRate();
				m_CurrentOutSampleTime=NoSamples/m_OutSampleRate; //The ideal time interval
				m_InSampleRate=(double)m_InputDevice->GetSampleRate();
				//The in stream may not be available yet... so we guess it will be the same rate
				if (m_InSampleRate==0)
					m_InSampleRate=m_OutSampleRate;
				m_CurrentInSampleTime=NoSamples/m_InSampleRate; //The ideal time interval

				m_CurrentTransferRate=m_ReferencePoint=m_InSampleRate/m_OutSampleRate;


				size_t NoBuffers=m_LatencyQueueDepth; //round up... need to have enough buffers to adjust
				NoBuffers+=2; //add a couple more buffers to avoid stalling
				//Now we know the buffer size so start the TBC... note we need to give the extra buffers to start
				//This will ensure that the device will have buffers available when needing to increase the sample
				//rate
				m_InputStreamer.StartTBCQueue(NoBuffers,NoBuffers);
				//wait until the buffer is filled to get a good start
				size_t QueueDepth=0,TimeOut=0;
				do 
				{
					//DebugOutput(L"Waiting to fill buffer %d\n",TimeOut);
					Sleep(10);
					m_InputStreamer.GetTBCQueue_Lock().lock();
					std::vector<Devices::BufferInterface *>	&l_TBCQueue=m_InputStreamer.GetTBCQueue();
					QueueDepth=l_TBCQueue.size();
					m_InputStreamer.GetTBCQueue_Lock().unlock();
				} while((QueueDepth<m_LatencyQueueDepth)&&(TimeOut++<50));

				//Note: it is possible for it to timeout if device is instantiating the TBC... if it does then there is a wait, but everything
				//will still work just fine... it will just have a pitch shifting start
				if (!(TimeOut<50))
					DebugOutput(L"Audio TBC Timed Out, there may be a slight pitch shift when starting\n");
				m_StartedTBC=true;
			}
		}
	}
	return m_StartedTBC;
}

void AudioTBC_Internal::WaitForNextBuffer()
{
	//size_t Counter=false; //for debug purposes only
	bool NotDone=true;
	do 
	{
		// Lock the TBC queue
		size_t QueueDepth;
		std::vector<Devices::BufferInterface *>	&l_TBCQueue=m_InputStreamer.GetTBCQueue();
		critical_section &l_TBCQueue_Lock=m_InputStreamer.GetTBCQueue_Lock();

		l_TBCQueue_Lock.lock();
		QueueDepth=l_TBCQueue.size();
		//there needs to be at least 2 buffers... and 3 to really be safe
		NotDone=(QueueDepth<3);
		l_TBCQueue_Lock.unlock();

		if (NotDone)
		{
			//DebugOutput(L"AudioTBC_Internal::WaitForNextBuffer needs to wait %d\n",Counter++);
			//printf("AudioTBC_Internal::WaitForNextBuffer needs to wait %d\n",Counter++);
			Sleep(1);
		}
	} while((NotDone)&&(!m_OutputStreamer.GetAbortThread()));

}

//This is where the rubber meets the road ;)
bool AudioTBC_Internal::FillBuffer(byte *DestBuffer,size_t NoSamples,FormatInterface *Format,TimeStamp &DeviceTime)
{	
	if (!m_StartedTBC)
		return false;

	//Test a signal
	#undef	__TestSignal__
	#ifndef __TestSignal__

	size_t NoOutChannels=Format->GetNoChannels();
	size_t BytesPerSample=Devices::Audio::Base::aud_convert::GetFormatBitDepth(Format->GetSampleFormat()) * NoOutChannels;  //we can assume float now
	//size_t BytesPerSample=sizeof(float) * NoOutChannels;

	float *Destination=(float *)DestBuffer;

	// Lock the TBC queue
	size_t QueueDepth;
	std::vector<Devices::BufferInterface *>	&l_TBCQueue=m_InputStreamer.GetTBCQueue();
	critical_section &l_TBCQueue_Lock=m_InputStreamer.GetTBCQueue_Lock();

	bool HaveValidInStreamBufs=false;
	AudioBuffer *l_NextBuffer;
	do 
	{
		l_NextBuffer=NULL;
		l_TBCQueue_Lock.lock();

		QueueDepth=l_TBCQueue.size();
		//#ifdef __ShowInfo__
		{	// Cycle over the list to debug
			//__int64 freq;
			//QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

			//printf("TBC Queue : %d, ",QueueDepth);

			//DebugOutput(L"TBC Queue : %d,",QueueDepth);
			//for( size_t i=0 ; i<m_TBCQueue.NoItems() ; i++ )
			//	DebugOutput(L"%x[%x] ",m_TBCQueue[i],((long)(((double)m_TBCQueue[i]->m_TimeReleased/(double)freq) * 1000))&0xffff);
			//DebugOutput(L"\n");
		}
		//#endif __ShowInfo__

		// The Buffers used for rendering (I need at least 2 for overlapping sub samples
		if (QueueDepth>1)
		{	// Get the first 2 frames
			m_pCurrentBuffer = dynamic_cast<AudioBuffer *>(l_TBCQueue[0]);
			l_NextBuffer = dynamic_cast<AudioBuffer *>(l_TBCQueue[1]);
		}

		// Unlock the TBC queue
		l_TBCQueue_Lock.unlock();

		// Could not get TBC frames !
		if (!l_NextBuffer) 
		{
			//This should not be happening... there should be enough latency compensation
			//printf( "**No TBC Frames current->%p next->%p\n",m_pCurrentBuffer,l_NextBuffer);
			return false; //Note: we should get the correct sample rate even if/when this exits here
		}

		//Ensure these in-stream buffers have the same number of channels (they should, except for in dynamic format changes)
		if (m_pCurrentBuffer->GetNoChannels()==Format->GetNoChannels())
			HaveValidInStreamBufs=true;
		else
		{
			m_InputStreamer.AdvanceTBC();
			//Might as well check the next buffer too (although I could let it loop)
			if (*l_NextBuffer!=*Format)
				m_InputStreamer.AdvanceTBC();
			DebugOutput(L"AudioTBC_Internal::FillBuffer InStream buffer!=OutStream... discarding sample packets\n");
		}
	} while(!HaveValidInStreamBufs);

	double TransferRateDelta;
	{ //The heart of TBC... figure out the time correction rate
		//This just keeps a constant queue depth
		//Parabola formula y=a(x-h)^2 + k
		const double Area=(8/QueueDepth)*.01; //This formula keeps a scaled ratio of .01 for 8 buffers which has been tested to be a good amount of correction
		double Correction=(Area*(
			(
				((double)QueueDepth-m_LatencyQueueDepth) *
				(fabs((double)QueueDepth-m_LatencyQueueDepth)) //Not quite a parabola... we will retain negative values
			)
			)) + 1.0;
		

		//printf(" Correction %f \n",Correction);
		double SmoothedReadingRate=(1.0-c_SmoothingRate) * m_CurrentTransferRate + (c_SmoothingRate * (m_InSampleRate/m_OutSampleRate));

		//apply the correction
		{
			double IdealRate=Correction*m_ReferencePoint;
			//Note we smooth out the change in a linear slope to avoid a rigid change (ie. it will sound like a fast vibrato)
			double Slope=(IdealRate-SmoothedReadingRate)*c_SmoothingRate;
			//printf("C=%f, CR=%f, IR=%f, Slope=%f\n ",Correction,SmoothedReadingRate,IdealRate,Slope);
			//DebugOutput(L"IR=%f, Slope=%f, ",IdealRate,Slope);
			SmoothedReadingRate=Slope+m_CurrentTransferRate;

			//Snapping samples implementation-  When the sample rates match we try to nudge the sample pointer on an integer boundary
			//Doing this will cancel the need for sample linear interpolation giving the highest quality possible
			//Note: 1E-3 is just enough to "bite" without any noticeable pitch shift
			if ((IdealRate==1.0)&&(fabs(SmoothedReadingRate-1.0)<1e-3))
			{
				m_CurrentTransferRate=1.0;
				SmoothedReadingRate=((1.0-(m_CurrentLocalPosition-floor(m_CurrentLocalPosition)))/NoSamples) + 1.0;
				//DebugOutput(L"Snapping samples %.17g %.17g\n",SmoothedReadingRate,m_CurrentLocalPosition);
				//printf("Snapping samples %.17g %.17g\n",SmoothedReadingRate,m_CurrentLocalPosition);
			}
			//Due to horrible stress potential from the device... we need to enforce some tolerable range of pitch shifting... if the stress
			//is worse than this tolerable range the device itself will either insert silence or drop extra packets back to the device
			//It is hard to say what the correct tolerance range should be, but I suspect it would be something where the pitch is not
			//a noticeable change, and keep the skip pops to a minimum as well
			else if (m_CurrentTransferRate<m_ReferencePoint-.01)
			{
				m_CurrentTransferRate=SmoothedReadingRate=m_ReferencePoint-.01;
			}
			else if (m_CurrentTransferRate>m_ReferencePoint+.01)
			{
				m_CurrentTransferRate=SmoothedReadingRate=m_ReferencePoint+.01;
			}
		}

		//if the queue depth is low then we don't want our rate to exceed 1.0 (it will cause buffer underruns)
		//Use this test if pops are heard to determine if this is the problem
		//if ((QueueDepth<4)&&(SmoothedReadingRate>1.0))
		//{
		//	SmoothedReadingRate=1.0;
		//	assert(false);
		//}

		//printf	  ("QD=%d, Co=%f\n",QueueDepth,Correction);
		//DebugOutput(L"QD=%d, Co=%f\n",QueueDepth,Correction);
		//printf	  ("In=%f Out=%f, Smoothed %f \n",m_InSampleRate,m_OutSampleRate,SmoothedReadingRate);
		//DebugOutput(L"In=%f Out=%f, Smoothed %f \n",m_InSampleRate,m_OutSampleRate,SmoothedReadingRate);
		//printf      ("QD=%d, Smoothed=%f, Co=%f  InSR=%f \n",QueueDepth,SmoothedReadingRate,Correction,m_InSampleRate);
		//DebugOutput(L"QD=%d, Smoothed=%f, Co=%f, InSR=%f \n",QueueDepth,SmoothedReadingRate,Correction,m_InSampleRate);

		TransferRateDelta=(SmoothedReadingRate-m_CurrentTransferRate)/(double)NoSamples;
		//printf("transfer delta %.17g \n",TransferRateDelta);
		//DebugOutput(L" transfer %.17g \n",m_CurrentTransferRate);
	}


	{//Pass the stream's time stamp down stream
		double WeightedFirstSample=m_CurrentLocalPosition/(double)m_pCurrentBuffer->GetNoSamples();
		DeviceTime=(__int64)((double)m_pCurrentBuffer->GetTimeStamp()* (1.0-WeightedFirstSample) + (double)l_NextBuffer->GetTimeStamp() * WeightedFirstSample);
	}
	//Now to transfer samples to buffer

	bool BuyMeSomeMoreTime=false; //wait till the last moment to see if we can get the next buffer

	for (size_t i=0;i<NoSamples;i++)
	{
		//float ScaleFactor;
		float Sample;

		double position=m_CurrentLocalPosition,position2;
		double positionend=(double)m_pCurrentBuffer->GetNoSamples();
		float *BasePointer=(float *)m_pCurrentBuffer->pGetBufferData(NULL),*BasePointer2;
		bool bAdvanceTBC;

		if (m_CurrentLocalPosition>=positionend-1.0)
		{

			if (BuyMeSomeMoreTime)
			{
				//This is the last possible moment to request for the next buffer!
				l_TBCQueue_Lock.lock();
				QueueDepth=l_TBCQueue.size();
				if (QueueDepth>1)
					l_NextBuffer = dynamic_cast<AudioBuffer *>(l_TBCQueue[1]);
				else
					DebugOutput(L"AudioTBC_Internal::FillBuffer (%f) buffer underrun- popping may sound\n",m_CurrentTransferRate);

				l_TBCQueue_Lock.unlock();
			}

			position2=1.0 - (positionend-position);
			BasePointer2=(float *)l_NextBuffer->pGetBufferData(NULL);
			bAdvanceTBC=true;
			m_CurrentLocalPosition-=positionend;

			//its possible the amount of skip caused the position to skip to the next buffer as well
			if (position>positionend)
			{
				position-=positionend;
				BasePointer=(float *)l_NextBuffer->pGetBufferData(NULL);
			}

			//if my current local position is way off then reset to zero
			//This could happen if one of the sample rates is erroneous
			if (m_CurrentLocalPosition>positionend)
			{
				m_CurrentLocalPosition=0.0;
				assert(false);
			}
		}
		else
		{
			position2=m_CurrentLocalPosition+1.0;
			BasePointer2=BasePointer;
			bAdvanceTBC=false;
		}
	
		const Devices::Audio::DeviceInterface *l_pAudioDevice=m_InputDevice;

		//Note: I can get away with this de-interleaved memory storing for Audio... yes it will have mis-cache writes
		//but it is insignificant for performance, and so I want my code to be simple and readable.
		for (size_t j=0;j<NoOutChannels;j++)
		{
			SubSample<float>(BasePointer+(NoSamples*j),BasePointer2+(NoSamples*j),position,position2,Sample);
			*(Destination+(NoSamples*j))=Sample;

			#ifdef	__WriteWavOutgoingBuffers__
			//Debug test purposes only... test how the TBC has sampled them
			{
				static FileWriter::Wave::WaveFile wave;
				static bool Started=false;
				if (!Started)
				{
					wave.OpenForWrite("c:\\Test.wav",48000,-32,2);
					//wave.OpenForWrite("c:\\Test.wav",48000,16,2);
					Started=true;
				}
				wave.WriteMonoFloatSample(Sample);
				//wave.WriteMonoSample((short)Sample);
			}
			#endif

		}
		Destination++;

		//Advance my cursor
		m_CurrentLocalPosition+=m_CurrentTransferRate;
		m_CurrentTransferRate+=TransferRateDelta;
		//m_CurrentLocalPosition+=0.9; //testing

		//Now that I have my samples I can advance the TBC when necessary
		if (bAdvanceTBC)
		{
			m_InputStreamer.AdvanceTBC();
			m_pCurrentBuffer=l_NextBuffer;
			BuyMeSomeMoreTime=true; //will wait till the last moment to grab the next buffer

			#ifdef __WriteWavIncomingBuffers__
			//Debug test purposes only... test the incoming buffers
			{
				static FileWriter::Wave::WaveFile wave;
				static bool Started=false;
				if (!Started)
				{
					wave.OpenForWrite("c:\\Temp\\Test.wav",(DWORD)m_InputDevice->GetSampleRate(),-32,(WORD)GetNoChannels());
					Started=true;
				}
				for (size_t i=0;i<NoSamples;i++)
				{
					switch(GetNoChannels())
					{
						case 1:
							wave.WriteMonoFloatSample(BasePointer[i]);
							break;
						case 2:
							//From de-interleaved to interleaved
							wave.WriteStereoFloatSample(BasePointer[i],BasePointer[i+NoSamples]);
							break;
						default:
							assert(false);
					}
				}
			}
			#endif
		}
	}

	TimeStamp NewTime;
	QueryPerformanceCounter((LARGE_INTEGER *)NewTime);
	if ((__int64)m_LastOutBufferSentTime)
	{
		TimeStamp Frequency;
		//Compute the high precision sample rate here
		QueryPerformanceFrequency((LARGE_INTEGER *)Frequency);
		double dif=(double)(NewTime-m_LastOutBufferSentTime);
		//For best results we smooth the time intervals!
		m_CurrentOutSampleTime=(1.0-c_SmoothingRate) * m_CurrentOutSampleTime + ( c_SmoothingRate * (dif/(double)Frequency) );
		double CurrentSampleRate=(double)NoSamples / m_CurrentOutSampleTime;
		//m_OutSampleRate=(1.0-c_SmoothingRate) * m_OutSampleRate + ( c_SmoothingRate * CurrentSampleRate );
		m_OutSampleRate=CurrentSampleRate;
		//printf("Out Samples Time %f\n",m_CurrentOutSampleTime);
		//printf("Current Out Samples %f\n",CurrentSampleRate);
		//DebugOutput(L"Current Out Samples %f\n",dif/(double)Frequency);
		//DebugOutput(L"Samples %f\n",m_OutSampleRate);
	}

	m_LastOutBufferSentTime=NewTime;


	#else __TestSignal__
	{
		static double	rho=(3.1415926 / 2);
		long			 freq_hz=1000;
		float			amplitude=1.0;

		double			 theta, scale, pi2;

		size_t temp=0; //array index of buffer

		pi2 = 3.1415926;
		theta = (double)freq_hz / 44100.0;
		theta *= (pi2 * 2.0);
		pi2 *= 2.0;

		scale = (double) amplitude;

		{
			size_t				 siz=NoSamples;
			double				 t;


			while( siz-- )
			{
				t = cos( rho ) * scale;

				rho += theta;
				if ( rho > pi2 )
					rho -= pi2;

				// store channel
				*(((long *)(DestBuffer))+(temp++))=(long)(t*(double)0x7fffffff);
				*(((long *)(DestBuffer))+(temp++))=(long)(t*(double)0x7fffffff);
				//*(((float *)(DestBuffer))+(temp++))=(float)t;
				//*(((float *)(DestBuffer))+(temp++))=(float)t;

			}
		}
	}
	#endif __TestSignal__

	return true;
}



bool AudioTBC_Internal::FillBuffer(PBYTE DestBuffer,size_t BufferSizeInBytes,TimeStamp &DeviceTime)
{
	size_t BytesPerSample=aud_convert::GetFormatBitDepth(GetSampleFormat()) * GetNoChannels();  //we can assume float now
	size_t NoSamples = (BufferSizeInBytes/BytesPerSample);
	return FillBuffer(DestBuffer,NoSamples,m_OutputStreamer.GetAudioDevice(),DeviceTime);
}
