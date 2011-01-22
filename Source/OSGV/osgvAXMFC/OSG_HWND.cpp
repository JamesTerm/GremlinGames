// MFC_OSG.cpp : implementation of the OSG_HWND class
//
#include "stdafx.h"
#include "OSG_HWND.h"
#include <direct.h>

// Used for a variety of OSG path information, including the lib path and the content dir
const char* GetRegPath(const char* regVal)
{
	if (!regVal || !regVal[0])
		return NULL;

	const char* RegKey = "SOFTWARE\\GremlinGames\\General";
	HKEY    hKey;
	static char	szVALUE[512]; 
	szVALUE[0] = 0;
	DWORD	dwSize = 512;
	DWORD	dwType = REG_SZ;

	try
	{
		if (RegOpenKeyA(HKEY_LOCAL_MACHINE, RegKey, &hKey) == ERROR_SUCCESS)
		{
			RegQueryValueExA(hKey, regVal,  NULL, &dwType, reinterpret_cast<BYTE *>(szVALUE), &dwSize);
			RegCloseKey(hKey);
		}
	}
	catch (...)
	{
		// TODO: Provide some better handling?
		return NULL;
	}

	return szVALUE;
}
//////////////////////////////////////////////////////////////////////////

void Initialize_OSG_LibPath()
{
#ifdef _DEBUG
	const char* libPathRegVal = "OSG_LibPath_DEBUG";
#else
	const char* libPathRegVal = "OSG_LibPath";
#endif

	const char* libPath = GetRegPath(libPathRegVal);
	if (libPath[0] != 0)
		osgDB::Registry::instance()->setLibraryFilePathList(libPath);
}
//////////////////////////////////////////////////////////////////////////


OSG_HWND::OSG_HWND(HWND hWnd) : m_bkgdColor(0,0,0,1), CurrFrameNum(0), m_hWnd(hWnd), mDone(false)
{
}

OSG_HWND::~OSG_HWND()
{
	mDone=true;
	while (mViewer)
		Sleep(100);
}

void OSG_HWND::InitOSG(std::string modelname, std::string contentRegKey)
{
	// Store the name of the model to load
	m_modelName = modelname;
	m_contentRegKey = contentRegKey;

	// Init different parts of OSG
	InitManipulators();
	InitCameraConfig();
}

void OSG_HWND::SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b)
{
	m_bkgdColor = osg::Vec4(r/255.0, g/255.0, b/255.0, 1.0);
}

void OSG_HWND::InitManipulators(void)
{
	// Create a trackball manipulator
	trackball = new osgGA::TrackballManipulator();

	// Create a Manipulator Switcher
	keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

	// Add our trackball manipulator to the switcher
	keyswitchManipulator->addMatrixManipulator( '1', "Trackball", trackball.get());

	// Init the switcher to the first manipulator (in this case the only manipulator)
	keyswitchManipulator->selectMatrixManipulator(0);  // Zero based index Value
}


void OSG_HWND::InitSceneGraph()
{
	// Set the appropriate library path for the OSGDB plugins
	Initialize_OSG_LibPath();

	// Remember my old PWD
	char oldPWD[512];
	_getcwd(oldPWD, 512);

	// Init the main Root Node/Group
	mRoot  = new osg::Group;

	// Set the PWD based on what content dir the user wants
	const char* newPWD = GetRegPath(m_contentRegKey.c_str());
	if (newPWD && newPWD[0])
		_chdir(newPWD);

	// Load the Model from the model name
	mModel = osgDB::readNodeFile(m_modelName);
	if (mModel.valid())
	{
		// Optimize the model
		osgUtil::Optimizer optimizer;
		optimizer.optimize(mModel.get());
		optimizer.reset();

		// Add the model to the scene
		mRoot->addChild(mModel.get());
	}

	// Reset the PWD
	_chdir(oldPWD);
}

void OSG_HWND::InitCameraConfig(void)
{
	// Local Variable to hold window size data
	RECT rect;

	// Create the viewer for this window
	mViewer = new osgViewer::Viewer();
	mViewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

	// We do NOT want to allow the escape key to kill the viewer
	mViewer->setKeyEventSetsDone(0);

	// Add a Stats Handler to the viewer
	osg::ref_ptr<osgViewer::StatsHandler> stats = new osgViewer::StatsHandler;
	mViewer->addEventHandler(stats.get());

	// Get the current window size
	::GetWindowRect(m_hWnd, &rect);

	// Init the GraphicsContext Traits
	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;

	// Init the Windata Variable that holds the handle for the Window to display OSG in.
	osg::ref_ptr<osg::Referenced> windata = new osgViewer::GraphicsWindowWin32::WindowData(m_hWnd);

	// Setup the traits parameters
	traits->x = 0;
	traits->y = 0;
	traits->width = rect.right - rect.left;
	traits->height = rect.bottom - rect.top;
	traits->windowDecoration = false;
	traits->doubleBuffer = true;
	traits->sharedContext = 0;
	traits->setInheritedWindowPixelFormat = true;
	traits->inheritedWindowData = windata;

	// Create the Graphics Context
	osg::GraphicsContext* gc = osg::GraphicsContext::createGraphicsContext(traits.get());

	// Init a new Camera (Master for this View)
	m_camera = new osg::Camera;

	// Assign Graphics Context to the Camera
	m_camera->setGraphicsContext(gc);

	// Set the viewport for the Camera
	m_camera->setViewport(new osg::Viewport(traits->x, traits->y, traits->width, traits->height));

	// And the clear color the first time
	m_camera->setClearColor(m_bkgdColor);
	m_bkgdColor[3] = 0.0;	// Remember it has already been set

	// Add the Camera to the Viewer
	mViewer->addSlave(m_camera.get());

	// Add the Camera Manipulator to the Viewer
	mViewer->setCameraManipulator(keyswitchManipulator.get());

	// Realize the Viewer
	mViewer->realize();

	// Set the Scene Data
	InitSceneGraph();
	mViewer->setSceneData(mRoot.get());
}

void OSG_HWND::PreFrameUpdate()
{
	// Do any preframe updates in this routine
}

void OSG_HWND::PostFrameUpdate()
{
	// Do any postframe updates in this routine
}

void OSG_HWND::Render(void* ptr)
{
	OSG_HWND* osg = (OSG_HWND*)ptr;

	// You have two options for the main viewer loop
	//      osg->mViewer->run()   or
	//      while(!osg->mViewer->done()) { osg->mViewer->frame(); }
	
	
	// Note that there are times when I am not able to get out of this thread.
	// I may need to provide some other handler for knowing when the window is being closed

	// viewer->run();
	long currFrame = 0;
	while(!osg->mViewer->done() && !osg->mDone)
	{
		osg->PreFrameUpdate();
		osg->mViewer->frame();
		osg->PostFrameUpdate();

		// Playing with events, not thread safe
		++currFrame;
		if ((currFrame % 1000) == 0)
			osg->CurrFrameNum = currFrame;	
		//Sleep(10);         // Use this command if you need to allow other processes to have cpu time
	}

	// For some reason this has to be here to avoid issue: 
	// if you have multiple OSG windows up 
	// and you exit one then all stop rendering
	// MessageBox("Exit Rendering Thread", "INFO");

	osg->mViewer->setDone(true);
	osg->mViewer->stopThreading();
	osgViewer::Viewer* viewer = osg->mViewer;
	osg->mViewer = NULL;
	delete viewer;
	_endthread();
}
