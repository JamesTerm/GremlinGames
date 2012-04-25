
#include "stdafx.h"
#include "Module.Hardware.DirectSound.Output.h"

#include <algorithm>
#include <dsound.h>

using namespace FrameWork::Debug;
using namespace FrameWork::Threads;
using namespace Modules::Hardware::DirectSound;
using namespace Modules::Hardware::HD::Base;
using namespace FC3;
using namespace std;

#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#undef  __UseTestSineWave__
#undef __DisableResampling__
#undef  __AlwaysUseThreading__

//2.1 The reference signal shall be the digital representation, in twos complement format, of a1000-Hz sine
//	wave whose positive peaks attain the value 0CCD (hex) and whose negative peaks attain F333 (hex). This is
//	20 dB below the full-scale signal, or system maximum, in which the positive peak attains the value 7FFF (hex)
//	and the negative peak attains the value 8000 (hex), considering only the 16 most significant bits.
//	The reference signal amplitude shall be denoted 20 dBFS, indicating its relation to the full-scale signal. 
const double c_output_reference_scale_factor=(1.0/10.0);

#define TestIterationRate \
{ \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	printf("rate=%f\n",((double)(currenttime-prevtime)) * 1000.0); \
	prevtime=currenttime; \
}

#define TestIterationRate2 \
{ \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	printf("\r rate=%f                ",((double)(currenttime-prevtime)) * 1000.0); \
	prevtime=currenttime; \
}

#define TestIterationRate3 \
{ \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	debug_output(p_debug_category,L"rate=%f\n",((double)(currenttime-prevtime)) * 1000.0); \
	prevtime=currenttime; \
}

//TODO move
  /***********************************************************************************************************/
 /*													DS_Input												*/
/***********************************************************************************************************/

DS_Input::DS_Input(int AudioInput,const wchar_t AudioName[])
{

}

DS_Input::~DS_Input()
{

}

namespace Modules
{
	namespace Hardware
	{
		namespace DirectSound
		{

DirectSound_Globals *g_DirectSoundInitializer=NULL;
//uncomment when DSGlobals has been finished

class DS_Output_Internal
{
	public:
		enum AudioFormatEnum
		{
			eU8, eS16, eS32, eF32
		};
		AudioFormatEnum GetFormat(WAVEFORMATEX *wfm);

		DS_Output_Internal(bool UseDisplayedCallbacks);
		~DS_Output_Internal();

		void deliver_audio(const FC3::audio::message *Message);

		virtual void StartStreaming( void );
		virtual void StopStreaming( void );
		//returns time in seconds left before needing a new packet
		double FillBuffer( void );

		void SetUseDisplayedCallbacks(bool UseDisplayedCallbacks);
		bool GetUsingThreading() const {return m_UseThreading;}

	protected:
		bool m_IsStreaming;
		size_t m_AudioOutput;

	private:
		///This class works with a pointer to frame as input and pointer to buffer for output.
		class AudioMessageConverter
		{
			public:
				AudioMessageConverter(DS_Output_Internal * _m_pParent, size_t _NoChannels, AudioFormatEnum _Format, size_t _SampleRate, size_t _BlockSize);
				~AudioMessageConverter();

				///This will store a copy in an intermediate queue
				void AddInput(const FC3::audio::message *Message);
				struct SourcePacket 
				{
					SourcePacket(const FC3::audio::message *_Message,const buffer_f32 *_BufferToProcess) :
						Message(_Message),BufferToProcess(_BufferToProcess) {}
					const FC3::audio::message *Message;  //Keep this bundled so we can use it to call displayed
					const buffer_f32 *BufferToProcess;  //This will be the resampled buffer to process
				};

				///This will grab the next frame to process (and will pop from queue) the buffer gets deleted in FillOutput()
				/// \ret this will return a packet either empty (NULL) or filled with the current pointers in the queue
				SourcePacket GetNextFrame();
				/// \param Source if this is null it will fill with silence
				void FillOutput(const buffer_f32 *Source,size_t SampleStart,PBYTE Dest,size_t SampleSize);
				/// This keeps the creation and destruction within the same class client code call this when finished with the frame
				/// This will tidy up the pointers in the packet structure properly
				void ClosePacket(SourcePacket &pkt);

				///These pertain to the output size
				size_t size( void ) const; // no samples
				size_t size_in_bytes( void ) const; // no samples * sizeof(sample_type)
			private:
				template<typename sample_type>
				void ConvertBitrate(const buffer_f32 &Source,size_t SampleStart,PBYTE Dest,size_t SampleSize);

				struct AudioMessageData
				{
					size_t NoChannels;
					AudioFormatEnum Format;
					size_t SampleRate;
					size_t BlockSize;
				} m_AudioMessageData;

				DWORD BytesRead;	//A sacred count of what was read into this buffer
				DS_Output_Internal * const m_pParent;
				const FC3::audio::message *m_AudioFrame;
				union BufferTypes
				{
					BYTE *p_u8;
					short *p_s16;
					long *p_s32;
					float *p_f32;
				};

				std::queue<SourcePacket> m_Queue;
				FrameWork::Threads::critical_section m_BlockQueue;  //protect the incoming buffer queue

				size_t m_ByteDepthSize; //cache the byte depth size
				std::vector<FrameWork::Audio2::utils::resampler *> m_Resampler; //this handles the resampling

				#ifdef __UseTestSineWave__
				void SWfreq(BufferTypes DestBuffer,size_t NoSamples,const int NoChannels,double freq_hz=1000.0,double SampleRate=48000.0,double amplitude=1.0);
				double m_rho;
				double m_rho2;
				#endif

		} m_AudioMessageConverter;

		//The extra thread is primarily for non-displayed clients which gives more tolerance for scheduling
		void operator() ( const void* );
		FrameWork::Threads::thread<DS_Output_Internal>	*m_pThread;
		friend FrameWork::Threads::thread<DS_Output_Internal>;
		FrameWork::Threads::event m_Event;

		critical_section	m_BlockDirectSound;
		critical_section m_BlockStreamingCalls;

		LPDIRECTSOUNDBUFFER m_lpdsb;
		DWORD m_BufferSizeInBytes;
		DWORD m_FillPosition; //keeps track of where the buffer has been filled
		DWORD m_PreviousFillPosition;
		__int64 m_ClockPhaseOffset;  //This tunes the phase of the clock with the audio clock

		//This is for GetTimeElapsed
		long m_GTE_LastCalled;
		bool m_UseDisplayedCallbacks;
		//This helps manage whether or not to use threading
		bool m_UseThreading;
};

class DS_Output_Core
{
	public:
		DS_Output_Core();
		~DS_Output_Core();

		//If Null this will use the desktop window
		void StartUp(HWND ParentWindow=NULL);
		void ShutDown();
		void SetWaveFormat(WAVEFORMATEX &wfx,WORD wFormatTag,WORD nChannels,DWORD nSamplesPerSecond,WORD wBitsPerSample);

