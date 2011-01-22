// SinglePlayer_GameClient.h
#pragma once

class SinglePlayer_GameClient : public Fringe::Base::UI_GameClient
{
public:
	SinglePlayer_GameClient(	GG_Framework::Logic::Network::IClient& client, 
		GG_Framework::Logic::Network::SynchronizedTimer& timer, const char* contentDirLW) :
	UI_GameClient(client, timer, contentDirLW)
	{
		GG_Framework::UI::MainWindow* mainWin = GG_Framework::UI::MainWindow::GetMainWindow();

		// The Help screen is F1
		m_helpScreen = new Fringe::Base::HelpScreen();
		m_helpScreen->Enable(true);
		mainWin->GetKeyboard_Mouse().AddKeyBindingR(false, "HELP", osgGA::GUIEventAdapter::KEY_F1);
		mainWin->GetMainCamera()->addPostDrawCallback(*m_helpScreen);
		mainWin->GetKeyboard_Mouse().GlobalEventMap.Event_Map["HELP"].Subscribe(
			m_helpScreen->ehl, (Fringe::Base::HelpScreen&)(*m_helpScreen), &Fringe::Base::HelpScreen::ToggleEnabled);
	}

protected:

	virtual void GameTimerUpdate(double time_s)
	{
		static int numPasses = 0;
		++numPasses;
		__super::GameTimerUpdate(time_s);

		// Wait before the timer is up to start auto-pilot, let the player disable
		if (m_helpScreen && (time_s > 30.0))
		{
			// Turn off the help screen and place in auto pilot if we have not started moving on our own yet
			if (m_helpScreen->IsEnabled())
			{
				Fringe::Base::ThrustShip2* currShip = dynamic_cast<Fringe::Base::ThrustShip2*>(this->GetControlledEntity());
				if (currShip)
				{
					if (currShip->GetLinearVelocity().length2() == 0.0)
					{
						// Let the player disable the Help screen
						// m_helpScreen->Enable(false);
						//						currShip->GetController()->SetAutoPilot(true);
						m_UI_Controller.SetAutoPilot(true);
					}
				}
			}
			m_helpScreen = NULL;	// We no longer need to keep a handle to it
		}
	}

private:
	// To listen to the Timer Changes
	IEvent::HandlerList ehl;

	// Keep the help screen around so we can toggle it off after a few seconds
	Fringe::Base::HelpScreen* m_helpScreen;

};