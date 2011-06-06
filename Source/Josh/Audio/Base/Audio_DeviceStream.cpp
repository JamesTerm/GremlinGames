#include "Stdafx.h"
#include "Devices_Audio_Base.h"

using namespace std;
using namespace FrameWork::Base;
using namespace FrameWork::Threads;
using namespace Devices::Audio::Base::Streams;
using namespace Devices::Streams;
using namespace Devices;

  /***************************************************************************************************/
 /*											AudioBuffer												*/
/***************************************************************************************************/

AudioBuffer::AudioBuffer(AudioInputStream *pParent,const FormatInterface *pFormat) : 
AudioBuffer_Base(pParent,pFormat)
{	
	size_t BytesPerSample=Devices::Audio::Base::aud_convert::GetFormatBitDepth(m_SampleFormat)*m_NoChannels; //we can assume float now
	//size_t BytesPerSample=sizeof(float)*m_NoChannels;

	m_NoSamples=pParent->GetParent()->GetBufferSampleSize();
	assert(m_NoSamples); //ensure my parent can safely obtain this information
	m_BufferSize=m_NoSamples*BytesPerSample;
	m_Memory=(byte *)_aligned_malloc(m_BufferSize,16);
}

AudioBuffer::~AudioBuffer() 
{
	_aligned_free(m_Memory);
}

void *AudioBuffer::pGetBufferData( size_t *pMemorySize ) 
{
	if (pMemorySize)
		*pMemorySize=m_BufferSize;
	return m_Memory;
}

void AudioBuffer::ReleaseBufferData(const TimeStamp *pFrameTime , const wchar_t *pErrorMessage) 
{
	assert(pFrameTime);
	if (pFrameTime)
		m_TimeReleased=*pFrameTime;
	m_pParent->AddBufferToTBCList( this , pErrorMessage );
	//if (pErrorMessage)
	//	printf("**Buffer Error-%s\n",pErrorMessage);
}

  /***************************************************************************************************/
 /*										AudioDynamicBuffer											*/
/***************************************************************************************************/

AudioDynamicBuffer::AudioDynamicBuffer(AudioInputStream *pParent,const FormatInterface *pFormat) :
	AudioBuffer_Base(pParent,pFormat),m_RawBuffer(pParent)
{
}

void *AudioDynamicBuffer::pGetBufferData( size_t *pMemorySize )
{
	return m_RawBuffer.pGetBufferData(pMemorySize);
}
void AudioDynamicBuffer::ReleaseBufferData( const TimeStamp *pFrameTime, const wchar_t *pErrorMessage)
{
	assert(pFrameTime);
	if (pFrameTime)
		m_TimeReleased=*pFrameTime;
	m_pParent->AddBufferToTBCList( this , pErrorMessage );
}



  /***************************************************************************************************/
 /*								Audio_QueueControlDeviceClient										*/
/***************************************************************************************************/


Audio_QueueControlDeviceClient::Audio_QueueControlDeviceClient(Audio::DeviceInterface *AudioDeviceInput,bool BypassFormatCheck) :
	m_OutStream(this,L"Audio_QueueControlDeviceClient"),m_AudioDeviceInput(AudioDeviceInput),m_BypassFormatCheck(BypassFormatCheck),
	m_InputStreamer(this,AudioDeviceInput),m_ChannelMixer(this)
{
	m_SampleSize=0;
	// Start the thread
	//m_pThread = new Thread<Audio_QueueControlDeviceClient>(this);
	m_InputStreamer.StartTBCQueue();
	m_VolumeMatrix=NULL;
	m_ChannelMixerDestNoChannels=m_ChannelMixerSourceNoChannels=0;
}

Audio_QueueControlDeviceClient::~Audio_QueueControlDeviceClient()
{
	m_InputStreamer.FinishTBCQueue();
	if (m_VolumeMatrix)
	{
		Audio::Base::aud_convert::ChannelMixer::DestroyDefaultVolumeMatrix(m_VolumeMatrix);
		m_VolumeMatrix=NULL;
	}
}

