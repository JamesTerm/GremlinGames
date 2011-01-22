// Fringe.Client main.cpp
#include "stdafx.h"
#include "Fringe.Client.h"
using namespace std;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;
using namespace GG_Framework::UI::OSG;
using namespace Fringe::Base;
using namespace GG_Framework::Logic;

#include <conio.h>

#include "../../GG_Framework/Base/ExeMain.hpp"
namespace Fringe
{
	namespace Client
	{
		class EXE : public IExeMain
		{
		public:
			virtual int Main(unsigned argc, const char* argv[]);
		};
	}
}

class FringeClientMainWindow : public GG_Framework::UI::MainWindow
{
public:
	FringeClientMainWindow(bool useAntiAlias, double throttle_fps, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs, GG_Framework::Base::Timer& osg_timer) :
	  GG_Framework::UI::MainWindow(useUserPrefs, throttle_fps, screenWidth, screenHeight, useUserPrefs, osg_timer) {}

	  virtual void PostRealize()
	  {
		  SetFullScreen(true);
		  SetWindowText("Rim-Space Client");
		  __super::PostRealize();
	  }
};

#include <osg/Notify>

int main(unsigned argc, const char** argv)
{
	Fringe::Client::EXE exe;
	return exe.Run(argc, argv);
}

int Fringe::Client::EXE::Main(unsigned argc, const char* argv[])
{
	// We will be using Threads
	OpenThreads::Thread::Init();

	// Make the MainWindow multi-threaded or not here:
	GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW = false;

	// Strip out the flagged and non-flagged options
	GG_Framework::UI::ViewerApp_ArgumentParser argParser;
	std::vector<string> argList = argParser.ProcessArguments(argc, argv);
	osg::setNotifyLevel(argParser.NotifySeverity);

	// Content Directory
	char contentDIR[512];
	_getcwd(contentDIR, 512);
	printf("Content Directory: %s\n", contentDIR); 

	// Server IP Address
	char serverIP[128];
	cout << "Enter Server IP Address (or '0' for loop-back): ";
	cin >> serverIP;
	if (!stricmp(serverIP, "0"))
		strcpy(serverIP, GG_Framework::Logic::Network::LOOPBACK_URL);

	// Get the server port number.  Watch for negative numbers.
	unsigned serverPortNum = GG_Framework::Logic::Network::DEFAULT_SERVER_PORT;
	cout << "Enter the Server Port #(" << serverPortNum << "): ";
	cin >> serverPortNum;
	if (serverPortNum <= 0)
		serverPortNum = GG_Framework::Logic::Network::DEFAULT_SERVER_PORT;

	// Get the client port number.  Watch for negative numbers.
	unsigned clientPortNum = GG_Framework::Logic::Network::DEFAULT_CLIENT_PORT;
	cout << "Enter the Client Port #(" << clientPortNum << "): ";
	cin >> clientPortNum;
	if (clientPortNum <= 0)
		clientPortNum = GG_Framework::Logic::Network::DEFAULT_CLIENT_PORT;

	// Show the User what we are doing before the Logo Scene
	cout << endl << "Connecting Client:" << endl;
	cout << "Server IP: " << serverIP << endl;
	cout << "Server Port#: " << serverPortNum << endl;
	cout << "Client Port#: " << clientPortNum << endl;

	// Create the single sound system
	GG_Framework::UI::Audio::ISoundSystem* sound;
	if (argParser.MockAudio)
		sound = new GG_Framework::UI::Audio::SoundSystem_Mock();
	else
		sound = new GG_Framework::UI::Audio::SoundSystem();

	// Create a new scope, so all auto-variables will be deleted when they fall out
	{
		// Create the Client and start a thread to wait for connections and load game data
		GG_Framework::Logic::Network::Client net_client(serverIP, serverPortNum, clientPortNum);

		// The is the logic_timer that will be fired
		GG_Framework::Logic::Network::SynchronizedTimer logic_timer("Fringe.Client Logic Timer Log.csv");
		GG_Framework::UI::OSG::OSG_Timer osg_timer("Fringe.Client OSG Timer Log.csv");
		GG_Framework::Base::Timer* osg_timer_ref = NULL;
		if (GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW)
			osg_timer_ref = &logic_timer;
		else
			osg_timer_ref = &osg_timer;

		// There will be a single camera, KBM, sound system, etc.
		FringeClientMainWindow mainWin(argParser.UseAntiAliasing, argParser.MAX_FPS, argParser.RES_WIDTH, argParser.RES_HEIGHT, true, *osg_timer_ref);

		// Here is a good example of setting up an easy text PDCB
		GG_Framework::UI::Framerate_PDCB* fpdcb = new GG_Framework::UI::Framerate_PDCB(mainWin);
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ShowFR", osgGA::GUIEventAdapter::KEY_F2);
		mainWin.GetMainCamera()->addPostDrawCallback(*fpdcb);
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ShowFR"].Subscribe(
			fpdcb->ehl, (GG_Framework::UI::Text_PDCB&)(*fpdcb), &GG_Framework::UI::Text_PDCB::ToggleEnabled);

		// And a debug one
		GG_Framework::UI::DebugOut_PDCB* dpdcb = new GG_Framework::UI::DebugOut_PDCB(mainWin);
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ShowDebug", osgGA::GUIEventAdapter::KEY_F9);
		mainWin.GetMainCamera()->addPostDrawCallback(*dpdcb);
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ShowDebug"].Subscribe(
			dpdcb->ehl, (GG_Framework::UI::Text_PDCB&)(*dpdcb), &GG_Framework::UI::Text_PDCB::ToggleEnabled);

		// Make the 'm' key record
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "RecordFrames", 'm');
		GG_Framework::UI::ScreenCaptureTool sct(mainWin, mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["RecordFrames"]);

		// We can tie the events to Toggle fullscreen  to F3
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFullScreen", osgGA::GUIEventAdapter::KEY_F3);

		// Logging the timer
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFrameLog", osgGA::GUIEventAdapter::KEY_F7);
		logic_timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);
		if (!GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW)
			osg_timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);

		// This Game Client is what is connecting to the Client
		Fringe::Base::UI_GameClient mainGameClient(net_client, logic_timer, *osg_timer_ref, contentDIR);

		// We are going to spawn a thread to read the initial game data
		GG_Framework::Logic::ThreadedClientGameLoader threadGameLoader(mainGameClient);


		// While Waiting, Spin a Logo.  This is what calls run on the threadGameLoader.
		//**** If you want a Spinning Logo, Uncomment the following two lines
		// -------------------------------------------------------------------------
