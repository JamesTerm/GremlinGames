// GuardianVR.Single main.cpp
#include "stdafx.h"
#include "GuardianVR.Single.h"
using namespace std;

using namespace GG_Framework::UI;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Logic::Network;
using namespace GuardianVR::Base;

#include "../../GG_Framework/Base/ExeMain.hpp"
namespace GuardianVR
{
	namespace Single
	{
		class EXE : public IExeMain
		{
		public:
			EXE()
			{
				GG_Framework::UI::OSG::LoadStatus::Instance.LoadStatusUpdate.Subscribe(ehl, *this, &EXE::LoadStatusUpdate);
			}
			virtual int Main(unsigned argc, const char* argv[]);
			void LoadStatusUpdate(float timeSoFar, float timeRemaining)
			{
				if (timeRemaining > 0.0f)
				{
					int pcntHalf = (int)(50.0f * timeSoFar / (timeSoFar+timeRemaining));
					char bar[51];
					for (int i = 0; i < 51; ++i)
						bar[i] = (i < pcntHalf) ? '-' : ' ';
					bar[50] = 0;
					printf("\rLoad Status: [%s] Rem: %i s.  ", bar, (int)timeRemaining);
				}
				else if (timeRemaining == 0.0f)
				{
					printf("\rLoad COMPLETE!                                                                \n");
				}
			}

		private:
			IEvent::HandlerList ehl;
		};
	}
}

#include <osg/Notify>

int main(unsigned argc, const char** argv)
{
	GuardianVR::Single::EXE exe;
	return exe.Run(argc, argv);
}

// Do we want a threaded logo?
// #define THREADED_LOGO

