// MainWindow.h
#pragma once
#include <osgViewer/Viewer>

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API Window
		{
		public:
			// Use this block for all threading updates
			static OpenThreads::Mutex UpdateMutex;
			class FRAMEWORK_UI_API ThreadSafeViewer : public osgViewer::Viewer
			{
			public:
				Event0 PostUpdateEvent;
				/*
				virtual void updateTraversal()
				{
					// Just wrap the update traversal in a mutex
					GG_Framework::Base::RefMutexWrapper rmw(Window::UpdateMutex);
					__super::updateTraversal();
					PostUpdateEvent.Fire();
				}
				*/
			};

			Window(bool useAntiAlias, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs);
			virtual ~Window();

			void SetWindowRectangle(int x, int y, int w, int h, bool resize);
			void GetWindowRectangle(int& x, int& y, unsigned& w, unsigned& h);
			void SetFullScreen(bool fs);
			bool IsFullScreen();
			void SetWindowText(const char* windowTitle);
			bool IsAntiAliased(){return m_useAntiAlias;}
			virtual void Realize();
			virtual bool Update(double currTime_s, double dTick_s);
			virtual double GetThrottleFPS(){return 0.0;}

			double GetFrameRate(){return m_frameRate;}
			double GetAverageFramerate(){return m_frameRateAvg;}
			int GetPerformanceIndex(){return m_performanceIndex;}
			Event2<int, int> PerformanceIndexChange; //! provides old and new values, smaller means struggling
			GG_Framework::UI::OSG::ICamera* GetMainCamera();

			// Used for performance indexing
			static double PERFORMANCE_MIN;
			static double PERFORMANCE_MAX;
			static int PERFORMANCE_INIT_INDEX;

			// Work with the Cursor, we will eventually be able to manipulate the cursor itself
			void UseCursor( bool flag );

			// Call this just as we are starting the main loop to enable the camera and get one more frame in
			void EnableMouse();

			// Position the pointer explicitly
			void PositionPointer( float x, float y );

			/** compute, from normalized mouse coords (x,y) the,  for the specified 
			* RenderSurface, the pixel coordinates (pixel_x,pixel_y). return true 
			* if pixel_x and pixel_y have been successful computed, otherwise return 
			* false with pixel_x and pixel_y left unchanged.*/
			bool ComputePixelCoords(float x, float y, float& pixel_x, float& pixel_y);

			ThreadSafeViewer* GetViewer(){return m_camGroup.get();}

			KeyboardMouse_CB &GetKeyboard_Mouse() const { return *m_Keyboard_Mouse; }
			JoyStick_Binder &GetJoystick() const { return *m_Joystick; }

		protected:
			osgViewer::GraphicsWindow* GetGraphicsWindow();
			bool m_useAntiAlias;
			osg::ref_ptr<ThreadSafeViewer>	m_camGroup;
			bool m_isFullScreen;
			unsigned m_origScreenWidth, m_origScreenHeight;
			unsigned m_newScreenWidth, m_newScreenHeight;
			int m_lastX, m_lastY;
			int m_lastWidth, m_lastHeight;

			Camera m_mainCam;

			// Used for calculating frame-rate
			void UpdateFrameRate(double currTime_s, double lastDrawnFrameDur);
			double m_frameRate;
			double m_frameRateAvg;
			Averager<double, 30> m_framerateAverager;
			double m_lastFrameRateCheckTime_s;
			int m_frameNum;
			int m_performanceIndex;
			double m_waitForPerformanceIndex_s;
			int m_performanceStrikes;

			bool UpdateCameraGroup(double currTime_s);
			void UpdateSound(double dTick_s);
			GG_Framework::UI::OSG::VectorDerivativeOverTimeAverager m_velocityAvg;

		private:
			// Attach to events with this, also change where the callbacks go
			osg::ref_ptr<KeyboardMouse_CB> refKBM;
			KeyboardMouse_CB *m_Keyboard_Mouse;
			JoyStick_Binder *m_Joystick;					// Scoped pointer.
			AudioVolumeControls_PlusBLS *m_VolumeControls;	// Scoped pointer.

			ConfigurationManager m_ConfigurationManager;

		};


		class FRAMEWORK_UI_API MainWindow : public Window
		{
		public:
			MainWindow(bool useAntiAlias, double throttle_fps, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs);
			virtual bool Update(double currTime_s, double dTick_s);
			virtual ~MainWindow();
			virtual void Realize();
			bool IsQuitting(){return m_quitting;}
			void ToggleFullScreen(){SetFullScreen(!IsFullScreen());}
			void TryClose();

			Event2<MainWindow*, bool&> Closing;
			Event2<MainWindow*, bool&> EscapeQuit;
			Event1<MainWindow*> Close;

			// Here is the singleton
			static MainWindow* GetMainWindow(){return s_mainWindow;}

			// This is public so we can tie events to it
			IEvent::HandlerList ehl;

			virtual double GetThrottleFPS(){return m_throttleFPS;}
			void SetThrottleFPS(double throttleFPS);

			GG_Framework::Base::ProfilingLogger UpdateTimerLogger;

		private:
			void OnEscape();
			static MainWindow* s_mainWindow;
			bool m_quitting;
			osg::Timer m_throttleTimer;
			double m_throttleFPS;

			void ThrottleFrame();

#ifndef __UseSingleThreadMainLoop__
			class FRAMEWORK_UI_API GUIThread : public GG_Framework::Base::ThreadedClass
			{
				public:
					GUIThread(MainWindow *Parent) : m_pParent(Parent),m_currTime_s(0.0) {start();}
					~GUIThread() {cancel();}
					//Set and Get the Signal Event
					OpenThreads::Condition &SignalEvent() {return m_SignalEvent;}
					//Set and Get the current time
					double &currTime_s() {return m_currTime_s;}
				protected:
					void tryRun() 
					{
						//Rick, we may want to remove this code and let the scene start immediately and then we can show a progress bar
						//by using osg itself
#if 1
						//wait until we get an initial signal before running the scene
						m_BlockSignalEvent.lock();
						//hmmm I usually do not like to wait indefinitely but it seems to be the right thing
						m_SignalEvent.wait(&m_BlockSignalEvent);
						m_BlockSignalEvent.unlock();
#endif
						//start the initial call to set the done status
						m_pParent->UpdateCameraGroup(m_currTime_s);
						while (!m_pParent->m_camGroup->done())
						{
							//There is s reversed scope mutex inside the wait... so we have to lock unlock around it
							m_BlockSignalEvent.lock();
							m_SignalEvent.wait(&m_BlockSignalEvent,1000);
							m_BlockSignalEvent.unlock();
							m_pParent->UpdateCameraGroup(m_currTime_s);
						}
						printf("Exiting GUI Thread\n");
					}
				private:
					MainWindow * const m_pParent;
					OpenThreads::Condition m_SignalEvent;
					OpenThreads::Mutex m_BlockSignalEvent; //used internally inside the signal event to wait
					double m_currTime_s;
			} m_GUIThread;
#endif
		};
	}
}
