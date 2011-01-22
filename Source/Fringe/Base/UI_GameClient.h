// Fringe.Base UI_GameClient.h
#pragma once

class StardustBlock;
namespace Fringe
{
	namespace Base
	{
		//! This version of the UI Game Scene provides the star dust field and allows for switching ships
		class FRINGE_BASE_API UI_GameClient : public GG_Framework::Logic::UI_GameClient
		{
		public:
			UI_GameClient(	GG_Framework::Logic::Network::IClient& client, 
				GG_Framework::Logic::Network::SynchronizedTimer& logic_timer, 
				GG_Framework::Base::Timer& osg_timer, const char* contentDirLW);
			~UI_GameClient();

			void ShipChangedControl(Ship* ship, PlayerID controllingPlayerID);

		protected:
			virtual bool LoadOrnamentalOSGV();
			void ControlledEntityChangedCallback(GG_Framework::Logic::Entity3D* oldEntity, GG_Framework::Logic::Entity3D* newEntity);

			bool CanPlayerControl(GG_Framework::Logic::Entity3D* tryEntity);
			void FlyNextShip();
			virtual void SetControlledEntity(GG_Framework::Logic::Entity3D* newEntity);

			StardustBlock* m_stardust;
			//The one, the only!
			UI_Controller m_UI_Controller;

		private:
			//! To listen to all events
			IEvent::HandlerList ehl;
		};
	}
}