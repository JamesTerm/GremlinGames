#include "StdAfx.h"
#include "GG_Framework.UI.Audio.h"
#include "..\..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Base;
using namespace GG_Framework::UI::Audio;

//Pointers are free (no unwinding issues) :)
ISoundSystem *ISoundSystem::s_Instance=NULL;

  /***************************************************************************************************************************/
 /*												FMODwrapper::Sound															*/
/***************************************************************************************************************************/


FMOD::Sound *Sound::GetSoundInternal() 
{
	return m_SoundInternal;
}

void Sound::Release() 
{
	m_SoundInternal->release();
}

bool Sound::set3DMinMaxDistance(float minDistance,float maxDistance)
{
	return (m_SoundInternal->set3DMinMaxDistance(minDistance,maxDistance)==FMOD_OK);
}

bool Sound::setMode(SoundMode_Flags mode)
{
	//This works assuming I keep my SoundMode_Flags up to date with the FMOD_MODE flags; otherwise I will need a translation table
	return (m_SoundInternal->setMode((FMOD_MODE)mode)==FMOD_OK);
}

bool Sound::setLoopPoints(size_t LoopStart,size_t LoopEnd)
{
	return (m_SoundInternal->setLoopPoints((unsigned int)LoopStart,FMOD_TIMEUNIT_PCM,(unsigned int)LoopEnd,FMOD_TIMEUNIT_PCM)==FMOD_OK);
}

size_t Sound::getLength()
{
	unsigned int length=-1;
	if (m_SoundInternal->getLength(&length,FMOD_TIMEUNIT_PCM)!=FMOD_OK)
		length=-1;

	return (size_t)length;
}

bool Sound::getDefaults(float *frequency,float *volume,float *pan,int *priority)
{
	return (m_SoundInternal->getDefaults(frequency,volume,pan,priority)==FMOD_OK);
}
bool Sound::setDefaults(float frequency,float volume,float pan,int priority)
{
	return (m_SoundInternal->setDefaults(frequency,volume,pan,priority)==FMOD_OK);
}

  /***************************************************************************************************************************/
 /*												FMODwrapper::Channel														*/
/***************************************************************************************************************************/

FMOD::Channel *Channel::GetChannelInternal() 
{
	return m_ChannelInternal;
}

void Channel::Stop() 
{
	m_ChannelInternal->stop();
}

bool Channel::Set3DAttributes(const Vec3 *pos,const Vec3 *vel) 
{
	DEBUG_SOUNDPOSITIONS("Channel::Set3DAttributes\n\tpos=(%3.2f,%3.2f,%3.2f) vel=(%3.2f,%3.2f,%3.2f)\n", 
		pos->x, pos->y, pos->z, vel->x, vel->y, vel->z);
	return (m_ChannelInternal->set3DAttributes((FMOD_VECTOR *)pos,(FMOD_VECTOR *)vel)==FMOD_OK);
}

bool Channel::IsPlaying(bool &is_playing_status)
{
	return (m_ChannelInternal->isPlaying(&is_playing_status)==FMOD_OK);
}
bool Channel::GetPaused(bool &is_paused_status)
{
	return (m_ChannelInternal->getPaused(&is_paused_status)==FMOD_OK);
}
bool Channel::SetPaused(bool paused)
{
	return (m_ChannelInternal->setPaused(paused)==FMOD_OK);
}
bool Channel::set3DMinMaxDistance(float minDistance,float maxDistance)
{
	return (m_ChannelInternal->set3DMinMaxDistance(minDistance,maxDistance)==FMOD_OK);
}
bool Channel::setLoopPoints(size_t LoopStart,size_t LoopEnd)
{
	return (m_ChannelInternal->setLoopPoints((unsigned int)LoopStart,FMOD_TIMEUNIT_PCM,(unsigned int)LoopEnd,FMOD_TIMEUNIT_PCM)==FMOD_OK);
}
bool Channel::setFrequency(float  frequency)
{
	return (m_ChannelInternal->setFrequency(frequency)==FMOD_OK);
}
bool Channel::setVolume(float Volume)
{
	return (m_ChannelInternal->setVolume(ComputeVolumeLevel(Volume))==FMOD_OK);
}
bool Channel::setLoopCount(size_t loopcount)
{
	return (m_ChannelInternal->setLoopCount((int)loopcount)==FMOD_OK);
}
bool Channel::getMode(SoundMode_Flags &mode)
{
	//This casting works assuming the flags are the same (and they are)
	return (m_ChannelInternal->getMode((FMOD_MODE *)&mode)==FMOD_OK);
}

  /***************************************************************************************************************************/
 /*											FMODwrapper::SoundSystemBase													*/
