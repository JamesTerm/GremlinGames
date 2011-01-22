/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */


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

        bool resetTrackball() { return _checkToggle(_resetTrackball); }

    protected:
		virtual ~ExampleKeyboardMouseCallback() {}

    private:
		bool _done;
		float _mx, _my;
		unsigned int _mbutton;
        bool _resetTrackball;

        bool _checkToggle( bool &toggle )
        {
            if( toggle == true )
            {
                toggle = false;
                return true;
            }
            return false;
        }
};
#endif
