#pragma once

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/api/win32/GraphicsWindowWin32>
#include <osgGA/TrackballManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgDB/DatabasePager>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <string>

class OSG_HWND
{
public:
	OSG_HWND(HWND hWnd);
	~OSG_HWND();

	void InitOSG(std::string filename, std::string contentRegKey);
	void InitManipulators(void);
	void InitSceneGraph(void);
	void InitCameraConfig(void);
	void SetupWindow(void);
	void SetupCamera(void);
	void PreFrameUpdate(void);
	void PostFrameUpdate(void);
	void Done(bool value) { mDone = value; }
	void SetBackgroundColor(unsigned char r, unsigned char g, unsigned char b); 
	bool Done(void) { return mDone; }
	static void Render(void* ptr);

	osgViewer::Viewer* mViewer;

	// For Testing, something to fire off in an event
	long CurrFrameNum;

private:
	bool mDone;
	std::string m_modelName, m_contentRegKey;
	HWND m_hWnd;
	
	osg::ref_ptr<osg::Group> mRoot;
	osg::ref_ptr<osg::Node> mModel;
	osg::ref_ptr<osg::Camera> m_camera;
	osg::ref_ptr<osgGA::TrackballManipulator> trackball;
	osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator;
	osg::Vec4 m_bkgdColor;
};
