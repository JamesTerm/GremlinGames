// GG_Framework.UI MainWindow.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

#define DEBUG_SCREEN_RESIZE // printf
#define THREADING_MODEL osgViewer::ViewerBase::ThreadPerCamera
#define OSG_RefreshFPS 30.0

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;
using namespace std;

double Window::PERFORMANCE_MIN = 0.0; // All performance is ignored for now because it is not on the OSG thread anyway
double Window::PERFORMANCE_MAX = 60.0;
int Window::PERFORMANCE_INIT_INDEX = 0;

bool MainWindow::SINGLE_THREADED_MAIN_WINDOW = false;
int MainWindow::LOGIC_THREAD_FRAME_MS=0;
int MainWindow::AI_THREAD_FRAME_MS=0;
int MainWindow::OSG_THREAD_FRAME_MS=0;

Window::Window(bool useAntiAlias, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs) : 
	m_useAntiAlias(useAntiAlias), 
	m_viewer(new osgViewer::Viewer),
	m_mainCam(m_viewer.get()),
	m_ConfigurationManager(useUserPrefs),
	m_lastWidth(640), m_lastHeight(480), m_lastX(100), m_lastY(100),
	m_isFullScreen(false),
	m_newScreenWidth(screenWidth),
	m_newScreenHeight(screenHeight),
	m_origScreenWidth(0),
	m_origScreenHeight(0),
	m_velocityAvg(15),
	m_waitForPerformanceIndex_s(6.0), // Wait a few seconds before making performance updates
	m_performanceStrikes(0), // We need to have a few seconds on a row of performance changes before we switch
	m_performanceIndex(PERFORMANCE_INIT_INDEX),
	m_realizeComplete(false)
{
	// Get the display settings for some special effects	
	osg::DisplaySettings* ds = new osg::DisplaySettings();

	// Try with multi-threading draw dispatch
	ds->setSerializeDrawDispatch(false);

	// Try it with Anti-Aliasing to make it look really pretty!
	if (m_useAntiAlias)
		ds->setNumMultiSamples(4);

	m_viewer->setDisplaySettings(ds);

	// We do NOT want the escape key to dismiss this window
	m_viewer->setKeyEventSetsDone(0);

	// Play with different threading models
#ifdef THREADING_MODEL
	m_viewer->setThreadingModel(THREADING_MODEL);
#endif

	// We want a stats handler, but bound to the F5 key
	osg::ref_ptr<osgViewer::StatsHandler> statsHandler = new osgViewer::StatsHandler;
	statsHandler->setKeyEventPrintsOutStats(osgGA::GUIEventAdapter::KEY_F5);
	statsHandler->setKeyEventTogglesOnScreenStats(osgGA::GUIEventAdapter::KEY_F5);
	m_viewer->addEventHandler(statsHandler.get());

	// Init variables for finding frame-rate
	m_frameRate = 0.0;
	m_frameRateAvg = 0.0;
	m_lastFrameRateCheckTime_s = -1.0;
	m_frameNum = -3; // Wait a few frames before starting to count frame rate

	// Config Manager
	m_Joystick = new JoyStick_Binder(&m_ConfigurationManager);
	refKBM = new KeyboardMouse_CB(&m_ConfigurationManager);
	m_Keyboard_Mouse = refKBM.get();

	//TODO fix this to whatever keys and operations are intuitive
	GetKeyboard_Mouse().AddKeyBindingR(false, "VolumeDown", 'v');
	GetKeyboard_Mouse().AddKeyBindingR(false, "VolumeUp", 'b');
	GetKeyboard_Mouse().AddKeyBindingR(false, "VolumeSelect", 'n');
	m_VolumeControls=new AudioVolumeControls_PlusBLS  (
		GetKeyboard_Mouse().GlobalEventMap.Event_Map["VolumeSelect"],
		GetKeyboard_Mouse().GlobalEventMap.Event_Map["VolumeUp"],
		GetKeyboard_Mouse().GlobalEventMap.Event_Map["VolumeDown"]
		);

	// Populate list
	m_ConfigurationManager.AddConfigLoadSaveInterface(m_Joystick);
	m_ConfigurationManager.AddConfigLoadSaveInterface(m_Keyboard_Mouse);
	m_ConfigurationManager.AddConfigLoadSaveInterface(m_VolumeControls);
	//Now to load em
	m_ConfigurationManager.UpdateSettings_Load();
}
//////////////////////////////////////////////////////////////////////////

