// GuardianVR SinglePlayer_GameClient.h
#pragma once

//! This game scene allows the player to command different mechs and shows a help screen
class SinglePlayer_GameClient : public GG_Framework::Logic::UI_GameClient
{
public:
	SinglePlayer_GameClient(	GG_Framework::Logic::Network::IClient& client, 
		GG_Framework::Logic::Network::SynchronizedTimer& timer, const char* contentDirLW) :
	GG_Framework::Logic::UI_GameClient(client, timer, contentDirLW)
	{
		GG_Framework::UI::MainWindow* mainWin = GG_Framework::UI::MainWindow::GetMainWindow();
		mainWin->Keyboard_Mouse.AddKeyBinding(']', "CmdNextMech", false);
		mainWin->Keyboard_Mouse.GlobalEventMap.Event_Map["CmdNextMech"].Subscribe
			(ehl, *this, &SinglePlayer_GameClient::CmdNextMech);

		// The Help screen is F1
		m_helpScreen = new GuardianVR::Base::HelpScreen();
		mainWin->Keyboard_Mouse.AddKeyBindingR(false, "HELP", osgGA::GUIEventAdapter::KEY_F1);
		mainWin->GetMainCamera()->addPostDrawCallback(*m_helpScreen);
		mainWin->Keyboard_Mouse.GlobalEventMap.Event_Map["HELP"].Subscribe(
			m_helpScreen->ehl, (GuardianVR::Base::HelpScreen&)(*m_helpScreen), &GuardianVR::Base::HelpScreen::ToggleEnabled);

		// Turn HELP off for now
		m_helpScreen->Enable(false);
	}

	void CmdNextMech()
	{
		std::vector<GG_Framework::Logic::Entity3D*> entityList = this->ListCurrentEntities();
		GG_Framework::Logic::Entity3D* currEntity = this->GetControlledEntity();

		// Find where this entity is in the list
		int nextIndex = 0;
		int numEntities = entityList.size();
		if (currEntity != NULL)
		{
			for (int i = 0; i < numEntities; ++i)
			{
				if (entityList[i] == currEntity)
				{
					nextIndex = i + 1;
					break;
				}
			}
		}

		// Watch for wrapping to the beginning of the loop
		if (nextIndex == entityList.size())
			nextIndex = 0;

		// Loop to find the next controllable entity
		int tryIndex = nextIndex;
		do {
			// Watch for wrapping to the beginning of the loop
			if (tryIndex == entityList.size())
				tryIndex = 0;

			// We only want to fly other ships (not rings or whatever)
			// We also only want to fly things that are showing in the scene
			GG_Framework::Logic::Entity3D* tryEntity = entityList[tryIndex];
			if (tryEntity && tryEntity->IsShowing())
			{
				GuardianVR::Base::Mech* mech = dynamic_cast<GuardianVR::Base::Mech*>(tryEntity);
				if (mech){
					this->SetControlledEntity(tryEntity);
					return;
				}
			}

			++tryIndex;
		} while (tryIndex != nextIndex);

		// We could not find any other ships to fly :(
	}

private:
	// To listen to the Timer Changes
	IEvent::HandlerList ehl;

	// Keep the help screen around so we can toggle it off after a few seconds
	GuardianVR::Base::HelpScreen* m_helpScreen;

};