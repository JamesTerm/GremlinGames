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

#include <Producer/RenderSurface>
#include <Producer/Trackball>
#include <Producer/KeyboardMouse>
#include <Producer/Window3D>

using namespace Producer;
using namespace std;


class Window3DKbdMouseCallback : public KeyboardMouseCallback
{
    public:
        Window3DKbdMouseCallback() : KeyboardMouseCallback()
        {
            _tb = NULL;
            _button_state = 0;
            _clientKeyCallback = 0L;
            _done = false;
        }

        virtual void buttonPress( float mx , float my, unsigned int button) 
        {
            _mx = mx;
            _my = my;
            _button_state |= (1<<(button-1));

            if( _tb != NULL ) 
                _tb->input(_mx, _my, _button_state );
        }

        virtual void buttonRelease( float mx, float my, unsigned int button ) 
        {
            _mx = mx;
            _my = my;
            _button_state &= ~(1<<(button-1));

            if( _tb != NULL ) 
                _tb->input(_mx, _my, _button_state );
        }

        virtual void mouseMotion( float mx, float my) 
        {
            _mx = mx;
            _my = my;
        }

        void setTrackball( Trackball *tb ) { _tb = tb; }
        void setClientKeyboardCallback( Window3D::KeyboardCallback *callback )
        {
            _clientKeyCallback = callback;
        }

        float mx() { return _mx; }
        float my() { return _my; }
        unsigned int mbutton() { return _button_state; }

        virtual void specialKeyPress( KeyCharacter key )
        {
            switch( key )
            {
                case Producer::KeyChar_Escape:
                    _done = true;
                    break;
                default:
                    break;
            }
            if( _clientKeyCallback != NULL )
                (*_clientKeyCallback)( key );
        }

        virtual void keyPress( KeyCharacter key )
        {
            switch( key )
            {
                case ' ':
                    _tb->reset();
                    break;
                default:
                    break;
            }
            if( _clientKeyCallback != NULL )
                (*_clientKeyCallback)( key );
        }

        void shutdown() { _done = true; }

        bool done() { return _done; }

    private :
        Trackball *_tb;
        float _mx;
        float _my;
        bool _done;
        unsigned int _button_state;
        Window3D::KeyboardCallback *_clientKeyCallback;
};

class Window3D::Implementation {
    public :
        Implementation() 
        {
            _rs = new RenderSurface;
            _kbdMouse = new KeyboardMouse(_rs.get());
        }

        RenderSurface *renderSurface() { return _rs.get(); }
        Trackball *trackball() { return _trackball.get(); }
        KeyboardMouse *kbdMouse() { return _kbdMouse.get(); }
        Window3DKbdMouseCallback &kbdMouseCallback() { return _kmcb; }

        void enableTrackball()
        {
            if( !_kbdMouse.valid() ) 
                _kbdMouse = new KeyboardMouse(_rs.get());
            if( !_trackball.valid()) 
                _trackball = new Trackball;
            _trackball->setOrientation( Trackball::Y_UP );
            _trackball->setComputeOrientation( false );
            _kmcb.setTrackball( _trackball.get() );
        }

        Matrix::value_type *getTrackballMatrix()
        {
            if( !_trackball.valid()) 
                enableTrackball();
            return _trackball->getMatrix().ptr();
        }

        void disableTrackball()
        {
            if( !_trackball.valid() ) return;
            _trackball = 0L;
        }

        void setKeyboardCallback( KeyboardCallback *callback )
        {
            if( !_kbdMouse.valid() ) 
                _kbdMouse = new KeyboardMouse(_rs.get());
            _kmcb.setClientKeyboardCallback( callback );
        }

        void update()
        {
            if( !_kbdMouse.valid() ) return;
                _kbdMouse->update(_kmcb);

            if( !_trackball.valid() ) return;
                _trackball->input( _kmcb.mx(), _kmcb.my(), _kmcb.mbutton() );
        }

        bool done() { return _kmcb.done(); }

    private :
        ref_ptr<RenderSurface>   _rs;
        ref_ptr<Trackball>       _trackball;
        ref_ptr<KeyboardMouse>   _kbdMouse;
        Window3DKbdMouseCallback _kmcb;
};

Window3D::Window3D(
        const std::string name,
        int posx,
        int posy,
        unsigned int width,
        unsigned int height,
        unsigned int parent )
{
    string _name;
    if( name == "" )
    _name = "Window3D"; 
    else
    _name = name;

    _implementation = new Implementation;
    _implementation->renderSurface()->setWindowName( name );
    _implementation->renderSurface()->setParentWindow( Window(parent) );

    if( width != (unsigned int)-1 || height != (unsigned int)-1 )
        _implementation->renderSurface()->setWindowRectangle( posx, posy, width, height );

    _implementation->renderSurface()->realize();
}

Window3D::~Window3D()
{
    if( _implementation != NULL )
        delete _implementation;

}

unsigned int Window3D::width( void ) const
{
    return _implementation->renderSurface()->getWindowWidth();
}

unsigned int Window3D::height( void ) const
{
    return _implementation->renderSurface()->getWindowHeight();
}

void Window3D::getDimensions( unsigned int &width, unsigned int &height )
{
    int  x, y;
    unsigned int w, h;
    _implementation->renderSurface()->getWindowRectangle( x, y,w, h );
    width = w;
    height = h;
}

void Window3D::swapBuffers( void )
{
    _implementation->renderSurface()->swapBuffers();

    _implementation->update();
}

void Window3D::sync( int n )
{
    _implementation->renderSurface()->sync(n);
}

void Window3D::setKeyboardCallback( KeyboardCallback *callback )
{
    _implementation->setKeyboardCallback( callback );
}

int Window3D::mouseX(void)
{
    int  x, y;
    unsigned int w, h;
    _implementation->renderSurface()->getWindowRectangle( x, y, w, h );
    return int((1 + _implementation->kbdMouseCallback().mx()) * float(w))>>1;
}

int Window3D::mouseY(void)
{
    int  x, y;
    unsigned int w, h;
    _implementation->renderSurface()->getWindowRectangle( x, y, w, h );
    return int((1+_implementation->kbdMouseCallback().my()) * float(h))>>1;
}

unsigned int Window3D::mouseButton(void)
{
    return _implementation->kbdMouseCallback().mbutton();
}

void Window3D::enableTrackball( void )
{
    _implementation->enableTrackball();
}

void Window3D::disableTrackball( void )
{
    _implementation->disableTrackball();
}

Matrix::value_type *Window3D::getTrackballMatrix(void)
{
    return _implementation->getTrackballMatrix();
}

void Window3D::setTrackballScale( float s )
{
   _implementation->trackball()->setScale( s );
}


Trackball &Window3D::getTrackball( void ) 
{
    return *(_implementation->trackball());
}

bool Window3D::done() { return  _implementation->done(); }