Window::~Window()
{
	m_ConfigurationManager.UpdateSettings_Save();
	delete m_Joystick;
	m_Joystick = NULL;
	delete m_VolumeControls;
	m_VolumeControls=NULL;
	//Note refKBM is a osg ref pointer and does not require a delete call it will unreference itself automatically
}
//////////////////////////////////////////////////////////////////////////

// Called from our own thread, internally only
void Window::EnableMouse()
{
	PositionPointer(0.0f, 0.0f);
	m_Keyboard_Mouse->IgnoreMouseMotion = false;
}
//////////////////////////////////////////////////////////////////////////

// Needs to be called from Logic Thread, Should be OK
void Window::GetWindowRectangle(int& x, int& y, unsigned& w, unsigned& h)
{
	const osg::Viewport* vp = m_viewer->getCamera()->getViewport();
	//For dual setup we'll just get the first monitor (we may want to provide a sum area)
	if (!vp)
		vp=m_viewer->getSlave(0)._camera->getViewport();
	x = vp->x();
	y = vp->y();
	w = vp->width();
	h = vp->height();
}
//////////////////////////////////////////////////////////////////////////

GG_Framework::UI::OSG::ICamera* Window::GetMainCamera()
{
	return &m_mainCam;
}
//////////////////////////////////////////////////////////////////////////

// Must run on the OSG Thread
void Window::SetFullScreen(bool fs)
{
	DEBUG_SCREEN_RESIZE("Window::SetFullScreen(%s) - was %s\n", fs ? "true" : "false", m_isFullScreen ? "true" : "false");
	// No need to make changes
	if (m_isFullScreen == fs)
		return;

	// MAke sure we have a proper graphics window and windowing system interface
	osgViewer::GraphicsWindow *window = GetGraphicsWindow();
	if (!window)
		return;
	osg::GraphicsContext::WindowingSystemInterface    *wsi = osg::GraphicsContext::getWindowingSystemInterface();
	if (!wsi) 
		return;

	bool ignoringMouse = m_Keyboard_Mouse->IgnoreMouseMotion;
	m_Keyboard_Mouse->IgnoreMouseMotion = true;

	m_isFullScreen = fs;

	if (m_isFullScreen)
	{
		// Remember the last position and size of the window for when it is set back
		window->getWindowRectangle(m_lastX, m_lastY, m_lastWidth, m_lastHeight);

		// We need to get the original size of the screen, in case we want to set it back later
		wsi->getScreenResolution(*(window->getTraits()), m_origScreenWidth, m_origScreenHeight);

		// When in full screen mode, remove the title bar and borders
		window->setWindowDecoration(false);

		// When setting full screen, the user may want to resize the screen based on the parameters passed to the c'tor
		if ((m_newScreenHeight > 0) && (m_newScreenWidth > 0))
		{
			// Place the window properly first
			window->setWindowRectangle(0, 0, m_newScreenWidth, m_newScreenHeight);

			// Set the new screen height and width
			wsi->setScreenResolution(*(window->getTraits()), m_newScreenWidth, m_newScreenHeight);
		}
		else
		{
			// Otherwise we just set the the original screen size
			window->setWindowRectangle(0, 0, m_origScreenWidth, m_origScreenHeight);
		}
	}
	else // Windowed mode
	{
		// Make sure the screen is set back to its previous resolution
		if ((m_newScreenHeight > 0) && (m_newScreenWidth > 0))
		{
			// Set the new screen height and width
			wsi->setScreenResolution(*(window->getTraits()), m_origScreenWidth, m_origScreenHeight);
		}

		// When in windowed mode, use the title bar and borders
		window->setWindowDecoration(true);

		// Use the previous window sizes
		window->setWindowRectangle(m_lastX, m_lastY, m_lastWidth, m_lastHeight);
	}

	// Always make sure we have focus
	window->grabFocusIfPointerInWindow();

	// Place the mouse back at 0,0
	PositionPointer(0.0f, 0.0f);

	// Start listening to the mouse gain if we were before
	m_Keyboard_Mouse->IgnoreMouseMotion = ignoringMouse;
}
//////////////////////////////////////////////////////////////////////////

bool Window::IsFullScreen()
{
	return m_isFullScreen;
}
//////////////////////////////////////////////////////////////////////////

// 
osgViewer::GraphicsWindow* Window::GetGraphicsWindow()
{
	osgViewer::ViewerBase::Windows windows;
	m_viewer->getWindows(windows);
	if (windows.size() > 0)
		return (windows[0]);
	else
		return NULL;
}
//////////////////////////////////////////////////////////////////////////