bool Audio_QueueControlDeviceClient::Audio_OutStream_QueueSource_FillBuffer(byte *DestBuffer,size_t NoSamples,FormatInterface *OutFormat)
{
	bool success=false;
	//Ideally this will be false for best performance
	bool OutputConversionNeeded=(
		(GetNoChannels()!=OutFormat->GetNoChannels())||(OutFormat->GetSampleFormat()!=Audio::FormatInterface::eSampleFormat_FP_32) ||
		(OutFormat->GetIsBufferInterleaved())
		);

	size_t BlockAlign=Audio::Base::aud_convert::GetFormatBitDepth(m_AudioDeviceInput->GetSampleFormat()) * m_AudioDeviceInput->GetNoChannels();
	size_t SourceBufferSizeInBytes=NoSamples*BlockAlign;
	if (OutputConversionNeeded)
	{
		size_t DestBlockAlign=Audio::Base::aud_convert::GetFormatBitDepth(OutFormat->GetSampleFormat()) * OutFormat->GetNoChannels();
		BlockAlign=max(BlockAlign,DestBlockAlign);
	}
	//I'll make the temp buffer large enough in case I need to use it for the destination format as well
	size_t BufferSizeInBytes=NoSamples*BlockAlign;
	//Set up a temp buffer that is aligned to 16 bytes
	byte *SourceBuf=(byte *)_alloca(BufferSizeInBytes + 16);
	byte *AlignedSourceBuf=(byte *)(((size_t)SourceBuf+0x0f)&~0x0f); //We need this aligned to a 16 byte boundary

	if (m_FloatingQueue.TakeFromQueue(AlignedSourceBuf,SourceBufferSizeInBytes))
	{
		#ifdef __WriteWavBeforeConversion__
		//Debug test purposes only
		{
			static FileWriter::Wave::WaveFile wave;
			static bool Started=false;
			if (!Started)
			{
				wave.OpenForWrite("c:\\Temp\\Test.wav",(DWORD)m_AudioDeviceInput->GetSampleRate(),
					(short)(Audio::Base::aud_convert::GetFormatBitDepth(m_AudioDeviceInput->GetSampleFormat())<<3),
					(WORD)m_AudioDeviceInput->GetNoChannels());
				Started=true;
			}
			//wave.WriteData(AlignedSourceBuf,(DWORD)BufferSizeInBytes);
			wave.WriteData(AlignedSourceBuf,
				(DWORD)(NoSamples*m_AudioDeviceInput->GetNoChannels()*
				Audio::Base::aud_convert::GetFormatBitDepth(m_AudioDeviceInput->GetSampleFormat())));
		}
		#endif

		//convert to non-interleaved float
		Audio::Base::aud_convert::ConvertToFloat(AlignedSourceBuf,m_AudioDeviceInput->GetSampleFormat(),NoSamples,
			(float *)DestBuffer,GetNoChannels(),1.0f,m_AudioDeviceInput->GetIsBufferInterleaved(),false);

		#ifdef	__WriteWavAfterConversion__
		//Debug test purposes only
		{
			static FileWriter::Wave::WaveFile wave;
			static bool Started=false;
			if (!Started)
			{
				//wave.OpenForWrite("c:\\Temp\\Test.wav",48000,16,2);
				wave.OpenForWrite("c:\\Temp\\Test.wav",OutFormat->GetSampleRate(),-32,OutFormat->GetNoChannels());
				Started=true;
			}
			//wave.WriteData(AlignedSourceBuf,(DWORD)BufferSizeInBytes);
			wave.WriteData(DestBuffer,(DWORD)(NoSamples*OutFormat->GetNoChannels()*Audio::Base::aud_convert::GetFormatBitDepth(OutFormat->GetSampleFormat())));
		}
		#endif

		//Ideally the device client works in float, non-interleaved and has the same number of channels.  However if not we can handle it here
		if (OutputConversionNeeded)
		{
			//Set up the channel mixer
			if ((m_ChannelMixerDestNoChannels!=OutFormat->GetNoChannels()) ||
				(m_ChannelMixerSourceNoChannels)!=GetNoChannels()
				)
			{
				 //reset the matrix since the dimensions have changed
				if (m_VolumeMatrix)
				{
					Audio::Base::aud_convert::ChannelMixer::DestroyDefaultVolumeMatrix(m_VolumeMatrix);
					m_VolumeMatrix=NULL; 
				}
			}

			m_ChannelMixerDestNoChannels=OutFormat->GetNoChannels();
			m_ChannelMixerSourceNoChannels=GetNoChannels();

			//Create the volume matrix if it doesn't exist
			if (!m_VolumeMatrix)
				m_VolumeMatrix=Audio::Base::aud_convert::ChannelMixer::CreateDefaultVolumeMatrix(m_ChannelMixerSourceNoChannels,m_ChannelMixerDestNoChannels);

			m_ChannelMixer.Mix((float *)DestBuffer,(float *)AlignedSourceBuf,NoSamples,false);
			//Now the AlignedSourceBuf has output
			Audio::Base::aud_convert::ConvertFromFloat((float *)AlignedSourceBuf,NoSamples,DestBuffer,OutFormat->GetSampleFormat(),
				m_ChannelMixerDestNoChannels,1.0f,false,OutFormat->GetIsBufferInterleaved());
		}
		success=true;
	}
	return success;

}