/***************************************************************************************************************************/

SoundSystemBase::SoundSystemBase() : m_MasterVolume(1.0),m_SoundsVolume(1.0),m_StreamsVolume(1.0)
{
}

void SoundSystemBase::SoundSystemBase_ShutDown()
{
	//TODO determine why sounds may be NULL... for now we check for NULL
	MappedSoundFiles::iterator pos;
	for (pos = m_Sounds.begin(); pos != m_Sounds.end() ; pos++)
	{
		if ((*pos).second)
			(*pos).second->Release();
	}
	m_Sounds.clear();
	for (pos = m_Streams.begin(); pos != m_Streams.end() ; pos++)
	{
		if ((*pos).second)
			(*pos).second->Release();
	}
	m_Streams.clear();
}

ISound *SoundSystemBase::CreateSound(const char *FileName,SoundMode_Flags mode)
{
	ISound *element = m_Sounds[FileName];
	if (!element)
	{
		element = CreateSound_internal(FileName,mode);		//This could fail (e.g. file not found)
		if (element)
		{
			m_Sounds[FileName] = element;
			//Get the current volume settings
			double SoundVolume=GetSoundsVolumeWithMaster();
			float frequency,volume,pan;
			int priority;
			element->getDefaults(&frequency,&volume,&pan,&priority);
			volume=SoundVolume;
			element->setDefaults(frequency,volume,pan,priority);
		}
		else
			DebugOutput("Could not create sound %f\n",FileName); //soft warning
	}
	return element;
}

ISound *SoundSystemBase::CreateStream(const char *FileName,SoundMode_Flags mode)
{
	//Note: we'll cache these as well, but it should be noted if two instances attempt to play this at the same time... the last one will
	//override the previous request.  Typically the streaming nature of the client should not have a problem
	ISound *element = m_Streams[FileName];
	if (!element)
	{
		element = CreateStream_internal(FileName,mode);		//This could fail (e.g. file not found)
		if (element)
		{
			m_Streams[FileName] = element;
			//Get the current volume settings
			double StreamVolume=GetStreamsVolumeWithMaster();
			float frequency,volume,pan;
			int priority;
			element->getDefaults(&frequency,&volume,&pan,&priority);
			volume=StreamVolume;
			element->setDefaults(frequency,volume,pan,priority);
		}
		else
			DebugOutput("Could not create stream %f\n",FileName); //soft warning
	}
	return element;
}

void SoundSystemBase::SetMasterVolume(float volume)
{
	m_MasterVolume=volume;
	double SoundsVolumeWithMaster=GetSoundsVolumeWithMaster();
	MappedSoundFiles::iterator pos;
	for (pos = m_Sounds.begin(); pos != m_Sounds.end() ; ++pos)
	{
		ISound *sound=(*pos).second;
		if (sound)
		{
			float frequency,volume,pan;
			int priority;
			sound->getDefaults(&frequency,&volume,&pan,&priority);
			volume=SoundsVolumeWithMaster;
			sound->setDefaults(frequency,volume,pan,priority);
		}
	}
	double StreamsVolumeWithMaster=GetStreamsVolumeWithMaster();
	for (pos = m_Streams.begin(); pos != m_Streams.end() ; ++pos)
	{
		ISound *sound=(*pos).second;
		if (sound)
		{
			float frequency,volume,pan;
			int priority;
			sound->getDefaults(&frequency,&volume,&pan,&priority);
			volume=StreamsVolumeWithMaster;
			sound->setDefaults(frequency,volume,pan,priority);
		}
	}
}
void SoundSystemBase::SetSoundsVolume(float volume)
{
	m_SoundsVolume=volume;
	double SoundsVolumeWithMaster=GetSoundsVolumeWithMaster();
	MappedSoundFiles::iterator pos;
	for (pos = m_Sounds.begin(); pos != m_Sounds.end() ; ++pos)
	{
		ISound *sound=(*pos).second;
		if (sound)
		{
			float frequency,volume,pan;
			int priority;
			sound->getDefaults(&frequency,&volume,&pan,&priority);
			volume=SoundsVolumeWithMaster;
			sound->setDefaults(frequency,volume,pan,priority);
		}
	}
}