void Window::SetWindowText(const char* windowTitle)
{
	ASSERT(windowTitle != NULL);
	osgViewer::GraphicsWindow *window = GetGraphicsWindow();
	if (window)
		window->setWindowName(windowTitle);
}
//////////////////////////////////////////////////////////////////////////

void Window::UseCursor( bool flag )
{
	osgViewer::GraphicsWindow *window = GetGraphicsWindow();
	if (window)
		window->useCursor(flag);
}
//////////////////////////////////////////////////////////////////////////

void Window::PositionPointer( float x, float y )
{
	float pixel_x, pixel_y;
	if (!ComputePixelCoords(x,y,pixel_x,pixel_y))
		return;
	m_viewer->requestWarpPointer(pixel_x,pixel_y);
}
//////////////////////////////////////////////////////////////////////////

// Needs to be called from the Logic Thread or the OSG Thread
bool Window::ComputePixelCoords(float x, float y, float& pixel_x, float& pixel_y)
{
	// Copied from KeyboardMouse::computePixelCoords() when there is not input area
	if (!m_viewer->isRealized())
		return false;
	if (x<-1.0f) return false;
	if (x>1.0f) return false;

	if (y<-1.0f) return false;
	if (y>1.0f) return false;

	float rx = (x+1.0f)*0.5f;
	float ry = (y+1.0f)*0.5f;

	int wx, wy;
	unsigned int w, h;
	GetWindowRectangle( wx, wy, w, h );

	pixel_x = (float)wx + ((float)w)* rx;
	pixel_y = (float)wy + ((float)h)* ry;

	return true;
}
//////////////////////////////////////////////////////////////////////////

// Only called from the OSG thread
void Window::Realize()
{
	//If the user has not explicitly set the OSG_SCREEN environment variable then
	//Lets explicitly work with only one monitor (TODO zero does not necessarily mean primary display, we should fix)
	if (getenv("OSG_SCREEN") == 0)
		putenv("OSG_SCREEN=0");
	// Make sure this only happens once
	if (!m_viewer->isRealized())
	{
		m_viewer->addEventHandler(m_Keyboard_Mouse);
		m_viewer->realize();

		// See if we can get a better FOV
		double hfov, aspect, nearClip, farClip;
		m_viewer->getCamera()->getProjectionMatrixAsPerspective(hfov, aspect, nearClip, farClip);
		double adjust = 1.5;
		m_viewer->getCamera()->setProjectionMatrixAsPerspective(hfov*adjust, aspect, nearClip, farClip);

		// Set as Windowed mode, with the default size
		osgViewer::GraphicsWindow *window = GetGraphicsWindow();
		window->setWindowDecoration(true);
		window->setWindowRectangle(m_lastX, m_lastY, m_lastWidth, m_lastHeight);
		m_isFullScreen = false;

		// Adjust the LOD scale based on the initial values
		int i = 0;
		for (i = 0; i < m_performanceIndex; ++i)
			m_viewer->getCamera()->setLODScale(m_viewer->getCamera()->getLODScale()/1.25f);
		for (i = 0; i > m_performanceIndex; --i)
			m_viewer->getCamera()->setLODScale(m_viewer->getCamera()->getLODScale()*1.25f);

		//Make the sound listener start WAY far away, rather than the middle of the scene
		AudioVector3f Position(1e6, 1e6, 1e6);
		AudioVector3f Velocity(0,0,0);
		AudioVector3f Forward(0,1,0);
		AudioVector3f Up(0,0,1);
		SOUNDSYSTEM.Set3DListenerAttributes(0,&Position,&Velocity,&Forward,&Up);

		// Call frame at time 0 to get it all initialized
		m_viewer->frame(0.0);

		// Position the pointer at 0 to get started
		PositionPointer(0.0f, 0.0f);

		PostRealize();
	}
}
//////////////////////////////////////////////////////////////////////////