#ifdef THREADED_LOGO
		LogoSceneRunner logoSceneRunner("The Fringe", threadGameLoader);
		logoSceneRunner.MyRunScene("../../Gremlin ART/Simple 01.osgv", cam, kbm);
#else
		// -------------------------------------------------------------------------
		//**** UnComment the following line to NOT use a spinning logo, loads the main scene on this thread
		// -------------------------------------------------------------------------
		
		threadGameLoader.run();
		// -------------------------------------------------------------------------
#endif

		// If the USER exited before a connection
		if (!threadGameLoader.IsLoadComplete())
		{
			cout << "User ABORTED before Connection, ABORTING" << endl;
			threadGameLoader.cancel();

			return 0;
		}
		
		// Set the scene and realize the camera at full size
		osg::Node* mainScene = mainGameClient.Get_UI_ActorScene()->GetScene();
		mainWin.GetMainCamera()->SetSceneNode(mainScene);
		mainWin.Realize();

		// Let the server know we are really ready
		threadGameLoader.NotifyServerReady();

		// Let all of the scene know we are starting
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["START"].Fire();
		for (unsigned i = 0; i < (unsigned)mainGameClient.MapList.size(); ++i)
			mainGameClient.MapList[i]->Event_Map["START"].Fire();
		mainGameClient.StartUI();

		// Let's play a tune
		if (!argParser.SoundTrack.empty())
		{
			// TODO: Have error handling in case the file does not exist or something
			GG_Framework::UI::Audio::ISound* music = SOUNDSYSTEM.CreateStream(argParser.SoundTrack.c_str());
			if (music)
				SOUNDSYSTEM.PlaySound(music, false);
			else
				osg::notify(osg::FATAL) << "Failed to load music \"" << argParser.SoundTrack << "\"" << std::endl;
		}

		// Connect the argParser to the camera, in case it wants to handle stats (I do not know that I like this here)
		argParser.AttatchCamera(&mainWin, osg_timer_ref);

		// The updater is complete, it can write out its times
		GG_Framework::UI::OSG::LoadStatus::Instance.LoadComplete();

		// Here is the primary game loop
		double dTime_s = 0.0;
		double currTime_s = 0.0;
		double syncTimer_s = 0.0;
		do
		{
			dTime_s = logic_timer.FireTimer();
			currTime_s = logic_timer.GetCurrTime_s();

			// Every 5 seconds or so, make sure my OSG thread has the same time
			if (syncTimer_s > 5.0)
			{
				syncTimer_s += dTime_s;
				osg_timer_ref->SetCurrTime_NoEvent(currTime_s);
				syncTimer_s = 0.0;
			}
		}
		while(mainWin.Update(currTime_s, dTime_s));

		// Write the log file
		logic_timer.Logger.WriteLog();

		// Sleep to make sure everything completes
		GG_Framework::Base::ThreadSleep(500);
	}

	// Done with the sound system
	delete sound;

	// If any debug output files were written, hold the console so we all know about them
	if (GG_Framework::Base::ReleaseDebugFile::FilesWritten)
	{
		std::cout << "DEBUG Files written to CONTENT DIRECTORY" << std::endl;
		std::cout << "Press any key to continue..." << std::endl;
		getch();
	}

	return 0;
}
