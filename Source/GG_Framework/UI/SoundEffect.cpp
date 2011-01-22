// GG_Framework.UI SoundEffect.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"
#include "..\..\DebugPrintMacros.hpp"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

using namespace GG_Framework::Base;

namespace GG_Framework
{
	namespace UI
	{


  /***********************************************************************************************************************/
 /*												Sound3dOSGNodeBinder													*/
/***********************************************************************************************************************/


Sound3dOSGNodeBinder::Sound3dOSGNodeBinder(osg::Node *ObjectToBind) : m_Node(ObjectToBind), m_velocityAvg(15), SoundLocOffset(0,0,0)
{
	ASSERT(m_Node);
	m_Channel=NULL;
	SOUNDSYSTEM.GetEventTalker().Subscribe(ehl, *this, &Sound3dOSGNodeBinder::UpdateCallback);
}
void Sound3dOSGNodeBinder::SetChannel(GG_Framework::UI::Audio::IChannel *channel) 
{
	if (m_Channel != channel)
	{
		m_Channel=channel;	
		if (m_Channel)
		{
			osg::Timer timer;
			m_lastTimerTick = timer.tick();
			m_velocityAvg.Reset();
		}
	}
}

void Sound3dOSGNodeBinder::EndChannel(bool StopImmediatly)
{
	if (m_Channel)
	{
		if (StopImmediatly)
			m_Channel->Stop();
		else
			m_Channel->setLoopCount(0);
	}
}

void Sound3dOSGNodeBinder::UpdateCallback()
{
	if (!m_Channel)
		return;

	bool isPlaying;
	bool isPaused;

	if 
		(
		((m_Channel->GetPaused(isPaused)==true)&&(isPaused)) ||
		((m_Channel->IsPlaying(isPlaying)==true)&&(isPlaying))
		)
	{
		// Get the position of the sound based on some offset of where the node is
		osg::Vec3d ObjVec(SoundLocOffset*OSG::GetNodeMatrix(m_Node, NULL, NULL));
		// osg::Vec3d ObjVec(OSG::GetNodePosition(m_Node));

		DEBUG_SOUND_OFFSETS("Sound3dOSGNodeBinder::UpdateCallback{%4.3f, %4.3f, %4.3f}\n", ObjVec[0], ObjVec[1], ObjVec[2]);


		osg::Timer timer;
		osg::Timer_t thisTimerTick = timer.tick();
		double dTime_s=timer.delta_s(m_lastTimerTick,thisTimerTick);
		AudioVector3f Position(ObjVec);  //That was easy
		AudioVector3f Velocity(m_velocityAvg.GetVectorDerivative(ObjVec, dTime_s));
		m_lastTimerTick = thisTimerTick;

		//Keep this around... see if things are working
		//DebugOutput("Vector update x=%f,y=%f,z=%f   xv=%f,yv=%f,zv=%f\n",Position.x,Position.y,Position.z,Velocity.x,Velocity.y,Velocity.z);
		//if (Velocity.x+Velocity.y+Velocity.z>0.0)
		//	printf("\r %f,%f,%f   %f,%f,%f             ",Position.x,Position.y,Position.z,Velocity.x,Velocity.y,Velocity.z);

		m_Channel->Set3DAttributes(&Position,&Velocity);
	}
	else
		if (!isPlaying)  // No longer playing
		{
			//Once it is done playing we need not play it
			SetChannel(NULL);
		}
}



INamedEffect* SoundEffect::SoundEffectClassFactory::CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
{
	if (!stricmp(name, "SoundEffect"))
		return new SoundEffect(actorScene, parent);
	else return NULL;
}

void SoundEffect::SoundEffectClassFactory::DeleteEffect(INamedEffect* effect)
{
	delete dynamic_cast<SoundEffect*>(effect);
}



