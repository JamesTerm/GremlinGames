// GG_Framework.UI ScreenCaptureTool.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

std::string DIRECTORY_NAME_PREFIX("ScreenCaptures_");
std::string FILE_NAME_PREFIX("Image_");
std::string EXTENSION("bmp");

using namespace GG_Framework::UI;

ScreenCaptureTool::ScreenCaptureTool(MainWindow& mainWindow, Event0& toggleEvent) : m_mainWindow(mainWindow), m_enabled(true)
{
	// Listen for the toggle event
	toggleEvent.Subscribe(ehl, *this, &ScreenCaptureTool::ToggleRecording);

	// Watch for the window closing
	m_mainWindow.Close.Subscribe(ehl, *this, &ScreenCaptureTool::OnClose);
}
///////////////////////////////////////////////////////////////////////

void ScreenCaptureTool::OnClose(MainWindow* mw)
{
	// Stop any existing recording and make sure no other one starts
	StopRecording(); 
	m_enabled = false;
}
///////////////////////////////////////////////////////////////////////

bool ScreenCaptureTool::IsRecording()
{
	return m_currRecorder.valid();
}
///////////////////////////////////////////////////////////////////////

void ScreenCaptureTool::ToggleRecording()
{
	if (IsRecording())
		StopRecording();
	else
		StartRecording();
}
///////////////////////////////////////////////////////////////////////

bool ScreenCaptureTool::StartRecording()
{

	// Do not try to record if not enabled or already recording
	if (IsRecording() || !m_enabled)
		return false;

	// Create a new recorder that is tied to the camera
	m_currRecorder = new ScreenCaptureTool::Recorder(m_mainWindow);

	// All is well
	return true;
}
///////////////////////////////////////////////////////////////////////

bool ScreenCaptureTool::StopRecording()
{
	if (!IsRecording())
		return false;

	// Tell the current recorder to stop recording frames
	m_currRecorder->StopRecording();

	// No longer hold a reference to it, it will be deleted when it is finished
	m_currRecorder = NULL;

	// All is well
	return true;
}
///////////////////////////////////////////////////////////////////////

ScreenCaptureTool::Recorder::Recorder(MainWindow& mainWindow) : 
	m_mainWindow(mainWindow), m_prevThrottle_fps(mainWindow.GetThrottleFPS_OSG()), m_imageIndex(0), m_recording(-1)
{
	// makes the folder to write to using a timer for a unique name
	unsigned folderIndex = osg::Timer::instance()->tick();
	m_folderName = GG_Framework::Base::BuildString("%s%012i", DIRECTORY_NAME_PREFIX.c_str(), folderIndex);
	osg::notify(osg::NOTICE) << "ScreenCaptureTool creating directory \"" << m_folderName << "\"\n";
	if (!osgDB::makeDirectory(m_folderName))
		osg::notify(osg::FATAL) << "ERROR: ScreenCaptureTool problem creating directory \"" << m_folderName << "\"\n";

	// Adds itself as a final post-draw callback
	mainWindow.GetMainCamera()->setFinalDrawCallback(this);

	// grabs the current screen size and position.
	m_mainWindow.GetWindowRectangle(X, Y, W, H);

	// Maintains a reference to itself so when it is done it can delete itself
	m_selfRef = this;

	// Makes sure the throttle is set to 30 fps (but remembers the previous throttle in the member init list)
	m_mainWindow.SetThrottleFPS_OSG(30.0);

	// We do not want a high priority, so we do not kill the processor
	this->setSchedulePriority(OpenThreads::Thread::THREAD_PRIORITY_LOW);

	// Print notification to the console
	printf("Recording Started\n");
}
///////////////////////////////////////////////////////////////////////

void ScreenCaptureTool::Recorder::tryRun()
{
	bool recording = true;
	osg::ref_ptr<osg::Image>* image = NULL;

	while (recording || image)
	{
		// Sleep to make sure we do not starve the other thread
		GG_Framework::Base::ThreadSleep(0);

		if (image)
		{
			// We can write this image out to a file
			std::string filename = GG_Framework::Base::BuildString("%s\\%s%04i.%s", m_folderName.c_str(), FILE_NAME_PREFIX.c_str(), m_imageIndex++, EXTENSION.c_str());
			osg::notify(osg::NOTICE) << "ScreenCaptureTool writing image file \"" << filename << "\"\n";
			if (!osgDB::writeImageFile(*((*image).get()), filename))
				osg::notify(osg::FATAL) << "ERROR: ScreenCaptureTool problem writing image file \"" << filename << "\"\n";
		}

		{ // This part needs to be mutex'd
			GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
			recording = (m_recording == 1);
			if (image)
			{
				// Place it at the head of the recycle list
				m_recycleImageList.push_back(image);
				image = NULL;
			}
			if (!m_readyImageList.empty())
			{
				// Get the next one off the ready list if there is one
				image = m_readyImageList.front();
				m_readyImageList.pop_front();
			}
		}
	}

	// Print to the console to let us know recording is complete
	printf("Recording Complete, %i images written\n", m_imageIndex);

	// When we are all done, we can delete ourselves by killing the last remaining reference
	m_selfRef = NULL;
}
///////////////////////////////////////////////////////////////////////

void ScreenCaptureTool::Recorder::operator () (osg::RenderInfo& renderInfo) const
{
	osg::ref_ptr<osg::Image>* image = NULL;
	{ // Needs a mutex
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
		if (!m_recycleImageList.empty())
		{
			// Get the next one off the ready list if there is one
			image = m_recycleImageList.front();
			m_recycleImageList.pop_front();
		}
	}

	if (!image)
	{
		// Make a new one if there was not one on the list
		image = new osg::ref_ptr<osg::Image>;
		(*image) = new osg::Image();
	}

	// Grab the image off the frame
	(*image)->readPixels(X,Y,W,H,
					GL_RGB,GL_UNSIGNED_BYTE);

	// Store the image on the ready list to be written
	{ // Needs a mutex
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
		m_readyImageList.push_back(image);

		// If not already running, start now
		if (m_recording == -1)
		{
			m_recording = 1;
			const_cast<ScreenCaptureTool::Recorder*>(this)->start(); // Since the virtual function is const [I hate const ;( ]
		}
	}
}
///////////////////////////////////////////////////////////////////////

void ScreenCaptureTool::Recorder::StopRecording()
{
	// Stop the recording (boolean that lets the writing thread know it can abort when it is done with frames
	{ // Needs a mutex
		GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
		m_recording = 0;
	}

	// Remove self from the main window final callback
	m_mainWindow.GetMainCamera()->setFinalDrawCallback(NULL);

	// Set the throttle speed back on the main camera
	m_mainWindow.SetThrottleFPS_OSG(m_prevThrottle_fps);

	// Print notification to the console
	printf("Recording Stopped\n");
}
///////////////////////////////////////////////////////////////////////

ScreenCaptureTool::Recorder::~Recorder()
{
	// Loop through all of the images in both lists to delete
	std::list<osg::ref_ptr<osg::Image>*>::iterator pos;
	for (pos = m_readyImageList.begin(); pos != m_readyImageList.end(); ++pos)
	{
		osg::ref_ptr<osg::Image>* image = *pos;
		delete image;
	}
	for (pos = m_recycleImageList.begin(); pos != m_recycleImageList.end(); ++pos)
	{
		osg::ref_ptr<osg::Image>* image = *pos;
		delete image;
	}
}
///////////////////////////////////////////////////////////////////////
