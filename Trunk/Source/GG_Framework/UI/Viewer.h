// Viewer.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

class KeyboardMouseCallback;

class FRAMEWORK_UI_API Viewer
{
public:
	Viewer(GG_Framework::Base::EventMap& globalMap, GG_Framework::Base::Timer& timer,
		GG_Framework::UI::OSG::IKeyboardMouse& kbm, GG_Framework::UI::OSG::ICamera& camera,
		GG_Framework::UI::ViewPortReference& vpRef, GG_Framework::Base::KeyboardEvents& kbEvents);

	//! The change in time is just used to calculate the frame rate
	bool UpdateMe(double dTick_s);

	// Setting this param true makes the viewer die on the next pass
	bool m_killMe;

	const char* GetWindowTitle(){return m_VP_ActorTitle.c_str();}
	GG_Framework::UI::OSG::ICamera* GetCamera(){return m_camera;}

	GG_Framework::UI::OSG::TrackBall&		GetTrackBall(){return *(m_trackBall.get());}
	PostDrawCallback&	GetPDCB(){return *(m_postDrawCB.get());}

	double GetFrameRate(){return m_frameRate;}

	KeyboardMouseCallback* GetKBMCB(){return m_kmcb.get();}

	PostDrawCallback* GetPostDrawCallback(){return m_postDrawCB.get();}

	// Get the current ViewPortReference.  Set it through the KBMC
	GG_Framework::UI::ViewPortReference* GetViewPortReference(){return m_vpRef;}

	void CleanUp();
	GG_Framework::UI::OSG::SnapImageDrawCallback* GetSnapCallback(){return m_snapImageCallback.get();}

	const char* Get_VP_ActorTitle(){return m_VP_ActorTitle.c_str();}

protected:

	osg::ref_ptr<GG_Framework::UI::OSG::TrackBall>		m_trackBall;
	osg::ref_ptr<KeyboardMouseCallback>		m_kmcb;
	osg::ref_ptr<PostDrawCallback>			m_postDrawCB;
	osg::ref_ptr<GG_Framework::UI::OSG::SnapImageDrawCallback> m_snapImageCallback;

	virtual void UpdateViewpointActorTitle(bool forceUpdate);
	std::string m_VP_ActorTitle;
	int m_lastTitleUpdate;

	enum TitleUpdate_Flags {
		TitleUpdate_Hidden = 16,
		TitleUpdate_NonShaded = 32,
		TitleUpdate_Shaded = 64,
		TitleUpdate_Inherit = 128,
	};

	// Used for calculating frame-rate
	double m_frameRate;
	double m_lastFrameTime;
	DWORD m_frameNum;

	GG_Framework::UI::ViewPortReference*	m_vpRef;

	void QUIT_EventHandler();

private:
	IEvent::HandlerList ehl;
	GG_Framework::UI::OSG::ICamera* const m_camera;
	GG_Framework::UI::OSG::IKeyboardMouse* const m_kbm;
};

	}
}
