//C++ header file - Open Producer - Copyright (C) 2004 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

#ifndef PRODUCER_EXAMPLES_MYKEYBOARD_MOUSE_CALLBACK
#define PRODUCER_EXAMPLES_MYKEYBOARD_MOUSE_CALLBACK
#include <stdio.h>
#include <Producer/RenderSurface> // For definition of KeySymbol
#include <Producer/KeyboardMouse>

#ifdef WIN32
#include <windows.h>
#endif

class ExampleKeyboardMouseCallback : public Producer::KeyboardMouseCallback
{
    public:
		ExampleKeyboardMouseCallback() :
			Producer::KeyboardMouseCallback(),
			_mx(0.0f),_my(0.0f),_mbutton(0),
            _done(false),
            _resetTrackball(false)
			{}

		virtual void keyPress( Producer::KeyCharacter key )
		{
            // Keys are equivalent to their ascii representation (eg. 'c').
			switch( key )
            {
                case ' ':
                    _resetTrackball = true;
                    break;
            }
		}

		virtual void specialKeyPress( Producer::KeyCharacter key )
		{
			switch( key )
			{
				case Producer::KeyChar_Escape:
					_done = true;
					break;
			}
		}

        virtual void shutdown()
        {
            _done = true; 
        }

		virtual void mouseMotion( float mx, float my ) 
		{
			_mx = mx;
			_my = my;
		}

		virtual void buttonPress( float mx, float my, unsigned int mbutton ) 
		{
			_mx = mx;
			_my = my;
			_mbutton |= (1<<(mbutton-1));
		}

		virtual void buttonRelease( float mx, float my, unsigned int mbutton ) 
		{
			_mx = mx;
			_my = my;
			_mbutton &= ~(1<<(mbutton-1));
		}

		bool done() { return _done; }
		float mx()  { return _mx; }
		float my()  { return _my; }
		unsigned int mbutton()  { return _mbutton; }

        bool resetTrackball() { return _checkToggle( _resetTrackball ); }

    protected:
		virtual ~ExampleKeyboardMouseCallback() {}

    private:
		bool _done;
		float _mx, _my;
		unsigned int _mbutton;
        bool _resetTrackball;

        bool _checkToggle( bool &toggle )
        {
            if( toggle )
            {
                toggle = false;
                return true;
            }
            return false;
        }
};
#endif