// Called from the logic thread only
bool Window::Update(double currTime_s, double dTick_s)
{	
	// Handle input from the user
	m_Keyboard_Mouse->IncrementTime(dTick_s);
	m_Joystick->UpdateJoyStick(dTick_s);	//! < Update the Joystick too

	// Move the camera
	m_mainCam.Update(dTick_s);			//! < Then the Camera Matrix

	// Turn the mouse back on with the first pass through
	if (m_Keyboard_Mouse->IgnoreMouseMotion)
		EnableMouse();	// Enable the mouse the first time, do not mess with framerate

	if (!MainWindow::SINGLE_THREADED_MAIN_WINDOW)
	{
		// The keyboard is now updated in the frame() call, but if running multi-threaded, we need to process all the
		// events we already got from the OSG thread.
		m_Keyboard_Mouse->ProcessThreadedEvents();
		return true;
	}
	else
		return UpdateCamera(currTime_s, dTick_s);
}
//////////////////////////////////////////////////////////////////////////

// Only from the OSG Thread, either from Window::Update or from MainWindow::OsgThread::tryRun()
bool Window::UpdateCamera(double currTime_s, double dTick_s)
{
	if (m_viewer->done())
		return false;

	// Update the sound
	Audio::ISoundSystem::Instance().SystemFrameUpdate();
	UpdateSound(dTick_s);				//! < Then the sound that uses the camera matrix
	
	m_viewer->frame(currTime_s);
	return true;
}
//////////////////////////////////////////////////////////////////////////

// We may eventually replace all of this with the Viewer status indicator
// Only called from OSG Thread
void Window::UpdateFrameRate(double currTime_s, double lastDrawnFrameDur)
{
	// Set the first time
	if (m_lastFrameRateCheckTime_s < 0.0)
	{
		m_lastFrameRateCheckTime_s = currTime_s;
		return;
	}

	// Work out the frame-rate
	double dTime_s = currTime_s - m_lastFrameRateCheckTime_s;
	if (dTime_s > 1.0)	// Checks about every second
	{
		m_frameRate = (double)m_frameNum / dTime_s;
		m_frameRateAvg = m_framerateAverager.GetAverage(m_frameRate);	//!< Keeps a 30 second average
		m_lastFrameRateCheckTime_s = currTime_s;
		m_frameNum = 0;

		// Wait a few seconds before we make a performance update
		if (m_waitForPerformanceIndex_s > 0.0)
			m_waitForPerformanceIndex_s -= dTime_s;
		else if (lastDrawnFrameDur > 0.0)
		{
			// Make the performance index adjustments based ONLY on what the drawing does
			double perfFramerate = 1.0 / lastDrawnFrameDur;

			// Update the performance index, but only after we have been running a little bit
			if ((perfFramerate < PERFORMANCE_MIN) && (m_performanceIndex > -20))
			{
				// Watch for being low because we are throttling, no need to drop index for that
				if ((GetThrottleFPS() == 0.0) || (perfFramerate < (GetThrottleFPS()-1.0)))
				{
					// Wait until we have had three seconds in a row of bad frame rates
					if (m_performanceStrikes <= -3)
					{
						m_performanceStrikes = 0;
						--m_performanceIndex;
						PerformanceIndexChange.Fire(m_performanceIndex+1, m_performanceIndex);

						// Set the LOD scale a little lower
						if (m_performanceIndex < 1)
							m_viewer->getCamera()->setLODScale(m_viewer->getCamera()->getLODScale()*1.25f);
					}
					else if (m_performanceStrikes <= 0)
						--m_performanceStrikes;
					else
						m_performanceStrikes = 0;
				}
			}
			else if ((perfFramerate > PERFORMANCE_MAX) && (perfFramerate < 10))
			{
				// Wait until we have had two seconds in a row of good frame rates
				if (m_performanceStrikes >= 2)
				{
					m_performanceStrikes = 0;
					++m_performanceIndex;
					PerformanceIndexChange.Fire(m_performanceIndex-1, m_performanceIndex);

					// We can set the LOD scale to make things a little closer as well (only one time)
					if (m_performanceIndex < 2)
						m_viewer->getCamera()->setLODScale(m_viewer->getCamera()->getLODScale()/1.25f);
				}
				else if (m_performanceStrikes >= 0)
					++m_performanceStrikes;
				else
					m_performanceStrikes = 0;
			}
			else
				m_performanceStrikes = 0;
		}
	}

	++m_frameNum;
}
//////////////////////////////////////////////////////////////////////////

