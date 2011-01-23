#include "stdafx.h"
#include <conio.h>
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Viewer.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

void Viewer::ViewerCallback::operator()(osg::Node *node,osg::NodeVisitor *nv)
{
	if (m_pParent->m_Callback)
		m_pParent->m_Callback->UpdateScene(m_pParent->m_RootNode,m_pParent->m_Geode);
	traverse(node,nv);
}
void Viewer::Start()
{
	// Content Directory
	//char contentDIR[512];
	//_getcwd(contentDIR, 512);
	//printf("Content Directory: %s\n", contentDIR);

	Audio::ISoundSystem* sound = new Audio::SoundSystem_Mock();

	// Create a new scope, so all auto-variables will be deleted when they fall out
	{
		// Create the singletons, These must be instantiated before the scene manager too - Rick
		m_MainWin = new MainWindow(true, 0.0, 0, 0, true);
		MainWindow &mainWin=*m_MainWin;

		// We are going to use this single timer to fire against
		OSG::OSG_Timer timer("OSGV Timer Log.csv");
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFrameLog", osgGA::GUIEventAdapter::KEY_F7);
		timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);

		// Create the scene manager with each of the files
		osg::Group* mainGroup = NULL;

		// Here is a good example of setting up an easy text PDCB
		Framerate_PDCB* fpdcb = new Framerate_PDCB(mainWin);
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ShowFR", osgGA::GUIEventAdapter::KEY_F2);
		mainWin.GetMainCamera()->addPostDrawCallback(*fpdcb);
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ShowFR"].Subscribe(
			fpdcb->ehl, (Text_PDCB&)(*fpdcb), &Text_PDCB::ToggleEnabled);

		// And a debug one
		DebugOut_PDCB* dpdcb = new DebugOut_PDCB(mainWin);
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ShowDebug", osgGA::GUIEventAdapter::KEY_F9);
		mainWin.GetMainCamera()->addPostDrawCallback(*dpdcb);
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ShowDebug"].Subscribe(
			dpdcb->ehl, (Text_PDCB&)(*dpdcb), &Text_PDCB::ToggleEnabled);

		// Make the 'm' key record
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "RecordFrames", 'm');
		ScreenCaptureTool sct(mainWin, mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["RecordFrames"]);

		// We can tie the events to Toggle fullscreen
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFullScreen", osgGA::GUIEventAdapter::KEY_F3);
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFullScreen"].Subscribe(mainWin.ehl, mainWin, &MainWindow::ToggleFullScreen);

		// Set the scene and realize the camera at full size
		//mainWin.GetMainCamera()->SetSceneNode(actorScene.GetScene(), 0.0f);
		//mainWin.GetMainCamera()->SetSceneNode(createHUDText(), 0.0f);
		// make sure the root node is group so we can add extra nodes to it.
		osg::Group* group = new osg::Group;
		{
			// create the hud.
			osg::Camera* camera = new osg::Camera;
			camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
			camera->setProjectionMatrixAsOrtho2D(0,c_Scene_XRes_InPixels,0,c_Scene_YRes_InPixels);
			camera->setViewMatrix(osg::Matrix::identity());
			camera->setClearMask(GL_DEPTH_BUFFER_BIT);
			m_RootNode = new osg::Group;
			m_Geode  = new osg::Geode;
			m_ViewerCallback = new ViewerCallback(this);
			m_RootNode->setUpdateCallback(m_ViewerCallback);
			m_RootNode->addChild(m_Geode);
			camera->addChild(m_RootNode);
			camera->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

			group->addChild(camera);
		}
		mainWin.GetMainCamera()->SetSceneNode(group, 0.0f);

		mainWin.Realize();
		mainWin.SetFullScreen(false);
		mainWin.SetWindowText("AI Tester");

		// Let all of the scene know we are starting
		mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["START"].Fire();

		// Have a frame stamp to run the non-Actor parts
		osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
		osgUtil::UpdateVisitor update;
		update.setFrameStamp(frameStamp.get());
		frameStamp->setSimulationTime(0.0);
		frameStamp->setFrameNumber(0);

		// Connect the argParser to the camera, in case it wants to handle stats (I do not know that I like this here)
		//argParser.AttatchCamera(&mainWin, &timer);

		// Loop while we are waiting for the Connection to be made and all of the scenes
		double dTime_s = 0.0;
		double currTime = 0.0;
		do
		{
			if (!m_UseSyntheticTimeDeltas)
				dTime_s = timer.FireTimer();
			else
				dTime_s = 0.016;  //hard code a typical 60 fps

			currTime = timer.GetCurrTime_s();
			if (m_Callback)
			{
				m_Callback->UpdateData(dTime_s);
				m_Callback->UpdateScene(m_RootNode,m_Geode);
			}
			Audio::ISoundSystem::Instance().SystemFrameUpdate();
			//printf("\r %f      ",dTime_s);
		}
		while(mainWin.Update(currTime, dTime_s));

		// Write the log file
		timer.Logger.WriteLog();

		// Sleep to make sure everything completes
		ThreadSleep(200);
	}

	// Done with the sound
	delete sound;

	// If any debug output files were written, hold the console so we all know about them
	if (ReleaseDebugFile::FilesWritten)
	{
		std::cout << "DEBUG Files written to CONTENT DIRECTORY" << std::endl;
		std::cout << "Press any key to continue..." << std::endl;
		getch();
	}
}

Viewer::~Viewer()
{
	if (m_MainWin)
	{
		m_MainWin->TryClose();
		// Sleep to make sure everything completes
		ThreadSleep(500);
		delete m_MainWin;
		m_MainWin=NULL;
	}
};