void Audio_QueueControlDeviceClient::InStreamClosing(void *ThisChild)
{
	//There is nothing to do here since we were borrowing the upstream thread for the down stream
}

void Audio_QueueControlDeviceClient::AddBufferToTBCList( Devices::BufferInterface *pBuffer , const wchar_t *pErrorMessage )
{
	//We can't fill in buffers if they have errors
	if (pErrorMessage)
		return;

	size_t SizeInBytes=0;  //Need zero for raw read rules
	byte *pbData=(byte *)pBuffer->pGetBufferData(&SizeInBytes);

	//size_t NumberOfSamples=TotalSampleSize/(m_WaveFormat.nChannels*(m_WaveFormat.wBitsPerSample>>3));
	//DebugOutput(L"SampleSize=%d\n",NumberOfSamples);

	#ifdef __WriteWavAddToTBCList__
	//Debug test purposes only
	{
		static FileWriter::Wave::WaveFile wave;
		static bool Started=false;
		if (!Started)
		{
			wave.OpenForWrite("c:\\Testpbdata.wav",48000,16,2);
			//wave.OpenForWrite("c:\\Test.wav",(DWORD)GetSampleRate(),(short)GetFormatBitDepth(GetSampleFormat()),(WORD)GetNoChannels());
			Started=true;
		}
		for (size_t i=0;i<NumberOfSamples;i++)
			wave.WriteStereoSample(((short *)pbData)[i<<1],((short *)pbData)[(i<<1)+1]);
	}
	#endif


	//This will come in handy... the TBC should keep this to a reasonable size
	size_t BlockAlign=m_AudioDeviceInput->GetNoChannels() * Audio::Base::aud_convert::GetFormatBitDepth(m_AudioDeviceInput->GetSampleFormat());
	size_t QueueDepth_NoSamples=m_FloatingQueue.GetSizeInBytes()/BlockAlign;
	//DebugOutput(L"Queue Depth %d\n",QueueDepth_NoSamples);

	//While the TBC should be capable of keeping the queue depth to a reasonable level, I am adding a queue depth check because of murphy's law ;)
	if (QueueDepth_NoSamples<(m_AudioDeviceInput->GetSampleRate()>>1))	//a half second of depth
		m_FloatingQueue.AddToQueue(pbData,SizeInBytes);
	else
	{
		DebugOutput(L"Audio_QueueControlDeviceClient::AddBufferToTBCList Dropping Samples, maximum queue depth exceeded %d \n",QueueDepth_NoSamples);
		//assert(false);
	}
	//Once we reach this point all buffers submitted are in a floating queue... so to avoid having to use another thread well send any
	//OTHER buffers back (ie. advance the tbc)... the buffer this time around will remain and be sent during the next iteration
	{
		auto_lock(m_InputStreamer.GetTBCQueue_Lock());
		std::vector<Devices::BufferInterface *>	&m_TBCQueue=m_InputStreamer.GetTBCQueue();
		//Make sure there are some buffers in the device queue
		while (m_TBCQueue.size() && m_TBCQueue[0])
			m_InputStreamer.AdvanceTBC();
	}
	m_OutStream.TransferBuffers();
}

