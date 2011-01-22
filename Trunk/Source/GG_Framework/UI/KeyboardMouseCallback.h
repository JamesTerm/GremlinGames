// KeyboardMouseCallback.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

class Viewer;
class KeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
public :
	KeyboardMouseCallback(Viewer* viewer, GG_Framework::UI::ViewPortReference& vpRef,
		GG_Framework::Base::KeyboardEvents& kbEvents);
	
	virtual void mouseMotion( float mx, float my );
	virtual void buttonPress( float mx, float my, unsigned int button );
	virtual void buttonRelease( float mx, float my, unsigned int button );
	virtual void keyRelease( Producer::KeyCharacter key );
	virtual void keyPress( Producer::KeyCharacter key );
	virtual void specialKeyPress( Producer::KeyCharacter key );
	virtual void specialKeyRelease( Producer::KeyCharacter key );

	GG_Framework::UI::ViewPortReference& TrackBallViewPortRef(bool& keepOldRot);

public :	// For easy access
	unsigned int _button_state;
	float _mx, _my;

	bool _isAlt;
	bool _keepOldRot;
	std::stack<unsigned> _downStack;

	// Keep track of the buttons already pressed
	std::set<Producer::KeyCharacter> _pressedKeys;
	GG_Framework::UI::ViewPortReference* _vpRef;

	Viewer* const m_viewer;
	// Fired for key press and key release from the KBMCB
	GG_Framework::Base::KeyboardEvents& m_kbEvents;
};

	}
}