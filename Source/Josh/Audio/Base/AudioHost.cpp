
#include "stdafx.h"
// Include the main project
#include "Devices_Audio_Base.h"
// Local utilities
#include "Private_Utilities.h"

using namespace std;
using namespace FrameWork::Base;
using namespace FrameWork::Threads;
using namespace Devices::Audio;
using namespace Devices::Audio::Base::Host;
using namespace Devices::Audio::Base::aud_convert;
using namespace Devices::Audio::Base::Streams;
using namespace Devices::Streams;
//for debug test purposes only
//#include "../../../../User/James Killian/AudioTesting/Wave/Wave.h"

#undef __ShowInfo__

  /***************************************************************************************************/
 /*											AudioHostMixer											*/
/***************************************************************************************************/

AudioHostMixer::AudioHostMixer(AudioHost_Interface *pHost) :
m_pHost(pHost),m_ChannelMixer(this)
{
	//Temporary debug purposes
	//m_LastOutBufferSentTime=0;
	//m_OutSampleTime=2048.0/48000.0;
}

AudioHostMixer::~AudioHostMixer()
{
	//No longer filling buffers
	m_pHost->AudioInStreamClosing(this);

	// Remove all remaining inputs
	FrameWork::Threads::auto_lock ListLock( &m_AudioStreams_Lock );
	while(m_AudioStreams.size())
		RemoveAudioStream( m_AudioStreams[0] );
}


void AudioHostMixer::InStreamClosing(void *ThisChild) 
{	
	RemoveAudioStream((AudioHostInputStream *)ThisChild);
}
Devices::BufferInterface *AudioHostMixer::InStream_Interface_GetNewBuffer(InputStream *pInputStream)
{
	//We simply dynamic cast the pInputStream so that we don't have to determine which list entry it is referring to
	AudioHostInputStream *host=dynamic_cast<AudioHostInputStream *>(pInputStream);
	assert(host);
	//Note the buffer format corresponds to the device (the host can handle the format changes)
	return new AudioBuffer(host,host);
}

void AudioHostMixer::InStream_Interface_FreeBuffer(Devices::BufferInterface *buffer)
{
	delete dynamic_cast<AudioBuffer *>(buffer);
}

size_t AudioHostMixer::GetBufferSampleSize() 
{
	return m_pHost->GetBufferSampleSize();
}

size_t AudioHostMixer::GetSampleRate( void ) const 
{	
	return m_pHost->GetSampleRate();
}

Devices::Audio::FormatInterface::eSampleFormat AudioHostMixer::GetSampleFormat( void ) const
{
	return m_pHost->GetSampleFormat();
}

size_t AudioHostMixer::GetNoChannels( void ) const 
{	
	return m_pHost->GetNoChannels();
}

bool AudioHostMixer::GetIsBufferInterleaved( void ) const
{
	return m_pHost->GetIsBufferInterleaved();
}

float *AudioHostMixer::GetVolumeMatrix(AudioHostInputStream *stream) const 
{	
	return stream->GetVolumeMatrix();
}
size_t AudioHostMixer::GetSourceNoChannels(AudioHostInputStream *stream) const 
{	
	return stream->GetNoChannels();
}

size_t AudioHostMixer::ChannelMixer_GetSourceNoChannels(void *UserData) const 
{	
	return GetSourceNoChannels((AudioHostInputStream *)UserData);
}

size_t AudioHostMixer::ChannelMixer_GetDestNoChannels(void *UserData) const 
{	
	return m_pHost->GetNoChannels();
}
float *AudioHostMixer::ChannelMixer_GetVolumeMatrix(void *UserData) const 
{	
	return GetVolumeMatrix((AudioHostInputStream *)UserData);
}


bool AudioHostMixer::AddAudioStream(AudioHostInputStream *pAudioStream)
{
		// Already on list ?
		m_AudioStreams_Lock.lock();
		//const bool AlreadyOnList = m_AudioStreams.Exists( pAudioStream );
		const bool AlreadyOnList = std::find(m_AudioStreams.begin(),m_AudioStreams.end(), pAudioStream )!=m_AudioStreams.end();
		m_AudioStreams_Lock.unlock();
		if (AlreadyOnList) return true;

		// Try starting up the audio stream
		if (!pAudioStream->StartTBCQueue(-1,1)) //start with only one buffer to add
		{	// We failed to add it
			pAudioStream->FinishTBCQueue();

			// Error
			return false;
		}

		// Lock ths list
		FrameWork::Threads::auto_lock	ListLock( &m_AudioStreams_Lock );

		m_AudioStreams_Lock.lock();
		// Add it to the list
		m_AudioStreams.insert(m_AudioStreams.end(), pAudioStream );
		//let our host add its stream (if it is handling mixing internally)
		m_pHost->AudioHost_CreateNewStream((void *)pAudioStream);
		//m_Inputs_TimeStamp.PingTimeStamp();
		m_AudioStreams_Lock.unlock();

		// Success
		return true;

}