void SoundSystemBase::SetStreamsVolume(float volume)
{
	m_StreamsVolume=volume;
	double StreamsVolumeWithMaster=GetStreamsVolumeWithMaster();
	MappedSoundFiles::iterator pos;
	for (pos = m_Streams.begin(); pos != m_Streams.end() ; ++pos)
	{
		ISound *sound=(*pos).second;
		if (sound)
		{
			float frequency,volume,pan;
			int priority;
			sound->getDefaults(&frequency,&volume,&pan,&priority);
			volume=StreamsVolumeWithMaster;
			sound->setDefaults(frequency,volume,pan,priority);
		}
	}
}

  /***************************************************************************************************************************/
 /*												FMODwrapper::SoundSystem													*/
/***************************************************************************************************************************/


const int MaxNoChannels=100;
//TODO see if this const is really needed
const float DISTANCEFACTOR = 1.0f;

void SoundSystem::ShutDown()
{
	SoundSystemBase_ShutDown();
	if (m_System)
	{
		m_System->close();
		m_System->release();
		m_System=NULL;
	}
	//Delete all our channel containers
	for (MappedChannelIndexes::iterator iter=m_Channels.begin(); iter != m_Channels.end(); iter++)
	{
		if ((*iter).second)
			delete ((*iter).second);
	}
}

