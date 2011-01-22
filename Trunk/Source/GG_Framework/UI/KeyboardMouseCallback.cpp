// KeyboardMouseCallback.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::UI::OSG;

KeyboardMouseCallback::KeyboardMouseCallback(Viewer* viewer, ViewPortReference& vpRef,
											 GG_Framework::Base::KeyboardEvents& kbEvents) :  
	Producer::KeyboardMouseCallback(), m_viewer(viewer), _isAlt(false), _vpRef(&vpRef),
		m_kbEvents(kbEvents), _arrowsChangeVPR(false)
{
	ASSERT(m_viewer);
	_mx = _my = -1.0f;
	_button_state = 0;
	_keepOldRot = false;
	
	// My background color starts off blue, so its easier to see when things disappear
	m_viewer->GetCamera()->setClearColor( osg::Vec4(0.2f,0.2f,0.4f,1.0f));
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouseCallback::mouseMotion( float mx, float my )
{
	_mx = mx;
	_my = my;
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouseCallback::buttonPress( float mx, float my, unsigned int button )
{
	_mx = mx;
	_my = my;
	_button_state |= (1<<(button-1));
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouseCallback::buttonRelease( float mx, float my, unsigned int button )
{
	_mx = mx;
	_my = my;
	_button_state &= ~(1<<(button-1));
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouseCallback::keyPress( Producer::KeyCharacter key )
{
	// Watch for the key already being pressed
	if (!_pressedKeys.count(key))
	{
		_pressedKeys.insert(key);
		m_kbEvents.KBCB_KeyDnUp.Fire(key, true);
		m_kbEvents.KBCB_KeyDn.Fire(key);
	}
}

void KeyboardMouseCallback::keyRelease( Producer::KeyCharacter key )
{
	// Fires an event, eventually all of the following will happen in an event handler
	_pressedKeys.erase(key);
	m_kbEvents.KBCB_KeyDnUp.Fire(key, false);

	switch( key )
	{
	case ' ':
		m_viewer->GetTrackBall().ResetFixed();
		break;

	case 'a':
		if (_isAlt)
		{
			if( m_viewer->GetPDCB().isEnabled( PostDrawCallback::DisplayAnim ) )
			{
				m_viewer->GetPDCB().disable( PostDrawCallback::DisplayAnim );
			}
			else
			{
				m_viewer->GetPDCB().enable( PostDrawCallback::DisplayAnim );
			}
		}
		break;

	case 'b':
		if (_isAlt)
		{
			osg::Vec4 c;
			m_viewer->GetCamera()->getClearColor(c);
			if (c[0] == 0.2f)
				m_viewer->GetCamera()->setClearColor( osg::Vec4(0.0f,0.0f,0.0f,1.0f));
			else if (c[0] == 0.0f)
				m_viewer->GetCamera()->setClearColor( osg::Vec4(0.8f,0.8f,0.8f,1.0f));
			else
				m_viewer->GetCamera()->setClearColor( osg::Vec4(0.2f,0.2f,0.4f,1.0f));
		}
		break;

	case 'd':
		if (_isAlt)
		{
			if( m_viewer->GetPDCB().isEnabled( PostDrawCallback::DisplayDist ) )
			{
				m_viewer->GetPDCB().disable( PostDrawCallback::DisplayDist );
			}
			else
			{
				m_viewer->GetPDCB().enable( PostDrawCallback::DisplayDist );
			}
		}
		break;

	case 'f':
		if (_isAlt)
		{
			bool fs = m_viewer->GetCamera()->isFullScreen();
			m_viewer->GetCamera()->fullScreen(!fs);
		}
		break;

	case 's':
		if (_isAlt)
		{
			if( m_viewer->GetPDCB().isEnabled( PostDrawCallback::DisplayFrameRate ) )
			{
				m_viewer->GetPDCB().disable( PostDrawCallback::DisplayFrameRate );
			}
			else
			{
				m_viewer->GetPDCB().enable( PostDrawCallback::DisplayFrameRate );
			}
		}
		break;

	case 'm':
		if (_isAlt)
		{
			if( m_viewer->GetPDCB().isEnabled( PostDrawCallback::DisplayCenterRef ) )
			{
				m_viewer->GetPDCB().disable( PostDrawCallback::DisplayCenterRef );
			}
			else
			{
				m_viewer->GetPDCB().enable( PostDrawCallback::DisplayCenterRef );
			}
		}
		break;

	case 'p':
		if (_isAlt)
			m_viewer->GetSnapCallback()->SnapImageOnNextFrame("c:\\OSGV_ScreenCapture.bmp");
		break;

	case '?':
	case 'h':
		if( m_viewer->GetPDCB().isEnabled( PostDrawCallback::DisplayHelp ) )
		{
			m_viewer->GetPDCB().disable( PostDrawCallback::DisplayHelp );
		}
		else
		{
			m_viewer->GetPDCB().enable( PostDrawCallback::DisplayHelp );
		}
		break;

	case 'l' :
		if (_isAlt)
		{
			osg::Node* node = m_viewer->GetViewPortReference()->GetNode();
			if (node)
			{
				node->getOrCreateStateSet()->setMode( GL_NORMALIZE, osg::StateAttribute::ON );
				osg::StateAttribute::GLModeValue l = node->getOrCreateStateSet()->getMode(GL_LIGHTING);
				if (l & osg::StateAttribute::INHERIT)
					node->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF);
				else if (l & osg::StateAttribute::ON)
					node->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::INHERIT);
				else
					node->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::ON);
					
			}
		}
		break;

	case 'w' :
		if (_isAlt)
		{
			osg::Node* node = m_viewer->GetViewPortReference()->GetNode();
			if (node)
			{
				osg::ref_ptr<osg::StateSet> drawState = node->getOrCreateStateSet();
				osg::PolygonMode* polyModeObj = dynamic_cast<osg::PolygonMode*>(drawState->getAttribute(osg::StateAttribute::POLYGONMODE));
				if (!polyModeObj) 
				{
					polyModeObj = new osg::PolygonMode;
					drawState->setAttribute(polyModeObj);
				}

				// cycle through the available modes.  
				switch(polyModeObj->getMode(osg::PolygonMode::FRONT_AND_BACK))
				{
					case osg::PolygonMode::FILL : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE); break;
					case osg::PolygonMode::LINE : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::POINT); break;
					case osg::PolygonMode::POINT : polyModeObj->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::FILL); break;
				}
			}
		}
		break;

	default: break;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouseCallback::specialKeyPress( Producer::KeyCharacter key )
{
	if (!_pressedKeys.count(key))
	{
		_pressedKeys.insert(key);
		m_kbEvents.KBCB_KeyDnUp.Fire(key, true);
		m_kbEvents.KBCB_KeyDn.Fire(key);

		if ((key == Producer::KeyChar_Alt_L) || (key == Producer::KeyChar_Alt_R))
			_isAlt = true;
	}
}