		double GetTimeElapsed();

		size_t GetSampleRate( void ) const;
		size_t GetNoChannels( void ) const;
		//This is the size of the buffer (in samples) that DSound will allocate
		size_t GetBufferSampleSize( void ) const;
		size_t GetBitsPerSample( void ) const;


		WAVEFORMATEX *GetWaveFormatToUse( void );
		LPDIRECTSOUND8 GetDirectSoundObject( void );

		static DS_Output_Core &GetDS_Output_Core( void );

	protected:
		critical_section m_BlockAudioStreamsInterface;
	private:
		LPDIRECTSOUND8 m_IDS8_Interface;
		LPDIRECTSOUNDBUFFER m_IDS8_Buffer; // primary
		DWORD m_BufferSizeInBytes;
		DWORD m_FillPosition; //keeps track of where the buffer has been filled
		HANDLE m_NotificationTimer;	
		WAVEFORMATEX m_FormatToUse;

		long m_GTE_LastCalled;
};

class DirectSound_Globals
{
	public:
		DirectSound_Globals();
		~DirectSound_Globals();
		void init();  //no work is to be done within the constructor
		/// \return true if hardware is present

		DS_Output_Core &GetDS_Output_Core() {return *m_DS_Output_Core;}

	private:
		DS_Output_Core * m_DS_Output_Core;
};


		}
	}
}


  /***********************************************************************************************************/
 /*									 DS_Output_Internal::AudioMessageConverter								*/
/***********************************************************************************************************/
const size_t cDS_Output_Internal_ThresholdBlockCount=2;

DS_Output_Internal::AudioMessageConverter::AudioMessageConverter(DS_Output_Internal * _m_pParent, size_t _NoChannels, AudioFormatEnum _Format, size_t _SampleRate, size_t _BlockSize)
: m_pParent(_m_pParent)
{
	AudioMessageData &_=m_AudioMessageData;
	_.NoChannels = _NoChannels;
	_.Format = _Format;
	_.BlockSize = _BlockSize;
	_.SampleRate = _SampleRate;
	switch (m_AudioMessageData.Format)
	{
		case eU8:			m_ByteDepthSize=sizeof(BYTE);			break;
		case eS16:			m_ByteDepthSize=sizeof(short);			break;
		case eS32:			m_ByteDepthSize=sizeof(long);			break;
		case eF32:			m_ByteDepthSize=sizeof(float);			break;
	}
	#ifdef __UseTestSineWave__
	m_rho=m_rho2=0;
	#endif

	#ifndef __DisableResampling__
	//Instantiate the resampler one per channel
	assert ((m_Resampler.size()==0)||(m_Resampler.size()==_NoChannels));
	for (size_t i=0;i<_NoChannels;i++)
	{
		if (m_Resampler.size()<=i)
		{
			//Set up new one
			m_Resampler.push_back(new resampler((float)_SampleRate,(float)_SampleRate));
		}
		else
			m_Resampler[i]->set_sample_rates((float)_SampleRate,(float)_SampleRate); //ensure these are updated in case rate has changed
	}
	#endif 
}

void DS_Output_Internal::AudioMessageConverter::ClosePacket(SourcePacket &pkt)
{
	delete pkt.BufferToProcess;
	if (pkt.Message)
		pkt.Message->release();
}

DS_Output_Internal::AudioMessageConverter::~AudioMessageConverter()
{
	while(!m_Queue.empty())
	{
		SourcePacket &pkt=m_Queue.front();
		ClosePacket(pkt);
		m_Queue.pop();
	}
	for (size_t i=0;i<m_Resampler.size();i++)
	{
		//This is just a idiot check if the resampler is disabled
		#ifdef __DisableResampling__
		assert (false);
		#endif
		assert(m_Resampler[i]);
		delete m_Resampler[i];
		m_Resampler[i]=NULL;
	}
	m_Resampler.clear();

}

#ifdef __UseTestSineWave__
void DS_Output_Internal::AudioMessageConverter::SWfreq(BufferTypes DestBuffer,size_t NoSamples,const int NoChannels,double freq_hz,double SampleRate,double amplitude)
{
	double			 theta,theta2;
	size_t index=0; //array index of buffer

	const double pi2 = 3.1415926 * 2.0;
	//Compute the angle ratio unit we are going to use
	//Multiply times pi 2 to Convert the angle ratio unit into radians
	theta = (freq_hz / SampleRate) * pi2;
	theta2 = ((freq_hz*0.5) / SampleRate) * pi2;


	//set our scale... this is also the size of the radius 
	const double scale = amplitude;

	{
		size_t				 siz=NoSamples;
		double				 Sample,Sample2;


		while( siz-- )
		{
			//Find Y given the hypotenuse (scale) and the angle (rho)
			//Note: using sin will solve for Y, and give us an initial 0 size
			Sample = sin( m_rho ) * scale;
			Sample2 = sin (m_rho2) * scale;
			//increase our angular measurement
			m_rho += theta;
			m_rho2 += theta2;
			//bring back the angular measurement by the length of the circle when it has completed a revolution
			if ( m_rho > pi2 )
				m_rho -= pi2;
			if ( m_rho2 > pi2 )
				m_rho2 -= pi2;

			for (size_t j=0;j<NoChannels;j++)
			{
				double s=(j&1)?Sample2:Sample;  //odd channels use sample2
				switch (m_AudioMessageData.Format)
				{
				case eU8:
					*(((DestBuffer.p_u8))+(index++))=(BYTE)((s*(double)0x7f)+0x80);
					break;
				case eS16:
					*(((DestBuffer.p_s16))+(index++))=(short)(s*(double)0x7fff);
					break;
				case eS32:
					*(((DestBuffer.p_s32))+(index++))=(long)(s*(double)0x7fffffff);
					break;
				case eF32:
					*(((DestBuffer.p_f32))+(index++))=(float)s;
					break;
				}
			}
		}
	}
}
#endif

template<typename sample_type>
void DS_Output_Internal::AudioMessageConverter::ConvertBitrate(const buffer_f32 &Source,size_t SampleStart,PBYTE Dest,size_t SampleSize)
{
	using namespace FrameWork::Audio2;
	size_t NoChannels=m_AudioMessageData.NoChannels;

	//Now to convert the Frame to our Hardware format
	buffer<sample_type> DeInterleaved((int)NoChannels,(int)SampleSize,16);  //align for best SSE2 conversions
	for (int i=0;i<(int)NoChannels;i++)
	{
		//Now we have an de-interleaved s32 converted format with compatible number of channels
		if (i<Source.no_channels())
		{
			buffer<sample_type>(DeInterleaved(i),1,(int)SampleSize)=buffer_f32(&Source(i,(int)SampleStart),1,(int)SampleSize);
		}
		else
			FrameWork::Bitmaps::memset(DeInterleaved(i),0,DeInterleaved.no_samples_in_bytes()); //fill with silence
	}
	//Now to interleave it
	utils::convert_to_interleaved<sample_type>(DeInterleaved,(sample_type *)Dest);
}