// Remove an input from the Audio host mixer
bool AudioHostMixer::RemoveAudioStream (AudioHostInputStream *pAudioStreamToRemove)
{	
	//avoid lockup by notifying the host to stop streaming for this stream
	m_pHost->AudioHost_RemoveStream(pAudioStreamToRemove);

	// Look for the input
	m_AudioStreams_Lock.lock();
	//const bool InputFound = m_AudioStreams.Delete( pAudioStreamToRemove );
	const bool InputFound=EraseFromList(m_AudioStreams,pAudioStreamToRemove);
	m_AudioStreams_Lock.unlock();

	// Error ?
	if (!InputFound) return false;

	// Call it's method
	pAudioStreamToRemove->FinishTBCQueue();
	//m_Inputs_TimeStamp.PingTimeStamp();

	// Success
	return true;
}

bool AudioHostMixer::FillBuffer(byte *Buffer,size_t NoSamples,AudioHostInputStream *AudioInputStream_ptr,bool mix)
{	
	bool Success=false;

	// buffer size in samples
	//TODO: for now we can assume all buffers available in the TBC queue will be equal to the host
	//But I need to support format conversion changes here since we may want to have Audio In Streams which
	//pull the format from the device (The AudioTBC is taking advantage of having to resample the buffers)
	//I will probably need this when we can have direct clock access to our hardware
	//ALSO!!... while were at conversions, this may be the best place to apply master panning eq effects etc!!

	assert(NoSamples<=m_pHost->GetBufferSampleSize()); //check here because I don't trust direct sound ;)
	size_t buffSize = min(m_pHost->GetBufferSampleSize(),NoSamples);

	//If there are no devices attached nothing will be transferred
	{
		// Lock the TBC queue
		size_t QueueDepth;
		vector<Devices::BufferInterface *>	&l_TBCQueue=AudioInputStream_ptr->GetTBCQueue();
		FrameWork::Threads::critical_section &l_TBCQueue_Lock=AudioInputStream_ptr->GetTBCQueue_Lock();

		l_TBCQueue_Lock.lock();
		Devices::BufferInterface *l_AudioBuffer=NULL;
		QueueDepth=l_TBCQueue.size();
		if (QueueDepth)
			l_AudioBuffer=l_TBCQueue[0];
		l_TBCQueue_Lock.unlock();

		if (l_AudioBuffer)
		{

			//Here is a manual way to test a few channels
			//size_t BytesPerSample=sizeof(float) * GetNoChannels();
			//memcpy (Buffer,l_AudioBuffer->m_Memory , buffSize*BytesPerSample);

			//Note this will have the device format mixed down to the hosts number of channels
			m_ChannelMixer.Mix((float *)l_AudioBuffer->pGetBufferData(NULL),(float *)Buffer,buffSize,mix,(void *)AudioInputStream_ptr);

			AudioInputStream_ptr->AdvanceTBC();
			Success=true; //true if at least one device could fill in the memory

			/*
			{ //This is just debugging info only
				const double c_SmoothingRate=0.01; //I can't imagine this needing to change
				__int64 NewTime;
				QueryPerformanceCounter((LARGE_INTEGER *)&NewTime);
				if (m_LastOutBufferSentTime)
				{
					__int64 Frequency;
					//Compute the high precision sample rate here
					QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency);
					double dif=(double)(NewTime-m_LastOutBufferSentTime);
					m_OutSampleTime=(1.0-c_SmoothingRate) * m_OutSampleTime + ( c_SmoothingRate * (dif/(double)Frequency) );
					double CurrentSampleRate=(double)buffSize / m_OutSampleTime;
					//DebugOutput("%d time -> %f\n",buffSize,m_OutSampleTime);
					//m_OutSampleRate=(1.0-c_SmoothingRate) * m_OutSampleRate + ( c_SmoothingRate * CurrentSampleRate );
					DebugOutput("AM Current Out Samples %f\n",CurrentSampleRate);
					//DebugOutput("AudioMixer Samplerate-> %f\n",m_OutSampleRate);
				}
				m_LastOutBufferSentTime=NewTime;
			}
			*/
		}
		else
		{
			//printf("No Audio Buffer found\n");
			//here is a perfect place to fill with silence
			//the trick will be to only put silence for this channel and this stream
			size_t NoChannels=ChannelMixer_GetSourceNoChannels((void *)AudioInputStream_ptr);
			size_t ChannelSizeInBytes=NoSamples*sizeof(float);
			size_t BufferSizeInBytes=ChannelSizeInBytes*NoChannels;

			byte *SourceBuf=(byte *)_alloca(BufferSizeInBytes + 16);
			byte *AlignedSourceBuf=(byte *)(((size_t)SourceBuf+0x0f)&~0x0f); //We need this aligned to a 16 byte boundary
			memset(AlignedSourceBuf,0,BufferSizeInBytes);

			m_ChannelMixer.Mix((float *)SourceBuf,(float *)Buffer,buffSize,mix,(void *)AudioInputStream_ptr);
			Success=true;
		}
	}
	return Success;
}

