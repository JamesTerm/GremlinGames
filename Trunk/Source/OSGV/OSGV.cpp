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
		// Create the singletons, These must be instantiated before the scene manager too - Rick
		GG_Framework::UI::MainWindow mainWin(argParser.UseAntiAliasing, argParser.MAX_FPS, argParser.RES_WIDTH, argParser.RES_HEIGHT, false);

		// We are going to use this single timer to fire against
		GG_Framework::UI::OSG::OSG_Timer timer("OSGV Timer Log.csv");
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFrameLog", osgGA::GUIEventAdapter::KEY_F7);
		timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);

		// Create the scene manager with each of the files
		GG_Framework::UI::ActorScene actorScene(timer, contentDIR, false);
		osg::Group* mainGroup = NULL;
		for (i = 0; i < fileList.size(); ++i)
		{
			osg::Group* group = actorScene.AddActorFile(mainWin.GetKeyboard_Mouse().GlobalEventMap, fileList[i].c_str());
			if (i == 0)
			{
				/*
				// Play with making this node transparent
				osg::StateSet* state = group->getOrCreateStateSet();
				state->setMode(GL_BLEND,osg:: StateAttribute::ON| osg::StateAttribute::OVERRIDE); 
				osg::Material* mat = new osg::Material; 
				mat->setAlpha(osg::Material::FRONT_AND_BACK, 1.0); 
				state->setAttributeAndModes(mat,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE); 
				osg::BlendFunc* bf = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA ); 
				state->setAttributeAndModes(bf); state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN); 
				// state->setMode(GL_LIGHTING, osg::StateAttribute::ON); 
				*/

				mainGroup = group;
			}
		}

		// Create a trackball style manipulator for the main camera
		GG_Framework::UI::CenteredTrackball_CamManipulator trackball;
		trackball.setOrientation(GG_Framework::UI::OSG::Producer_Trackball::Z_UP);
		trackball.setRotationalMode(GG_Framework::UI::OSG::Producer_Trackball::FixedAxis);
		trackball.SetCenteredNode(mainGroup);
		mainWin.GetMainCamera()->SetCameraManipulator(&trackball);
		GG_Framework::Base::UserInputEvents& kbm = mainWin.GetKeyboard_Mouse().GlobalEventMap.KBM_Events;
		kbm.MouseMove.Subscribe(trackball.ehl, trackball, &CenteredTrackball_CamManipulator::OnMouseMove);
		kbm.MouseBtnPress.Subscribe(trackball.ehl, trackball, &CenteredTrackball_CamManipulator::OnMouseBtnPress);
		kbm.MouseBtnRelease.Subscribe(trackball.ehl, trackball, &CenteredTrackball_CamManipulator::OnMouseBtnRelease);
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
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFullScreen"].Subscribe(
			mainWin.ehl, mainWin, &MainWindow::ToggleFullScreen);

		// Set the scene and realize the camera at full size
		mainWin.GetMainCamera()->SetSceneNode(actorScene.GetScene(), 0.0f);
		mainWin.Realize();
		mainWin.SetFullScreen(true);
		mainWin.SetWindowText("OSGV");

		// Let all of the scene know we are starting
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["START"].Fire();

		// Have a frame stamp to run the non-Actor parts
		osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
		osgUtil::UpdateVisitor update;
		update.setFrameStamp(frameStamp.get());
		frameStamp->setSimulationTime(0.0);
		frameStamp->setFrameNumber(0);
		actorScene.GetScene()->accept(update);

		// Connect the argParser to the camera, in case it wants to handle stats (I do not know that I like this here)
		argParser.AttatchCamera(&mainWin, &timer);

		// Loop while we are waiting for the Connection to be made and all of the scenes
		double dTime_s = 0.0;
		double currTime = 0.0;
		do
		{
			dTime_s = timer.FireTimer();
			currTime = timer.GetCurrTime_s();
			Audio::ISoundSystem::Instance().SystemFrameUpdate();
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

