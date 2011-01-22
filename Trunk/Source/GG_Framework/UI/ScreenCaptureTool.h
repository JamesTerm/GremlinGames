// GG_Framework.UI ScreenCaptureTool.h
#pragma once

#include "MainWindow.h"
#include <osg/Camera>

namespace GG_Framework
{
	namespace UI
	{

/// This class can be tied to a kb combo to start capturing frames
class FRAMEWORK_UI_API ScreenCaptureTool
{
public:
	ScreenCaptureTool(MainWindow& mainWindow, Event0& toggleEvent);

	bool IsRecording();
	void ToggleRecording();
	bool StartRecording();
	bool StopRecording();

private:
	IEvent::HandlerList ehl;

	class Recorder : public osg::Camera::DrawCallback, public GG_Framework::Base::ThreadedClass
	{
	public:
		Recorder(MainWindow& mainWindow);
		virtual void operator () (osg::RenderInfo& renderInfo) const;
		void StopRecording();

	protected:	
		virtual void tryRun();
		virtual ~Recorder();
	
	private:
		MainWindow& m_mainWindow;
		double m_prevThrottle_fps;
		std::string m_folderName;
		unsigned m_imageIndex;
		int X,Y;
		unsigned W,H;
		mutable int m_recording; // -1 for thread not started, 1 for thread started, 0 (false) for ready to stop
		mutable OpenThreads::Mutex m_mutex;
		osg::ref_ptr<osg::Camera::DrawCallback> m_selfRef;	// a self-reference to make sure we are not deleted

		// Lists of images so we do not need to make them
		mutable std::list<osg::ref_ptr<osg::Image>*> m_readyImageList;
		mutable std::list<osg::ref_ptr<osg::Image>*> m_recycleImageList;
	};

	osg::ref_ptr<Recorder> m_currRecorder;
	MainWindow& m_mainWindow;
	bool m_enabled;

	// Called when the MainWindow closes, stop recording
	void OnClose(MainWindow* mw);
};

	}
}

/*
// GG_Framework.UI ScreenCaptureTool.h
#pragma once

#include "MainWindow.h"
#include <osg/Camera>

namespace GG_Framework
{
	namespace UI
	{

/// This class can be tied to a kb combo to start capturing frames
class FRAMEWORK_UI_API ScreenCaptureTool
{
public:
	ScreenCaptureTool(MainWindow& mainWindow, Event0& toggleEvent);

	bool IsRecording();
	void ToggleRecording();
	bool StartRecording();
	bool StopRecording(bool writeAVI);

private:
	IEvent::HandlerList ehl;

	class Recorder : public osg::Referenced, public GG_Framework::Base::ThreadedClass
	{
	public:
		Recorder(MainWindow& mainWindow);
		void OnPostUpdate();
		// virtual void operator () (osg::RenderInfo& renderInfo) const;
		void StopRecording(bool writeAVI);

	protected:	
		virtual void tryRun();
		virtual ~Recorder();
	
	private:
		IEvent::HandlerList ehl;
		MainWindow& m_mainWindow;
		double m_prevThrottle_fps;
		int X,Y;
		unsigned W,H;

		osg::ref_ptr<Recorder> m_selfRef;	// a self-reference to make sure we are not deleted

		// Lists of images so we do not need to make them
		mutable std::list<GG_Framework::Base::I_AVI_Image*> m_readyImageList;
	};

	osg::ref_ptr<Recorder> m_currRecorder;
	MainWindow& m_mainWindow;
	bool m_enabled;

	// Called when the MainWindow closes, stop recording
	void OnClose(MainWindow* mw);
};

	}
}
*/