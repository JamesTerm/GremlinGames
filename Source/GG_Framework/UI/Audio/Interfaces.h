#pragma once

//This will help the code be a bit easier to read
#define  SOUNDSYSTEM GG_Framework::UI::Audio::ISoundSystem::Instance()

namespace GG_Framework
{
	namespace UI
	{
		namespace Audio
		{


struct Vec3
{
	Vec3()	{}
	Vec3(float _x,float _y,float _z) : x(_x),y(_y),z(_z) {}
	//Coordinates in 3d space
	float x,y,z;
};


enum SoundMode_Flags
{
	fDefault	=0x00,  //Equivalent to all the defaults listed below.  FMOD_LOOP_OFF, FMOD_2D, FMOD_HARDWARE.
	fLoopOff	=0x01,  //For non looping sounds. (DEFAULT).  Overrides FMOD_LOOP_NORMAL / FMOD_LOOP_BIDI.
	fLoopNormal	=0x02,	//For forward looping sounds.
	fLoopBiDi	=0x04,  //For bidirectional looping sounds. (only works on software mixed static sounds).
	f2DSound	=0x08,	// Ignores any 3d processing. (DEFAULT)
	f3DSound	=0x10,	// Makes the sound position-able in 3D.  Overrides FMOD_2D.
	fUseHardware=0x20,	// Attempts to make sounds use hardware acceleration. (DEFAULT).
	fUseSoftware=0x40,	// Makes the sound be mixed by the FMOD CPU based software mixer.  Overrides FMOD_HARDWARE.  
						//Use this for FFT, DSP, compressed sample support, 2D multi-speaker support and other software related features.
	fUnique   =0x20000  //music, Unique sound, can only be played one at a time 
	//Add more modes as we need them...
};

class SoundSystemBase;
//Once a sound has been created it can be set to how it will be played
class FRAMEWORK_UI_AUDIO_API ISound
{
	public:
		virtual ~ISound() {}
		//TODO add loop info
		//In practice we should never need to adjust the maxDistance; however the min distance will establish how "loud" something is
		//in 3d space.  Using meters as our unit of measurement 0.1 would be a good setting for a bee, and 100.0 would be a good setting
		//for a jet.  The Default is 1.0
		virtual bool set3DMinMaxDistance(float minDistance=1.0,float maxDistance=10000.0)=0;
		virtual bool setMode(SoundMode_Flags mode)=0;
		//These times are in sample time... this must be used in conjunction with setMode to LoopNormal to take effect
		virtual bool setLoopPoints(size_t LoopStart,size_t LoopEnd)=0;
		///Retrieves the length of the sound in samples
		/// \return >0 for length -1 for error
		///Warning! Using a VBR source that does not have an associated length information in milliseconds or pcm samples 
		///(such as MP3 or MOD/S3M/XM/IT) may return inaccurate lengths (since we are retrieving length in samples).
		///Let me know if you need to getLength for these kind of files
		virtual size_t getLength()=0;

		///Retrieves a sound's default attributes for when it is played on a channel with System::playSound.  
		/// \return true if successful
		virtual bool getDefaults(
			float *frequency, ///< receives the default frequency for the sound. Optional. Specify 0 or NULL to ignore. 
			float *volume, ///< receives the default volume for the sound. Result will be from 0.0 to 1.0. 0.0 = Silent, 1.0 = full volume. Default = 1.0. Optional. Specify 0 or NULL to ignore. 
			float *pan, ///< receives the default pan for the sound. Result will be from -1.0 to +1.0. -1.0 = Full left, 0.0 = center, 1.0 = full right. Default = 0.0. Optional. Specify 0 or NULL to ignore. 
			int *priority ///< receives the default priority for the sound when played on a channel. Result will be from 0 to 256. 0 = most important, 256 = least important. Default = 128. Optional. Specify 0 or NULL to ignore. 
			)=0;

		/// Sets a sounds's default attributes, so when it is played it uses these values without having to specify them later for each channel each time 
		///the sound is played.  \note There are no 'ignore' values for these parameters. Use getDefaults if you want to change only 1 and leave others 
		///unaltered.  \see getDefaults
		/// \return true if successful
		virtual bool setDefaults(
			float frequency, ///<Default playback frequency for the sound, in hz. (ie 44100hz). 
			float volume, ///< Default volume for the sound. 0.0 to 1.0. 0.0 = Silent, 1.0 = full volume. Default = 1.0. 
			float pan, ///< Default pan for the sound. -1.0 to +1.0. -1.0 = Full left, 0.0 = center, 1.0 = full right. Default = 0.0. 
			int priority ///< Default priority for the sound when played on a channel. 0 to 256. 0 = most important, 256 = least important. Default = 128. 
			)=0;
	private:
		friend SoundSystemBase;
		virtual void Release()=0;  //Client need not worry about releasing the sound!
};


///This is an instance of a sound. You can play a sound many times at once, and each time you play a sound you will get a new channel 
///handle. \note that this is only if it is not a stream. 
class FRAMEWORK_UI_AUDIO_API IChannel
{
	public:
		virtual ~IChannel() {}
		virtual void Stop()=0;
		/// \return true if successful
		/// \param vel Velocity is measured in meters per second (e.g. (posx-lastposx) / timedelta )
		///where timedelta was a measurement of one frame in seconds (e.g. .03336)
		virtual bool Set3DAttributes(const Vec3 *pos,const Vec3 *vel)=0;
		/// \return true if successful.  Can be used to determine when play session has completed
		virtual bool IsPlaying(bool &is_playing_status)=0;
		/// \return true if successful.  Can be used to determine when play session has started, as well as when it is paused
		virtual bool GetPaused(bool &is_paused_status)=0;
		/// \return true if successful.  This can be used to unpause the sound during the time when settings are tweaked
		virtual bool SetPaused(bool paused)=0;
		/// \see ISound for details... it is the same except that in this case you can set each channel
		virtual bool set3DMinMaxDistance(float minDistance=1.0,float maxDistance=10000.0)=0;