int GuardianVR::Single::EXE::Main(unsigned argc, const char* argv[])
{
	// We will be using Threads
	OpenThreads::Thread::Init();

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
	string scriptFN = fileList[0];

	// Verify that we can read the script
	// printf("Loading LUA Script from %s ...\n", scriptFN.c_str());
	GuardianVR::Base::GuardianVR_Script script;
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
				

	// Create the player side connections, both client and server
	MockServer player_server("Player_Server");
	MockClient player_client(player_server, MOCK_SINGLE_PLAYER_ID);
	MockServer ai_server("AI_Server");
	MockClient ai_client(ai_server, MOCK_AI_PLAYER_ID);
	GG_Framework::Logic::ServerManager serverManager(&script, &player_server, &ai_server, contentDIR);

	// There will be a single camera, KBM, sound system, etc.
	GG_Framework::UI::Audio::SoundSystem sound;
	GG_Framework::UI::MainWindow mainWin(argParser.UseAntiAliasing, argParser.MAX_FPS, argParser.RES_WIDTH, argParser.RES_HEIGHT);


	// Here is a good example of setting up an easy text PDCB
	GG_Framework::UI::Framerate_PDCB* fpdcb = new GG_Framework::UI::Framerate_PDCB(mainWin);
	mainWin.Keyboard_Mouse.AddKeyBindingR(false, "ShowFR", osgGA::GUIEventAdapter::KEY_F2);
	mainWin.GetMainCamera()->addPostDrawCallback(*fpdcb);
	mainWin.Keyboard_Mouse.GlobalEventMap.Event_Map["ShowFR"].Subscribe(
		fpdcb->ehl, (GG_Framework::UI::Text_PDCB&)(*fpdcb), &GG_Framework::UI::Text_PDCB::ToggleEnabled);

	// And a debug one
	GG_Framework::UI::DebugOut_PDCB* dpdcb = new GG_Framework::UI::DebugOut_PDCB(mainWin);
	mainWin.Keyboard_Mouse.AddKeyBindingR(false, "ShowDebug", osgGA::GUIEventAdapter::KEY_F9);
	mainWin.GetMainCamera()->addPostDrawCallback(*dpdcb);
	mainWin.Keyboard_Mouse.GlobalEventMap.Event_Map["ShowDebug"].Subscribe(
		dpdcb->ehl, (GG_Framework::UI::Text_PDCB&)(*dpdcb), &GG_Framework::UI::Text_PDCB::ToggleEnabled);

	// Make the 'm' key record
	mainWin.Keyboard_Mouse.AddKeyBindingR(false, "RecordFrames", 'm');
	GG_Framework::UI::ScreenCaptureTool sct(mainWin, mainWin.Keyboard_Mouse.GlobalEventMap.Event_Map["RecordFrames"]);

	
	// We can tie the events to Toggle fullscreen  to F3
	mainWin.Keyboard_Mouse.AddKeyBindingR(false, "ToggleFS", osgGA::GUIEventAdapter::KEY_F3);
	mainWin.Keyboard_Mouse.GlobalEventMap.Event_Map["ToggleFS"].Subscribe(
		mainWin.ehl, mainWin, &MainWindow::ToggleFullScreen);

	// The is the timer that will be fired
	GG_Framework::Logic::Network::SynchronizedTimer timer;

	// This Game Client is what is connecting to the Client
	SinglePlayer_GameClient mainGameClient(player_client, timer, contentDIR);

	// We are going to spawn a thread to read the initial game data
	GG_Framework::Logic::ThreadedClientGameLoader threadGameLoader(mainGameClient);
	

	// While Waiting, Spin a Logo.  This is what calls run on the threadGameLoader AND the serverManager
	//**** If you want a Spinning Logo, Uncomment the following lines
	// -------------------------------------------------------------------------
#ifdef THREADED_LOGO
	LogoSceneRunner logoSceneRunner("The GuardianVR", threadGameLoader, &serverManager);
	logoSceneRunner.MyRunScene("../../Gremlin ART/Simple 01.osgv", cam, kbm);
#else
	// -------------------------------------------------------------------------
	//**** UnComment the following lines to NOT use a spinning logo, loads the main scene on this thread
	// -------------------------------------------------------------------------
	serverManager.start();		//< Run the server in a separate thread
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
	mainWin.GetMainCamera()->SetSceneNode(mainScene, argParser.MAX_DISTORTION);
	mainWin.Realize();
	mainWin.SetFullScreen(true);
	mainWin.SetWindowText("GuardianVR Demo");

	// The updater is complete, it can write out its times
	GG_Framework::UI::OSG::LoadStatus::Instance.LoadComplete();

	// Let all of the scene know we are starting
	osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
	osgUtil::UpdateVisitor update;
	update.setFrameStamp(frameStamp.get());
#ifndef __Use_OSG_Svn__
	frameStamp->setReferenceTime(0.0);
#else
	frameStamp->setSimulationTime(0.0);
#endif
	frameStamp->setFrameNumber(0);
	mainScene->accept(update);
	mainWin.Keyboard_Mouse.GlobalEventMap.Event_Map["START"].Fire();
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
	argParser.AttatchCamera(&mainWin, &timer);

	// Loop while we are waiting for the Connection to be made and all of the scenes
	double dTime_s = 0.0;
	double currTime = 0.0;
	do
	{
		dTime_s = timer.FireTimer();
		currTime = timer.GetCurrTime_s();
#ifndef __Use_OSG_Svn__
		frameStamp->setReferenceTime(currTime);
		frameStamp->setFrameNumber(TIME_2_FRAME(currTime));
		mainScene->accept(update);
#else
		// In the new osgViewer::Viewer, we already are setting the time
#endif
		Audio::ISoundSystem::Instance().SystemFrameUpdate();
		// GG_Framework::Base::ThreadSleep(1);
	}
	while(mainWin.Update(currTime, dTime_s));


	// Now that the user has exited, we can cancel the serverManager thread as well
	serverManager.cancel();
	return 0;
}

// USe Pragma comments to link to these libs, only for the old OSG
#ifndef __Use_OSG_Svn__

#ifdef _DEBUG
#pragma comment(lib, "Producerd.lib")
#else
#pragma comment(lib, "Producer.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib, "osgGAd.lib")
#else
#pragma comment(lib, "osgGA.lib")
#endif

#endif