SoundSystem::SoundSystem() :
m_System(NULL)
{
	FMOD_RESULT result;
	enum Preconditions
	{
		eSystemCreate,
		eSystemInit
	};
	const char * const ErrorMessage[]=
	{
		"System_Create Failed",
		"System->Init Failed"
	};

	try
	{
		if ((result=FMOD::System_Create(&m_System)) != FMOD_OK) throw eSystemCreate;
		if ((result=m_System->init(MaxNoChannels, FMOD_INIT_NORMAL, 0)) != FMOD_OK) throw eSystemInit;

		//I'm not sure if this is needed, but now we have a good default
		result = m_System->set3DSettings(1.0, DISTANCEFACTOR, 1.0f);
	}
	catch (int ErrorCode)
	{
		DebugOutput("SoundSystem_FMOD::SoundSystem_FMOD %s\n",ErrorMessage[ErrorCode]);
		DebugOutput("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		ShutDown();  //self clean (make life easy for the client)
		ASSERT(false);  //TODO error handle if this fails
	}
}

SoundSystem::~SoundSystem()
{
	ShutDown();  //Nothing more to do
}

ISound *SoundSystem::CreateSound_internal(const char *FileName,SoundMode_Flags mode)
{
	FMOD_RESULT result;
	FMOD::Sound *FMODSound;
	ISound *ret=NULL;
	result = m_System->createSound(FileName, (FMOD_MODE) mode, 0, &FMODSound);
	if (result == FMOD_OK)
		ret=new Sound(FMODSound);
	return ret;
}

ISound *SoundSystem::CreateStream_internal(const char *FileName,SoundMode_Flags mode)
{
	FMOD_RESULT result;
	FMOD::Sound *FMODSound;
	ISound *ret=NULL;
	result = m_System->createStream(FileName, (FMOD_MODE) mode, 0, &FMODSound);
	if (result == FMOD_OK)
		ret=new Sound(FMODSound);
	return ret;
}

bool SoundSystem::PlaySound(ISound *sound,bool paused,IChannel **channel)
{
	ASSERT(sound);
	FMOD_RESULT result;
	FMOD::Channel * FMODChannel=(channel && *channel)!=NULL?dynamic_cast<Channel *>(*channel)->GetChannelInternal():NULL;	

	if (!FMODChannel)
	{
		result = m_System->playSound(FMOD_CHANNEL_FREE, dynamic_cast<Sound *>(sound)->GetSoundInternal(), paused, &FMODChannel);
	}
	else
	{
		int channelindex;
		FMODChannel->getIndex(&channelindex);
		result = m_System->playSound((FMOD_CHANNELINDEX)channelindex,dynamic_cast<Sound *>(sound)->GetSoundInternal(), paused, &FMODChannel);
	}
    
	if (result != FMOD_OK)
	{
		DebugOutput("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		ASSERT(false);
	}
	else
	{
		Channel_Base *MappedChannel=m_Channels[FMODChannel];
		if (!MappedChannel)
		{
			MappedChannel=m_Channels[FMODChannel]=new Channel(FMODChannel);
			{	//Apply the default volume to the new channel
				SoundMode_Flags mode;
				bool result=MappedChannel->getMode(mode);
				ASSERT(result);
				double soundvolume=GetSoundsVolumeWithMaster();
				double streamvolume=GetStreamsVolumeWithMaster();
				double volume=(mode&fUnique)?streamvolume:soundvolume;
				//printf("mode=%x\n",mode);
				SetChannelBaseDefaultVolume(MappedChannel,volume);
			}
			ASSERT(MappedChannel);
		}
		if (channel)
			*channel=MappedChannel;
		//Typically this grows at a slow rate probably 100 items in 5 minutes.  Each item is very small and the collection does not take anytime to complete
		//The only thing to be watchful for is to put a critical section around this and volume control if they are called from separate threads.
		//printf("\r%d        ",m_Channels.size());
		if (m_Channels.size()>200)
		{	//Garbage collect channels that are no longer being used
			for (MappedChannelIndexes::iterator iter=m_Channels.begin();iter != m_Channels.end();)
			{
				bool isPlaying=false;
				//don't nuke the one we just made
				if ((*iter).second &&
					((*iter).second!=MappedChannel)&&
					(((*iter).second->IsPlaying(isPlaying)==false) || !isPlaying))
				{
					delete (*iter).second;
					iter=m_Channels.erase(iter);
				}
				else
					iter++;
			}
		}
	}
	return result==FMOD_OK;
}
void SoundSystem::SetMasterVolume(float volume)
{
	__super::SetMasterVolume(volume);
	double soundvolume=GetSoundsVolumeWithMaster();
	double streamvolume=GetStreamsVolumeWithMaster();
	for (MappedChannelIndexes::iterator iter=m_Channels.begin();iter != m_Channels.end(); iter++)
	{
		bool isPlaying=false;
		if ((*iter).second && ((*iter).second->IsPlaying(isPlaying)==true) && isPlaying)
		{
			SoundMode_Flags mode;
			bool result=(*iter).second->getMode(mode);
			ASSERT(result);
			double volume=(mode&fUnique)?streamvolume:soundvolume;
			//printf("mode=%x\n",mode);
			SetChannelBaseDefaultVolume((*iter).second,volume);
		}
	}
}
void SoundSystem::SetSoundsVolume(float volume)
{
	__super::SetSoundsVolume(volume);
	double soundvolume=GetSoundsVolumeWithMaster();
	for (MappedChannelIndexes::iterator iter=m_Channels.begin();iter != m_Channels.end(); iter++)
	{
		bool isPlaying=false;
		if ((*iter).second && ((*iter).second->IsPlaying(isPlaying)==true) && isPlaying)
		{
			SoundMode_Flags mode;
			bool result=(*iter).second->getMode(mode);
			ASSERT(result);
			if (mode&fUnique) continue;
			//printf("mode=%x\n",mode);
			SetChannelBaseDefaultVolume((*iter).second,soundvolume);
		}
	}

}
void SoundSystem::SetStreamsVolume(float volume)
{
	__super::SetStreamsVolume(volume);
	double streamvolume=GetStreamsVolumeWithMaster();
	for (MappedChannelIndexes::iterator iter=m_Channels.begin();iter != m_Channels.end(); iter++)
	{
		bool isPlaying=false;
		if ((*iter).second && ((*iter).second->IsPlaying(isPlaying)==true) && isPlaying)
		{
			SoundMode_Flags mode;
			bool result=(*iter).second->getMode(mode);
			ASSERT(result);
			if ((mode&fUnique)==0) continue;
			//printf("mode=%x\n",mode);
			SetChannelBaseDefaultVolume((*iter).second,streamvolume);
		}
	}
}

bool SoundSystem::Set3DListenerAttributes(int listener,const Vec3 *pos,const Vec3 *vel,const Vec3 *forward,const Vec3 *up)
{
	FMOD_RESULT result;
	DEBUG_SOUNDPOSITIONS("SoundSystem::Set3DListenerAttributes\n\tpos=(%3.2f,%3.2f,%3.2f) vel=(%3.2f,%3.2f,%3.2f)\n\tforward(%3.2f,%3.2f,%3.2f) up=(%3.2f,%3.2f%,%3.2f)\n", 
		pos->x, pos->y, pos->z, vel->x, vel->y, vel->z, forward->x, forward->y, forward->z, up->x, up->y, up->z);
	result=m_System->set3DListenerAttributes(listener,(FMOD_VECTOR *)pos,(FMOD_VECTOR *)vel,(FMOD_VECTOR *)forward,(FMOD_VECTOR *)up);
	if (result != FMOD_OK)
	{
		DebugOutput("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		ASSERT(false);
	}
	return result==FMOD_OK;
}

void SoundSystem::SystemFrameUpdate()
{
	m_System->update();
	//call predecessor (so it can fire the event)
	__super::SystemFrameUpdate();
}



  /***************************************************************************************************************************/
 /*													SoundSystem Mock														*/
/***************************************************************************************************************************/

Sound_Mock::Sound_Mock()
{
	DebugOutput("%p Sound_Mock::Sound_Mock()\n",this);
}
Sound_Mock::~Sound_Mock()
{
	DebugOutput("%p Sound_Mock::~Sound_Mock()\n",this);
}

void Sound_Mock::Release()
{
	DebugOutput("%p Sound_Mock::Release()\n",this);
}

bool Sound_Mock::set3DMinMaxDistance(float minDistance,float maxDistance)
{
	return true;
}

bool Sound_Mock::setMode(SoundMode_Flags mode)
{
	return true;
}
bool Sound_Mock::setLoopPoints(size_t LoopStart,size_t LoopEnd)
{
	return true;
}
size_t Sound_Mock::getLength()
{
	return 2048;
}
bool Sound_Mock::getDefaults(float *frequency,float *volume,float *pan,int *priority)
{
	return true;
}
bool Sound_Mock::setDefaults(float frequency,float volume,float pan,int priority)
{
	return true;
}

Channel_Mock::Channel_Mock()
{
	DebugOutput("%p Channel_Mock::Channel_Mock()\n",this);
}
Channel_Mock::~Channel_Mock()
{
	DebugOutput("%p Channel_Mock::~Channel_Mock()\n",this);
}
void Channel_Mock::Stop()
{
	DebugOutput("%p Channel_Mock::Stop()\n",this);
}
bool Channel_Mock::Set3DAttributes(const Vec3 *pos,const Vec3 *vel)
{
	return true;
}
bool Channel_Mock::IsPlaying(bool &is_playing_status)
{
	is_playing_status=false;
	return true;
}
bool Channel_Mock::GetPaused(bool &is_paused_status)
{
	is_paused_status=true;
	return true;
}
bool Channel_Mock::SetPaused(bool paused)
{
	return true;
}
bool Channel_Mock::set3DMinMaxDistance(float minDistance,float maxDistance)
{
	return true;
}
bool Channel_Mock::setLoopPoints(size_t LoopStart,size_t LoopEnd)
{
	return true;
}
bool Channel_Mock::setLoopCount(size_t loopcount)
{
	return true;
}

bool Channel_Mock::setFrequency(float  frequency)
{
	return true;
}
bool Channel_Mock::setVolume(float Volume)
{
	return true;
}
bool Channel_Mock::getMode(SoundMode_Flags &mode)
{
	return true;
}

SoundSystem_Mock::SoundSystem_Mock()
{
	DebugOutput("%p SoundSystem_Mock::SoundSystem_Mock()\n",this);
}
SoundSystem_Mock::~SoundSystem_Mock()
{
	DebugOutput("%p SoundSystem_Mock::~SoundSystem_Mock()\n",this);
}

ISound *SoundSystem_Mock::CreateSound(const char *FileName,SoundMode_Flags mode)
{
	DebugOutput("%p SoundSystem_Mock::CreateSound()\n",this);
	return new Sound_Mock;
}
ISound *SoundSystem_Mock::CreateStream(const char *FileName,SoundMode_Flags mode)
{
	DebugOutput("%p SoundSystem_Mock::CreateStream()\n",this);
	return new Sound_Mock;
}
bool SoundSystem_Mock::PlaySound(ISound *sound,bool paused,IChannel **channel)
{
	DebugOutput("%p SoundSystem_Mock::PlaySound(sound=%p,paused=%d,channel=%p)\n",this,sound,paused,channel);
	if (channel && *channel==NULL)
	{
		*channel=&m_Channel;
	}
	if (channel)
		DebugOutput("SoundSystem_Mock::PlaySound continued... *channel=%p\n",*channel);
	return true;
}
bool SoundSystem_Mock::Set3DListenerAttributes(int listener,const Vec3 *pos,const Vec3 *vel,const Vec3 *forward,const Vec3 *up)
{
	return true;
}

void SoundSystem_Mock::SystemFrameUpdate()
{
}
void SoundSystem_Mock::SetMasterVolume(float volume)
{
	DebugOutput("SoundSystem_Mock::SetMasterVolume volume=%f\n",volume);
}
void SoundSystem_Mock::SetSoundsVolume(float volume)
{
	DebugOutput("SoundSystem_Mock::SetSoundsVolume volume=%f\n",volume);
}
void SoundSystem_Mock::SetStreamsVolume(float volume)
{
	DebugOutput("SoundSystem_Mock::SetStreamsVolume volume=%f\n",volume);
}

