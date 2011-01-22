#include "stdafx.h"
#include "Fringe.Base.h"

using namespace Fringe::Base;


UI_GameClient::UI_GameClient(	GG_Framework::Logic::Network::IClient& client, 
	GG_Framework::Logic::Network::SynchronizedTimer& timer, const char* contentDirLW) :
GG_Framework::Logic::UI_GameClient(client, timer, contentDirLW), m_stardust(NULL)
{
	GG_Framework::UI::MainWindow* mainWin = GG_Framework::UI::MainWindow::GetMainWindow();
	mainWin->GetKeyboard_Mouse().AddKeyBinding(']', "FlyNextShip", false);
	mainWin->GetKeyboard_Mouse().GlobalEventMap.Event_Map["FlyNextShip"].Subscribe
		(ehl, *this, &UI_GameClient::FlyNextShip);

	ControlledEntityChanged.Subscribe(ehl, *this, &UI_GameClient::ControlledEntityChangedCallback);
}
//////////////////////////////////////////////////////////////////////////

bool UI_GameClient::CanPlayerControl(GG_Framework::Logic::Entity3D* tryEntity)
{
	if (!tryEntity) return false;
	if (!tryEntity->IsShowing()) return false;
	if (tryEntity->GetControllingPlayerID() != GG_Framework::Logic::Network::MOCK_AI_PLAYER_ID) return false;
	// TODO: Put other logic here, like teams and other observable logic
	if (!dynamic_cast<Fringe::Base::Ship*>(tryEntity)) return false;
	
	// All else checked out, we can control this ship
	return true;
}
//////////////////////////////////////////////////////////////////////////

void UI_GameClient::FlyNextShip()
{//printf("UI_GameClient::FlyNextShip()\n");
	GG_Framework::Logic::Entity3D* currEntity = this->GetControlledEntity();

	// Find where this entity is in the list
	int nextIndex = 0;
	int numEntities = GetNumEntities();
	if (currEntity != NULL)
	{
		for (int i = 0; i < numEntities; ++i)
		{
			if (FindEntity(i) == currEntity)
			{
				nextIndex = i + 1;
				break;
			}
		}
	}

	// Watch for wrapping to the beginning of the loop
	if (nextIndex == GetNumEntities())
		nextIndex = 0;

	// Loop to find the next controllable entity
	int tryIndex = nextIndex;
	do {
		// Watch for wrapping to the beginning of the loop
		if (tryIndex == GetNumEntities())
			tryIndex = 0;

		// We only want to fly other ships (not rings or whatever)
		// We also only want to fly things that are showing in the scene
		GG_Framework::Logic::Entity3D* tryEntity = FindEntity(tryIndex);
		if (CanPlayerControl(tryEntity))
		{
			this->RequestControlEntity(tryEntity);
			return;
		}

		++tryIndex;
	} while (tryIndex != nextIndex);

	// We could not find any other ships to fly :(
}
//////////////////////////////////////////////////////////////////////////

void UI_GameClient::SetControlledEntity(GG_Framework::Logic::Entity3D* newEntity)
{//printf("UI_GameClient::SetControlledEntity\n");
	if (newEntity != m_controlledEntity)
	{
		//First associate our controller to the AI controller
		ThrustShip2 *ship=dynamic_cast<ThrustShip2 *>(newEntity);
		if (!ship) return;
		AI_Base_Controller *controller=ship->GetController();
		assert(controller);
		//Disconnect the UI first
		if (m_controlledEntity)
		{
			m_UI_Controller.HookUpUI(false);
			AI_Base_Controller *OldController=NULL;
			{
				ThrustShip2 *oldship=dynamic_cast<ThrustShip2 *>(m_controlledEntity);
				OldController=(oldship)?oldship->GetController():NULL;
			}
			//Detach association from the old controller
			if (OldController)
				OldController->Try_SetUIController(NULL);
		}

		// JAMES: Not sure of the error handling here.  Can Try_SetUIController fail? if so, why do Set_AI_Base_Controller
		// Only to have to set it back to NULL?  What does it mean to set it to NULL?  Is this an invalid state?

		//Rick:  Try_SetUIController currently does not ever fail.  So everything is safe, the real question is to determine what ships we can
		//fly to and at what level do we manage this.  This is somewhat related to the idea of only being able to have access to the team's ships, or
		//full access for people observing the game.  There may be other logic such as another player has control of the ship etc.
		//If these things are managed elsewhere then we can change the method to not return bool.  If it is determined that this would be the level
		//to manage this we can add a method to check before doing the pointer assignments.  I suspect access might be managed elsewhere.

		m_UI_Controller.Set_AI_Base_Controller(controller);
		//Now see if the AI will allow us to use it
		if (controller->Try_SetUIController(&m_UI_Controller))
		{
			//Success... now to let the entity set things up
			m_UI_Controller.HookUpUI(true);
			__super::SetControlledEntity(newEntity);
		}
		else
			m_UI_Controller.Set_AI_Base_Controller(NULL);   //no luck... flush ship association
	}
}
//////////////////////////////////////////////////////////////////////////

void UI_GameClient::ShipChangedControl(Ship* ship, PlayerID controllingPlayerID)
{//printf("UI_GameClient::ShipChangedControl\n");
	ASSERT(ship);
	if ((controllingPlayerID == GetMyPlayerID()) && (m_controlledEntity != ship))
	{
		// What ship are we already controlling?
		GG_Framework::Logic::Entity3D* oldControlledEntity = m_controlledEntity;

		// We now control this entity
		SetControlledEntity(ship);

		// We can pass control back to the AI controller.  Maybe someone else would like it
		if (oldControlledEntity)
			SendChangeEntityController(oldControlledEntity, GG_Framework::Logic::Network::MOCK_AI_PLAYER_ID);
	}
}
//////////////////////////////////////////////////////////////////////////

UI_GameClient::~UI_GameClient()
{
	// If we are still controlling a ship, turn control back over
	if (m_controlledEntity)
		SendChangeEntityController(m_controlledEntity, GG_Framework::Logic::Network::MOCK_AI_PLAYER_ID);
}
//////////////////////////////////////////////////////////////////////////

