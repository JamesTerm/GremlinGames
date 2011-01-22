// FrameWork.ActorActions.Tester HighLevelRegression.cpp
#include "stdafx.h"
#include "../../../CompilerSettings.h"
#include "../FrameWork_ActorActions.h"
#include "../../Audio/SoundEffects3d/Interfaces.h"

namespace FrameWork
{
	namespace ActorActions
	{
		namespace Tester
		{

class StubbedSoundSystem : public FrameWork::Audio::ISoundSystem
{
protected:
	//Returns a valid handle if successful NULL otherwise
	virtual FrameWork::Audio::ISound *CreateSound(const char *FileName,FrameWork::Audio::SoundMode_Flags mode=FrameWork::Audio::f3DSound){return NULL;}
	virtual FrameWork::Audio::ISound *CreateStream(const char *FileName,FrameWork::Audio::SoundMode_Flags mode=FrameWork::Audio::f2DSound){return NULL;}

	virtual bool PlaySound(FrameWork::Audio::ISound *sound,bool paused,FrameWork::Audio::IChannel **channel=NULL){return false;}
	virtual bool Set3DListenerAttributes(int listener=0,const FrameWork::Audio::Vec3 *pos=NULL,const FrameWork::Audio::Vec3 *vel=NULL,const FrameWork::Audio::Vec3 *forward=NULL,const FrameWork::Audio::Vec3 *up=NULL){return  false;}

	//This is called once a frame (This pipes directly into FMOD's update)
	virtual void SystemFrameUpdate(){}
};


SUITE(HighLevelRegression_Tester)
{
	TEST(LoadTestShot1)
	{
		// This test merely checks that the test file does not crash when loading
		FrameWork::ActorActions::EventMap eventMap;
		StubbedSoundSystem sss;	// This needs to come before the sm, because sm is dependent on closing on sss
		FrameWork::ActorActions::SceneManager sm;
		
		//Rick, I have changed that path here to be relative to this project, so now the test will pass
		sm.AddActorFile("Test Data\\TestA\\testShot.osgv");
	}		
}

		}
	}
}