  /***********************************************************************************************************************/
 /*														SoundEffect														*/
/***********************************************************************************************************************/


SoundEffect::SoundEffect(ActorScene* actorScene, osg::Node& parent) : INamedEffect(actorScene, parent),m_Sound(NULL),
m_NodeSound(&parent), m_nominalDistance(-1.0), m_scaleDistance(1.0),m_scaleVolume(1.0),m_playerControlledScale(1.0)
{
	m_SmartLoop=true; 
	m_OneAtATime=true; 
	m_LoopStart=0,m_LoopStop=0;
}
void SoundEffect::LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt)
{
	if (start)
	{
		m_NodeSound.SoundLocOffset = pt;
		if (m_Sound)
		{
			bool IsPlaying=false;
			if (m_NodeSound.GetChannel())
			{
				m_NodeSound.GetChannel()->IsPlaying(IsPlaying);
				m_NodeSound.GetChannel()->SetPaused(false);  //we could check if paused, but this should be harmless
			}

			//The GetLooping() will continue to call here, so we need to handle this case properly
			if (!IsPlaying)
			{
				Audio::IChannel *channel=NULL;  //must be set to null!
				//start out paused to get initial 3d vectors set
				if (SOUNDSYSTEM.PlaySound(m_Sound,true,&channel))
				{
					if ((m_SmartLoop)&&(m_LoopStop!=0))
						channel->setLoopPoints(m_LoopStart,m_LoopStop);
					m_NodeSound.SetChannel(channel);
					UpdateVolumes();
					//We must call Set3DAttributes with a good initial position before enabling the sound; fortunately this is all that UpdateCallback() does!
					//if this changes we'll need to break out the functionality where this call just sets up the node position
					m_NodeSound.UpdateCallback();
					//Now we are ready to go
					channel->SetPaused(false);
				}
				else
					ASSERT(false);
			}
			else // Already playing, just keep looping 
				m_NodeSound.GetChannel()->setLoopCount(-1);
		}
	}
	else
	{
		//Stop the sound
		m_NodeSound.EndChannel(!m_SmartLoop);
		//For smart loop we'll check and see if we want one at a time... if so then we keep the channel active for LaunchEffect(true) to manage
		if ((!m_SmartLoop)||(!m_OneAtATime))
		{
			//For non-smart loop we halt the sound immediately (e.g. short thrusts)
			if ((!m_SmartLoop)&&(m_NodeSound.GetChannel()))
				m_NodeSound.GetChannel()->Stop();
			m_NodeSound.SetChannel(NULL);
		}
	}
}

SoundEffect::~SoundEffect()
{
	//Ensure the channel is free
	m_NodeSound.SetChannel(NULL);
	//the sound is implicitly freed through the system
}
//////////////////////////////////////////////////////////////////////////

void SoundEffect::ReadParameters(EventMap& localMap, const char*& remLineFromFile)
{
	m_LoopStart=0,m_LoopStop=0; //init here just to be safe 
	INamedEffect::ReadParameters(localMap, remLineFromFile);
	float minDistance=1.0;
	char FileName[MAX_PATH];
	sscanf(remLineFromFile, "%s %f %d %d %d %d %lf",FileName,&minDistance,&m_LoopStart,&m_LoopStop,&m_SmartLoop,&m_OneAtATime,&m_playerControlledScale );

	ASSERT(!m_Sound);

	// Listen for the actor sending an event when player controlled
	localMap.EventOnOff_Map["PLAYER_CONTROLLED"].Subscribe(ehl, *this, &SoundEffect::OnPlayerControlled);

	//construct the full path
	char Buffer[MAX_PATH];
	_getcwd(Buffer,MAX_PATH);
	strcat(Buffer,"\\");
	strcat(Buffer,FileName);
	//This may fail (e.g. if the file doesn't exist)
	//To be safe lets always set fUseSoftware so that we have unlimited channels (Vista has weak hardware channel support)
	int mode=Audio::f3DSound|Audio::fUseSoftware;
	if (GetMode() != ON)	// This is what determines looping
		mode|=Audio::fLoopNormal;
	m_Sound=SOUNDSYSTEM.CreateSound(Buffer,(Audio::SoundMode_Flags)mode);
	SetNominalDistance(minDistance);
	if (m_Sound)
	{
		//For smart loop we delay setting the loop points until after the sound has already started (this is how it plays from start)
		if (!m_SmartLoop)
			m_Sound->setLoopPoints(m_LoopStart,m_LoopStop); //does not matter if we want looping or not, so no if check necessary
	}
	else
	{
		osg::notify(osg::FATAL) << "SoundEffect Could not open the file \"" << Buffer << "\"\n";
	}

	// Only pass on more info if there is a '~'
	const char* tilde = strchr(remLineFromFile, '~');
	remLineFromFile = tilde ? tilde+1 : NULL;

	// this might be a task that could take some time, lets fire a task complete to keep the times updated
	GG_Framework::UI::OSG::LoadStatus::Instance.TaskComplete();
}
//////////////////////////////////////////////////////////////////////////

void SoundEffect::SetNominalDistance(double nomDist)
{
	ASSERT(nomDist > 0);
	if (nomDist != m_nominalDistance)
	{
		m_nominalDistance = nomDist;
		UpdateVolumes();
	}
}
//////////////////////////////////////////////////////////////////////////

void SoundEffect::ScaleVolumeDistance(double scale)
{
	ASSERT(scale > 0);
	if (scale != m_scaleDistance)
	{
		DEBUG_SOUND_DAMPENING("SoundEffect::ScaleVolumeDistance(%f)\n", scale);
		m_scaleDistance = scale;
		UpdateVolumes();
	}
}

void SoundEffect::ScaleVolume(double scale)
{
	if (scale != m_scaleVolume)
	{
		DEBUG_SOUND_DAMPENING("SoundEffect::ScaleVolume(%f)\n", scale);
		m_scaleVolume = scale;
		UpdateVolumes();
	}
}

void SoundEffect::UpdateVolumes()
{
	if (m_Sound)
	{
		//Note: We cannot set the sound here for anything as it will impact other instances
		//If the channel is already active then apply to it directly
		if (m_NodeSound.GetChannel())
		{
			m_NodeSound.GetChannel()->set3DMinMaxDistance(m_nominalDistance*m_scaleDistance, m_nominalDistance*m_scaleDistance * 100.0);
			m_NodeSound.GetChannel()->setVolume(m_scaleVolume);  //Note: the master and sound levels are applied implicitly
		}
	}
}

void SoundEffect::OnPlayerControlled(bool playerControlled)
{
	m_scaleVolume = m_scaleDistance = (playerControlled ? m_playerControlledScale : 1.0);
	UpdateVolumes();
}
//////////////////////////////////////////////////////////////////////////

	}
}