void DS_Output_Internal::AudioMessageConverter::AddInput(const FC3::audio::message *Message)
{
	//apply sample rate conversion
	#ifndef __DisableResampling__

	m_Resampler[0]->set_sample_rates((float)Message->sample_rate(),(float)m_AudioMessageData.SampleRate);
	size_t NoSamples=(size_t)m_Resampler[0]->get_max_buffer_length_in_samples(Message->audio().no_samples());

	//determine number of channels that we will have (this way we can pre-allocate how many sample converters to create)
	int no_channels=min(Message->audio().no_channels(),(int)m_AudioMessageData.NoChannels);
	//I had better use the heap since I can have large buffers :(
	buffer_f32 *ResampledSource=NULL;
	buffer_f32 ResampledChannel(1,(int)NoSamples,16);

	size_t ActualSampleSize;
	for (int i=0;i<no_channels;i++)
	{
			//Now to setup the resampler
			resampler::dst_data_desc dest;
			resampler::src_data_desc src;
			src.first=Message->audio()(i);
			src.second=src.first + Message->audio().no_samples();
			dest.first=ResampledChannel();
			dest.second=dest.first + NoSamples;
			//Dynamically set the source rate
			m_Resampler[i]->set_sample_rates((float)Message->sample_rate(),(float)m_AudioMessageData.SampleRate);
			//Invoke the resampler operation
			resampler::e_reason result=(*m_Resampler[i])(src,dest);
			assert(result==resampler::e_source_empty);

			//keep track of the actual size
			ActualSampleSize=dest.first-ResampledChannel();

			//Note: in the past I'd monitor how often the channels will be different sizes from each other
			//but after years of testing with positive result I'm going to skip this 
			//(see HD_Output_Internal::AudioBuffer::AudioMessageConverter::SetInput())

			//Now that we have the actual sample size we can create the buffer (this is typically a few samples smaller, and never larger)
			if (i==0)
				ResampledSource=new buffer_f32(no_channels,(int)ActualSampleSize,16);
			buffer_f32((*ResampledSource)(i),1,(int)ActualSampleSize)=buffer_f32(ResampledChannel(),1,(int)ActualSampleSize);
	}
	#else
	size_t ActualSampleSize=Message->audio().no_samples();
	buffer_f32 *ResampledSource=new buffer_f32(Message->audio());
	#endif

	//While this is needed for our hardware... it apparently does not apply to direct sound 
	//const_cast<buffer_f32 &>(Message->audio()) *= c_output_reference_scale_factor;

	//Add to queue here
	auto_lock Threadsafe(m_BlockQueue);
	Message->addref();  //hold on to message for displayed
	m_Queue.push(SourcePacket(Message,ResampledSource));
	//Overflow management... note: this should only happen in threaded mode and there is another check for the fc3 messages in DS_Output::deliver_audio()
	while (m_Queue.size() > cDS_Output_Internal_ThresholdBlockCount)
	{
		debug_output(p_debug_category,L"AudioMessageConverter::AddInput overflow %d.. discarding\n",m_Queue.size());
		SourcePacket &pkt=m_Queue.front();
		ClosePacket(pkt);
		m_Queue.pop();
	}
}

DS_Output_Internal::AudioMessageConverter::SourcePacket DS_Output_Internal::AudioMessageConverter::GetNextFrame()
{
	auto_lock Threadsafe(m_BlockQueue);
	SourcePacket ret(NULL,NULL);
	if (!m_Queue.empty())
	{
		ret=m_Queue.front();  //copy the contents (these are all pointer not owned by this structure)
		m_Queue.pop();
	}
	return ret;
}

void DS_Output_Internal::AudioMessageConverter::FillOutput(const buffer_f32 *Source,size_t SampleStart,PBYTE Dest,size_t SampleSize)
{
	const size_t BlockSize=m_AudioMessageData.BlockSize;
	size_t NoChannels=m_AudioMessageData.NoChannels;
	//Sanity check
	assert(Dest);

	if (Source)  //underflow check
	{
		//apply bit rate conversion
		switch (m_AudioMessageData.Format)
		{
		case eU8:
			ConvertBitrate<unsigned char>(*Source,SampleStart,Dest,SampleSize);
			break;
		case eS16:
			ConvertBitrate<short>(*Source,SampleStart,Dest,SampleSize);
			break;
		case eS32:
			ConvertBitrate<int>(*Source,SampleStart,Dest,SampleSize);
			break;
		case eF32:
			ConvertBitrate<float>(*Source,SampleStart,Dest,SampleSize);
			break;
		}
	}
	else
	{
		//debug_output(p_debug_category,L"AudioMessageConverter::FillOutput underflow, filling in with silence\n");
		if (m_AudioMessageData.Format!=eU8)
			memset(Dest,0,SampleSize*m_ByteDepthSize * m_AudioMessageData.NoChannels);
		else
			memset(Dest,128,SampleSize*m_ByteDepthSize * m_AudioMessageData.NoChannels);
	}

	#ifdef __UseTestSineWave__
	{
		BufferTypes btDest;
		btDest.p_u8=Dest;
		SWfreq(btDest,SampleSize,(int)NoChannels,1000.0,(int)m_AudioMessageData.SampleRate,0.5);
	}
	#endif
}


size_t DS_Output_Internal::AudioMessageConverter::size( void ) const
{
	return m_AudioMessageData.BlockSize;
}
size_t DS_Output_Internal::AudioMessageConverter::size_in_bytes( void ) const
{
	return m_AudioMessageData.BlockSize * m_ByteDepthSize * m_AudioMessageData.NoChannels;	
}


  /*************************************************************************/
 /*							 DS_Output_Internal							  */			
/*************************************************************************/

DS_Output_Internal::AudioFormatEnum DS_Output_Internal::GetFormat(WAVEFORMATEX *wfm)
{
	AudioFormatEnum ret;
	size_t BitDepth=DS_Output_Core::GetDS_Output_Core().GetBitsPerSample();
	switch (BitDepth)
	{
		case 8:		ret=eU8;	break;
		case 16:	ret=eS16;	break;
		case 32:
			WORD wfm_tag=wfm->wFormatTag;
			if (wfm_tag==WAVE_FORMAT_IEEE_FLOAT)
				ret=eF32;
			else
			{
				assert(wfm_tag==WAVE_FORMAT_PCM);  //sanity check
				ret=eS32;
			}
			break;
	}
	return ret;
}