		///The loop parameters here are also in sample time like they are in ISound \see ISound.  This is not supported by static sounds 
		///created with FMOD_HARDWARE.  It is supported by sounds created with FMOD_SOFTWARE, or sounds of any type (hardware or software) 
		///created with System::createStream or FMOD_CREATESTREAM. If a sound was 1000ms long and you wanted to loop the whole sound, 
		///loopstart would be 0, and loopend would be 999, not 1000!
		/// \param LoopEnd If this is smaller or equal to loop start, it will result in an error.
		///If loop start or loop end is larger than the length of the sound, it will result in an error.
		/// \return true if succesful
		virtual bool setLoopPoints(size_t LoopStart,size_t LoopEnd)=0;
		///Sets a channel to loop a specified number of times before stopping.  
		/// \param loopcount
		///Number of times to loop before stopping. 0 = one shot. 1 = loop once then stop. -1 = loop forever. Default = -1
		virtual bool setLoopCount(size_t loopcount)=0;

		///Sets the channel's frequency or playback rate, in HZ.  
		/// \return true if successful
		///When a sound is played, it plays at the default frequency
		///	For most file formats, the volume is determined by the audio format.
		virtual bool setFrequency(float  frequency)=0;

		/// \note When a sound is played, it plays at the default volume of the sound which can be set by Sound::setDefaults.
		/// For most file formats, the volume is determined by the audio format.
		/// \param volume	A volume level, from 0.0 to 1.0 inclusive. 0.0 = silent, 1.0 = full volume. Default = 1.0. 
		/// \return true if successful
		/// \note derived classes must multiply the master and sound / stream volumes for this to be set to the correct level
		/// if the client code needs to override these levels we can add a parameter to support this
		virtual bool setVolume(float Volume)=0;
		///This tells the current mode of a channel as defined by SoundMode_Flags
		/// \return true if successful
		virtual bool getMode(SoundMode_Flags &mode)=0;
};

//Note: This is a singleton
class FRAMEWORK_UI_AUDIO_API ISoundSystem
{
	public:
		ISoundSystem() {ASSERT(!s_Instance); s_Instance = this;}
		virtual ~ISoundSystem() {s_Instance = NULL;}
		static ISoundSystem& Instance() {ASSERT_MSG(s_Instance, "ISoundSystem::s_Instance"); return *s_Instance;}

		//Returns a valid handle if successful NULL otherwise
		//Note: Some flags like fUseSoftware and f3DSound may only apply during the creation of the sound, and may not dynamically change
		virtual ISound *CreateSound(const char *FileName,SoundMode_Flags mode=f3DSound)=0;
		//This one should be used for files which are long and need to be streamed such as music
		virtual ISound *CreateStream(const char *FileName,SoundMode_Flags mode=f2DSound)=0;
		virtual bool PlaySound(ISound *sound,bool paused,
			IChannel **channel=NULL  //Can be ignored, and a new channel will be found
			//If you provide this, the value must either be set to NULL or some existing
			//handle to use the same channel.
			//The channel (sound instance) can be used to set properties while the sound is playing
			)=0;

		//Note: On any vector parameter you can use NULL to not update... thus you can pick and choose what to update
		//returns true if successful
		//Orientation vectors are expected to be of unit length.  This means the magnitude of the vector should be 1.0
		//A 'distance unit' is specified by System::Set3DSettings (TODO).  By default this is set to meters which is a distance scale of 1.0
		//By default FMOD used a left-handed co-ordinate system.  This means +x is right, +y is up, and +z is forwards.
		virtual bool Set3DListenerAttributes(
			int listener=0,				//Listener ID in a multi-listener environment.  Specify 0 if there is only one listener
			const Vec3 *pos=NULL,		//provide position of listener in world space, measured in distance units.
			const Vec3 *vel=NULL,		//provide velocity measured in distance units per second.  
			const Vec3 *forward=NULL,	//provide forward orientation of the listener.  
										//This must be perpendicular to the up vector, and of unit length
			const Vec3 *up=NULL			//provide upwards orientation of the listener.  
										//This must be perpendicular to the forward vector, and of unit length
			)=0;

		//Apply volume scaler to all sounds
		virtual void SetMasterVolume(float volume)=0;
		virtual void SetSoundsVolume(float volume)=0;	//all sound effects
		virtual void SetStreamsVolume(float volume)=0;  //all music

		//This is called once a frame (This pipes directly into FMOD's update)
		virtual void SystemFrameUpdate() {m_Event.Fire();}
		Event0 &GetEventTalker() {return m_Event;}
	private:
		static ISoundSystem *s_Instance;
		//This event gets fired per each SystemFrameUpdate
		Event0 m_Event;
};

		}
	}
}