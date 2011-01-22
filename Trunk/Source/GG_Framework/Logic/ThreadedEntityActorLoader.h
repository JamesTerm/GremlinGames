// ThreadedEntityActorLoader.h
#pragma once

#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

namespace GG_Framework
{
	namespace Logic
	{
		class Entity3D;
		class FRAMEWORK_LOGIC_API ThreadedEntityActorLoader : public GG_Framework::Base::ThreadedClass
		{
		private:
			std::string m_filename;
			GG_Framework::UI::ActorScene* m_uiActorScene;
			GG_Framework::UI::ActorScene* m_bbActorScene;
			OpenThreads::Mutex m_mutex;
			int volatile m_state;

		protected:
			// This is the part that is really run, surrounded by try/catch blocks
			virtual void tryRun();

		public:
			// Get these when you are done
			osg::ref_ptr<GG_Framework::UI::ActorTransform> uiActorTransform;
			osg::ref_ptr<GG_Framework::UI::ActorTransform> bbActorTransform;
			Entity3D* entity3D;

			ThreadedEntityActorLoader(
				GG_Framework::UI::ActorScene* uiActorScene, 
				GG_Framework::UI::ActorScene* bbActorScene, Entity3D* entity, std::string filename) :
			m_uiActorScene(uiActorScene), m_bbActorScene(bbActorScene), m_state(0),
				entity3D(entity), m_filename(filename), asynchPlayerID(UNASSIGNED_PLAYER_ID)
			{}

			Entity3D* Complete();

			int UpdateState();

			PlayerID asynchPlayerID;

		};
		//////////////////////////////////////////////////////////////////////////
	}
}
