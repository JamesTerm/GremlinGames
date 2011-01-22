// SoundEffects3D.h
#pragma once

//for client project access... stub out the FMOD reference
#ifndef FRAMEWORK_UI_AUDIO_EXPORTS
namespace FMOD
{
	class Sound;
	class Channel;
	class System;
}
#endif	FRAMEWORK_UI_AUDIO_EXPORTS

namespace GG_Framework
{
	namespace UI
	{
		namespace Audio
		{

class FRAMEWORK_UI_AUDIO_API Sound : public ISound
{
	public:
		Sound(FMOD::Sound *sound) : m_SoundInternal(sound) {}
		FMOD::Sound *GetSoundInternal(); // returns m_SoundInternal
	protected:
		virtual void Release();
		virtual bool set3DMinMaxDistance(float minDistance=1.0,float maxDistance=10000.0);

		virtual bool setMode(SoundMode_Flags mode);
		virtual bool setLoopPoints(size_t LoopStart,size_t LoopEnd);
		virtual size_t getLength();

		virtual bool Sound::getDefaults(float *frequency,float *volume,float *pan,int *priority);
		virtual bool setDefaults(float frequency,float volume,float pan,int priority);

	private:
		FMOD::Sound * const m_SoundInternal;
};


//In the base class of the channels the SoundSystem base can apply the default levels here
class SoundSystemBase;
class FRAMEWORK_UI_AUDIO_API Channel_Base : public IChannel
{
	public:
		Channel_Base() : m_DefaultVolume(1.0),m_CurrentLocalVolume(1.0) {}
		//Set volume should pass in its volume here and use the return value to apply
		double ComputeVolumeLevel(double LocalVolume)
		{
			m_CurrentLocalVolume=LocalVolume;
			return LocalVolume*m_DefaultVolume;
		}
	protected:
		friend SoundSystemBase;
		void SetDefaultVolume(double level) 
		{
			m_DefaultVolume=level;
			setVolume(m_CurrentLocalVolume);  //force the volume to re-compute
		}
		double GetDefaultVolume() {return m_DefaultVolume;}
	private:
		double m_DefaultVolume;
		double m_CurrentLocalVolume;  //cache current local to properly adjust default volume
};

class FRAMEWORK_UI_AUDIO_API Channel : public Channel_Base
{
	public:
		Channel(FMOD::Channel *channel) : m_ChannelInternal(channel) {}
		FMOD::Channel *GetChannelInternal(); //returns m_ChannelInternal
	protected:
		virtual void Stop();
		virtual bool Set3DAttributes(const Vec3 *pos,const Vec3 *vel);
		virtual bool IsPlaying(bool &is_playing_status);
		virtual bool GetPaused(bool &is_paused_status);
		virtual bool SetPaused(bool paused);
		virtual bool set3DMinMaxDistance(float minDistance=1.0,float maxDistance=10000.0);
		virtual bool setLoopPoints(size_t LoopStart,size_t LoopEnd);
		virtual bool setLoopCount(size_t loopcount);
		virtual bool setFrequency(float  frequency);
		virtual bool setVolume(float Volume);
		virtual bool getMode(SoundMode_Flags &mode);
	private:
		FMOD::Channel * const m_ChannelInternal;
};

class FRAMEWORK_UI_AUDIO_API SoundSystemBase : public ISoundSystem
{
	public:
		SoundSystemBase();
		//Be sure this is called before the system shuts down
		void SoundSystemBase_ShutDown(); 
		//These will cache all duplicate filenames
		virtual ISound *CreateSound(const char *FileName,SoundMode_Flags mode=f3DSound);
		virtual ISound *CreateStream(const char *FileName,SoundMode_Flags mode=f2DSound);

		float GetSoundsVolumeWithMaster() {return m_MasterVolume*m_SoundsVolume;}
		float GetStreamsVolumeWithMaster() {return m_MasterVolume*m_StreamsVolume;}
	protected:
		//override these with the actual creation
		virtual ISound *CreateSound_internal(const char *FileName,SoundMode_Flags mode=f3DSound)=0;
		virtual ISound *CreateStream_internal(const char *FileName,SoundMode_Flags mode=f2DSound)=0;

