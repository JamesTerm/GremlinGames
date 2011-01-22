// PostDrawCallback.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

class Viewer;

class FRAMEWORK_UI_API PostDrawCallback : public Producer::Camera::Callback
{
public:
	enum EnableMask{
		EnableNone = 0,
		DisplayDist = 1,
		DisplayHelp=2,
		DisplayAnim=4,
		DisplayFrameRate=8,
		DisplayCenterRef=16,
	};

	PostDrawCallback(Viewer& viewer, GG_Framework::Base::Timer& timer) : m_viewer(viewer), m_timer(timer)
	{
		_enableMask = DisplayDist|DisplayAnim|DisplayFrameRate|DisplayCenterRef;
	}

	void operator () (const Producer::Camera &cam );

	void setmasks(unsigned int em){_enableMask = em;}
	unsigned int getmasks(){return _enableMask;}
	void enable( EnableMask item ) { _enableMask |= (unsigned int)(item); }
	void disable(EnableMask item)  { _enableMask &= ~((unsigned int)(item)); }
	bool isEnabled(EnableMask item) { return (_enableMask & item) != 0 ? true: false; }

private:
	unsigned int _enableMask;
	GG_Framework::UI::OSG::Text m_text;
	Viewer& m_viewer;
	GG_Framework::Base::Timer& m_timer;
};

	}
}