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
			Window(bool useAntiAlias, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs);
			virtual ~Window();

			bool IsFullScreen();
			bool IsAntiAliased(){return m_useAntiAlias;}
			virtual void Realize();
			virtual void PostRealize(){m_realizeComplete=true;}
			bool IsRealized(){return m_realizeComplete;}
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
			virtual void UseCursor( bool flag );

			// Position the pointer explicitly
			virtual void PositionPointer( float x, float y );

			/** compute, from normalized mouse coords (x,y) the,  for the specified 
			* RenderSurface, the pixel coordinates (pixel_x,pixel_y). return true 
			* if pixel_x and pixel_y have been successful computed, otherwise return 
			* false with pixel_x and pixel_y left unchanged.*/
			bool ComputePixelCoords(float x, float y, float& pixel_x, float& pixel_y);

			KeyboardMouse_CB &GetKeyboard_Mouse() const { return *m_Keyboard_Mouse; }
			JoyStick_Binder &GetJoystick() const { return *m_Joystick; }


		// Needs to be called from Logic Thread
			void GetWindowRectangle(int& x, int& y, unsigned& w, unsigned& h);

		// This version needs the OSG thread, but MainWindow will override it
			virtual void SetFullScreen(bool fs);

		protected:
			osgViewer::GraphicsWindow* GetGraphicsWindow();
			void SetWindowText(const char* windowTitle);
			bool m_useAntiAlias;
			osg::ref_ptr<osgViewer::Viewer>	m_viewer;
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
			bool m_realizeComplete;

			bool UpdateCamera(double currTime_s, double dTick_s);
			void UpdateSound(double dTick_s);
			GG_Framework::UI::OSG::VectorDerivativeOverTimeAverager m_velocityAvg;

		protected:
		// Called internally from our thread
			// Call this just as we are starting the main loop to enable the camera and get one more frame in
			void EnableMouse();

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
			static bool SINGLE_THREADED_MAIN_WINDOW;
			static int LOGIC_THREAD_FRAME_MS;
			static int AI_THREAD_FRAME_MS;
			static int OSG_THREAD_FRAME_MS;

		public:
			MainWindow(bool useAntiAlias, double throttle_fps, unsigned screenWidth, unsigned screenHeight, bool useUserPrefs,
				GG_Framework::Base::Timer& timer);
			virtual bool Update(double currTime_s, double dTick_s);
			virtual ~MainWindow();
			virtual void Realize();
			bool IsQuitting(){return m_quitting;}
			void TryClose();

			Event2<MainWindow*, bool&> Closing;
			Event2<MainWindow*, bool&> EscapeQuit;
			Event1<MainWindow*> Close;

			// Here is the singleton
			static MainWindow* GetMainWindow(){return s_mainWindow;}

			// This is public so we can tie events to it
			IEvent::HandlerList ehl;

			double GetThrottleFPS(){return m_ThrottleFrames.GetThrottleFPS();}
			void SetThrottleFPS(double throttleFPS){m_ThrottleFrames.SetThrottleFPS(throttleFPS);}

			double GetThrottleFPS_OSG();
			void SetThrottleFPS_OSG(double throttleFPS);

			GG_Framework::Base::ProfilingLogger UpdateTimerLogger;

			// Must be on the Logic Thread
			void ToggleFullScreen();

			// Handles the thread safety, if Multi-threaded
			virtual void SetFullScreen(bool fs);

			// Work with the Cursor, we will eventually be able to manipulate the cursor itself
			virtual void UseCursor( bool flag );

			// Position the pointer explicitly
			virtual void PositionPointer( float x, float y );


		private:
			void InitializeThrottle();
			void OnEscape();
			static MainWindow* s_mainWindow;
			bool m_quitting;
			GG_Framework::Base::ThrottleFrame m_ThrottleFrames;

			class FRAMEWORK_UI_API OsgThread : public GG_Framework::Base::ThreadedClass
			{
				public:
					OsgThread(MainWindow *Parent, GG_Framework::Base::Timer& timer);
					void SetFullScreen(bool fullScreen){m_fullScreen=fullScreen;}
					void UseCursor( bool flag ){m_useCursor = flag;}
					void Quit(){m_quit = true;}
					void ReadyForUpdates(){m_readyForUpdates=true;}

					virtual double GetThrottleFPS(){return m_ThrottleFrames.GetThrottleFPS();}
					void SetThrottleFPS(double throttleFPS){m_ThrottleFrames.SetThrottleFPS(throttleFPS);}

					struct MousePos
					{
						float X,Y;
						bool setNextFrame;
						MousePos() : setNextFrame(false) {}
						MousePos(float x, float y) : X(x), Y(y), setNextFrame(true) {}
					};
					GG_Framework::Base::AtomicT<MousePos> mousePos;

				protected:
					void tryRun();
				
				private:
					MainWindow * const m_pParent;
					GG_Framework::Base::Timer& m_timer;
					GG_Framework::Base::ThrottleFrame m_ThrottleFrames;
					bool m_fullScreen, m_useCursor, m_quit, m_readyForUpdates; // These should all be atomic
			} m_OsgThread;
		};
	}
}
