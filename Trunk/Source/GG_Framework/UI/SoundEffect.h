// GG_Framework.UI SoundEffect.h
#pragma once
#include <osg/Timer>

namespace GG_Framework
{
	namespace UI
	{

		//This is used as a handy general purpose tool for the camera as well as the NodeBinder.  So far this project is the only one which
		//has access to both osg and SoundEffects3d.  We may wish to move is we wish to add more dependencies
		class AudioVector3f : public Audio::Vec3
		{
		public:
			//Note: Y and Z are swapped
			AudioVector3f(float x, float y, float z) : Audio::Vec3(x,z,y)
			{}
			AudioVector3f(osg::Vec3 osgVec3) : Audio::Vec3(osgVec3.x(),osgVec3.z(),osgVec3.y())
			{}
		};


	

		class FRAMEWORK_UI_API Sound3dOSGNodeBinder
		{
			public:
				Sound3dOSGNodeBinder(osg::Node *ObjectToBind);
				void SetChannel(GG_Framework::UI::Audio::IChannel *channel);
				//For sounds that loop you should call this to stop it
				void EndChannel(bool StopImmediatly=false);
				GG_Framework::UI::Audio::IChannel *GetChannel() const {return m_Channel;}
				void UpdateCallback(); //perform updates

				osg::Vec3d SoundLocOffset;
			private:
				IEvent::HandlerList ehl;  //listener to listen for updates from sound system
				GG_Framework::UI::Audio::IChannel *m_Channel;  //Channel to bind
				osg::Node * const m_Node;  //node (object) for the channel to bind to

				//To compute velocity we will need to keep track of the previous position and time
				//These can be initialized within the constructor
				osg::Timer_t m_lastTimerTick;
				GG_Framework::UI::OSG::VectorDerivativeOverTimeAverager m_velocityAvg;
		};

		class FRAMEWORK_UI_API SoundEffect : public INamedEffect
		{
			public:
				SoundEffect(ActorScene* actorScene, osg::Node& parent);
				virtual void LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt);

				double GetNominalDistance(){return m_nominalDistance;}
				void SetNominalDistance(double nomDist);

				// This function ONLY scales if the effect is constant
				void ScaleVolumeDistance(double scale);
				double GetVolumeDistancesScale(){return m_scaleDistance;}

				//This scale is applied to existing master and sound levels
				void ScaleVolume(double scale);
				double GetScaleVolume(){return m_scaleVolume;}

			protected:
				virtual ~SoundEffect();
				virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);

				class SoundEffectClassFactory : public INamedEffectClassFactory
				{
				protected:
					virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent);
					virtual void DeleteEffect(INamedEffect* effect);
				};

			private:
				GG_Framework::UI::Audio::ISound *m_Sound;
				Sound3dOSGNodeBinder m_NodeSound;

				//Cached to be used for smart loop
				size_t m_LoopStart,m_LoopStop;

				double m_nominalDistance;
				double m_scaleDistance;
				double m_scaleVolume;

				static SoundEffectClassFactory s_classFactory;
				friend SoundEffectClassFactory;
				///If this is true (default) it will use a technique where the sound plays from 0 loops, and when it ends it will play to the end (decay);
				///otherwise, the loop settings are committed and will stop immediately (e.g. thrust)
				bool m_SmartLoop;
				///If this is true (default) it will wait for the previous sound to complete before starting the next
				///otherwise it will allow the ability to overlap sounds
				bool m_OneAtATime;

				// When the player is controlling an entity, they will not want to hear certain local effects
				// for that entity played so loudly, for instance the engine roar.  This value scales the volumes
				// And the effective distances for sound effects when the player controls.  The default value is 15% (0.15)
				double m_playerControlledScale;
				void OnPlayerControlled(bool playerControlled);

				void UpdateVolumes();

				// Listen for adjustments to all sound scales from the actor
				IEvent::HandlerList ehl;
		};

	}
}