void Audio_QueueControlDeviceClient::In_FormatChangeDetected()
{
	//Since I do not cache any format information I shouldn't need to do anything here
	//This may get called if the dynamic buffer had to guess the format and when the device finally loads its format is different
}

Devices::BufferInterface *Audio_QueueControlDeviceClient::InStream_Interface_GetNewBuffer(InputStream *pInputStream)
{
	return new AudioDynamicBuffer(&m_InputStreamer,m_AudioDeviceInput);
}

void Audio_QueueControlDeviceClient::InStream_Interface_FreeBuffer(Devices::BufferInterface *buffer)
{
	delete dynamic_cast<AudioDynamicBuffer *>(buffer);
}

  /***************************************************************************************************/
 /*										Audio_FrameStreamer											*/
/***************************************************************************************************/

Audio_FrameStreamer::Audio_FrameStreamer(Audio::DeviceInterface *AudioDeviceInput,double FrameRate) :
Audio_QueueControlDeviceClient(AudioDeviceInput),
m_FrameRate(FrameRate)
{
	m_rho=0;
}

Audio_FrameStreamer::~Audio_FrameStreamer( void )
{
}

size_t Audio_FrameStreamer::Audio_OutStream_QueueSource_GetBufferSizeInBytes(Audio::FormatInterface *OutFormat)
{
	double SampleRate=(double)m_AudioDeviceInput->GetSampleRate();
	//Guess for if the device is not yet ready
	if (SampleRate==0)
		SampleRate=48000;
	double Theta=SampleRate/m_FrameRate;
	//it doesn't matter much whether this is rounded or truncated
	m_rho+=Theta;

	size_t SamplesToTake=(m_rho>Theta)?(size_t)(m_rho):(size_t)(m_rho + 0.5);

	m_rho-=SamplesToTake;

	//Finally to convert the samples to bytes
	size_t BlockAlign=Audio::Base::aud_convert::GetFormatBitDepth(OutFormat->GetSampleFormat()) * OutFormat->GetNoChannels();
	size_t BufferSizeInBytes=SamplesToTake*BlockAlign;

	return BufferSizeInBytes;
}


  /***************************************************************************************************/
 /*									AudioHostInputStream											*/
/***************************************************************************************************/


AudioHostInputStream::AudioHostInputStream(AudioInStream_Interface *pParent,Devices::Audio::DeviceInterface *pAudioDevice,FormatInterface *pBufferFormat,size_t DefaultTBCQueueDepth) :
m_pBufferFormat(pBufferFormat),m_pAudioDevice(pAudioDevice),
AudioInputStream(pParent,pAudioDevice,DefaultTBCQueueDepth) 
{
	m_DestNoChannels=pBufferFormat->GetNoChannels();
	m_SourceNoChannels=m_pAudioDevice->GetNoChannels();
	m_pVolumeMatrix=aud_convert::ChannelMixer::CreateDefaultVolumeMatrix(m_SourceNoChannels,m_DestNoChannels);
}

AudioHostInputStream::~AudioHostInputStream()
{
	if (m_pVolumeMatrix)
	{
		aud_convert::ChannelMixer::DestroyDefaultVolumeMatrix(m_pVolumeMatrix);
		m_pVolumeMatrix=NULL;
	}
}

