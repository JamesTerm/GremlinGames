#if defined(WIN32) && ! defined(__CYGWIN__)
#include <windows.h>
#define sleep(x)	Sleep(x*1000);
#else
#include <unistd.h>
#endif

#include <GL/gl.h>
#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>

#include "Text.h"


using namespace Producer;

class Renderer
{
    public:
        Renderer( RenderSurface &rs ): 
	    _initialized(false),
            _keyChar(Producer::KeyChar_Unknown),
            _rs(rs), 
            _mbutton(0),
            _scroll_display_counter(0)
			{
				_mx = _my = _pmx = _pmy = 0;
			}

        void draw()
        {
            if( !_initialized ) init();
            glViewport( 0, 0, _rs.getWindowWidth(), _rs.getWindowHeight() );
            glClear( GL_COLOR_BUFFER_BIT );

            glMatrixMode( GL_PROJECTION );
            glLoadIdentity();
            glOrtho( 0.0, double(_rs.getWindowWidth()), 0.0, double(_rs.getWindowHeight()), -1.0, 1.0);
            glMatrixMode( GL_MODELVIEW );
            glLoadIdentity();

            char buff[128];
            float x, y;
            if( _keyChar != Producer::KeyChar_Unknown )
            {
                x = (_rs.getWindowWidth() - (22 * 10)) * 0.5; 
                y = (_rs.getWindowHeight() - 10) * 0.5;
                glColor4f( 1, 1, 0, 1 );
                if( Keyboard::isSpecialKey(_keyChar) )
                {
                    y -= 30.0;
                    sprintf( buff, "Special Key Press: '0x%04x'",  _keyChar );
                }
                else
                    sprintf( buff, "        Key Press: '%c'",  _keyChar );
                glRasterPos2f(x,y);
                Text::getSingleton()->drawString(Text::BitmapFont, buff );
            }

            x = 10.0;
            y = float(_rs.getWindowHeight()) - 50.0;
            glRasterPos2f(x,y);
            glColor4f( 1, 1, 0, 1 );
            sprintf( buff, "Passive Mouse Motion:  %8.4f %8.4f", _pmx, _pmy ); 
            Text::getSingleton()->drawString( Text::BitmapFont, buff );
            y -= 30.0;
            glRasterPos2f(x,y);
            sprintf( buff, " Active Mouse Motion:  %8.4f %8.4f  button = 0x%x", _mx, _my, _mbutton ); 
            Text::getSingleton()->drawString( Text::BitmapFont, buff );

            y -= 30.0;
            glRasterPos2f(x,y);
            sprintf( buff, "        Mouse Scroll: %s", 
                    _scroll == KeyboardMouseCallback::ScrollUp ?  "UP" :
                    _scroll == KeyboardMouseCallback::ScrollDown ?  "DOWN" : "    " );
            Text::getSingleton()->drawString( Text::BitmapFont, buff );
            if( _scroll_display_counter > 0 )
            {
                if( --_scroll_display_counter <= 0 )
                    _scroll = KeyboardMouseCallback::ScrollNone;
            }
        }

        void setKeyChar(Producer::KeyCharacter keyChar ) { _keyChar = keyChar; }
        void unsetKeyChar() { _keyChar = Producer::KeyChar_Unknown; }

        void setPassiveMouse( float mx, float my )
        {
            _pmx = mx;
            _pmy = my;
        }

        void setMouse(float mx, float my, unsigned int mbutton )
        {
            _mx = mx;
            _my = my;
            _mbutton = mbutton;
        }

        void setScroll( KeyboardMouseCallback::ScrollingMotion scroll )
        {
            _scroll = scroll; 
            _scroll_display_counter = 10;
        }

    private:
        bool _initialized;
        Producer::KeyCharacter _keyChar;
        Producer::RenderSurface &_rs;
        float _mx, _my;
        float _pmx, _pmy;
        unsigned int _mbutton;
        KeyboardMouseCallback::ScrollingMotion _scroll;
        unsigned int _scroll_display_counter;

        void init()
        {
            glClearColor( 0.2f, 0.2f, 0.4f, 1.0f );
            _initialized = true;
        }
};

class myKeyboardMouseCallback : public KeyboardMouseCallback
{
	public:
		myKeyboardMouseCallback( Renderer &renderer ): 
            _renderer(renderer),
            _done(false), 
            _button(0)
            {}

        virtual ~myKeyboardMouseCallback() {}
		
		void keyPress( Producer::KeyCharacter key )
		{
            _renderer.setKeyChar( key );
		}

		void keyRelease( Producer::KeyCharacter /*key*/ )
		{
            _renderer.unsetKeyChar();
		}

		void specialKeyPress( Producer::KeyCharacter key )
		{
            _renderer.setKeyChar( key );
		}

		void specialKeyRelease( Producer::KeyCharacter key )
		{
            _renderer.unsetKeyChar();
            if( key == Producer::KeyChar_Escape )
                _done = true;
		}

        void mouseScroll ( Producer::KeyboardMouseCallback::ScrollingMotion scroll )
        {
            _renderer.setScroll( scroll );
        }

        void mouseMotion( float mx, float my )
        {
            _renderer.setMouse(mx,my,_button);
        }

        void buttonPress( float mx, float my, unsigned int button )
        {
            _button |= (1<<(button-1));
            _renderer.setMouse(mx,my,_button);
        }
        
        void buttonRelease( float mx, float my, unsigned int button )
        {
            _button &= ~(1<<(button-1));
            _renderer.setMouse(mx,my,_button);
        }

        void passiveMouseMotion( float mx, float my )
        {
            _renderer.setPassiveMouse(mx,my);
        }

		void shutdown() { _done = true; }

		bool done() { return _done; }

    private:
        Renderer &_renderer;
		bool _done;
        unsigned int _button;
};


int main()
{
  ref_ptr<RenderSurface> rs = new RenderSurface;
  rs->setWindowRectangle(100,100,640,480);
  rs->setWindowName( "Keyboard Mouse Test" );

  Renderer renderer(*rs.get());

  ref_ptr<KeyboardMouse> kbm = new KeyboardMouse(rs.get());
  ref_ptr<myKeyboardMouseCallback> kbmcb = new myKeyboardMouseCallback(renderer);
  kbm->setCallback( kbmcb.get() );

  Keyboard::getSingleton()->mapKey( KeyCombination( KeyMod_Control|KeyMod_Alt, Key_A ), KeyChar_percent);

  kbm->startThread();
  rs->realize();

  while( !kbmcb->done() )
    {
      renderer.draw();
      rs->swapBuffers();
    }
  rs = 0;
  kbmcb = 0;
  kbm = 0;

  return 0;
}