// Called from the Logic Thread
void Window::UpdateSound(double dTick_s)
{
	//update the vectors
	osg::Vec3 eye,center,up;

	// Note: we could support zoom in here by providing the lookDistance parameter
	// What does that mean for us?  -- Rick
	m_mainCam.GetCameraMatrix().getLookAt(eye,center,up);

	// The eye is really the center, and the center is the focal point where the eye is looking
	// By the comments, Set3DListenerAttributes() wants unit vectors.  Lets see how that effects things.
	// James, could you please take a look and see if this works as expected? -- Rick
	center = center-eye; center.normalize();
	up.normalize();
	AudioVector3f Position(eye),Forward(center),Up(up);
	AudioVector3f Velocity(m_velocityAvg.GetVectorDerivative(eye, dTick_s));

	//Keep this around... see if things are working
	//printf("x=%f,y=%f,z=%f	",Position.x,Position.y,Position.z);
	//printf("fx=%f,fy=%f,fz=%f   ux=%f,uy=%f,uz=%f\n",Forward.x,Forward.y,Forward.z,Up.x,Up.y,Up.z);

	//DebugOutput("Vector update x=%f,y=%f,z=%f   xv=%f,yv=%f,zv=%f\n",Position.x,Position.y,Position.z,Velocity.x,Velocity.y,Velocity.z);
	//DebugOutput("fx=%f,fy=%f,fz=%f   ux=%f,uy=%f,uz=%f\n",Forward.x,Forward.y,Forward.z,Up.x,Up.y,Up.z);
	SOUNDSYSTEM.Set3DListenerAttributes(0,&Position,&Velocity,&Forward,&Up);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MainWindow* MainWindow::s_mainWindow = NULL;

MainWindow::MainWindow(bool useAntiAlias, double throttle_fps, unsigned screenWidth, 
					   unsigned screenHeight, bool useUserPrefs, GG_Framework::Base::Timer& timer) : 
	Window(useAntiAlias, screenWidth, screenHeight, useUserPrefs), m_quitting(false), UpdateTimerLogger("OSG"),
	m_ThrottleFrames(throttle_fps) // Do we have a user defined throttle?
	,m_OsgThread(this, timer)
{
	ASSERT(!s_mainWindow);
	s_mainWindow = this;

	GetKeyboard_Mouse().AddKeyBinding(osgGA::GUIEventAdapter::KEY_Escape, "ESC", false);
	GetKeyboard_Mouse().GlobalEventMap.Event_Map["ESC"].Subscribe(ehl, *this, &MainWindow::OnEscape);

	GetKeyboard_Mouse().GlobalEventMap.Event_Map["ToggleFullScreen"].Subscribe(
		ehl, *this, &MainWindow::ToggleFullScreen);
}
//////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
	ASSERT(s_mainWindow == this);
	s_mainWindow = NULL;
}
//////////////////////////////////////////////////////////////////////////

// Called from the Logic Thread
bool MainWindow::Update(double currTime_s, double dTick_s)
{
	if (!m_quitting)
	{
		bool ret = Window::Update(currTime_s, dTick_s);

		// The throttle should be the LAST thing that happens, at the end of each frame
		double actFrameTime = m_ThrottleFrames();  // Wait a bit if we need to
		GG_Framework::UI::MainWindow::LOGIC_THREAD_FRAME_MS = (int)(actFrameTime * 1000.0);
		DOUT2("Logic: %i, AI: %i, OSG: %i", LOGIC_THREAD_FRAME_MS, AI_THREAD_FRAME_MS, OSG_THREAD_FRAME_MS);

		// TODO: Some aspects of this will need to move to the OSG Thread
		// For now, it will merely display the throttled framerate
		UpdateTimerLogger.End();
		UpdateFrameRate(currTime_s, UpdateTimerLogger.V);
		UpdateTimerLogger.Start();

		if (!SINGLE_THREADED_MAIN_WINDOW)
		{
			// These operations should be thread safe
			m_OsgThread.ReadyForUpdates();
			ret = (!m_viewer->done());
		}
		return ret;
	}
	else return false;
}
//////////////////////////////////////////////////////////////////////////

