// EXE.cpp
#include "EXE.h"

#include <osg/Notify>

// Do we want a threaded logo?
// #define THREADED_LOGO

class FringeSingleMainWindow : public GG_Framework::UI::MainWindow
{
public:
	FringeSingleMainWindow(bool useAntiAlias, double throttle_fps, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs, GG_Framework::Base::Timer& osg_timer) :
	  GG_Framework::UI::MainWindow(useUserPrefs, throttle_fps, screenWidth, screenHeight, useUserPrefs, osg_timer) {}

	virtual void PostRealize()
	{
		SetFullScreen(true);
		SetWindowText("Rim-Space Proto");
		__super::PostRealize();
	}
};

int Fringe::Single::EXE::Main(unsigned argc, const char* argv[])
{
	// We will be using Threads
	OpenThreads::Thread::Init();

	// Make the MainWindow multi-threaded or not here:
	GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW = true;

	// Strip out the flagged and non-flagged options
	Demo_ArgumentParser argParser;
	std::vector<string> argList = argParser.ProcessArguments(argc, argv);
	osg::setNotifyLevel(argParser.NotifySeverity);

	// Content Directory
	char contentDIR[512];
	_getcwd(contentDIR, 512);
	printf("Content Directory: %s\n", contentDIR);

	// Use the parameters, Config file, and File Dialog to find the name of the script to run
	GG_Framework::Base::ConfigFileIO configFile;
	vector<string> fileList;
	if (!configFile.GetFilesToLoad(argList, fileList, "Select Script File", NULL))
	{
		printf("User Aborted, QUITTING\n");
		return 0;
	}

	// Build the full file path, watch for fileList[0] being a relative or absolute path
	string scriptFN = (fileList[0].find(':')!=string::npos) ? 
		(fileList[0]) : 
		(string(contentDIR) + string("\\") + fileList[0]);

	// Verify that we can read the script
	Fringe::Base::FringeScript script;
	printf("Loading Script file: %s\n", scriptFN.c_str());
	const char* err = script.LoadScript(scriptFN.c_str(), true);
	if (err)
	{
		printf("Unable to load script file: %s\n", err);
		return 1;
	}
	else
		printf("Script File Loaded OK\n");

	// After reading the script, we have to reset the CWD
	_chdir(contentDIR);
	osg::notify(osg::NOTICE) << GG_Framework::Base::BuildString(
		"_chdir: \"%s\"\n", contentDIR);

	// Reset the loading logic_timer
	GG_Framework::UI::OSG::LoadStatus::Instance.StartLoadingTimer();

	// Create the single sound system
	GG_Framework::UI::Audio::ISoundSystem* sound;
	if (argParser.MockAudio)
		sound = new GG_Framework::UI::Audio::SoundSystem_Mock();
	else
		sound = new GG_Framework::UI::Audio::SoundSystem();

	// Create a new scope, so all auto-variables will be deleted when they fall out
	{			
		// Create the player side connections, both client and server
		MockServer player_server("Player_Server");
		MockClient player_client(player_server, MOCK_SINGLE_PLAYER_ID);
		MockServer ai_server("AI_Server");
		MockClient ai_client(ai_server, MOCK_AI_PLAYER_ID);
		GG_Framework::Logic::Network::SynchronizedTimer aiTimer("AI_Timer Log.csv");
		GG_Framework::Logic::AI_GameClient aiGameClient(*(ai_server.GetMockClient()), aiTimer, contentDIR);
		GG_Framework::Logic::ServerManager serverManager(&script, aiTimer, &player_server, &ai_server, aiGameClient);
		if (argParser.LogTimer)
			serverManager.AI_Timer.Logger.ToggleActive();

		// The is the logic_timer that will be fired, AND the OSG Timer
		GG_Framework::Logic::Network::SynchronizedTimer logic_timer("Fringe.Single Logic Timer Log.csv");
		GG_Framework::UI::OSG::OSG_Timer osg_timer("Fringe.Single OSG Timer Log.csv");
		GG_Framework::Base::Timer* osg_timer_ref = NULL;
		if (GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW)
			osg_timer_ref = &logic_timer;
		else
			osg_timer_ref = &osg_timer;

		// Our single window
		FringeSingleMainWindow mainWin(argParser.UseAntiAliasing, argParser.MAX_FPS, argParser.RES_WIDTH, argParser.RES_HEIGHT, true, *osg_timer_ref);


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

		// Watch for logging the timer
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFrameLog", osgGA::GUIEventAdapter::KEY_F7);
		logic_timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);
		if (!GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW)
			osg_timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);

		// This Game Client is what is connecting to the Client
		SinglePlayer_GameClient mainGameClient(player_client, logic_timer, *osg_timer_ref, contentDIR);

		// We are going to spawn a thread to read the initial game data
		GG_Framework::Logic::ThreadedClientGameLoader threadGameLoader(mainGameClient);


		// While Waiting, Spin a Logo.  This is what calls run on the threadGameLoader AND the serverManager
		//**** If you want a Spinning Logo, Uncomment the following lines
		// -------------------------------------------------------------------------
#ifdef THREADED_LOGO
		LogoSceneRunner logoSceneRunner("The Fringe", threadGameLoader, &serverManager);
		logoSceneRunner.MyRunScene("../../Gremlin ART/Simple 01.osgv", cam, kbm);
#else
		// -------------------------------------------------------------------------
		//**** UnComment the following lines to NOT use a spinning logo, loads the main scene on this thread
		// -------------------------------------------------------------------------
		serverManager.start();		//< Run the server in a separate thread
		// GG_Framework::Base::ThreadSleep(10000);
		threadGameLoader.run();		//< Then run the game loader in this thread
#endif
		// -------------------------------------------------------------------------

		// If the USER exited before a connection
		if (!threadGameLoader.IsLoadComplete())
		{
			cout << "User ABORTED before Connection, ABORTING" << endl;
			threadGameLoader.cancel();
			serverManager.cancel();
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

		// Now that the user has exited, we can cancel the serverManager thread as well
		serverManager.cancel();

		// Write the log file
		logic_timer.Logger.WriteLog();

		// Sleep to make sure everything completes
		GG_Framework::Base::ThreadSleep(500);
	}

	// Done with the sound
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