AudioHostInputStream *AudioHostMixer::GetAudioHostInputStream(Devices::DeviceInterface *AudioDevice) const
{
	m_AudioStreams_Lock.lock();
	AudioHostInputStream *stream=NULL;
	for (size_t i=0;i<m_AudioStreams.size();i++)
	{
		if (m_AudioStreams[i]->GetDevice()==AudioDevice)
		{
			stream=m_AudioStreams[i];
			break;
		}
	}
	m_AudioStreams_Lock.unlock();

	return stream;
}

float *AudioHostMixer::GetVolumeMatrix(Devices::DeviceInterface *AudioDevice) const
{
	float *ret=NULL;
	AudioHostInputStream *stream=GetAudioHostInputStream(AudioDevice);
	if (stream)
		ret=GetVolumeMatrix(stream);
	return ret;
}

size_t AudioHostMixer::GetSourceNoChannels(Devices::DeviceInterface *AudioDevice) const
{
	size_t ret=NULL;
	AudioHostInputStream *stream=GetAudioHostInputStream(AudioDevice);
	if (stream)
		ret=GetSourceNoChannels(stream);
	return ret;
}


bool AudioHostMixer::FillBuffer(byte *Buffer,size_t NoSamples,void *AudioInputStream_ptr)
{
	bool Ret=false;

	//Set up a temp buffer that can work with floating point
	byte *TempBuf=(byte *)_alloca(sizeof(float)*NoSamples*GetNoChannels() + 16);

	byte *AlignedBuf;
	if ((GetSampleFormat()!=Devices::Audio::FormatInterface::eSampleFormat_FP_32)||(m_pHost->GetIsBufferInterleaved()))
		AlignedBuf=(byte *)(((size_t)TempBuf+0x0f)&~0x0f); //We need this aligned to a 16 byte boundary
	else
		AlignedBuf=Buffer; //avoid a memcopy

	//A nice little trick I learned from Andrew... ;)
	if (!AudioInputStream_ptr)
	{
		m_AudioStreams_Lock.lock();
		for(size_t i=0;i<m_AudioStreams.size();i++)
			Ret=FillBuffer(AlignedBuf,NoSamples,m_AudioStreams[i],i!=0);
		m_AudioStreams_Lock.unlock();
	}
	else
		Ret=FillBuffer(AlignedBuf,NoSamples,(AudioHostInputStream *)AudioInputStream_ptr,false);

	aud_convert::ConvertFromFloat((float *)AlignedBuf,NoSamples,Buffer,GetSampleFormat(),GetNoChannels(),1.0,
		false,m_pHost->GetIsBufferInterleaved());

	return Ret;
}


  /***************************************************************************************************/
 /*											AudioHostDevice											*/
/***************************************************************************************************/

AudioHostDevice::AudioHostDevice(AudioHost_Interface *AudioHost,const wchar_t *pDeviceName) :
AudioOutStream_NotThreaded(AudioHost,pDeviceName),m_Mixer(AudioHost)
{
	//We can go ahead and link up the AudioHost to this class for the buffer callbacks
	AudioHost->SetAudioHostCallback_Interface(this);
}

AudioHostDevice::~AudioHostDevice()
{
	while(m_InputStreams.size())
		RemoveAudioDevice( m_InputStreams[0]->GetDevice() );
}

float *AudioHostDevice::GetVolumeMatrix(DeviceInterface *AudioDevice) const 
{	
	return m_Mixer.GetVolumeMatrix(AudioDevice);
}
size_t AudioHostDevice::GetSourceNoChannels(DeviceInterface *AudioDevice) const 
{	
	return m_Mixer.GetSourceNoChannels(AudioDevice);
}

bool AudioHostDevice::FillBuffer(byte *Buffer,size_t NoSamples,void *AudioInputStream_ptr)
{
	if (m_Mixer.FillBuffer(Buffer,NoSamples,AudioInputStream_ptr))
	{
		TransferBuffer(Buffer,NoSamples);
		return true;
	}
	return false;
}

AudioHostInputStream *AudioHostDevice::AddAudioDevice(Devices::Audio::DeviceInterface *pDevice,size_t DefaultTBCQueueDepth)
{
	//Create a new input stream which is connected to this device
	AudioHostInputStream *NewStream=new AudioHostInputStream(&m_Mixer,pDevice,&m_Mixer,DefaultTBCQueueDepth);
	m_InputStreams.insert(m_InputStreams.end(),NewStream);
	//This mixer will safely add this new stream into the mix
	m_Mixer.AddAudioStream(NewStream);
	return NewStream;
}

bool AudioHostDevice::RemoveAudioDevice(DeviceInterface *pDeviceToRemove)
{
	bool InputFound=false;
	AudioHostInputStream *StreamToRemove=NULL;
	size_t eoi=m_InputStreams.size();
	for (size_t i=0;i<eoi;i++)
	{
		AudioHostInputStream *stream=m_InputStreams[i];
		if (stream->GetDevice()==pDeviceToRemove)
		{
			StreamToRemove=stream;
			break;
		}
	}
	if (StreamToRemove)
	{
		InputFound=m_Mixer.RemoveAudioStream(StreamToRemove);
		assert(InputFound); //warn me if this fails
		InputFound=EraseFromList(m_InputStreams,StreamToRemove);
		assert(InputFound); //warn me if this fails
		delete StreamToRemove;
	}
	return InputFound;
}
