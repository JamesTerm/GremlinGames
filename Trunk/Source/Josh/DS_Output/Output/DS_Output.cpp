
#include "stdafx.h"
#include "Module.Hardware.DirectSound.Output.h"

#include <algorithm>
#include <dsound.h>

using namespace FrameWork::Debug;
using namespace FrameWork::Threads;
using namespace Modules::Hardware::DirectSound;
using namespace FC3;
using namespace std;

#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#undef  __UseTestSineWave__
#undef __DisableResampling__

//2.1 The reference signal shall be the digital representation, in twos complement format, of a1000-Hz sine
//	wave whose positive peaks attain the value 0CCD (hex) and whose negative peaks attain F333 (hex). This is
//	20 dB below the full-scale signal, or system maximum, in which the positive peak attains the value 7FFF (hex)
//	and the negative peak attains the value 8000 (hex), considering only the 16 most significant bits.
//	The reference signal amplitude shall be denoted 20 dBFS, indicating its relation to the full-scale signal. 
const double c_output_reference_scale_factor=(1.0/10.0);

#define TestIterationRate \
{ \
	using namespace HD::Base; \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	printf("rate=%f\n",((double)(currenttime-prevtime)) * 1000.0); \
	prevtime=currenttime; \
}

#define TestIterationRate2 \
{ \
	using namespace HD::Base; \
	static time_type prevtime=0.0; \
	time_type currenttime=time_type::get_current_time(); \
	printf("\r rate=%f                ",((double)(currenttime-prevtime)) * 1000.0); \
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

class AudioStreamInterface
{
	public:
		virtual void FillBuffer()=0;
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
		size_t GetBufferSampleSize( void ) const;
		size_t GetBitsPerSample( void ) const;


		WAVEFORMATEX *GetWaveFormatToUse( void );
		LPDIRECTSOUND8 GetDirectSoundObject( void );

		static DS_Output_Core &GetDS_Output_Core( void );

		void RemoveAudioStreamInterface(AudioStreamInterface *asi);
		void AddAudioStreamInterface(AudioStreamInterface *asi);

	protected:
		critical_section m_BlockAudioStreamsInterface;
		std::vector<AudioStreamInterface *> m_pAudioStreamInterface;
	private:
		void operator() ( const void* );

		LPDIRECTSOUND8 m_IDS8_Interface;
		LPDIRECTSOUNDBUFFER m_IDS8_Buffer; // primary
		DWORD m_BufferSizeInBytes;
		DWORD m_FillPosition; //keeps track of where the buffer has been filled
		HANDLE m_NotificationTimer;	
		WAVEFORMATEX m_FormatToUse;

		long m_GTE_LastCalled;

		FrameWork::Threads::thread<DS_Output_Core>	*m_pThread;
		friend FrameWork::Threads::thread<DS_Output_Core>;
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


class DS_Output_Internal : AudioStreamInterface
{
	public:
		enum AudioFormatEnum
		{
			eU8, eS16, eS32, eF32
		};

		AudioFormatEnum GetFormat(WAVEFORMATEX *wfm);

		DS_Output_Internal();
		~DS_Output_Internal();

		void AddInput(const FC3::audio::message *Message);

		virtual void StartStreaming( void );
		virtual void StopStreaming( void );
		virtual void FillBuffer( void );

		//TODO see if we need this

		//provide access to the current cursor's position
		//Note: This is a DirectSound specific call that really shouldn't be used... This could be temporary and may be replaced
		//with a clock interface so please don't use this unless you really need to.   - James
		//double GetTimeElapsed();

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

				///Once set, this will produce the output conversion (thus the input may be released)
				void AddInput(const FC3::audio::message *Message);

				///These pertain to the output size
				size_t size( void ) const; // no samples
				size_t size_in_bytes( void ) const; // no samples * sizeof(sample_type)
				//You can assume there is always output for underrun it will be silence
				PBYTE GetOutput();
			private:
				template<typename sample_type>
				void ConvertBitrate(const FrameWork::Audio2::buffer_f32 &Source);

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
				} m_uDMA_Samples_Buffer; //This just makes it easier to avoid casting (could be omitted)
				void *m_DMA_Samples_Buffer; //This is memory presented to the hardware (must be 64 byte aligned)
				HD::Base::Audio2_FloatingQueue m_Queue;
				FrameWork::Audio2::buffer_f32 m_SliceDiceBuffer;
				size_t m_OverflowThreshold;  //This number blends itself to the average sample block size coming in

				size_t m_ByteDepthSize; //cache the byte depth size
				std::vector<FrameWork::Audio2::utils::resampler *> m_Resampler; //this handles the resampling

				#ifdef __UseTestSineWave__
				void SWfreq(BufferTypes DestBuffer,size_t NoSamples,const int NoChannels,double freq_hz=1000.0,double SampleRate=48000.0,double amplitude=1.0);
				double m_rho;
				double m_rho2;
				#endif

		} m_AudioMessageConverter;

		critical_section	m_BlockDirectSound;
		critical_section m_BlockStreamingCalls;

		LPDIRECTSOUNDBUFFER m_lpdsb;
		DWORD m_BufferSizeInBytes;
		DWORD m_FillPosition; //keeps track of where the buffer has been filled

		//This is for GetTimeElapsed
		long m_GTE_LastCalled;
};

		}
	}
}


  /***********************************************************************************************************/
 /*									 DS_Output_Internal::AudioMessageConverter								*/