float *AudioHostInputStream::GetVolumeMatrix() 
{	
	//ensure we have the same dimensions:
	if ((m_SourceNoChannels!=m_pAudioDevice->GetNoChannels())||(m_DestNoChannels!=m_pBufferFormat->GetNoChannels()))
	{
		//Format change detected: We need a new matrix
		aud_convert::ChannelMixer::DestroyDefaultVolumeMatrix(m_pVolumeMatrix);
		m_DestNoChannels=m_pBufferFormat->GetNoChannels();
		m_SourceNoChannels=m_pAudioDevice->GetNoChannels();
		m_pVolumeMatrix=aud_convert::ChannelMixer::CreateDefaultVolumeMatrix(m_SourceNoChannels,m_DestNoChannels);
	}
	return m_pVolumeMatrix;
}

  /***************************************************************************************************/
 /*									AudioDeviceClientToDevice										*/
/***************************************************************************************************/


AudioDeviceClientToDevice::AudioDeviceClientToDevice(const wchar_t *pDeviceName, const wchar_t *InputDeviceClientName) :
	m_OutStream(this,pDeviceName),m_InputDeviceClientName(InputDeviceClientName)
{
	//Set up a typical format (really doesn't matter what it is just good guess)
	m_SampleRate=48000;
	m_NoChannels=2;
	m_SampleFormat=Audio::FormatInterface::eSampleFormat_I_16;
	m_bIsInterleaved=false;
}

bool AudioDeviceClientToDevice::AddBufferToQueue( BufferInterface *pBuffer )
{
	//We simply pass this incoming buffer into the OutStream's device queue.  A memcpy will happen implicitly
	size_t MemorySize;

	Audio::BufferInterface	*pBI = dynamic_cast<Audio::BufferInterface*>( pBuffer );
	if (!pBI) return false;

	const byte *Data=(const byte *)pBI->pGetBufferData(&MemorySize);
	m_SampleRate=pBI->GetSampleRate();
	m_NoChannels=pBI->GetNoChannels();
	m_SampleFormat=pBI->GetSampleFormat();
	m_bIsInterleaved=pBI->GetIsBufferInterleaved();

	m_OutStream.TransferBuffer(Data,MemorySize,pBI->GetTimeStamp());
	//Now that the buffer is transferred we can send it back
	pBuffer->ReleaseBufferData();
	return true;
}

