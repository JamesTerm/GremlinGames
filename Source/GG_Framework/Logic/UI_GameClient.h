// GG_Framework.Logic UI_GameClient.h
#pragma once
#include <string>
#include <map>

#include "..\..\DebugPrintMacros.hpp"
//#ifdef _DEBUG
// #define USE_SIMPLE_MODELS
//#endif

namespace GG_Framework
{
	namespace Logic
	{
		class Entity3D;
		struct TransmittedEntity;

		//! This version of the GameClient keeps an ActorScene that is made for viewing,
		//! along with the BoundingBox version.
		class FRAMEWORK_LOGIC_API UI_GameClient : public GameClient
		{
		public:
			UI_GameClient(	GG_Framework::Logic::Network::IClient& client, 
				GG_Framework::Logic::Network::SynchronizedTimer& timer, const char* contentDirLW);

			//! This will DEFINATELY be called on a separate thread, ThreadedClientGameLoader
			virtual bool LoadInitialGameData();

			//! Here is the scene the camera sees
#ifdef USE_SIMPLE_MODELS	// A debug macro that simplifies ALL models, 
							// should not have any textures or effects
			BB_ActorScene UI_ActorScene;
#else
			GG_Framework::UI::ActorScene UI_ActorScene;
#endif

			// There can be only one controlled entity
			Entity3D* GetControlledEntity(){return m_controlledEntity;}
			Event2<Entity3D*, Entity3D*> ControlledEntityChanged; //! (old, new)
			virtual void SetControlledEntity(Entity3D* newEntity);

			//! To be called when the Game actually starts running, the scene has been set inside a MainWindow
			virtual void StartUI();
			

			virtual GG_Framework::UI::ActorScene* Get_UI_ActorScene(){return &UI_ActorScene;}

		protected:
			//! Waits for its own Entity to control, called from LoadInitialGameData() in its own thread
			virtual bool LoadOwnEntity();
			virtual bool LoadOrnamentalOSGV();
			bool LoadPredictedLoadTimes();

			Entity3D* m_controlledEntity;

			// Sends the network message to request control of an entity
			void RequestControlEntity(Entity3D* tryEntity);
		private:
			
			bool GetSyncTimerEpoch();
			GG_Framework::Logic::Network::SynchronizedTimer& m_syncTimer;
		};
		//////////////////////////////////////////////////////////////////////////
	}
}

