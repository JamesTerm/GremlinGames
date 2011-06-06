#include "StdAfx.h"
#include "Devices_Audio_DirectSound.h"

//Make base classes look friendly
using namespace FrameWork::Base;
using namespace FrameWork::Threads;
using namespace Devices::Audio::Base::Streams;
using namespace Devices::Audio::Base::Host;
using namespace Devices::Audio::Base::aud_convert;
using namespace Devices::Audio::DirectSound;

#define INIT_DIRECTX_STRUCT(x) (ZeroMemory(&x, sizeof(x)), x.dwSize=sizeof(x))
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

  /***********************************************************************************************/
 /*										AudioStreamData											*/
/***********************************************************************************************/


DirectSoundHost_Internal::AudioStreamData::AudioStreamData(DirectSoundHost_Internal *pParent,void *AudioInputStreamHandle) :
m_pParent(pParent),
m_AudioInputStreamHandle(AudioInputStreamHandle),
m_GTE_LastCalled(0)
{
	m_FillPosition=(DWORD)m_pParent->GetBufferSampleSize(); //this way the first buffer will get filled when it starts
	//Set up the secondary buffer
	DSBUFFERDESC dsbdesc;
	INIT_DIRECTX_STRUCT(dsbdesc);
	dsbdesc.dwFlags         = 
		DSBCAPS_GLOBALFOCUS			|	//If my window is minimized I'm still going to be heard!
		DSBCAPS_STATIC				|	//Use hardware buffers if I can
		DSBCAPS_GETCURRENTPOSITION2;	//We are a new product use the best accurate code

	WAVEFORMATEX *FormatToUse=pParent->GetWaveFormatToUse();
	dsbdesc.dwBufferBytes=m_BufferSizeInBytes=(DWORD)FormatToUse->nBlockAlign * (pParent->GetBufferSampleSize()*2);
	dsbdesc.lpwfxFormat=FormatToUse;
	try
	{
		if FAILED(pParent->GetDirectSoundObject()->CreateSoundBuffer(&dsbdesc,&m_lpdsb,NULL))
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
		m_lpdsb->Play(0,0,DSBPLAY_LOOPING);
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

DirectSoundHost_Internal::AudioStreamData::~AudioStreamData()
{
	if (m_lpdsb)
	{
		m_lpdsb->Stop();
		m_lpdsb->Release();
		m_lpdsb=NULL;
	}
}

double DirectSoundHost_Internal::AudioStreamData::GetTimeElapsed()
{
	DWORD playpos;
	//assert m_lpdsb
	m_lpdsb->GetCurrentPosition(&playpos,NULL);
	playpos/=m_pParent->GetWaveFormatToUse()->nBlockAlign; //convert to samples

	long PointerDifference=long(playpos)-m_GTE_LastCalled;
	if (PointerDifference<0.0)
	{
		size_t BufferSize=m_pParent->GetBufferSampleSize()<<1;
		PointerDifference=((long)BufferSize-m_GTE_LastCalled) + (long)playpos;
	}
	m_GTE_LastCalled=(long)playpos;
	return (double)PointerDifference / (double)m_pParent->GetSampleRate();
	//return (double)PointerDifference;
}


void DirectSoundHost_Internal::AudioStreamData::FillBuffer()
{
	void *dsbuf1,*dsbuf2;
	DWORD dsbuflen1,dsbuflen2,playpos;
	DWORD HalfBufferSize=m_BufferSizeInBytes/2;

	m_lpdsb->GetCurrentPosition(&playpos,NULL);
	playpos/=m_pParent->GetWaveFormatToUse()->nBlockAlign; //convert to samples
	//DebugOutput(L"%d \n",playpos);
	if ((playpos & m_pParent->GetBufferSampleSize())!=(m_FillPosition))
	{
		if (m_lpdsb->Lock((m_FillPosition/m_pParent->GetBufferSampleSize())*HalfBufferSize,HalfBufferSize,&dsbuf1,&dsbuflen1,&dsbuf2,&dsbuflen2,0)==DS_OK)
		{
			//handy debug info
			//DebugOutput("cursor=%x,buf1=%x,buf1len=%d,buf2=%x,buf2len=%d\n",playpos,dsbuf1,dsbuflen1,dsbuf2,&dsbuflen2);

			//Now to get the callback sample info
			AudioHostCallback_Interface *ahi=m_pParent->GetAudioHostCallback_Interface();
			bool success=false;
			if (ahi)
				success=ahi->FillBuffer((byte *)dsbuf1,m_pParent->GetBufferSampleSize(),m_AudioInputStreamHandle);
			if (!success) //Fill with silence
				FillMemory((char *)dsbuf1,dsbuflen1,(BYTE)(m_pParent->GetWaveFormatToUse()->wBitsPerSample == 8 ? 128 : 0));

			m_FillPosition=playpos& m_pParent->GetBufferSampleSize();
			if FAILED(m_lpdsb->Unlock(dsbuf1,dsbuflen1,dsbuf2,dsbuflen2))
				printf("Unable to unlock DS buffer\n");
		}
		else
			DebugOutput(L"Error cursor=%x,buf1=%x,buf1len=%d,buf2=%x,buf2len=%d\n",playpos,dsbuf1,dsbuflen1,dsbuf2,&dsbuflen2);
	}
	//else
	//	DebugOutput("Skipping... cursor=%x,last=%x\n",playpos,m_FillPosition);
}


  /***********************************************************************************************/
 /*										DirectSoundHost_Internal								*/
/***********************************************************************************************/

DirectSoundHost_Internal::DirectSoundHost_Internal() :
m_lpDS(NULL),
m_pAudioHostCallback(NULL),
m_pThread(NULL)
{
	
	m_NotificationTimer=CreateWaitableTimer(NULL,false,NULL);
	assert(m_NotificationTimer); //should work right?
	//Note I can't set this until I know what sample rate that I will be using... (see in startup)
}

DirectSoundHost_Internal::~DirectSoundHost_Internal()
{
	//We must remove the stream before calling shutdown since the secondary buffers must be released before the direct sound interface
	while(m_AudioStreams.size())
		AudioHost_RemoveStream(m_AudioStreams[0]->AudioInputStream_ptr);

	ShutDown();
	
	CloseHandle(m_NotificationTimer);
}

double DirectSoundHost_Internal::GetTimeElapsed(void *AudioInputStream_ptr)
{
	double result=0.0;
	//Find the audio stream
	auto_lock ReadList(m_AudioStreams_Lock);

	size_t StreamIndex=-1;
	AudioHostStreamData *Container=NULL;

	for (size_t i=0;i<m_AudioStreams.size();i++)
	{
		Container=m_AudioStreams[i];
		if (Container->AudioInputStream_ptr==AudioInputStream_ptr)
		{
			StreamIndex=i;
			break;
		}
	}

	if (StreamIndex!=-1)
	{
		AudioStreamData *DSStream=(AudioStreamData *)Container->AudioHostData;
		assert(DSStream);

		result=DSStream->GetTimeElapsed();
	}
	return result;
}


AudioHostCallback_Interface *DirectSoundHost_Internal::GetAudioHostCallback_Interface( void )
{	
	return m_pAudioHostCallback;
}

void DirectSoundHost_Internal::SetAudioHostCallback_Interface(AudioHostCallback_Interface *ahi) 
{	
	m_pAudioHostCallback=ahi;
}		

size_t DirectSoundHost_Internal::GetSampleRate( void ) const 
{	
	return (size_t)m_FormatToUse.nSamplesPerSec;
}


size_t DirectSoundHost_Internal::GetNoChannels( void ) const 
{	
	return (size_t)m_FormatToUse.nChannels;
}

size_t DirectSoundHost_Internal::GetBufferSampleSize( void ) const
{	
	//Sample size Must be a power of 2 (so it can do some anding techniques)
	//1024 is too small for the notifications to be reliable
	//We want this small enough to minimize in latency
	return 2048;
}

WAVEFORMATEX *DirectSoundHost_Internal::GetWaveFormatToUse( void ) 
{	
	return &m_FormatToUse;
}

LPDIRECTSOUND8 DirectSoundHost_Internal::GetDirectSoundObject( void ) 
{
	return m_lpDS;
}

void DirectSoundHost_Internal::SetWaveFormat(WAVEFORMATEX &wfx,WORD wFormatTag,WORD nChannels,DWORD nSamplesPerSecond,WORD wBitsPerSample)
{
	memset(&wfx,0,sizeof(WAVEFORMATEX));
	wfx.wFormatTag=wFormatTag;
	wfx.nChannels=nChannels;
	wfx.nSamplesPerSec=nSamplesPerSecond;
	wfx.wBitsPerSample=wBitsPerSample;
	wfx.nBlockAlign=wfx.wBitsPerSample/8*wfx.nChannels;
	wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nBlockAlign;
}

Devices::Audio::FormatInterface::eSampleFormat DirectSoundHost_Internal::GetSampleFormat( void ) const
{
	Devices::Audio::FormatInterface::eSampleFormat ret=Devices::Audio::FormatInterface::eSampleFormat_U_8; //TODO make an unknown one
	//TODO figure out which channel to use
	WORD bps=m_FormatToUse.wBitsPerSample;
	switch(bps)
	{
	case 8:
		ret=Devices::Audio::FormatInterface::eSampleFormat_U_8;
		break;
	case 16:
		ret=Devices::Audio::FormatInterface::eSampleFormat_I_16;
		break;

	case 32:
		if (m_FormatToUse.wFormatTag==WAVE_FORMAT_IEEE_FLOAT)
			ret=Devices::Audio::FormatInterface::eSampleFormat_FP_32;
		else
			ret=Devices::Audio::FormatInterface::eSampleFormat_I_32;
		break;
	}
	return ret;
}

void DirectSoundHost_Internal::StartUp(HWND ParentWindow)
{
	LPDIRECTSOUNDBUFFER lpdsbPrimary=NULL;
	//DirectSoundInit area
	//Preconditions:
	{
		//TODO find out how to get the GUIDS of all WDM drivers
		if (FAILED(DirectSoundCreate8(NULL,&m_lpDS,NULL)))
			assert(false),throw "Unable to create DirectSound object" __FUNCSIG__;
		//Note: Cooperative level of DSSCL priority allows formating the primary buffer
		if (FAILED(m_lpDS->SetCooperativeLevel(ParentWindow,DSSCL_PRIORITY)))
			assert(false),throw "Unable to set CooperativeLevel in DS" __FUNCSIG__;
		//Obtain primary buffer
		DSBUFFERDESC dsbdesc;
		INIT_DIRECTX_STRUCT(dsbdesc);
		dsbdesc.dwFlags=DSBCAPS_PRIMARYBUFFER;
		if FAILED(m_lpDS->CreateSoundBuffer(&dsbdesc,&lpdsbPrimary,NULL))
			assert(false),throw "Unable to create DS primary sound buffer" __FUNCSIG__;


		{ //Well try some formats (until I can figure out how to do this better!)
			size_t i=0;
			const size_t NoElements=12;
			struct formatdata
			{
				WORD wFormatTag;
				WORD nChannels;
				DWORD nSamplesPerSecond;
				WORD wBitsPerSample;

			} trythis[NoElements]=
			{
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
			if (m_lpDS->GetCaps(&capabilities)==DS_OK)
			{
				//Since GetCaps doesn't support 32bit flags I'll include all of these anyway
				filter=0x01|0x02|0x08|0x10|0x40|0x80|0x200|0x400; 
				DWORD MinSampleRate=capabilities.dwMinSecondarySampleRate;
				DWORD MaxSampleRate=capabilities.dwMaxSecondarySampleRate;
				if  (!(
					(MaxSampleRate==48000) || (MinSampleRate=48000) ||
					((capabilities.dwFlags & DSCAPS_CONTINUOUSRATE)&&(MaxSampleRate>48000)&&(MinSampleRate<48000))
					))
					filter &= (0x01|0x02|0x04|0x08|0x10|0x20); //exclude 48000 sample rates

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
			//You can test a specific format like so...
			//filter=0x200;
			//TODO DirectSound8 has has speaker configuration to try... for now we just try the 32 bit formats first

			bool Success=false;
			do 
			{
				if (!((1<<i) & filter))
					continue;
				SetWaveFormat(m_FormatToUse,trythis[i].wFormatTag,trythis[i].nChannels,trythis[i].nSamplesPerSecond,trythis[i].wBitsPerSample);
				Success=(lpdsbPrimary->SetFormat(&m_FormatToUse)==DS_OK);
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
					dsbdesc.dwBufferBytes=(DWORD)FormatToUse->nBlockAlign * (GetBufferSampleSize()*2);
					dsbdesc.lpwfxFormat=FormatToUse;
					if FAILED(m_lpDS->CreateSoundBuffer(&dsbdesc,&lpdsb,NULL))
						Success=false;
					if (lpdsb)
						lpdsb->Release();

				}
			} while ((!Success)&&(i++<(NoElements-1)));
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
		m_pThread = new thread<DirectSoundHost_Internal>(this);
	}
	//I do not need a reference to the primary buffer anymore
	if (lpdsbPrimary)
		lpdsbPrimary->Release();

}

void DirectSoundHost_Internal::ShutDown()
{
	//m_AbortThread = true;
	SetEvent(m_NotificationTimer);
	//StopThread();
	assert (m_pThread);
	delete m_pThread;

	if (m_lpDS)
	{
		m_lpDS->Release();
		m_lpDS=NULL;
	}
}


void DirectSoundHost_Internal::AudioHost_CreateNewStream(void *AudioInputStream_ptr)
{
	AudioStreamData *stream=new AudioStreamData(this,AudioInputStream_ptr); //Note: a lot of stuff happens in the constructor here...
	AudioHostStreamData *Container=NULL;

	Container=new AudioHostStreamData;
	Container->AudioHostData=(void *)stream;
	Container->AudioInputStream_ptr=(void *)AudioInputStream_ptr;

	m_AudioStreams_Lock.lock();
	m_AudioStreams.insert(m_AudioStreams.end(),Container);
	m_AudioStreams_Lock.unlock();
}

void DirectSoundHost_Internal::AudioHost_RemoveStream(void *AudioInputStream_ToRemove)
{
	//remove from list first
	m_AudioStreams_Lock.lock();
	size_t StreamIndex=-1;
	AudioHostStreamData *Container=NULL;

	for (size_t i=0;i<m_AudioStreams.size();i++)
	{
		Container=m_AudioStreams[i];
		if (Container->AudioInputStream_ptr==AudioInputStream_ToRemove)
		{
			StreamIndex=i;
			break;
		}
	}

	bool InputFound=true;
	if (StreamIndex!=-1)
	{
		//m_AudioStreams.DeleteEntry(i);
		m_AudioStreams.erase(m_AudioStreams.begin()+StreamIndex);

		//free the resources
		AudioStreamData *stream=(AudioStreamData *)Container->AudioHostData;
		delete stream;
		delete Container;
	}
	else
		InputFound=false;

	m_AudioStreams_Lock.unlock();

	// Error ? ... nobody upstream cares... ;)
}

void DirectSoundHost_Internal::operator() ( const void* )
{
	WaitForSingleObject(m_NotificationTimer,1000);
	m_AudioStreams_Lock.lock();
	//Notify all streams
	for (size_t i=0;i<m_AudioStreams.size();i++)
	{
		AudioHostStreamData *Container=m_AudioStreams[i];
		AudioStreamData *stream=(AudioStreamData *)Container->AudioHostData;
		stream->FillBuffer(); //notify our stream to update
	}
	m_AudioStreams_Lock.unlock();
}

  /***********************************************************************************************************************/
 /*														DirectSoundHost													*/
/***********************************************************************************************************************/

double DirectSoundHost::GetTimeElapsed(void *AudioInputStream_ptr)
{
	return DirectSoundHost_Internal::GetTimeElapsed(AudioInputStream_ptr);
}

void DirectSoundHost::SetAudioHostCallback_Interface(AudioHostCallback_Interface *ahi) 
{
	DirectSoundHost_Internal::SetAudioHostCallback_Interface(ahi);
}
AudioHostCallback_Interface *DirectSoundHost::GetAudioHostCallback() 
{	
	return m_pAudioHostCallback;
}
void DirectSoundHost::AudioInStreamClosing(void *ThisChild) 
{	
	SetAudioHostCallback_Interface(NULL); //no longer pointing to this interface
}

size_t DirectSoundHost::GetSampleRate( void ) const 
{
	return DirectSoundHost_Internal::GetSampleRate();
}

Devices::Audio::FormatInterface::eSampleFormat DirectSoundHost::GetSampleFormat( void ) const
{
	return DirectSoundHost_Internal::GetSampleFormat();
}
bool DirectSoundHost::GetIsBufferInterleaved( void ) const
{	
	return true;
}
size_t DirectSoundHost::GetNoChannels( void ) const 
{
	return DirectSoundHost_Internal::GetNoChannels();
}

size_t DirectSoundHost::GetBufferSampleSize() 
{
	return DirectSoundHost_Internal::GetBufferSampleSize();
}

void DirectSoundHost::AudioHost_CreateNewStream(void *AudioInputStream_ptr) 
{
	DirectSoundHost_Internal::AudioHost_CreateNewStream(AudioInputStream_ptr);
}

void DirectSoundHost::AudioHost_RemoveStream(void *AudioInputStream_ToRemove) 
{
	DirectSoundHost_Internal::AudioHost_RemoveStream(AudioInputStream_ToRemove);
}


  /***********************************************************************************************************************/
 /*												DirectSoundOutputDevice													*/
/***********************************************************************************************************************/


DirectSoundOutputDevice::DirectSoundOutputDevice(HWND hwnd,const wchar_t *pDeviceName) :
m_AudioHostDevice(&m_AudioHost,pDeviceName),
DeviceWrapper(m_AudioHostDevice.GetAudioDevice()),
AudioHostDeviceWrapper(&m_AudioHostDevice)
{	
	m_AudioHost.StartUp(hwnd);
}

double DirectSoundOutputDevice::GetTimeElapsed(void *AudioInputStream_ptr)
{
	return m_AudioHost.GetTimeElapsed(AudioInputStream_ptr);
}