/***********************************************************************************************************/
const size_t cDS_Output_Internal_ThresholdBlockCount=2;

DS_Output_Internal::AudioMessageConverter::AudioMessageConverter(DS_Output_Internal * _m_pParent, size_t _NoChannels, AudioFormatEnum _Format, size_t _SampleRate, size_t _BlockSize)
: m_pParent(_m_pParent),m_DMA_Samples_Buffer(NULL)
{
	m_uDMA_Samples_Buffer.p_u8=NULL;
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
	m_SliceDiceBuffer.resize((int)_NoChannels,(int)_BlockSize);
	m_OverflowThreshold=_BlockSize*cDS_Output_Internal_ThresholdBlockCount; //two frames worth of destination size should be a good default
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

DS_Output_Internal::AudioMessageConverter::~AudioMessageConverter()
{
	if (m_DMA_Samples_Buffer)
	{
		_aligned_free(m_DMA_Samples_Buffer);
		m_DMA_Samples_Buffer=NULL;
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
void DS_Output_Internal::AudioMessageConverter::ConvertBitrate(const FrameWork::Audio2::buffer_f32 &Source)
{
	using namespace FrameWork::Audio2;
	size_t NoChannels=m_AudioMessageData.NoChannels;
	size_t NoSamples=Source.no_samples();

	//Now to convert the Frame to our Hardware format
	buffer<sample_type> DeInterleaved((int)NoChannels,(int)NoSamples,16);  //align for best SSE2 conversions
	for (int i=0;i<(int)NoChannels;i++)
	{
		//Now we have an de-interleaved s32 converted format with compatible number of channels
		if (i<Source.no_channels())
		{
			buffer<sample_type>(DeInterleaved(i),1,(int)NoSamples)=buffer_f32(Source(i),1,(int)NoSamples);
		}
		else
			FrameWork::Bitmaps::memset(DeInterleaved(i),0,DeInterleaved.no_samples_in_bytes()); //fill with silence
	}
	//Now to interleave it
	utils::convert_to_interleaved<sample_type>(DeInterleaved,(sample_type *)m_DMA_Samples_Buffer);

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

	//Add to queue here as this queue allows slice and dice operations
	m_Queue.AddToQueue(*ResampledSource);
	delete ResampledSource;

	//Update the new overflow threshold value based from the size of this message
	const size_t BlockSize=m_AudioMessageData.BlockSize;
	const size_t Min_BlockSizeThreshold=BlockSize * cDS_Output_Internal_ThresholdBlockCount;

	//To determine the overflow thresh-hold we'll want to get a running average of the block sizes coming in.  This way if we are receiving samples
	//from a source that has large packets the threshold would grow to twice the size of it, and dynamically shrink down to a size if the source gives
	//a size with a better packet.  In practice (i.e. the application) the audio mixer will give us a good size for the best latency case scenario
	//  [7/26/2011 JamesK]
	{	//blend new packet size against the existing

		const double smoothing=0.10;
		double NewPktSize=(double)ActualSampleSize * cDS_Output_Internal_ThresholdBlockCount;
		//printf("\rThreshold=%d pkt=%d            ",m_OverflowThreshold,ActualSampleSize);
		//ensure a smaller size submission is at least our min block size (we don't want a false positive of overflow)
		NewPktSize=(NewPktSize >= (Min_BlockSizeThreshold)) ? NewPktSize : Min_BlockSizeThreshold;
		m_OverflowThreshold= (size_t)(((smoothing * NewPktSize)  + ((1.0 - smoothing) * m_OverflowThreshold ) ) + 0.5);
	}
}

PBYTE DS_Output_Internal::AudioMessageConverter::GetOutput()
{
	const size_t BlockSize=m_AudioMessageData.BlockSize;
	const size_t Min_BlockSizeThreshold=BlockSize * cDS_Output_Internal_ThresholdBlockCount;

	size_t NoChannels=m_AudioMessageData.NoChannels;
	//We'll manage a growing DMA buffer here
	m_DMA_Samples_Buffer=_aligned_realloc(m_DMA_Samples_Buffer,BlockSize*NoChannels*m_ByteDepthSize,16);
	m_uDMA_Samples_Buffer.p_u8=(PBYTE)m_DMA_Samples_Buffer;

	size_t QueueDepth;
	QueueDepth=m_Queue.GetSizeInSamples();

	//printf("\rQueueDepth=%d       ",QueueDepth);
	if (QueueDepth>BlockSize)  //underflow check
	{
		bool result;
		//Overflow check
		if (QueueDepth>m_OverflowThreshold)
		{
			size_t DiscardSize=QueueDepth-m_OverflowThreshold;
			debug_output(p_debug_category,L"DS_Output_Internal overflow discarding %d samples\n",DiscardSize);
			while (DiscardSize!=0)
			{
				if (DiscardSize>BlockSize)
				{
					result=m_Queue.TakeFromQueue(m_SliceDiceBuffer, BlockSize);
					DiscardSize-=BlockSize;
					assert(result); //sanity check... should always work!
				}
				else
				{
					result=m_Queue.TakeFromQueue(m_SliceDiceBuffer, DiscardSize);
					DiscardSize=0;
					assert(result); //sanity check... should always work!
				}
			}
			assert(m_Queue.GetSizeInSamples() >= BlockSize); //sanity check
		}

		result=m_Queue.TakeFromQueue(m_SliceDiceBuffer, BlockSize);
		assert(result); //sanity check... should always work!

		//apply bit rate conversion
		switch (m_AudioMessageData.Format)
		{
		case eU8:
			ConvertBitrate<unsigned char>(m_SliceDiceBuffer);
			break;
		case eS16:
			ConvertBitrate<short>(m_SliceDiceBuffer);
			break;
		case eS32:
			ConvertBitrate<int>(m_SliceDiceBuffer);
			break;
		case eF32:
			ConvertBitrate<float>(m_SliceDiceBuffer);
			break;
		}
	}
	else
	{
		debug_output(p_debug_category,L"DS_Output_Internal underflow, filling in with silence\n");
		if (m_AudioMessageData.Format!=eU8)
			memset(m_DMA_Samples_Buffer,0,size_in_bytes());
		else
			memset(m_DMA_Samples_Buffer,128,size_in_bytes());
	}

	#ifdef __UseTestSineWave__
	SWfreq(m_uDMA_Samples_Buffer,BlockSize,(int)NoChannels,1000.0,(int)m_AudioMessageData.SampleRate,0.5);
	#endif

	return m_uDMA_Samples_Buffer.p_u8;
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


DS_Output_Internal::DS_Output_Internal() : 
	m_IsStreaming(false),m_AudioOutput(),
	m_AudioMessageConverter(this,
	DS_Output_Core::GetDS_Output_Core().GetNoChannels(),
	GetFormat(DS_Output_Core::GetDS_Output_Core().GetWaveFormatToUse()),
	DS_Output_Core::GetDS_Output_Core().GetSampleRate(),
	DS_Output_Core::GetDS_Output_Core().GetBufferSampleSize()),
	m_GTE_LastCalled(0)
{ 

	DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
	m_FillPosition=(DWORD)DSOC.GetBufferSampleSize(); //this way the first buffer will get filled when it starts
	//Set up the secondary buffer
	DSBUFFERDESC dsbdesc;
	INIT_DIRECTX_STRUCT(dsbdesc);
	dsbdesc.dwFlags         = 
		DSBCAPS_GLOBALFOCUS			|	//If my window is minimized I'm still going to be heard!
		DSBCAPS_STATIC				|	//Use hardware buffers if I can
		DSBCAPS_GETCURRENTPOSITION2;	//We are a new product use the best accurate code

	WAVEFORMATEX *FormatToUse=DSOC.GetWaveFormatToUse();
	dsbdesc.dwBufferBytes=m_BufferSizeInBytes=(DWORD)(FormatToUse->nBlockAlign * (DSOC.GetBufferSampleSize()*2));
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

	DS_Output_Core::GetDS_Output_Core().AddAudioStreamInterface(this);
}
DS_Output_Internal::~DS_Output_Internal()
{
	DS_Output_Core::GetDS_Output_Core().RemoveAudioStreamInterface(this);

	if (m_lpdsb)
	{
		m_lpdsb->Stop();
		m_lpdsb->Release();
		m_lpdsb=NULL;
	}
}

void DS_Output_Internal::StopStreaming( void ) 
{
	m_lpdsb->Stop();
	m_IsStreaming=false;
}

void DS_Output_Internal::StartStreaming( void )
{
	m_lpdsb->Play(0,0,DSBPLAY_LOOPING);
	m_IsStreaming=true;
}

void DS_Output_Internal::AddInput(const FC3::audio::message *Message)
{
	if (m_IsStreaming)
		m_AudioMessageConverter.AddInput(Message);
}

void DS_Output_Internal::FillBuffer()
{
	if (m_IsStreaming)
	{
		void *dsbuf1,*dsbuf2;
		DWORD dsbuflen1,dsbuflen2,playpos;
		DS_Output_Core &DSOC=DS_Output_Core::GetDS_Output_Core();
		const size_t BufferSize=m_BufferSizeInBytes;
		const DWORD HalfBufferSize=(DWORD)BufferSize/2;
		const size_t SampleSize=DSOC.GetBufferSampleSize();

		m_lpdsb->GetCurrentPosition(&playpos,NULL);
		playpos/=DSOC.GetWaveFormatToUse()->nBlockAlign; //convert to samples
		//debug_output(p_debug_category,L"%d \n",playpos);
		if ((playpos & SampleSize)!=(m_FillPosition))
		{
			if (m_lpdsb->Lock((m_FillPosition/(DWORD)SampleSize)*HalfBufferSize,HalfBufferSize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)==DS_OK)
			{
				assert(HalfBufferSize==m_AudioMessageConverter.size_in_bytes());
				//handy debug info
				//DebugOutput("cursor=%x,buf1=%x,buf1len=%d,buf2=%x,buf2len=%d\n",playpos,dsbuf1,dsbuflen1,dsbuf2,&dsbuflen2);
				memcpy((byte *)dsbuf1,m_AudioMessageConverter.GetOutput(),HalfBufferSize);
				m_FillPosition=playpos & SampleSize;
				if FAILED(m_lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
					printf("Unable to unlock DS buffer\n");
			}
			else
				debug_output(p_debug_category,L"Error cursor=%x,buf1=%x,buf1len=%d,buf2=%x,buf2len=%d\n",playpos,dsbuf1,dsbuflen1,dsbuf2,&dsbuflen2);
		}
		//else
		//	DebugOutput("Skipping... cursor=%x,last=%x\n",playpos,m_FillPosition);

	}
}


  /************************************************************************/
 /*							  DS_Output_Core							 */
/************************************************************************/
DS_Output_Core::DS_Output_Core()
:m_IDS8_Interface(NULL), m_pThread(NULL)
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
	//Sample size Must be a power of 2 (so it can do some anding techniques)
	//1024 is too small for the notifications to be reliable
	//We want this small enough to minimize in latency
	//return 2048;

	//Note: the 2048 makes clicking sounds on a sine wave test... we'll need to fix this
	//TODO get the latency lower
	return 4096;
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
void DS_Output_Core::AddAudioStreamInterface(AudioStreamInterface *asi)
{
	auto_lock ThreadSafe(m_BlockAudioStreamsInterface);
	m_pAudioStreamInterface.push_back(asi);
}
void DS_Output_Core::RemoveAudioStreamInterface(AudioStreamInterface *asi)
{
	auto_lock ThreadSafe(m_BlockAudioStreamsInterface);
	bool removed_successfully = false;
	for (size_t i = 0; i < m_pAudioStreamInterface.size(); i++)
	{
		if (asi == m_pAudioStreamInterface[i]){
			m_pAudioStreamInterface.erase(m_pAudioStreamInterface.begin() + i);
			removed_successfully = true;
		}
	}
	assert(removed_successfully);
}
void DS_Output_Core::operator() ( const void* )
{
	WaitForSingleObject(m_NotificationTimer,1000);
	//m_AudioStreams_Lock.lock();
	//Notify all streams
	for (size_t i=0;i<m_pAudioStreamInterface.size();i++)
		m_pAudioStreamInterface[i]->FillBuffer(); //notify our stream to update
	//m_AudioStreams_Lock.unlock();
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
				//TODO enable once we have sample rate conversion
				#if 0
				{WAVE_FORMAT_IEEE_FLOAT,4,96000,32},//....01
				{WAVE_FORMAT_PCM,4,96000,32},//...........02
				{WAVE_FORMAT_PCM,4,96000,16},//...........04
				{WAVE_FORMAT_IEEE_FLOAT,2,96000,32},//....08
				{WAVE_FORMAT_PCM,2,96000,32},//...........10
				{WAVE_FORMAT_PCM,2,96000,16},//...........20
				#endif

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
		{//Now that I have a sample rate I can compute a notification interval
			double BufferTime=(double)GetBufferSampleSize()/(double)GetSampleRate(); //e.g. 42 milliseconds for 2048 samples in 48000 sample rate
			BufferTime/=4.0; //we want enough notifies during this time (e.g. 10.5 milliseconds)
			LARGE_INTEGER li;
			li.QuadPart=(LONGLONG)(-BufferTime*10000000.0);
			if (!SetWaitableTimer(m_NotificationTimer,&li,(long)(BufferTime*1000.0),NULL,NULL,false))
				assert(false),throw "Unable to set WaitableTimer" __FUNCSIG__;
		}

		// Start the thread that will stream out the callback
		//m_AbortThread = false;
		//StartThread();
		assert(!m_pThread);
		m_pThread = new thread<DS_Output_Core>(this);
	}
}

void DS_Output_Core::ShutDown()
{
	//m_AbortThread = true;
	SetEvent(m_NotificationTimer);
	//StopThread();
	assert (m_pThread);
	delete m_pThread;

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

DS_Output::DS_Output(size_t AudioOutput,const wchar_t AudioName[]) : m_AudioName(AudioName),m_AudioOutput(AudioOutput),m_DS(new DS_Output_Internal)
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


FC3::audio::message *DS_Output::AllocateFrame_audio( const int no_samples, const int no_channels)
{
	using namespace FC3::audio;
	return new message(no_samples,no_channels);
}

void DS_Output::deliver_audio(	const FC3::audio::message* p_aud_msg )
{
	//auto_lock ThreadSafe(m_BlockDelivery);
	if ( ( m_Server_Audio ) && ( m_Server_Audio->queue_depth() > 1 ) ) //ask james about this one right here, still unsure
	{	//assert(false);
		debug_output( p_debug_category, L"HD_Output2::deliver_audio Frame dropped on output.\n" );
		return;
	}

	m_DS->AddInput(p_aud_msg);
	//static size_t Test=0;
	//printf("\r%d     ",Test++);
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