DS_Output_Internal::DS_Output_Internal(bool UseDisplayedCallbacks) : 
	m_IsStreaming(false),m_AudioOutput(),
	m_AudioMessageConverter(this,
	DS_Output_Core::GetDS_Output_Core().GetNoChannels(),
	GetFormat(DS_Output_Core::GetDS_Output_Core().GetWaveFormatToUse()),
	DS_Output_Core::GetDS_Output_Core().GetSampleRate(),
	DS_Output_Core::GetDS_Output_Core().GetBufferSampleSize()),
	m_pThread(NULL),m_ClockPhaseOffset(0),
	m_GTE_LastCalled(0),m_UseDisplayedCallbacks(UseDisplayedCallbacks)
{ 

	#ifdef __AlwaysUseThreading__
	m_UseThreading=true;
	#else
	m_UseThreading=UseDisplayedCallbacks;
	#endif

	DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
	//We shouldn't make this range the same value
	m_FillPosition=0;
	m_PreviousFillPosition=(DWORD)DSOC.GetBufferSampleSize() * DSOC.GetWaveFormatToUse()->nBlockAlign;

	//Set up the secondary buffer
	DSBUFFERDESC dsbdesc;
	INIT_DIRECTX_STRUCT(dsbdesc);
	dsbdesc.dwFlags         = 
		DSBCAPS_GLOBALFOCUS			|	//If my window is minimized I'm still going to be heard!
		DSBCAPS_STATIC				|	//Use hardware buffers if I can
		DSBCAPS_GETCURRENTPOSITION2;	//We are a new product use the best accurate code

	WAVEFORMATEX *FormatToUse=DSOC.GetWaveFormatToUse();
	dsbdesc.dwBufferBytes=m_BufferSizeInBytes=(DWORD)(FormatToUse->nBlockAlign * (DSOC.GetBufferSampleSize()));
	dsbdesc.lpwfxFormat=FormatToUse;
	try
	{
		if FAILED(DSOC.GetDirectSoundObject()->CreateSoundBuffer(&dsbdesc,&m_lpdsb,NULL))
			assert(false),throw"Unable to Create Secondary Sound Buffer" __FUNCSIG__;

		{	//Fill our buffer with silence
			void *dsbuf1,*dsbuf2;
			DWORD dsbuflen1,dsbuflen2;

			if FAILED(m_lpdsb->Lock(0,m_BufferSizeInBytes,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0))
				assert(false),throw"Unable to lock secondary buffer" __FUNCSIG__;
			if (dsbuf1)
				FillMemory((char *)dsbuf1,dsbuflen1,(BYTE)(FormatToUse->wBitsPerSample == 8 ? 128 : 0));
			if (dsbuf2)
				FillMemory((char *)dsbuf2,dsbuflen2,(BYTE)(FormatToUse->wBitsPerSample == 8 ? 128 : 0));
			if FAILED(m_lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
				assert(false),throw"Unable to unlock secondary buffer" __FUNCSIG__;
		}
		//Might as well start playing it
		m_lpdsb->SetCurrentPosition(0);
	}
	catch (char *Error)  //Close resources in constructor throws
	{
		if (m_lpdsb)
		{
			m_lpdsb->Stop();
			m_lpdsb->Release();
			m_lpdsb=NULL;
		}
		throw Error;
	}
}

DS_Output_Internal::~DS_Output_Internal()
{
	if (m_lpdsb)
	{
		m_lpdsb->Stop();
		m_lpdsb->Release();
		m_lpdsb=NULL;
	}
}

void DS_Output_Internal::StopStreaming( void ) 
{
	if (m_IsStreaming)
	{
		//I may or may not have a thread
		delete m_pThread;
		m_pThread=NULL;

		m_lpdsb->Stop();
		m_IsStreaming=false;
	}
}

void DS_Output_Internal::StartStreaming( void )
{
	if (!m_IsStreaming)
	{
		m_lpdsb->Play(0,0,DSBPLAY_LOOPING);
		m_IsStreaming=true;
		if (m_UseThreading)
		{
			m_pThread = new thread<DS_Output_Internal>(this);
			m_pThread->high_priority();
			m_pThread->set_thread_name( "Module.DirectSound.Output Main Thread" );
		}
		//now to set up the initial clock phase with the audio clock

		{
			DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
			DWORD BlockAlign=DSOC.GetWaveFormatToUse()->nBlockAlign;
			//m_BufferSampleSize=DSOC.GetBufferSampleSize();
			double SampleRate=(double)DSOC.GetSampleRate();

			Sleep(16); //give some time for the cursor to get going

			DWORD playpos;
			m_lpdsb->GetCurrentPosition(&playpos,NULL);
			playpos/=BlockAlign; //convert to samples

			time_type CurrentTime=time_type::get_current_time();
			time_type CPUClock=((__int64)CurrentTime) % 10000000;
			double CalibratePlayPos=(double)CPUClock*SampleRate;
			int PhaseOffset=(int)playpos-(int)CalibratePlayPos;
			//printf("playpos %d, %d  phase=%d \n",playpos,(size_t)(CalibratePlayPos),PhaseOffset);
			//m_ClockPhaseOffset= ((__int64)PhaseOffset * 10000000) / (__int64)DSOC.GetSampleRate();
			m_ClockPhaseOffset=(__int64)(((double)PhaseOffset / (double)DSOC.GetSampleRate()) * 10000000.0);

			//Check my math :)
			//CPUClock=((__int64)CurrentTime + m_ClockPhaseOffset) % 10000000;
			//CalibratePlayPos=(double)CPUClock*SampleRate;
			//printf("Test-> playpos %d, %d \n",playpos,(size_t)(CalibratePlayPos));
		}
	}
}

void DS_Output_Internal::SetUseDisplayedCallbacks(bool UseDisplayedCallbacks) 
{
	StopStreaming();
	#ifdef __AlwaysUseThreading__
	m_UseThreading=true;
	#else
	m_UseThreading=UseDisplayedCallbacks;
	#endif
	m_UseDisplayedCallbacks=UseDisplayedCallbacks;
	StartStreaming();
}

void DS_Output_Internal::deliver_audio(const FC3::audio::message *Message)
{
	if (m_IsStreaming)
	{
		m_AudioMessageConverter.AddInput(Message);
		if (m_UseThreading)
			m_Event.set();
		else
			FillBuffer();
	}
}

double DS_Output_Internal::FillBuffer()
{
	class Internal
	{
		public:
			Internal(LPDIRECTSOUNDBUFFER lpdsb,__int64 &ClockPhaseOffset) : m_lpdsb(lpdsb),
				m_ClockPhaseOffset(ClockPhaseOffset)
			{
				DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
				m_BlockAlign=DSOC.GetWaveFormatToUse()->nBlockAlign;
				m_BufferSampleSize=DSOC.GetBufferSampleSize();
				m_SampleRate=(double)DSOC.GetSampleRate();
			}
			///This works in sample units
			size_t AdvancePosition(size_t CurrentFillSamplePosition,size_t AdvInSamples)
			{
				size_t ret=CurrentFillSamplePosition+AdvInSamples;
				//since the buffer size may not be a power of 2 we'll need to branch (no biggy)
				//If we overflow subtract the length of the buffer
				if (ret>=m_BufferSampleSize)
					ret-=m_BufferSampleSize;
				return ret;
			}
			size_t GetPlayPos()
			{
				DWORD playpos;
				//size_t SampleOffset=0;
				size_t SampleOffset=(size_t)(0.030 * m_SampleRate);
				m_lpdsb->GetCurrentPosition(&playpos,NULL);
				playpos/=m_BlockAlign; //convert to samples

				//Note: This design assumes the buffer size (of our secondary buffer) is the same size as the sample rate.  This is fine for now, but at some
				//point I may need to make a distinction if I have to change the buffer size.
				
				time_type CPUClock=((__int64)time_type::get_current_time()+m_ClockPhaseOffset) % 10000000;
				double CalibratePlayPos=(double)CPUClock*m_SampleRate;
				int PhaseOffset=(int)playpos-(int)CalibratePlayPos;
				int iSampleRate=(int)m_SampleRate;
				//Check wrap-around case
				if (PhaseOffset > iSampleRate>>1)
					PhaseOffset= ((int)playpos-iSampleRate) - (int)CalibratePlayPos;
				else if (PhaseOffset < -(iSampleRate>>1))
					PhaseOffset= playpos - ((int)CalibratePlayPos-iSampleRate);

				double Current_ClockPhaseOffset=(double)m_ClockPhaseOffset + (((double)PhaseOffset / m_SampleRate) * 10000000.0);

				//check math
				//CPUClock=((__int64)time_type::get_current_time() + (__int64)Current_ClockPhaseOffset) % 10000000;
				//CalibratePlayPos=(double)CPUClock*m_SampleRate;
				//printf("Test-> playpos %d, %d \n",playpos,(size_t)(CalibratePlayPos));

				const double dSmoothingValue=0.1;
				//blend the current phase error to the current phase offset
				m_ClockPhaseOffset=(__int64) (((1.0-dSmoothingValue) * (double)m_ClockPhaseOffset) + (dSmoothingValue * Current_ClockPhaseOffset));
				//debug_output(p_debug_category,L"playpos %d, %d, %d\n",playpos,(size_t)(CalibratePlayPos),PhaseOffset);
				size_t Error=abs(PhaseOffset);
				//printf("\r%d          ",Error);
				//if (Error>1000)
				//	debug_output(p_debug_category,L"%d\n",Error);

				return AdvancePosition((size_t)(CalibratePlayPos),SampleOffset);
			}
			///Note in a circular buffer the begin range may be greater than the end range
			///This is all in samples
			/// \ret 0=in range 
			///		   less than 0 is somewhere before the range  
			///		greater than 0 is somewhere after  the range
			int IsInRange(size_t playpos, size_t BeginRange,size_t EndRange)
			{
				//geez.. there is a roladex going in my mind how to solve this.  This technique well branch into certain cases and use the
				//known assumptions to their fullest to solve.  This is probably a simplistic approach with the cost of some redundancy.
				int ret=0;
				double UnselectedRangePosition=-1; //If playpos is in the unselected range this will point position in a normalized scale
				size_t UnselectedRangeSize;
				//Determine what kind of range we have
				if (BeginRange<EndRange)  //normal range  -------BxxxxxE-------
				{
					size_t NormalRangeEndPointToEnd=m_BufferSampleSize-EndRange;
					UnselectedRangeSize=BeginRange + NormalRangeEndPointToEnd;
					if (playpos<BeginRange)
						UnselectedRangePosition=(double)(playpos+NormalRangeEndPointToEnd)/ (double)UnselectedRangeSize;
					else if (playpos>EndRange)
						UnselectedRangePosition=((double)(playpos-EndRange))/(double)UnselectedRangeSize;
				}
				else if (EndRange<BeginRange) //inverted range  xxxxE---------Bxxxx
				{
					UnselectedRangeSize=BeginRange - EndRange;
					if ((playpos>EndRange) && (playpos<BeginRange))
						UnselectedRangePosition=(double)(playpos-EndRange) / (double)UnselectedRangeSize;
				}
				else
					assert(false);  // this should not be happening
				int ScaledPosition=(int)((double)UnselectedRangePosition * UnselectedRangeSize);
				return UnselectedRangePosition==-1?  0 :
					(UnselectedRangePosition<0.5)?  ScaledPosition :  //this many samples after the end
					ScaledPosition - (int)UnselectedRangeSize; //This many samples before the beginning (will be negative)
			}
			//wait until the play cursor has hit within the range
			double WaitForPlay(size_t BeginRange,size_t EndRange)
			{
				#undef __WaitForPlay_Verbose__
				#ifdef __WaitForPlay_Verbose__
				vector<size_t> playpos_list;
				#endif
				//Get current play position
				size_t playpos,playpos_latencyguard;
				int result;
				{
					//This piece will give more cpu time by giving a larger sleep
					#if 1
					playpos=GetPlayPos();
					//playpos_latencyguard=AdvancePosition(playpos,(size_t)(0.004 * m_SampleRate));
					playpos_latencyguard=playpos;
					double SleepTime_ms= (((double)BeginRange-(double)playpos) * 1.0 / m_SampleRate) * 1000.0;
					DWORD dwSleepTime_ms=(SleepTime_ms>=1.0)?(DWORD)SleepTime_ms:1;
					//printf("%d\n",dwSleepTime_ms);
					//debug_output(p_debug_category,L"%d\n",dwSleepTime_ms);
					Sleep(dwSleepTime_ms);
					result=IsInRange(playpos_latencyguard,BeginRange,EndRange);
					//We need to ensure we have slept enough so we'll poll to see where we are... typically this may happen once after the initial big sleep
					if (result<0)
					#endif
					{
						do 
						{
							Sleep(1);
							//printf("Sleep 1\n");
							playpos=GetPlayPos();
							//The latency guard was needed for the old clocking mechanism of adding time for redundant cursor positions
							//It appears the new clocking mechanism no longer needs this; however, I'll want to keep this around incase the
							//overshooting problem happens in rare cases
							//playpos_latencyguard=AdvancePosition(playpos,(size_t)(0.001 * m_SampleRate));
							playpos_latencyguard=playpos;
							result=IsInRange(playpos_latencyguard,BeginRange,EndRange);
							#ifdef __WaitForPlay_Verbose__
							playpos_list.push_back(playpos);
							#endif
						} while (result<0);
					}
				}
				double ret=0.0;
				if (result==0)
				{
					double SampleTime=1.0/m_SampleRate;
					if (BeginRange<EndRange)
						ret=(double)(EndRange-playpos_latencyguard)* SampleTime;
					else
					{
						size_t NormalRangeEndPointToEnd=m_BufferSampleSize-BeginRange;
						size_t InvertedRangeSize=EndRange + NormalRangeEndPointToEnd;

						if (playpos_latencyguard>=BeginRange)
							ret=double(InvertedRangeSize - (playpos_latencyguard-BeginRange)) * SampleTime;
						else
							ret= double(InvertedRangeSize - (playpos_latencyguard + NormalRangeEndPointToEnd)) * SampleTime;
					}
					#ifdef __WaitForPlay_Verbose__
					DWORD playpos;
					m_lpdsb->GetCurrentPosition(&playpos,NULL);
					playpos/=m_BlockAlign; //convert to samples
					//if (playpos < playpos_list[playpos_list.size()-1])
					if (false)
					{
						wstring sList;
						wchar_t Buffer[16];
						for (size_t i=0;i<playpos_list.size();i++)
						{
							_itow((int)playpos_list[i],Buffer,10);
							sList+=Buffer;
							sList+=L",";
						}
						_itow((int)playpos,Buffer,10);
						sList+=L"Actual(";
						sList+=Buffer;
						sList+=L")";

						debug_output(p_debug_category,L"%s\n",sList.c_str());
					}
					#endif

				}
				else if (result>0)
				{
					result=IsInRange(playpos,BeginRange,EndRange);
					if (result>0)
					{
						debug_output(p_debug_category,L"FillBuffer wait for play overshot %d samples \n",result);
						#ifdef __WaitForPlay_Verbose__
						debug_output(p_debug_category,L"range from %d to %d\n",BeginRange,EndRange);
						wstring sList;
						wchar_t Buffer[16];
						for (size_t i=0;i<playpos_list.size();i++)
						{
							_itow((int)playpos_list[i],Buffer,10);
							sList+=Buffer;
							sList+=L",";
						}
						debug_output(p_debug_category,L"%s\n",sList.c_str());
						#endif
					}
				}

				return ret;
			}
		private:
			LPDIRECTSOUNDBUFFER m_lpdsb;
			DWORD m_BlockAlign;
			size_t m_BufferSampleSize;
			double m_SampleRate;
			__int64 &m_ClockPhaseOffset;
			double m_ClockScalar;
	} _(m_lpdsb,m_ClockPhaseOffset);

	double ret=0.0;  //if we are not streaming this value will not really matter
	DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
	DWORD BlockAlign=DSOC.GetWaveFormatToUse()->nBlockAlign;

	if (m_IsStreaming)
	{
		double SampleRate=(double)DSOC.GetSampleRate();
		//Get current play position
		size_t playpos=_.GetPlayPos();
		size_t WritePos=m_FillPosition/BlockAlign;

		//Note from previous to fill is the area that was just filled where the fill position is set to the beginning point where it is about to get filled
		//check if the position is in its correct range.  It should be in this range; however, due to the nature of the cursor skipping an update to
		//20ms intervals, it is possible that the cursor is on that phase and would show a stagnant update from the previous iteration, this should
		//be fine since it doesn't impact the write position, and will get resolve during the wait.  All we really care about here is if the
		//play cursor overshot
		if ((_.IsInRange(playpos,m_PreviousFillPosition/BlockAlign,WritePos))>0)
		{
			debug_output(p_debug_category,L"Error not in range cursor=%d,from=%d,to=%d\n",playpos,m_PreviousFillPosition/BlockAlign,WritePos);
			//Place fill position about 20ms of time ahead of where we are now
			m_FillPosition=((DWORD)_.AdvancePosition(playpos,(size_t)(0.020 * SampleRate))) * BlockAlign;
		}

		//printf("%d play=%d write=%d\n",WritePos-playpos,playpos,WritePos);
		//Grab the next frame to process we'll need its size to lock the buffer
		AudioMessageConverter::SourcePacket SourcePkt=m_AudioMessageConverter.GetNextFrame();
		const buffer_f32 *Source=SourcePkt.BufferToProcess;
		size_t SampleSizeToFill=Source?Source->no_samples():(size_t)(SampleRate * 0.020);

		//Attempt to lock
		size_t TimeOut=0;
		bool LockStatus=false;
		void *dsbuf1,*dsbuf2;
		DWORD dsbuflen1,dsbuflen2;
		do
		{
			if (m_lpdsb->Lock(m_FillPosition,(DWORD)SampleSizeToFill*BlockAlign,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)==DS_OK)
			{
				LockStatus=true;
			}
			else
				Sleep(1);  //retry
		} while ((TimeOut++<5)&&(LockStatus==false));

		if (LockStatus)
		{
			//Success fill the buffer
			size_t Sample1Length=dsbuflen1/BlockAlign;
			assert(dsbuf1); //sanity check, should always have this if the lock was successful
			m_AudioMessageConverter.FillOutput(Source,0,(PBYTE)dsbuf1,Sample1Length);
			//May have a the wrap around case buffer, so we fill it accordingly
			if (dsbuf2)
				m_AudioMessageConverter.FillOutput(Source,Sample1Length,(PBYTE)dsbuf2,dsbuflen2/BlockAlign);
			if FAILED(m_lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
				printf("Unable to unlock DS buffer\n");
		}
		else
			debug_output(p_debug_category,L"Error cursor=%d,buf1=%p,buf1len=%d,buf2=%p,buf2len=%d\n",playpos,dsbuf1,dsbuflen1,dsbuf2,&dsbuflen2);

		//Advance the position markers to the new target play region
		m_PreviousFillPosition=m_FillPosition;
		//The advance works in sample units
		m_FillPosition=((DWORD)_.AdvancePosition(m_FillPosition/BlockAlign,SampleSizeToFill)) * BlockAlign;
		//Wait for play to hit our new filled range
		ret=_.WaitForPlay(m_PreviousFillPosition/BlockAlign,m_FillPosition/BlockAlign);
		//We should be in the play cursor range now we can send the displayed
		if ((m_UseDisplayedCallbacks)&&(SourcePkt.Message))
			SourcePkt.Message->displayed();

		//clean / tidy up
		m_AudioMessageConverter.ClosePacket(SourcePkt);
	}
	return ret;
}

void DS_Output_Internal::operator() ( const void* )
{
	double TimeOut=FillBuffer(); //notify our stream to update
	//printf("%f\n",TimeOut*1000.0);
	if (TimeOut>0.0)
		m_Event.wait((DWORD)(TimeOut * 500.0));  //only wait half the time available (give it enough time to fill too)
}

  /************************************************************************/
 /*							  DS_Output_Core							 */
/************************************************************************/
DS_Output_Core::DS_Output_Core()
:m_IDS8_Interface(NULL)
{
	m_NotificationTimer=CreateWaitableTimer(NULL,false,NULL);
	assert(m_NotificationTimer);
	m_FillPosition=(DWORD)GetBufferSampleSize(); //this way the first buffer will get filled when it starts
}
DS_Output_Core::~DS_Output_Core()
{
	ShutDown();
	CloseHandle(m_NotificationTimer);
}
void DS_Output_Core::SetWaveFormat(WAVEFORMATEX &wfx,WORD wFormatTag,WORD nChannels,DWORD nSamplesPerSecond,WORD wBitsPerSample)
{
	memset(&wfx,0,sizeof(WAVEFORMATEX));
	wfx.wFormatTag=wFormatTag;
	wfx.nChannels=nChannels;
	wfx.nSamplesPerSec=nSamplesPerSecond;
	wfx.wBitsPerSample=wBitsPerSample;
	wfx.nBlockAlign=wfx.wBitsPerSample/8*wfx.nChannels;
	wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
}

size_t DS_Output_Core::GetSampleRate( void ) const 
{	
	return (size_t)m_FormatToUse.nSamplesPerSec;
}
size_t DS_Output_Core::GetNoChannels( void ) const 
{	
	return (size_t)m_FormatToUse.nChannels;
}
size_t DS_Output_Core::GetBufferSampleSize( void ) const
{	
	//There size is really flexible, we just want it large enough to avoid it circling too much 1 second seems to be good
	return (size_t)m_FormatToUse.nSamplesPerSec;
}
size_t DS_Output_Core::GetBitsPerSample( void ) const
{
	return (size_t)m_FormatToUse.wBitsPerSample;
}

double DS_Output_Core::GetTimeElapsed( void )
{
	DWORD playpos;
	//assert m_IDS8_Buffer
	m_IDS8_Buffer->GetCurrentPosition(&playpos,NULL);
	playpos/=GetWaveFormatToUse()->nBlockAlign; //convert to samples

	long PointerDifference=long(playpos)-m_GTE_LastCalled;
	if (PointerDifference<0.0)
	{
		size_t BufferSize=GetBufferSampleSize()<<1;
		PointerDifference=((long)BufferSize-m_GTE_LastCalled) + (long)playpos;
	}
	m_GTE_LastCalled=(long)playpos;
	return (double)PointerDifference / (double)GetSampleRate();
	//return (double)PointerDifference;
 
}

WAVEFORMATEX *DS_Output_Core::GetWaveFormatToUse( void ) 
{	
	return &m_FormatToUse;
}

LPDIRECTSOUND8 DS_Output_Core::GetDirectSoundObject( void ) 
{
	return m_IDS8_Interface;
}
DS_Output_Core &DS_Output_Core::GetDS_Output_Core( void ){
	return g_DirectSoundInitializer->GetDS_Output_Core();
}
void DS_Output_Core::StartUp(HWND ParentWindow)
{
	m_IDS8_Buffer=NULL;
	{
		if (FAILED(DirectSoundCreate8(NULL,&m_IDS8_Interface,NULL)))
			assert(false),throw "Unable to create DirectSound object" __FUNCSIG__;

		//Note: Cooperative level of DSSCL priority allows formating the primary buffer
		HWND hwndToUse=(ParentWindow)?ParentWindow:GetDesktopWindow();
		if (FAILED(m_IDS8_Interface->SetCooperativeLevel(hwndToUse,DSSCL_PRIORITY)))
			assert(false),throw "Unable to set CooperativeLevel in DS" __FUNCSIG__;

		//Obtain primary buffer
		DSBUFFERDESC dsbdesc;
		INIT_DIRECTX_STRUCT(dsbdesc);
		dsbdesc.dwFlags=DSBCAPS_PRIMARYBUFFER;
		if FAILED(m_IDS8_Interface->CreateSoundBuffer(&dsbdesc,&m_IDS8_Buffer,NULL))
			assert(false),throw "Unable to create DS primary sound buffer" __FUNCSIG__;
		// stuff already written

		{ //Well try some formats (until I can figure out how to do this better!)
			size_t i=0;
			const size_t NoElements=18;
			struct formatdata
			{
				WORD wFormatTag;
				WORD nChannels;
				DWORD nSamplesPerSecond;
				WORD wBitsPerSample;

			} trythis[NoElements]=
			{
				{WAVE_FORMAT_IEEE_FLOAT,4,96000,32},//....01
				{WAVE_FORMAT_PCM,4,96000,32},//...........02
				{WAVE_FORMAT_PCM,4,96000,16},//...........04
				{WAVE_FORMAT_IEEE_FLOAT,2,96000,32},//....08
				{WAVE_FORMAT_PCM,2,96000,32},//...........10
				{WAVE_FORMAT_PCM,2,96000,16},//...........20

				{WAVE_FORMAT_IEEE_FLOAT,4,48000,32},//....01
				{WAVE_FORMAT_PCM,4,48000,32},//...........02
				{WAVE_FORMAT_PCM,4,48000,16},//...........04
				{WAVE_FORMAT_IEEE_FLOAT,2,48000,32},//....08
				{WAVE_FORMAT_PCM,2,48000,32},//...........10
				{WAVE_FORMAT_PCM,2,48000,16},//...........20

				{WAVE_FORMAT_IEEE_FLOAT,4,44100,32},//....40
				{WAVE_FORMAT_PCM,4,44100,32},//...........80
				{WAVE_FORMAT_PCM,4,44100,16},//..........100
				{WAVE_FORMAT_IEEE_FLOAT,2,44100,32},//...200
				{WAVE_FORMAT_PCM,2,44100,32},//..........400
				{WAVE_FORMAT_PCM,2,44100,16}//...........800

			};

			DSCAPS capabilities;
			INIT_DIRECTX_STRUCT(capabilities);

			unsigned short filter=0xffff; //16 bits... 16 things to try
			if (m_IDS8_Interface->GetCaps(&capabilities)==DS_OK)
			{
				filter=0x01|0x02|0x08|0x10|0x40|0x80|0x200|0x400; 
				DWORD MinSampleRate=capabilities.dwMinSecondarySampleRate;
				DWORD MaxSampleRate=capabilities.dwMaxSecondarySampleRate;
				if  (!(
					(MaxSampleRate==48000) || (MinSampleRate=48000) ||
					((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>48000)&&(MinSampleRate<48000))
					))
					filter &= (0x01|0x02|0x04|0x08|0x10|0x20);

				if (capabilities.dwFlags & DSCAPS_SECONDARY16BIT)
				{
					if (capabilities.dwFlags & DSCAPS_SECONDARYSTEREO)
					{
						if  (
							(MaxSampleRate==48000) || (MinSampleRate=48000) ||
							((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>48000)&&(MinSampleRate<48000))
							)
							filter |= (0x04|0x20); //16bit 48000 for 2 and 4 channel

						if  (
							(MaxSampleRate==44100) || (MinSampleRate=44100) ||
							((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>48000)&&(MinSampleRate<48000))
							)
							filter |= (0x100|0x800); //16bit 44100 for 2 and 4 channel
					}
				}
			}
			bool Success=false;
			do 
			{
				if (!((1<<i) & filter))
					continue;
				SetWaveFormat(m_FormatToUse,trythis[i].wFormatTag,trythis[i].nChannels,trythis[i].nSamplesPerSecond,trythis[i].wBitsPerSample);
				Success=(m_IDS8_Buffer->SetFormat(&m_FormatToUse)==DS_OK);
				//Just because it works in the primary doesn't mean it will work in the secondary... drat!
				if (Success)
				{
					//Test the secondary buffer too
					LPDIRECTSOUNDBUFFER lpdsb;
					DSBUFFERDESC dsbdesc;
					INIT_DIRECTX_STRUCT(dsbdesc);
					dsbdesc.dwFlags         = 
						DSBCAPS_GLOBALFOCUS			|	//If my window is minimized I'm still going to be heard!
						DSBCAPS_STATIC				|	//Use hardware buffers if I can
						DSBCAPS_CTRLPOSITIONNOTIFY	|	//I'm going to be using a notification scheme
						DSBCAPS_GETCURRENTPOSITION2;	//We are a new product use the best accurate code

					WAVEFORMATEX *FormatToUse=&m_FormatToUse;
					dsbdesc.dwBufferBytes=(DWORD)FormatToUse->nBlockAlign * ((DWORD)GetBufferSampleSize()*2);
					dsbdesc.lpwfxFormat=FormatToUse;
					if FAILED(m_IDS8_Interface->CreateSoundBuffer(&dsbdesc,&lpdsb,NULL))
						Success=false;
					if (lpdsb)
						lpdsb->Release();

				}
			}while ((!Success)&&(i++<(NoElements-1)));
			if (i==NoElements)
				assert(false),throw "Unable to format DS default standard" __FUNCSIG__;
		}
	}
}

void DS_Output_Core::ShutDown()
{
	//m_AbortThread = true;
	SetEvent(m_NotificationTimer);
	//StopThread();

	if (m_IDS8_Buffer)
	{
		m_IDS8_Buffer->Stop();
		m_IDS8_Buffer->Release();
		m_IDS8_Buffer=NULL;
	}

	if (m_IDS8_Interface)
	{
		m_IDS8_Interface->Release();
		m_IDS8_Interface=NULL;
	}
}
  /************************************************************************/
 /*						  DirectSound_Globals							 */
/************************************************************************/
void DirectSound_Globals::init()
{
	m_DS_Output_Core=new DS_Output_Core;
	m_DS_Output_Core->StartUp();
}

DirectSound_Globals::DirectSound_Globals()
{

}

DirectSound_Globals::~DirectSound_Globals()
{
	delete m_DS_Output_Core;
	m_DS_Output_Core=NULL;
}

  /***********************************************************************/
 /*								DS_Output								*/			
/***********************************************************************/

DS_Output::DS_Output(size_t AudioOutput,const wchar_t AudioName[],bool UseDisplayedCallbacks) :
	m_AudioName(AudioName),m_AudioOutput(AudioOutput),m_DS(new DS_Output_Internal(UseDisplayedCallbacks))
{
	m_ServerStarted=false;
}

DS_Output::~DS_Output()
{
	#if 0
	debug_output(p_debug_category,L"Stopping server %s",m_VideoName.c_str());
	#endif
	//See case 27743:  I cannot use Block delivery critical section to ensure the buffers are not in flight... instead I can assume that
	//the destruction of the servers will ensure the messages have been processed before it closes.  I can then stop the hardware streaming
	m_Server_Audio.reset((audio::receive *)NULL);

	StopStreaming();

	delete m_DS;
	const_cast<DS_Output_Internal *>(m_DS)=NULL;  //Set to NULL as a safety (so it doesn't get deleted twice)
}

void DS_Output::SetAudioOutputChannel(size_t AudioOutput)
{
	m_AudioOutput=AudioOutput;
}
size_t DS_Output::GetAudioOutputChannel() const
{
	return m_AudioOutput;
}

void DS_Output::AddOutputChannel(size_t AudioOutput)
{
		assert(false);
}
void DS_Output::add_thru_dest(const wchar_t *audio_destination_name)
{
	DestThru NewEntry;
	if (audio_destination_name)	
		NewEntry.Audio=audio_destination_name;
	m_Dest_Thru.push_back(NewEntry);
	//	assert(false);
}

void DS_Output::StartServer()
{
	if (!m_ServerStarted)
	{
		m_ServerStarted=true;
		//create an audio server
		m_QueueDepthLevel=1;
		m_Server_Audio.reset(new audio::receive(m_AudioName.c_str(),this));
	}
}
void DS_Output::StartStreaming( void )
{
	StartServer();
	m_IsStreaming=true;
	m_DS->StartStreaming();
}

void DS_Output::StopStreaming( void )
{
		m_IsStreaming=false;
		m_DS->StopStreaming();
}



void DS_Output::deliver_audio(	const FC3::audio::message* p_aud_msg )
{
	//auto_lock ThreadSafe(m_BlockDelivery);
	if ( ( m_Server_Audio ) && ( m_Server_Audio->queue_depth() > 1 ) ) //ask james about this one right here, still unsure
	{	//assert(false);
		debug_output( p_debug_category, L"deliver_audio Frame dropped on output.\n" );
		return;
	}

	m_DS->deliver_audio(p_aud_msg);
	//static size_t Test=0;
	//printf("\r%d     ",Test++);
}


void DS_Output::SetUseDisplayedCallbacks(bool UseDisplayedCallbacks) 
{
	m_DS->SetUseDisplayedCallbacks(UseDisplayedCallbacks);
}
  /************************************************************************/
 /*						  DirectSound_Initializer					     */
/************************************************************************/

long DirectSound_Initializer::s_ref = 0;
critical_section	lock_directsound_startup;

bool DirectSound_Initializer::StartUp_DirectSound( void )
{
	auto_lock	lock_hw( lock_directsound_startup );
	s_ref++;

	if ( s_ref == 1 )
	{
		if (!g_DirectSoundInitializer)
		{
			g_DirectSoundInitializer= new DirectSound_Globals;
			g_DirectSoundInitializer->init();
		}
		else
			ShutDown_DirectSound();
	}

	return (g_DirectSoundInitializer!=NULL);
}

void DirectSound_Initializer::ShutDown_DirectSound( void )
{	
	auto_lock	lock_hw( lock_directsound_startup );
	s_ref--;

	if ( !s_ref )
	{
		if (g_DirectSoundInitializer)
		{	delete g_DirectSoundInitializer;
		g_DirectSoundInitializer=NULL;
		}
	}
}