		//The base implementation will distribute the volume setting to each sound (not channel)
		//client code will need to ensure that the sound (or system) will ensure channels in flight
		//get the change applied
		virtual void SetMasterVolume(float volume);
		virtual void SetSoundsVolume(float volume);
		virtual void SetStreamsVolume(float volume);
		//derived class should set this for channels in flight
		void SetChannelBaseDefaultVolume(Channel_Base *channel,double volume) {channel->SetDefaultVolume(volume);}
	private:
		typedef std::map<std::string,ISound *, std::greater<std::string> > MappedSoundFiles;
		MappedSoundFiles m_Sounds;
		MappedSoundFiles m_Streams;
		float m_MasterVolume,m_SoundsVolume,m_StreamsVolume;
};

class FRAMEWORK_UI_AUDIO_API SoundSystem : public SoundSystemBase
{
	public:
		SoundSystem();
		virtual ~SoundSystem();
	protected:
		//Returns a valid handle if successful NULL otherwise
		//Note: Some flags like fUseSoftware and f3DSound only apply during the creation of the sound, and may not dynamically change
		virtual ISound *CreateSound_internal(const char *FileName,SoundMode_Flags mode=f3DSound);
		virtual ISound *CreateStream_internal(const char *FileName,SoundMode_Flags mode=f2DSound);
		virtual bool PlaySound(ISound *sound,bool paused,IChannel **channel=NULL);
		virtual bool Set3DListenerAttributes(int listener=0,const Vec3 *pos=NULL,const Vec3 *vel=NULL,const Vec3 *forward=NULL,const Vec3 *up=NULL);

		//These get applied to all of the channels
		virtual void SetMasterVolume(float volume);
		virtual void SetSoundsVolume(float volume);
		virtual void SetStreamsVolume(float volume);

		//This is called once a frame (This pipes directly into FMOD's update)
		virtual void SystemFrameUpdate();
	private:
		void ShutDown();  //Clean resources on exit and on construction if any preconditions fail
		FMOD::System *m_System;
		typedef std::map<FMOD::Channel *,Channel_Base *, std::greater<FMOD::Channel *> > MappedChannelIndexes;
		MappedChannelIndexes m_Channels;  //Keep track of all channel indexes used
};

//Here is a mock object for test purposes
class FRAMEWORK_UI_AUDIO_API Sound_Mock : public ISound
{
	public:
		Sound_Mock(); 
		~Sound_Mock();
	protected:
		virtual void Release();
		virtual bool set3DMinMaxDistance(float minDistance=1.0,float maxDistance=10000.0);

		virtual bool setMode(SoundMode_Flags mode);
		virtual bool setLoopPoints(size_t LoopStart,size_t LoopEnd);
		virtual size_t getLength();
		virtual bool getDefaults(float *frequency,float *volume,float *pan,int *priority);
		virtual bool setDefaults(float frequency,float volume,float pan,int priority);
};

class FRAMEWORK_UI_AUDIO_API Channel_Mock : public IChannel
{
	public:
		Channel_Mock();
		~Channel_Mock();
	protected:
		virtual void Stop();
		virtual bool Set3DAttributes(const Vec3 *pos,const Vec3 *vel);
		virtual bool IsPlaying(bool &is_playing_status);
		virtual bool GetPaused(bool &is_paused_status);
		virtual bool SetPaused(bool paused);
		virtual bool set3DMinMaxDistance(float minDistance=1.0,float maxDistance=10000.0);
		virtual bool setLoopPoints(size_t LoopStart,size_t LoopEnd);
		virtual bool setLoopCount(size_t loopcount);
		virtual bool setFrequency(float  frequency);
		virtual bool setVolume(float Volume);
		virtual bool getMode(SoundMode_Flags &mode);
};

class FRAMEWORK_UI_AUDIO_API SoundSystem_Mock : public ISoundSystem
{
	public:
		SoundSystem_Mock();
		virtual ~SoundSystem_Mock();
	protected:
		virtual ISound *CreateSound(const char *FileName,SoundMode_Flags mode=f3DSound);
		virtual ISound *CreateStream(const char *FileName,SoundMode_Flags mode=f2DSound);
		virtual bool PlaySound(ISound *sound,bool paused,IChannel **channel=NULL);
		virtual bool Set3DListenerAttributes(int listener=0,const Vec3 *pos=NULL,const Vec3 *vel=NULL,const Vec3 *forward=NULL,const Vec3 *up=NULL);
		virtual void SystemFrameUpdate();
		virtual void SetMasterVolume(float volume);
		virtual void SetSoundsVolume(float volume);
		virtual void SetStreamsVolume(float volume);
	private:
		Channel_Mock m_Channel;
};



		}	//end namespace Audio
	}  // end namespace UI
}//end namespace GG_Framework