// GuardianVR.Client main.cpp
#include "stdafx.h"
#include "GuardianVR.Client.h"
using namespace std;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;
using namespace GG_Framework::UI::OSG;
using namespace GuardianVR::Base;
using namespace GG_Framework::Logic;

#include "../../GG_Framework/Base/ExeMain.hpp"
namespace GuardianVR
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

#include <osg/Notify>

int main(unsigned argc, const char** argv)
{
	GuardianVR::Client::EXE exe;
	return exe.Run(argc, argv);
}

int GuardianVR::Client::EXE::Main(unsigned argc, const char* argv[])
{
	// We will be using Threads
	OpenThreads::Thread::Init();

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

	// Create the Client and start a thread to wait for connections and load game data
	GG_Framework::Logic::Network::Client net_client(serverIP, serverPortNum, clientPortNum);

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


	// We can tie the events to Toggle fullscreen
#ifndef __Use_OSG_Svn__
	mainWin.Keyboard_Mouse.AddKeyBindingR(false, "ToggleFS", Producer::KeyChar_F3);
#else
	mainWin.Keyboard_Mouse.AddKeyBindingR(false, "ToggleFS", osgGA::GUIEventAdapter::KEY_F3);
#endif
	mainWin.Keyboard_Mouse.GlobalEventMap.Event_Map["ToggleFS"].Subscribe(
		mainWin.ehl, mainWin, &MainWindow::ToggleFullScreen);

	// The is the timer that will be fired
	GG_Framework::Logic::Network::SynchronizedTimer timer;

	// This Game Client is what is connecting to the Client
	GG_Framework::Logic::UI_GameClient mainGameClient(net_client, timer, contentDIR);

	// We are going to spawn a thread to read the initial game data
	GG_Framework::Logic::ThreadedClientGameLoader threadGameLoader(mainGameClient);

#ifdef THREADED_LOGO
	// While Waiting, Spin a Logo.  This is what calls run on the threadGameLoader.
	//**** If you want a Spinning Logo, Uncomment the following two lines
	// -------------------------------------------------------------------------
	LogoSceneRunner logoSceneRunner("The GuardianVR", threadGameLoader);
	logoSceneRunner.MyRunScene("../../Gremlin ART/Simple 01.osgv", cam, kbm);
	// -------------------------------------------------------------------------
#else
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
	mainWin.GetMainCamera()->SetSceneNode(mainScene, argParser.MAX_DISTORTION);
	mainWin.Realize();
	mainWin.SetFullScreen(true);
	mainWin.SetWindowText("GuardianVR Client");

	// The updater is complete, it can write out its times
	GG_Framework::UI::OSG::LoadStatus::Instance.LoadComplete();

	// Let all of the scene know we are starting
	osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
	osgUtil::UpdateVisitor update;
	update.setFrameStamp(frameStamp.get());
	
	frameStamp->setSimulationTime(0.0);

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
		SOUNDSYSTEM.PlaySound(music, false);
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

	// All is well for return
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