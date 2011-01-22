// Viewer.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::UI::OSG;

Viewer::Viewer(
	GG_Framework::Base::EventMap& globalMap, GG_Framework::Base::Timer& timer,
	GG_Framework::UI::OSG::IKeyboardMouse& kbm, GG_Framework::UI::OSG::ICamera& camera, ViewPortReference& vpRef,
	GG_Framework::Base::KeyboardEvents& kbEvents) : 
	m_vpRef(NULL), m_killMe(false), m_lastTitleUpdate(-1), m_camera(&camera), m_kbm(&kbm)
{
	// Set up the trackball control
	m_trackBall = new TrackBall;

	// Create the Post Draw Callback that will handle the help screen and other printing
	m_postDrawCB = new PostDrawCallback(*this, timer);
	m_camera->addPostDrawCallback( *(m_postDrawCB.get()) );

	// Create a Keyboard Mouse Callback to handle escaping
	m_kmcb = new KeyboardMouseCallback(this, vpRef, kbEvents);

	// Set the window size
	m_camera->setWindowRectangle(100,100, 600, 400, false);
	m_camera->fullScreen(false);

	// Bind the escape key and listen for the event
	globalMap.Event_Map["QUIT"].Subscribe(ehl, *this, &Viewer::QUIT_EventHandler);

	// Add the snap image callback
	m_snapImageCallback = new GG_Framework::UI::OSG::SnapImageDrawCallback();
	m_camera->addPostDrawCallback( *(m_snapImageCallback.get()) );

	// Init variables for finding frame-rate
	m_frameRate = 0.0;
	m_lastFrameTime = 0.0;
	m_frameNum = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////

void Viewer::UpdateViewpointActorTitle(bool forceUpdate)
{
	// Watch for needing to refresh everything
	if (forceUpdate) m_lastTitleUpdate = -1;

	// Get the proper entity and view-port reference
	osg::Node* node = m_vpRef->GetNode();
	if (!node) return;

	// Work out the proper values in the title
	int titleUpdate = 0;
	const char* firstTitle = "";
	const char* secondTitle = "";

	osg::PolygonMode *pm = dynamic_cast<osg::PolygonMode *>(node->getOrCreateStateSet()->getAttribute(
		osg::StateAttribute::POLYGONMODE ));
	if ( pm == NULL ) 
	{
		pm = new osg::PolygonMode;
		node->getOrCreateStateSet()->setAttribute( pm );
	}
	titleUpdate = (int)pm->getMode(osg::PolygonMode::FRONT);
	if (titleUpdate == osg::PolygonMode::LINE)
		firstTitle = "Wire Frame";
	else if (titleUpdate == osg::PolygonMode::POINT)
		firstTitle = "Point Cloud";
	else firstTitle = "Solid";

	if (node->getOrCreateStateSet()->getMode(GL_LIGHTING) & osg::StateAttribute::ON)
	{
		secondTitle = "Shaded";
		titleUpdate |= TitleUpdate_Shaded;
	}
	else if (node->getOrCreateStateSet()->getMode(GL_LIGHTING) & osg::StateAttribute::INHERIT)
	{
		secondTitle = "Inherit";
		titleUpdate |= TitleUpdate_Inherit;
	}
	else
	{
		secondTitle = "Non-Shaded";
		titleUpdate |= TitleUpdate_NonShaded;
	}

	// Only do more if there is a change
	if (m_lastTitleUpdate == titleUpdate) return;
	m_lastTitleUpdate = titleUpdate;

	// Create the second part of the title of the window
	m_VP_ActorTitle = GG_Framework::Base::BuildString("%s (%s, %s)",
		m_vpRef->GetNode()->getName().c_str(),
		firstTitle, 
		secondTitle);
}
//////////////////////////////////////////////////////////////////////////////////////////

void Viewer::QUIT_EventHandler()
{
	if( m_postDrawCB->isEnabled( PostDrawCallback::DisplayHelp ) )
		m_postDrawCB->disable( PostDrawCallback::DisplayHelp );
	else
		m_killMe = true;
}
//////////////////////////////////////////////////////////////////////////////////////////

bool Viewer::UpdateMe(double dTick_s)
{
	// Bounce out if no longer valid
	if (m_killMe) return false;

	// Work out the frame-rate
	m_lastFrameTime += dTick_s;
	if (m_lastFrameTime > 3.0)	// Checks about every 3 seconds
	{
		m_frameRate = (float)m_frameNum / m_lastFrameTime;
		m_lastFrameTime = 0.0;
		m_frameNum = 0;
	}
	++m_frameNum;

	// Check for a change in what the user wants for the camera reference
	bool keepOldRot;
	ViewPortReference& newRef = m_kmcb->TrackBallViewPortRef(keepOldRot);

	if (m_vpRef != &newRef)
	{// There HAS been a change in what the viewer wants to see
		// Remember the current position if it is good
		if (m_vpRef)
			m_vpRef->StoreTrackBall(*(m_trackBall.get()));

		// Update the Trackball position with the new index, only if the user said not to keep the old
		m_vpRef = &newRef;
		m_vpRef->UpdateTrackBall(*(m_trackBall.get()), keepOldRot);

		// Force an update of the window title
		UpdateViewpointActorTitle(true);
	}
	else 
	{
		m_kbm->update(*(m_kmcb.get()));
		m_trackBall->input(m_kmcb->_mx, m_kmcb->_my, m_kmcb->_button_state );

		// Make sure our window title is still OK
		UpdateViewpointActorTitle(false);
	}

	
	// Center the trackball around the proper point and then multiply the TB rotation
	osg::Matrix cam_center = m_vpRef->GetInverseTransform();
	osg::Matrix trackball(m_trackBall->getMatrix().ptr());
	osg::Matrix camMatrix =	osg::Matrix::translate(-cam_center(3,0), -cam_center(3,1), -cam_center(3,2)) *
		trackball;

	// Let the real camera finish up
	m_camera->SetMatrix(camMatrix);
	return m_camera->Update();
}
//////////////////////////////////////////////////////////////////////////////////////////
