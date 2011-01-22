// OSGV.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
using namespace std;

#include "../GG_Framework/UI/GG_Framework.UI.h"
#include "../GG_Framework/UI/Audio/GG_Framework.UI.Audio.h"

#include "../ExecutableVersionSystem.hpp"
#include <conio.h>

#include <osg/Material>
#include <osg/BlendFunc>

using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

#include "../GG_Framework/Base/ExeMain.hpp"
namespace OSGV
{
	class EXE : public IExeMain
	{
	public:
		virtual int Main(unsigned argc, const char* argv[]);
	};
}

class OSGV_MainWindow : public GG_Framework::UI::MainWindow
{
public:
	OSGV_MainWindow(bool useAntiAlias, double throttle_fps, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs, GG_Framework::Base::Timer& timer) :
	  GG_Framework::UI::MainWindow(useUserPrefs, throttle_fps, screenWidth, screenHeight, useUserPrefs, timer) {}

	  virtual void PostRealize()
	  {
		  SetFullScreen(true);
		  SetWindowText("OSGV");
		  __super::PostRealize();
	  }
};

int main(unsigned argc, const char** argv)
{
	OSGV::EXE exe;
	return exe.Run(argc, argv);
}

int OSGV::EXE::Main(unsigned argc, const char* argv[])
{
	// Content Directory
	char contentDIR[512];
	_getcwd(contentDIR, 512);
	printf("Content Directory: %s\n", contentDIR);

	// We always work with OSG single threaded
	GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW = true;

	// Strip out the flagged and non-flagged options
	GG_Framework::UI::ViewerApp_ArgumentParser argParser;
	std::vector<string> argList = argParser.ProcessArguments(argc, argv);
	osg::setNotifyLevel(argParser.NotifySeverity);

	// Find all of the files we are supposed to be loading
	GG_Framework::Base::ConfigFileIO configFile;
	vector<string> fileList;
	if (!configFile.GetFilesToLoad(argList, fileList, "Select Main File", "Select Additional Files"))
	{
		cout << "User Aborted, QUITTING" << endl;
		return 0;
	}

	// After prompting for the files, we have to reset the CWD
	_chdir(contentDIR);
	osg::notify(osg::NOTICE) << GG_Framework::Base::BuildString(
		"_chdir: \"%s\"\n", contentDIR);

	// Write out all of the files as a review for the user
	cout << "File List:" << endl;
	unsigned i;
	for (i = 0; i < fileList.size(); ++i)
		cout << fileList[i] << endl;
	cout << "====================" << endl;
	cout << endl;

	// Note: Sound must be instantiated before the UI -James
	GG_Framework::UI::Audio::ISoundSystem* sound;
	if (argParser.MockAudio)
		sound = new GG_Framework::UI::Audio::SoundSystem_Mock();
	else
		sound = new GG_Framework::UI::Audio::SoundSystem();

	// Create a new scope, so all auto-variables will be deleted when they fall out
	{
		// We are going to use this single timer to fire against
		GG_Framework::UI::OSG::OSG_Timer timer("OSGV Timer Log.csv");

		// Create the singletons, These must be instantiated before the scene manager too - Rick
		OSGV_MainWindow mainWin(argParser.UseAntiAliasing, argParser.MAX_FPS, argParser.RES_WIDTH, argParser.RES_HEIGHT, false, timer);

		// We can also look for toggling the timer logging
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFrameLog", osgGA::GUIEventAdapter::KEY_F7);
		timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);

		// Create the scene manager with each of the files
		GG_Framework::UI::ActorScene actorScene(timer, contentDIR, false);
		osg::Group* mainGroup = NULL;
		for (i = 0; i < fileList.size(); ++i)
		{
			osg::Group* group = actorScene.AddActorFile(mainWin.GetKeyboard_Mouse().GlobalEventMap, fileList[i].c_str());
		}

		// Create a trackball style manipulator for the main camera
		GG_Framework::UI::CenteredTrackball_CamManipulator trackball;
		trackball.setOrientation(GG_Framework::UI::OSG::Producer_Trackball::Z_UP);
		trackball.setRotationalMode(GG_Framework::UI::OSG::Producer_Trackball::FixedAxis);
		trackball.SetCenteredNode(mainGroup);
		mainWin.GetMainCamera()->SetCameraManipulator(&trackball);
		GG_Framework::Base::UserInputEvents& kbm = mainWin.GetKeyboard_Mouse().GlobalEventMap.KBM_Events;
		kbm.MouseMove.Subscribe(trackball.ehl, trackball, &CenteredTrackball_CamManipulator::OnMouseMove);
		kbm.KBCB_KeyDnUp.Subscribe(trackball.ehl, trackball, &CenteredTrackball_CamManipulator::KeyPress);
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ResetTB", ' ');
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ResetTB"].Subscribe(
			trackball.ehl, trackball, &CenteredTrackball_CamManipulator::ResetTB);

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

		// F4 displays the trackball dist in the corner
		GG_Framework::UI::TrackballDist_PDCB* tbdcb = new GG_Framework::UI::TrackballDist_PDCB(trackball);
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ShowTB", osgGA::GUIEventAdapter::KEY_F4);
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ShowTB"].Subscribe(
			tbdcb->ehl, (GG_Framework::UI::Text_PDCB&)(*tbdcb), &GG_Framework::UI::Text_PDCB::ToggleEnabled);
		mainWin.GetMainCamera()->addPostDrawCallback(*tbdcb);
		tbdcb->Enable(true);

		// We can tie the events to Toggle fullscreen
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFullScreen", osgGA::GUIEventAdapter::KEY_F3);

		// Set the scene and realize the camera at full size
		mainWin.GetMainCamera()->SetSceneNode(actorScene.GetScene());
		mainWin.Realize();

		// Let all of the scene know we are starting
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["START"].Fire();

		// Connect the argParser to the camera, in case it wants to handle stats (I do not know that I like this here)
		argParser.AttatchCamera(&mainWin, &timer);

		// Loop while we are waiting for the Connection to be made and all of the scenes
		double dTime_s = 0.0;
		double currTime = 0.0;
		do
		{
			dTime_s = timer.FireTimer();
			currTime = timer.GetCurrTime_s();
		}
		while(mainWin.Update(currTime, dTime_s));

		// Write the log file
		timer.Logger.WriteLog();

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

