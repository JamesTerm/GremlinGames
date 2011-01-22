// GG_Framework.UI SceneRunner.h
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;

ISceneRunner::ISceneRunner() : globalEventMap(&kbEvents), m_quitting(false)
{
	globalEventMap.GetKeyBinding("ESC").Key=Producer::KeyChar_Escape;
	globalEventMap.Event_Map["ESC"].Subscribe(ehl, *this, &ISceneRunner::OnEscape);
}
//////////////////////////////////////////////////////////////////////////

void ISceneRunner::OnEscape()
{
	m_quitting = true;
	Quit.Fire(m_quitting);	// If this returns with m_quitting still true, the loop will end
}
//////////////////////////////////////////////////////////////////////////

void ISceneRunner::RunScene(osg::Node* rootNode, osg::Node* mainGroup,
							OSG::ICamera& cam, OSG::IKeyboardMouse& kbm)
{
	// Realize the Camera here, might take a couple of seconds
	// If already realized, will just reset the scene
	cam.realize(rootNode);
	cam.SetWindowText("The Fringe");

	// Fire START on the event map(s)
	FireStart();

	// Have a frame stamp to run the non-Actor parts
	osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
	osgUtil::UpdateVisitor update;
	update.setFrameStamp(frameStamp.get());

	// TEMP, Debugging, Set the initial camera position
	osg::Matrix camMatrix;
	camMatrix.makeLookAt(osg::Vec3(0,0,0), osg::Vec3(0,1,0), osg::Vec3(0,0,1));

	// Loop while we are waiting for the Connection to be made and all of the scenes
	while(LoopUpdate(cam) && !m_quitting)
	{
		double dTime_s = GetTimerEvent().FireTimer();
		double currTime = GetTimerEvent().GetCurrTime_s();
		frameStamp->setReferenceTime(currTime);
		frameStamp->setFrameNumber(TIME_2_FRAME(currTime));
		rootNode->accept(update);
		Audio::ISoundSystem::Instance().SystemFrameUpdate();
	}
}
//////////////////////////////////////////////////////////////////////////

bool ISceneRunner::LoopUpdate(OSG::ICamera& cam)
{
	// TODO: Do we want this here?
	GG_Framework::Base::ThreadSleep(1);
	return cam.Update();
}
//////////////////////////////////////////////////////////////////////////

void ISceneRunner::FireStart()
{
	globalEventMap.Event_Map["START"].Fire();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//! This versions sets the window title to be the Viewer's VPR
bool OSG_Timer_SceneRunner::LoopUpdate(OSG::ICamera& cam)
{
	// TODO: Set the 
	cam.SetWindowText("TODO: Place Actor Title Here");
	return ISceneRunner::LoopUpdate(cam);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
