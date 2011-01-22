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

bool g_SingleThreaded=false;

class AITester_MainWindow : public MainWindow
{
	public:
		AITester_MainWindow(bool useAntiAlias, double throttle_fps, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs,Timer &timer) :
		  MainWindow(useUserPrefs, throttle_fps, screenWidth, screenHeight, useUserPrefs, timer) {}

		virtual void PostRealize()
		{
			SetFullScreen(false);
			SetWindowText("AI Tester");
			__super::PostRealize();
		}
};

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
		MainWindow::SINGLE_THREADED_MAIN_WINDOW = g_SingleThreaded;

		// We are going to use this single timer to fire against
		OSG::OSG_Timer osg_timer("AI_tester-OSG Timer Log.csv");
		OSG::OSG_Timer logic_timer("AI_tester-Logic Timer Log.csv");
		GG_Framework::Base::Timer* osg_timer_ref = NULL;
		if (GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW)
			osg_timer_ref = &logic_timer;
		else
			osg_timer_ref = &osg_timer;

		// Create the singletons, These must be instantiated before the scene manager too - Rick
		m_MainWin = new AITester_MainWindow(true, 60.0, 0, 0, true, *osg_timer_ref);
		MainWindow &mainWin=*m_MainWin;

		// Watch for logging the timer
		mainWin.GetKeyboard_Mouse().AddKeyBindingR(false, "ToggleFrameLog", osgGA::GUIEventAdapter::KEY_F7);
		logic_timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);
		if (!GG_Framework::UI::MainWindow::SINGLE_THREADED_MAIN_WINDOW)
			osg_timer.Logger.ListenForToggleEvent(mainWin.GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFrameLog"]);

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

		// Set the scene and realize the camera at full size
		//mainWin.GetMainCamera()->SetSceneNode(actorScene.GetScene(), 0.0f);
		//mainWin.GetMainCamera()->SetSceneNode(createHUDText(), 0.0f);
		// make sure the root node is group so we can add extra nodes to it.
		osg::Group* group = new osg::Group;
		{
			// create the empty scene (HUD style)
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
		mainWin.GetMainCamera()->SetSceneNode(group);
		mainWin.Realize();

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

		// Here is the logic loop
		double dTime_s = 0.0;
		double currTime_s = 0.0;
		double syncTimer_s = 0.0;
		do
		{
			if (!m_UseSyntheticTimeDeltas)
				dTime_s = logic_timer.FireTimer();
			else
				dTime_s = 0.016;  //hard code a typical 60 fps
			currTime_s = logic_timer.GetCurrTime_s();
			if (m_Callback)
				m_Callback->UpdateData(dTime_s);
			//printf("\r %f      ",dTime_s);
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