//#include "../../../user/james killian/audiotesting/wave/wave.h"
#ifndef _WIN64
bool ResamplingDevice::AddBufferToQueue( BufferInterface *pBuffer )
{
	Audio::BufferInterface	*pBI = dynamic_cast<Audio::BufferInterface*>( pBuffer );
	
	if(pBI->GetSampleRate() != m_OutStream.GetAudioDevice()->GetSampleRate())
	{	
		FrameWork::Audio::aud_buf_stereo_f32 i_buf((FrameWork::Audio::sample_type_stereo_f32 *)NULL, 0, pBI->GetSampleRate());
		
		size_t MemorySize;	
		
		void *data = pBI->pGetBufferData(&MemorySize);
		
		switch(pBI->GetNoChannels())
		{
		case 2:
				switch(pBI->GetSampleFormat())
				{
					case eSampleFormat_FP_32:
						i_buf = FrameWork::Audio::aud_buf_stereo_f32((FrameWork::Audio::sample_type_stereo_f32 *)data, MemorySize>>3, pBI->GetSampleRate());
						break;
					
					case eSampleFormat_I_16:
						i_buf = FrameWork::Audio::aud_buf_stereo_s16((FrameWork::Audio::sample_type_stereo_s16 *)data, MemorySize>>2, pBI->GetSampleRate());
						break;
					
					case eSampleFormat_U_8:
						i_buf = FrameWork::Audio::aud_buf_stereo_u8((FrameWork::Audio::sample_type_stereo_u8 *)data, MemorySize>>1, pBI->GetSampleRate());
						break;
				}
				break;

			case 4:
				switch(pBI->GetSampleFormat())
				{
					case eSampleFormat_FP_32:
						i_buf = FrameWork::Audio::aud_buf_quad_f32((FrameWork::Audio::sample_type_quad_f32 *)data, MemorySize>>4, pBI->GetSampleRate());
						break;
					
					case eSampleFormat_I_16:
						i_buf = FrameWork::Audio::aud_buf_quad_s16((FrameWork::Audio::sample_type_quad_s16 *)data, MemorySize>>3, pBI->GetSampleRate());
						break;
					
					case eSampleFormat_U_8:
						i_buf = FrameWork::Audio::aud_buf_quad_u8((FrameWork::Audio::sample_type_quad_u8 *)data, MemorySize>>2, pBI->GetSampleRate());
						break;
				}
				break;	
			
			case 1:
				switch(pBI->GetSampleFormat())
				{
					case eSampleFormat_FP_32:
						i_buf = FrameWork::Audio::aud_buf_mono_f32((FrameWork::Audio::sample_type_mono_f32 *)data, MemorySize>>2, pBI->GetSampleRate());
						break;

					case eSampleFormat_I_16:
						i_buf = FrameWork::Audio::aud_buf_mono_s16((FrameWork::Audio::sample_type_mono_s16 *)data, MemorySize>>1, pBI->GetSampleRate());
						break;

					case eSampleFormat_U_8:
						i_buf = FrameWork::Audio::aud_buf_mono_u8((FrameWork::Audio::sample_type_mono_u8 *)data, MemorySize, pBI->GetSampleRate());
						break;
				}
				break;
		}

		
/*
	//m_OutStream.GetAudioDevice()->GetSampleFormat();
	switch(m_OutStream.GetAudioDevice()->GetNoChannels())
	{
		case 2:
			//o_buf = new FrameWork::Audio::aud_buf_stereo_s16((FrameWork::Audio::sample_type_stereo_s16 *)NULL, 0, m_OutStream.GetAudioDevice()->GetSampleRate());
		//case 4:
	}
*/

		FrameWork::Audio::aud_buf_stereo_s16 o_buf2;
		FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_stereo_f32> *resampl = m_hq_resamplers[pBI->GetSampleRate()];

#ifdef _DEBUG	
		assert(resampl);		//if this is false then a rate is being supplied we havent set up a resampler for
#endif

		o_buf2 = (*resampl)(i_buf);

/*
		static FileWriter::Wave::WaveFile wave;
		static bool Started=false;
		if (!Started)
		{
			wave.OpenForWrite("c:\\Testpbdata.wav",48000,16,2);
			//wave.OpenForWrite("c:\\Test.wav",(DWORD)GetSampleRate(),(short)GetFormatBitDepth(GetSampleFormat()),(WORD)GetNoChannels());
			Started=true;
		}
		for (size_t i=0;i<o_buf2.size();i++)
			wave.WriteStereoSample( o_buf2[i][0], o_buf2[i][1] );

		if(false)
			wave.Close();
*/
		if(o_buf2.size())
			m_OutStream.TransferBuffer((const byte *)o_buf2(), o_buf2.size_in_bytes(), pBI->GetTimeStamp());
		
		pBuffer->ReleaseBufferData();
	}
	else
	{	
		__super::AddBufferToQueue(pBuffer);
	}

	
	return true;
}
#endif

void AudioDeviceClientToDevice::FlushAllBuffers( void )
{
	//No queue to flush
}

const wchar_t *AudioDeviceClientToDevice::pGetDeviceName( void ) const
{
	return m_InputDeviceClientName.c_str();
}

size_t AudioDeviceClientToDevice::GetSampleRate( void ) const 
{	return m_SampleRate;
}
size_t AudioDeviceClientToDevice::GetNoChannels( void ) const 
{	return m_NoChannels;
}

Audio::FormatInterface::eSampleFormat AudioDeviceClientToDevice::GetSampleFormat( void ) const
{	return m_SampleFormat;
}
bool AudioDeviceClientToDevice::GetIsBufferInterleaved( void ) const
{	return m_bIsInterleaved;
}