// Called from the logic thread
void MainWindow::TryClose()
{
	if (!m_quitting)
	{
		m_quitting = true;
		Closing.Fire(this, m_quitting);
		if (m_quitting)
		{
			// Fire the close message to let everything know
			if (SINGLE_THREADED_MAIN_WINDOW)
				m_viewer->setDone(true);
			else
			{
				m_OsgThread.Quit();
				//m_OsgThread.SignalEvent().signal();  //set the event to wake up the GUI thread
				ThreadSleep(1000);  //give it some time to close
			}
			Close.Fire(this);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void MainWindow::OnEscape()
{
	bool closeOnEsc = true;
	EscapeQuit.Fire(this, closeOnEsc);
	if (closeOnEsc)
		TryClose();
}
//////////////////////////////////////////////////////////////////////////

void MainWindow::Realize()
{
	if (SINGLE_THREADED_MAIN_WINDOW)
	{
		__super::Realize();
		UpdateTimerLogger.Start();
	}
	else
	{
		m_OsgThread.start();

		// Wait till the realize is done
		while (!IsRealized())
			GG_Framework::Base::ThreadSleep(500);
	}
	InitializeThrottle();
}
//////////////////////////////////////////////////////////////////////////

double MainWindow::GetThrottleFPS_OSG()
{
	if (SINGLE_THREADED_MAIN_WINDOW)
		return GetThrottleFPS();
	else
		return m_OsgThread.GetThrottleFPS();
}
void MainWindow::SetThrottleFPS_OSG(double throttleFPS)
{
	if (SINGLE_THREADED_MAIN_WINDOW)
		return SetThrottleFPS(throttleFPS);
	else
		m_OsgThread.SetThrottleFPS(throttleFPS);
}

void MainWindow::OsgThread::tryRun()
{
	// We handle the realize now in our thread
	m_pParent->Window::Realize();

	// Make sure we are ready to do all updates
	while (!m_readyForUpdates)
		GG_Framework::Base::ThreadSleep(500);

	// James:  I think you might be right,  I set the throttle at 0 and occasionally saw frames less than
	// 16 ms (though not often). We can place the throttle here.
	// Also, when I did let it just free-wheel without ANY sleeps, I could see that it was starving out
	// other threads.

	// TODO: USe the actual refresh rate of the monitor.  Can we get this from OSG?
	m_ThrottleFrames.SetThrottleFPS(OSG_RefreshFPS);
	
	do
	{
		double dTime_s = m_timer.FireTimer();
		double currTime_s = m_timer.GetCurrTime_s();

		m_pParent->Window::SetFullScreen(m_fullScreen);
		m_pParent->Window::UseCursor(m_useCursor);
		MousePos mp = mousePos;
		if (mp.setNextFrame)
		{
			m_pParent->Window::PositionPointer(mp.X, mp.Y);
			mousePos = MainWindow::OsgThread::MousePos(); // Default ctor does not set the mouse
		}
		m_pParent->UpdateCamera(currTime_s, dTime_s);

		// The throttle should be the LAST thing that happens, at the end of each frame
		double actFrameTime = m_ThrottleFrames();  // Wait a bit if we need to
		GG_Framework::UI::MainWindow::OSG_THREAD_FRAME_MS = (int)(actFrameTime * 1000.0);
	}
	while (!m_pParent->m_viewer->done() && !m_quit);

	printf("Exiting OSG Thread\n");
}

MainWindow::OsgThread::OsgThread(MainWindow *Parent, GG_Framework::Base::Timer& timer) :  
m_pParent(Parent),m_fullScreen(false),m_useCursor(true),m_quit(false),m_readyForUpdates(false),
m_timer(timer)
{
}

//////////////////////////////////////////////////////////////////////////

void MainWindow::InitializeThrottle()
{
	// Lets play with a very low throttle when multi-threading
	m_ThrottleFrames.SetThrottleFPS(SINGLE_THREADED_MAIN_WINDOW ? OSG_RefreshFPS : 100.0);
	m_ThrottleFrames.SetMinSleep(1);
}
//////////////////////////////////////////////////////////////////////////

void MainWindow::ToggleFullScreen()
{
	SetFullScreen(!IsFullScreen());
}
//////////////////////////////////////////////////////////////////////////

void MainWindow::SetFullScreen(bool fs)
{
	if (SINGLE_THREADED_MAIN_WINDOW)
		Window::SetFullScreen(fs);
	else
		m_OsgThread.SetFullScreen(fs);
}

// Work with the Cursor, we will eventually be able to manipulate the cursor itself
void MainWindow::UseCursor( bool flag )
{
	if (SINGLE_THREADED_MAIN_WINDOW)
		Window::UseCursor(flag);
	else
		m_OsgThread.UseCursor(flag);
}

// Position the pointer explicitly
void MainWindow::PositionPointer( float x, float y )
{
	if (SINGLE_THREADED_MAIN_WINDOW)
		Window::PositionPointer(x,y);
	else
		m_OsgThread.mousePos = OsgThread::MousePos(x,y);
}
//////////////////////////////////////////////////////////////////////////