void KeyboardMouseCallback::specialKeyRelease( Producer::KeyCharacter key )
{
	// Fires an event, eventually all of the following will happen in an event handler
	_pressedKeys.erase(key);
	m_kbEvents.KBCB_KeyDnUp.Fire(key, false);
	if (!_arrowsChangeVPR)
		return;

	switch (key)
	{
	case Producer::KeyChar_Left :	// Previous Sibling
		{
			ViewPortReference* parent = _vpRef->GetParent();
			if (parent)
			{
				ViewPortReference* newVPref = parent->GetChild(_vpRef->GetParentIndex()-1);
				if (newVPref)
					_vpRef = newVPref;
			}
		}
		break;

	case Producer::KeyChar_Alt_L:
	case Producer::KeyChar_Alt_R:
		_isAlt = false;
		break;

	case Producer::KeyChar_Up :		// Parent
		{
			ViewPortReference* parent = _vpRef->GetParent();
			if (parent)
			{
				_downStack.push(_vpRef->GetParentIndex());
				_vpRef = parent;
			}
		}
		break;

	case Producer::KeyChar_Right :	// Right Sibling
		{
			ViewPortReference* parent = _vpRef->GetParent();
			if (parent)
			{
				ViewPortReference* newVPref = parent->GetChild(_vpRef->GetParentIndex()+1);
				if (newVPref)
					_vpRef = newVPref;
			}
		}
		break;

	case Producer::KeyChar_Down :	// Child
		{
			unsigned dnStackPos = 0;
			if (!_downStack.empty())
			{
				dnStackPos = _downStack.top();
				_downStack.pop();
			}
			ViewPortReference* child = _vpRef->GetChild(dnStackPos);
			if (!child && dnStackPos>0) child = _vpRef->GetChild(0);
			if (child)
				_vpRef = child;
		}
		break;

	default: break;
	};
}
////////////////////////////////////////////////////////////////////////////////////////////

ViewPortReference& KeyboardMouseCallback::TrackBallViewPortRef(bool& keepOldRot)
{
	keepOldRot = _keepOldRot;
	return *_vpRef;
}
////////////////////////////////////////////////////////////////////////////////////////////
