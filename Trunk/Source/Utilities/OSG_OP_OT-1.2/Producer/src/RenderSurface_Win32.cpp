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



#ifdef WIN32 

#include <stdio.h>
#include <iostream>
#include <map>
#include <sstream>
#include <windows.h>
#include <Producer/RenderSurface>
#include <Producer/Events>
#include <Producer/Keyboard>
#include "WGLExtensions.h"


using namespace Producer;

// Class statics
std::map <Window, RenderSurface *>RenderSurface::registry;

unsigned int RenderSurface::getNumberOfScreens(void) { return 1; }

RenderSurface::Client::Client(unsigned long mask): _mask(mask)
{
    q = new EventQueue;
}

void RenderSurface::Client::queue( ref_ptr<Event> ev ) 
{ 
    q->push_back( ev ); 
}


LONG WINAPI RenderSurface::s_proc( Window hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    std::map <Window, RenderSurface *>::iterator p;
    p = registry.find( hWnd );
    if( p == registry.end() )
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    else
        return p->second->proc( hWnd, uMsg, wParam, lParam );
        
}

void RenderSurface::dispatch( ref_ptr<Event> ev )
{
    if( clients.size() == 0 ) return;

        
        std::vector <Producer::ref_ptr<Client> >::iterator p;
    for( p = clients.begin(); p != clients.end(); p++ )
    {
#if 0
        if( ev->type() == Event::MouseMotion )
        {
            // If MouseMotionMask is set, then send event regardless of
            // Mouse button state.
            if( (*p)->mask() & Event::MouseMotionMask )
                (*p)->queue(ev);
            // Otherwise, send event only if mouse button is pressed
            else if( (*p)->mask() & Event::ButtonMotionMask && _mbutton )
                (*p)->queue(ev);
        }
        else if( (*p)->mask() & (1<<ev->type()) )
            (*p)->queue( ev ); 
#endif

        if( (*p)->mask() & (1<<ev->type()) )
        {                
            (*p)->queue( ev ); 
        }
    } 
}


LONG WINAPI RenderSurface::proc( Window hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    LONG lRet = 1;
    RECT rect;
    ref_ptr<Event> ev;

    switch (uMsg)
    {
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 522
#endif
    case WM_MOUSEWHEEL:
        {
            KeyboardMouseCallback::ScrollingMotion scroll = 
                    (signed int)(wParam) < 0 ? KeyboardMouseCallback::ScrollDown:
                    KeyboardMouseCallback::ScrollUp;
            ev = new MouseScrollEvent(hWnd,scroll);
            dispatch(ev);
        }
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
         break;

    case WM_MOUSEMOVE :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));

        ev = new MouseMotionEvent( hWnd, _mx, _my );
        dispatch( ev );
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_LBUTTONDOWN :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        ev = new ButtonPressEvent( hWnd, 1, _mx, _my );
        dispatch( ev );
        _mbutton |= 1L;
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_LBUTTONUP :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));

        ev =  new ButtonReleaseEvent( hWnd, 1, _mx, _my );
        dispatch( ev );
        _mbutton &= ~1L;
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_LBUTTONDBLCLK :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        ev = new DoubleButtonPressEvent( hWnd, 1, _mx, _my );
        dispatch( ev );
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_RBUTTONDOWN :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        ev = new ButtonPressEvent( hWnd, 3, _mx, _my );
        dispatch( ev );
        _mbutton |= 4L;
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_RBUTTONUP  :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        ev = new ButtonReleaseEvent( hWnd, 3, _mx, _my );
        dispatch( ev );
        _mbutton &= ~4L;
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_RBUTTONDBLCLK : 
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        dispatch( new DoubleButtonPressEvent( hWnd, 3, _mx, _my ));
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_MBUTTONDOWN  :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        ev = new ButtonPressEvent( hWnd, 2, _mx, _my );
        dispatch( ev );
        _mbutton |= 2L;
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_MBUTTONUP :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        ev = new ButtonReleaseEvent( hWnd, 2, _mx, _my );
        dispatch( ev );
        _mbutton &= ~2L;
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_MBUTTONDBLCLK :
        _mx = ((int)(short)LOWORD(lParam));
        _my = ((int)(short)HIWORD(lParam));
        ev = new DoubleButtonPressEvent( hWnd, 2, _mx, _my );
        dispatch( ev );
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_PAINT:
        if (_ownWindow)
        {
            PAINTSTRUCT    ps;
            BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        else
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_CREATE:
    case WM_SIZE:
    case WM_MOVE:

        // only change window dimensions for non-child, 
        // non-fullscreen windows (which are all that 
        // should be moved or resized directly.
        if (!(_isFullScreen || _parent))
        {
            GetClientRect(hWnd, &rect);
            _windowWidth = rect.right - rect.left;
            _windowHeight = rect.bottom - rect.top;

            // map the window origin to the screen
            POINT p;
            p.x = rect.left;
            p.y = rect.top;
            ClientToScreen(hWnd, &p);

            // flip to Y up convention
            _windowX = p.x;
            _windowY = _screenHeight - p.y - _windowHeight;

        }

        ev = new WindowConfigEvent( hWnd, _windowX, _windowY,
                                        _windowWidth,
                                        _windowHeight);
        dispatch(ev);
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_CLOSE:
        if (!_ownWindow)
        {
            // PROBLEM: can't call KillGLWindow() here because the GL context
            // was created in another thread and releasing it now would cause 
            // an error!
            //KillGLWindow();
            _realized = false;
#ifdef __VS6_IMPLEMENTATION
            SetWindowLong(_win, GWL_WNDPROC, (LONG)_oldWndProc);
#else
            SetWindowLongPtr(_win, GWLP_WNDPROC, (LONG_PTR)_oldWndProc);
#endif
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
            ev = new ShutdownEvent(hWnd);
            dispatch(ev);
        }
        else
            DestroyWindow (hWnd);                
        break;

    case WM_QUIT:
    case WM_DESTROY:
        if (!_ownWindow)
        {
                        // PROBLEM: can't call KillGLWindow() here because the GL context
                        // was created in another thread and releasing it now would cause 
                        // an error!
            //KillGLWindow();
            _realized = false;
#ifdef __VS6_IMPLEMENTATION
            SetWindowLong(_win, GWL_WNDPROC, (LONG)_oldWndProc);
#else
            SetWindowLongPtr(_win, GWLP_WNDPROC, (LONG_PTR)_oldWndProc);
#endif
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        }
        else
        {
            PostQuitMessage (0);
            _realized = false;
        }
        ev = new ShutdownEvent(hWnd);
        dispatch(ev);
        break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
        {
            KeyCharacter keychar = Keyboard::getSingleton()->win32KeyEventToKeyChar( lParam, wParam );
            ev = new KeyPressEvent(hWnd, keychar);
            dispatch( ev );
        }
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
        {
            KeyCharacter keychar = Keyboard::getSingleton()->win32KeyEventToKeyChar( lParam, wParam );
            ev = new KeyReleaseEvent(hWnd, keychar);
            dispatch( ev );
        }
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;
        /*
    case WM_CHAR:
        if (lParam >> 31) {     // this bit will be set if this event is a key release
            ev = new KeyReleaseEvent(wParam);
        } else {
            ev = new KeyPressEvent(wParam);
        }
        dispatch( ev );

        if( _use_default_esc && wParam == VK_ESCAPE )
        {
            PostQuitMessage(0);
            return 0;
        }
        */

    case WM_TIMER:
        ev = new TimerEvent(hWnd, wParam);
        dispatch( ev );
        if (!_ownWindow)
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;

    case WM_SETCURSOR:
    {
        long lRet = TRUE;
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!_ownWindow)
                lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
            _setCursor(_currentCursor);
        }
        else
        {
            if (_ownWindow)
                lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
            else
                lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        }
        break;
    }

    default:
        if (_ownWindow)
            lRet = DefWindowProc (hWnd, uMsg, wParam, lParam);
        else
            lRet = CallWindowProc(_oldWndProc, hWnd, uMsg, wParam, lParam);
        break;
    }
    return lRet;
} 

bool RenderSurface::realize( VisualChooser *vc, GLContext sharedGLContext )
{
    if( _realized ) return _realized;

    if( sharedGLContext != 0L )
        _sharedGLContext = sharedGLContext;
    else
        _sharedGLContext = _globallySharedGLContext;

    if( vc != NULL )
        _visualChooser = vc;
    if( _useConfigEventThread && _ownWindow && _drawableType == DrawableType_Window )
    {
        startThread();
        _threadReady->block();
    }
    else
    {
      if ( !_init() )
      {
        return false;
      }
    }
        
    makeCurrent();

    if (_sharedGLContext)
        wglShareLists(_sharedGLContext, _glcontext);

    _realized = true;

    std::vector <Producer::ref_ptr<Callback> >::iterator p;
    for( p = _realizeCallbacks.begin(); p != _realizeCallbacks.end(); p++ )
    {
        if( (*p).valid() )
            (*(*p).get())( *this );
    }
    _realizeCallbacks.clear();

    _realizeBlock->release();

    return _realized;
}


static void doInternalError( char *msg )
{

    LPVOID lpMsgBuf;
    FormatMessage(   FORMAT_MESSAGE_ALLOCATE_BUFFER |
                     FORMAT_MESSAGE_FROM_SYSTEM |
                     FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL,
                     GetLastError(),
                     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                     (LPTSTR) &lpMsgBuf,\
                     0,NULL 
                     );
    char *szMessage = new char[strlen(msg)+5+strlen("Producer: Internal Error \n ")+2];
    sprintf( szMessage, "Producer: Internal Error \n %s", msg );
    MessageBox( NULL, (LPCTSTR)lpMsgBuf, szMessage, MB_OK | MB_ICONINFORMATION );
    LocalFree( lpMsgBuf );
    delete szMessage;
}


void RenderSurface::KillGLWindow()
{
    if (_glcontext)
    {
        makeCurrent();
    }

    if (_isFullScreen)
    {
        // Switch Back To The Desktop
        ChangeDisplaySettings(NULL,0);    
        ShowCursor(TRUE);
    }

    if ( _glcontext && !wglDeleteContext(_glcontext))
    {
        doInternalError("Release Rendering Context Failed.");
    }
    _glcontext = NULL;

    if (_hdc && !ReleaseDC(_win,_hdc))
    {
        doInternalError("Release Device Context Failed.");
    }
    _hdc = NULL;

    if( _visualInfo && _ownVisualInfo ) {
        delete _visualInfo;
        _visualInfo = NULL;
    }

    if (_ownWindow)
    {
        if (_win && !DestroyWindow(_win))
        {
            doInternalError("Could Not Release hWnd.");
        }
        _win = NULL;
        if (!UnregisterClass(_windowName.c_str(),_hinstance))
        {
            doInternalError("Could Not Unregister Class.");
        }
        _hinstance = NULL;
    }
}

bool RenderSurface::_createVisualInfo()
{
    if( _visualID == NULL ) 
    {
        if( _glcontext != NULL && _hdc != NULL ) 
        {
            // _hdc presence means that drawable (Window or Pbuffer) exists as well
            _visualID = GetPixelFormat( _hdc );
            if( _visualID == NULL && _visualInfo != NULL && _ownVisualInfo == false ) 
            {
                // it might be null but suggested _visualInfo was passed from outside world
                // I could leave it to the VisualChooser to select the visual info 
                // but VisualInfo in VisualChooser gets reset and overwritten 
                // by each addExtendedAttribute so its unlikely that it would work

                _visualID = ChoosePixelFormat( _hdc, _visualInfo );
            }
        } else {

            ref_ptr<VisualChooser> lvc;
            if (_visualChooser.valid())
            {
                lvc = _visualChooser;
            }
            else
            {
                lvc = new VisualChooser;
                lvc->setSimpleConfiguration(_drawableType != DrawableType_PBuffer);
            }

            if (_drawableType == DrawableType_PBuffer)
            {
                lvc->addExtendedAttribute(WGL_DRAW_TO_PBUFFER_ARB, true);
                switch (_rtt_mode)
                {
                    case RenderToRGBTexture: 
                        lvc->addExtendedAttribute(WGL_BIND_TO_TEXTURE_RGB_ARB, true);
                    break;
                    case RenderToRGBATexture: 
                        lvc->addExtendedAttribute(WGL_BIND_TO_TEXTURE_RGBA_ARB, true);
                    break;
                    default: ;
                }
            }

            if (_drawableType == DrawableType_Window)
            {
                lvc->addExtendedAttribute(WGL_DRAW_TO_WINDOW_ARB, true);
            }

            if ( lvc->choose(&_hdc, _screen,lvc->getStrictAdherence()) )
            {
                _visualID = lvc->getVisualID();    

            }
        }
    }

    if( _visualID == NULL )
        return false;

    if( _visualInfo == NULL ) 
    {
        // Try to find _visualInfo from _visualID
        _visualInfo = new VisualInfo;
        _ownVisualInfo = true;

        HDC hdc = _hdc;

        // HDC may be not set here but I hope that 
        // description for root window will be correct

        if( hdc == NULL ) 
            hdc = GetDC( NULL ); 
            
        if( !DescribePixelFormat( hdc, _visualID, sizeof(PIXELFORMATDESCRIPTOR), _visualInfo ) ) {
            delete _visualInfo;
            _visualInfo = NULL;            
        }

        if( hdc != _hdc )
            ReleaseDC( NULL, hdc );
    }

    // WHAT to do if we have valid _visualID but no _visualInfo ? 
    // I would consider this a succes because only _visualID will be used later 
    // If you decide otherwise ucomment line below
    // if( _visualInfo ) return NULL;

    return true;
}

bool RenderSurface::_init()
{
    if (_drawableType == DrawableType_Window)
    {
        _hinstance    = GetModuleHandle(NULL);

        RECT rect;
        DEVMODE dm;
        memset(&dm,0,sizeof(dm));
        dm.dmSize = sizeof(dm);
        EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
        int x, y, w, h;

        _screenWidth  = dm.dmPelsWidth;
        _screenHeight = dm.dmPelsHeight;


        if (_ownWindow )
        {
            DWORD dwExStyle;
            DWORD dwStyle;
            WNDCLASS wndclass;

            _defaultCursor = LoadCursor( NULL, IDC_ARROW);
            _currentCursor = _defaultCursor;

            wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
            wndclass.lpfnWndProc   = (WNDPROC)s_proc;
            wndclass.cbClsExtra    = 0;
            wndclass.cbWndExtra    = 0;
            wndclass.hInstance     = _hinstance;
            wndclass.hIcon         = LoadIcon( _hinstance, "PRODUCER_ICON" );
            wndclass.hCursor       = _defaultCursor;
            wndclass.hbrBackground = NULL;
            wndclass.lpszMenuName  = 0;

            wndclass.lpszClassName = _windowName.c_str();

            // mew 2003-12-14
            // if the .exe doesn't override the PRODUCER_ICON resource
            // definition, fall back to the icon in the producer .dll.
#ifdef _DEBUG
#  define LIBNAME "Producerd.dll"
#else
#  define LIBNAME "Producer.dll"
#endif
            if (wndclass.hIcon == NULL)
            {
                // Don - 2004-10-28  
                // Huh??  If this code is running, the library is loaded, no?  
                // Why do we have to LoadLibrary here?
                HINSTANCE hDLLInstance = LoadLibrary( LIBNAME );
                if (hDLLInstance != NULL)
                {
                    wndclass.hIcon = LoadIcon(hDLLInstance, "PRODUCER_ICON");
                }
            }
            RegisterClass(&wndclass);

            if (_parent)
            {
                _isFullScreen = false;
            }

            if (_isFullScreen)
            {
                if (ChangeDisplaySettings(&dm,CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
                {
                    // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
                    if (MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","",MB_YESNO|MB_ICONEXCLAMATION)==IDYES)
                    {
                        _isFullScreen = FALSE;
                    }
                    else
                    {
                        doInternalError("Program Will Now Close.");
                        return FALSE;
                    }
                }
            }

            if (_parent)
            {
                GetClientRect(_parent, &rect);

                // style is a child window
                dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
                dwStyle   = WS_CHILDWINDOW;

                //AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

                _windowWidth = (rect.right - rect.left);
                _windowX = 0;

                _windowHeight = (rect.bottom - rect.top);
                _windowY = 0;

                x = _windowX;
                y = _windowY;
                w = _windowWidth;
                h = _windowHeight;
            }
            else if (_isFullScreen)
            {
                dwExStyle = WS_EX_APPWINDOW;
                dwStyle   = WS_POPUP | WS_MAXIMIZE;

                if (_useCustomFullScreen)
                {
                    x = _customFullScreenOriginX;
                    y = _screenHeight - (_customFullScreenOriginY + _customFullScreenHeight);
                    w = _customFullScreenWidth;
                    h = _customFullScreenHeight;
                }
                else
                {
                    x = 0;
                    y = 0;
                    w = _screenWidth;
                    h = _screenHeight;
                }
            }
            else // own window, no parent, not fullscreen
            {
                if( _windowWidth == UnknownDimension ) 
                {
                    _windowWidth = _screenWidth/2;
                    _windowX = _windowWidth/2;
                } 

                if ( _windowHeight == UnknownDimension ) 
                {
                    _windowHeight = _screenHeight/2;
                    _windowY = _windowHeight/2;
                } 

                dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
                dwStyle   = WS_OVERLAPPEDWINDOW;

                // map the Y up dimensions to Y down for window
                rect.left = _windowX;
                rect.top = _screenHeight - _windowY - _windowHeight;
                rect.bottom = _screenHeight - _windowY;
                rect.right = _windowWidth + _windowX;

                if(_decorations) {
                  // map from client area dimensions to outer window dimensions
                  AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);
                }
                else {
                  dwStyle=WS_POPUP;
                }

                x = rect.left;
                y = rect.top;
                w = rect.right - rect.left;
                h = rect.bottom - rect.top;
            }

            if (!(_win = CreateWindowEx(dwExStyle,
                                        _windowName.c_str(),
                                        TEXT( _windowName.c_str() ),
                                        dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                                        x,
                                        y,
                                        w,
                                        h,
                                        _parent,
                                        NULL,
                                        _hinstance,
                                        NULL)))
            {
                doInternalError("Window Creation Error.");
                KillGLWindow();
                return FALSE;
            }
        }
        else 
        {
            if( (_windowWidth == UnknownDimension) && (_windowHeight == UnknownDimension) )
            {
                POINT p;

                GetClientRect(_win, &rect);
                _windowWidth = rect.right - rect.left;
                _windowHeight = rect.bottom - rect.top;

                // map window origin to screen
                p.x = rect.left;
                p.y = rect.top;
                ClientToScreen(_win, &p);

                _windowX = p.x;
                _windowY = _screenHeight - _windowHeight - p.y;
            }

            _isFullScreen = false;
#ifdef __VS6_IMPLEMENTATION
            _oldWndProc = (WNDPROC)SetWindowLong(_win, GWL_WNDPROC, (LONG)((WNDPROC)s_proc));
#else
            _oldWndProc = (WNDPROC)SetWindowLongPtr(_win, GWLP_WNDPROC, (LONG_PTR)((WNDPROC)s_proc));
#endif
        }

        if (!(_hdc=GetDC(_win)))
        {
            doInternalError("Can't Create A GL Device Context.");
            KillGLWindow();
            return FALSE;
        }
    }
    _setCursor(_defaultCursor);

    ref_ptr<TemporaryWindow> tempwnd;

    if (_drawableType == DrawableType_PBuffer)
    {
        tempwnd = new TemporaryWindow();
        _hdc = tempwnd->getDC();
        if (!_hdc)
        { 
            doInternalError("Can't create the temporary window.");
            return false;
        }
        if (!tempwnd->makeCurrent())
        {
            doInternalError("Can't make the temporary window's context current");
            return false;
        }
    }

    if( _visualID == 0 && !_createVisualInfo() ) 
    {
        doInternalError("Can't assign Visual (PIXELFORMAT) to surface.");
        if (_drawableType == DrawableType_Window)
            KillGLWindow();
        return FALSE;
    }

    if (_drawableType == DrawableType_Window)
    {
        PIXELFORMATDESCRIPTOR pfd;
        if (!SetPixelFormat(_hdc, _visualID, &pfd))
        {
            doInternalError("Can't Set The PixelFormat.");
            KillGLWindow();
            return FALSE;
        }
    }
    else
    {
        WGLExtensions *ext = WGLExtensions::instance();
        if (!ext)
        {
            std::cerr << "Producer::RenderSurface: WGL extensions not available" << std::endl;
            return false;
        }

        if (!ext->isSupported(WGLExtensions::ARB_pbuffer))
        {
            std::cerr << "Producer::RenderSurface: PBuffers are not supported" << std::endl;
            return false;
        }

        std::vector<int> pbattr(_user_pbattr);

        if (_rtt_mode != RenderToTextureMode_None)
        {
            if (!ext->isSupported(WGLExtensions::ARB_render_texture))
            {
                std::cerr << "Producer::RenderSurface: Render-To-Texture is not supported" << std::endl;
            }
            else
            {
                pbattr.push_back(WGL_TEXTURE_FORMAT_ARB);

                switch (_rtt_mode)
                {
                    case RenderToRGBTexture:
                        pbattr.push_back(WGL_TEXTURE_RGB_ARB);
                        break;
                    case RenderToRGBATexture:
                        pbattr.push_back(WGL_TEXTURE_RGBA_ARB);
                        break;
                    default: 
                        pbattr.push_back(static_cast<int>(_rtt_mode));
                }

                pbattr.push_back(WGL_TEXTURE_TARGET_ARB);

                switch (_rtt_target)
                {
                    case TextureCUBE:
                        pbattr.push_back(WGL_TEXTURE_CUBE_MAP_ARB);
                        break;
                    case Texture1D:
                        pbattr.push_back(WGL_TEXTURE_1D_ARB);
                        break;
                    case Texture2D:
                        pbattr.push_back(WGL_TEXTURE_2D_ARB);
                        break;
                    default: 
                        pbattr.push_back(static_cast<int>(_rtt_target));
                }

                if (_rtt_options & RequestSpaceForMipMaps)
                {
                    pbattr.push_back(WGL_MIPMAP_TEXTURE_ARB);
                    pbattr.push_back(1);
                }

                if (_rtt_options & RequestLargestPBuffer)
                {
                    pbattr.push_back(WGL_PBUFFER_LARGEST_ARB);
                    pbattr.push_back(1);
                }
            }
        }

        // Terminate array
        pbattr.push_back(0);

        _win = reinterpret_cast<Window>(ext->wglCreatePBuffer(_hdc, _visualID, _windowWidth, _windowHeight, &pbattr.front()));
        if (!_win)
        {
            doInternalError("Can't create the PBuffer");
            return false;
        }

        _hdc = ext->wglGetPBufferDC(reinterpret_cast<HPBUFFERARB>(_win));
        if (!_hdc)
        {
            doInternalError("Can't get a device context for the PBuffer");
            return false;
        }

        int actual_width = _windowWidth;
        int actual_height = _windowHeight;
        ext->wglQueryPBuffer(reinterpret_cast<HPBUFFERARB>(_win), WGL_PBUFFER_WIDTH_ARB, &actual_width);
        ext->wglQueryPBuffer(reinterpret_cast<HPBUFFERARB>(_win), WGL_PBUFFER_HEIGHT_ARB, &actual_height);
        _windowWidth = actual_width;
        _windowHeight = actual_height;
    }

    if( _glcontext == NULL )
    {
        if (!(_glcontext = wglCreateContext( _hdc )))
        {
            doInternalError("Can't Create A GL Rendering Context.");
            KillGLWindow();
            return FALSE;
        }
    }

    // Initialize globallySharedGLContext if global sharing is requested and 
    // the global variable is still NULL.
    // Caveat is if the user attempted to share GL contexts already through realize()
    if( _shareAllGLContexts == true && 
        _globallySharedGLContext == 0L &&
        _sharedGLContext == 0L )
    {
        _globallySharedGLContext = _glcontext;
    }

// ?? WHY DOES THIS CRASH HERE ??  nhv
//    if (!makeCurrent())
//    {
//        KillGLWindow();
//        MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
//        return FALSE;
//    }


    if (_drawableType == DrawableType_Window)
    {
        _setCursor(_currentCursor);
        ShowWindow(_win, SW_NORMAL );
        UpdateWindow(_win );
        SetForegroundWindow(_win);
        SetFocus(_win);

        registry.insert(std::pair<Window,RenderSurface *>(_win, this));
    }

    return true;
}

void RenderSurface::fullScreen( bool flag )
{

    if( flag == _isFullScreen )
        return;

    if( _drawableType != DrawableType_Window )
        return;

    if (_parent)
        return;

    _isFullScreen = flag;

    if (_isFullScreen)
    {
        /* force fullscreen mode */
        DWORD s = GetWindowLong(_win, GWL_STYLE);
        s &= ~WS_OVERLAPPEDWINDOW;
        s |= WS_POPUP;
        SetWindowLong(_win, GWL_STYLE, s);
        if( _useCustomFullScreen )
            SetWindowPos(_win, 
                         HWND_TOP,
                         _customFullScreenOriginX,
                         _screenHeight - (_customFullScreenOriginY + _customFullScreenHeight),
                         _customFullScreenWidth,
                         _customFullScreenHeight,
                         SWP_FRAMECHANGED);
        else
            SetWindowPos(_win, 
                         HWND_TOP,
                         0, 0, 
                         _screenWidth, _screenHeight, 
                         SWP_FRAMECHANGED);

    } 
    else 
    {
        UINT flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING |
                     SWP_NOSIZE | SWP_NOZORDER;

        DWORD style = GetWindowLong(_win, GWL_STYLE);

        /* Get rid of fullscreen mode, if it exists */
        if ( style & WS_POPUP ) {
            if(_decorations) { // Only add border if we want decorations
                style &= ~WS_POPUP;
                style |= WS_OVERLAPPEDWINDOW;
            }
            SetWindowLong(_win, GWL_STYLE, style);
            flags |= SWP_FRAMECHANGED;
        }

        flags &= ~SWP_NOMOVE;
        flags &= ~SWP_NOSIZE;

        RECT rect;

        // map Y up convention to Y down for windows
        rect.left = _windowX;
        rect.top = _screenHeight - _windowY - _windowHeight;
        rect.bottom = _screenHeight - _windowY;
        rect.right = _windowWidth + _windowX;

        if(_decorations) {
            // adjust from client dimensions to window outer dimentions
            AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
        }

        int x = rect.left;
        int y = rect.top;
        int w = rect.right - rect.left;
        int h = rect.bottom - rect.top;

        SetWindowPos(_win,
                     HWND_TOP,
                     x, y, w, h,
                     flags);
    }
}

bool RenderSurface::makeCurrent( void ) const
{
    if (_rtt_mode != RenderToTextureMode_None && _drawableType == DrawableType_PBuffer)    
    {        
        WGLExtensions *tempext = WGLExtensions::instance();
        if (tempext && tempext->isSupported(WGLExtensions::ARB_render_texture))
        {
            tempext->wglReleaseTexImage(reinterpret_cast<HPBUFFERARB>(_win), WGL_FRONT_LEFT_ARB);
            tempext->wglReleaseTexImage(reinterpret_cast<HPBUFFERARB>(_win), WGL_BACK_LEFT_ARB);
        }
    }

    if(_readDrawableRenderSurface && _readDrawableRenderSurface->isRealized())
    {
        WGLExtensions *tempext = WGLExtensions::instance();
        if (tempext && tempext->isSupported(WGLExtensions::ARB_make_current_read))
        {
            if (!tempext->wglMakeContextCurrent(_hdc, _readDrawableRenderSurface->_hdc, _glcontext))
                return false;
        }
        else
            return false;
    }
    else
    {
        if (!wglMakeCurrent(_hdc, _glcontext))
            return false;
    }

    WGLExtensions *ext = WGLExtensions::instance();
    if (!ext) 
    {
        std::cerr << "Producer::RenderSurface: WGL extensions not available" << std::endl;
        return false;
    }

    if (_rtt_dirty_mipmap && (_rtt_options & RequestSpaceForMipMaps) != 0)
    {
        if (ext->isSupported(WGLExtensions::ARB_render_texture))
        {
            std::vector<int> pbattr;
            pbattr.push_back(WGL_MIPMAP_LEVEL_ARB);
            pbattr.push_back(_rtt_mipmap);
            ext->wglSetPBufferAttrib(reinterpret_cast<HPBUFFERARB>(_win), &pbattr.front());
        }
    }

    if (_rtt_dirty_face && _rtt_target == TextureCUBE)
    {
        if (ext->isSupported(WGLExtensions::ARB_render_texture))
        {
            std::vector<int> pbattr;
            pbattr.push_back(WGL_CUBE_MAP_FACE_ARB);
            pbattr.push_back(0x207D + _rtt_face);
            ext->wglSetPBufferAttrib(reinterpret_cast<HPBUFFERARB>(_win), &pbattr.front());
        }
    }

    return true;
}

void RenderSurface::swapBuffers( void )
{
    makeCurrent();
    SwapBuffers( _hdc );
}

EventQueue * RenderSurface::selectInput( unsigned int mask )
{
    Client *client = new Client(mask);
    clients.push_back( client );
    // Need to inform clients that the windows has already
    // been created and what its size is
    if( _realized )
    {        client->queue( new WindowConfigEvent( _win,
            _windowX,
            _windowY,
            _windowWidth,
            _windowHeight ));
    }
    return client->getQueue();
}

void RenderSurface::sync( int divisor )
{
    divisor = divisor;
    glFinish();
}


void RenderSurface::run( void )
{
    _init();

    if( _threadReady != 0L)
        _threadReady->block();

    MSG msg;

    while( GetMessage( &msg, NULL, 0, 0) > 0)
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
                testCancel();
    }
}

void RenderSurface::_resizeWindow()
{
    RECT rect;
    int x, y, h, w;

    if (_parent)
    {
        // get the client area of the parent
        GetClientRect(_parent, &rect);

        // clip to width of parent
        if ((_windowWidth + _windowX) < (unsigned int)rect.right)
        {
            w = _windowWidth;
            x = _windowX;
        }
        else
        {
            w = rect.right - _windowX;
            x = _windowX;
        }

        // clip to height of parent and flip from
        // Y up convention to Y down for window
        if ((_windowY + _windowHeight) < (unsigned int)rect.bottom)
        {
            h = _windowHeight;
            y = rect.bottom - _windowY - _windowHeight;
        }
        else
        {
            h = rect.bottom - _windowY;
            y = 0;
        }
    }
    else if (_isFullScreen)
    {
        x = 0;
        y = 0;
        h = _screenHeight;
        w = _screenWidth;
    }
    else // normal window
    {
        RECT rect;

        // map from Y up to Y down
        rect.left = _windowX;
        rect.top = _screenHeight - _windowY - _windowHeight;
        rect.bottom = _screenHeight - _windowY;
        rect.right = _windowWidth + _windowX;

        // adjust from client dimension to outside dimension
        //AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);
        // Above line replaced with this contribution by Trajce Nikolov (Nick):
        // Leaving the old line in commented out in case...
        LONG style = GetWindowLong(_win,GWL_STYLE);
        LONG styleEx = GetWindowLong(_win,GWL_EXSTYLE);
        AdjustWindowRectEx(&rect, style, FALSE, styleEx);

        x = rect.left;
        y = rect.top;
        w = rect.right - rect.left;
        h = rect.bottom - rect.top;
    }

    SetWindowPos(_win,HWND_TOP, x, y, w,
                 h, SWP_SHOWWINDOW | SWP_FRAMECHANGED);
}

void RenderSurface::_setBorder( bool flag )
{
    //Flags that will be used for SetWindowPos
    UINT flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOSENDCHANGING |
                    SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED;

    //Get current style of window
    DWORD style = GetWindowLong(_win, GWL_STYLE);

    //Get current client dimensions
    RECT rect;
    GetClientRect( _win, &rect );

    //Variables that will hold new size of window
    int cx, cy;

    if(flag)
    {
        //Add border style to window
        style &= ~WS_POPUP;
        style |= WS_OVERLAPPEDWINDOW;

        //Calculate new size of window with border
        rect.left = 0;
        rect.top = 0;
        rect.right = rect.right - rect.left;
        rect.bottom = rect.bottom - rect.top;
        
        LONG styleEx = GetWindowLong(_win,GWL_EXSTYLE);
        AdjustWindowRectEx(&rect, style, FALSE, styleEx);

        cx = rect.right - rect.left;
        cy = rect.bottom - rect.top;
        flags &= ~(SWP_NOSIZE);
    }
    else
    {
        //Remove border style from window
        style &= ~WS_OVERLAPPEDWINDOW;
        style |= WS_POPUP;

        //Calculate new size of window without border
        cx = rect.right - rect.left;
        cy = rect.bottom - rect.top;
        flags &= ~(SWP_NOSIZE);
    }

    //Apply new window style
    SetWindowLong(_win, GWL_STYLE, style);
    
    //Need to call this to change window size and update frame style
    SetWindowPos(_win, 
                    HWND_TOP,
                    0, 0, 
                    cx, cy, 
                    flags);
}

void RenderSurface::_setWindowName( const std::string & name )
{
    SetWindowText(_win, name.c_str());
}

void RenderSurface::_useCursor(bool flag)
{
    _useCursorFlag = flag;
    if( _realized )
        _setCursor(_currentCursor);
}

void RenderSurface::_setCursor(Cursor cursor)
{
    if( _useCursorFlag == false )
    {
        ::SetCursor(_nullCursor);
    }
    else
    {
        _currentCursor = cursor ? cursor : _nullCursor;

        if ( _currentCursor == NULL )
        {
            _currentCursor = ::LoadCursor( NULL, IDC_ARROW );
        }

        ::SetCursor(_currentCursor);
    }
}

void RenderSurface::_setCursorToDefault()
{
    _setCursor(_defaultCursor);
}

void RenderSurface::_positionPointer(int x, int y)
{
    //****TODO***
    // This needs to be mapped from an x, y that
    // are interpreted in the Y up Producer world
    // to Y down in the window world.  Should x, y
    // be screen or window coordinates?

    POINT coords = { x, y };

    // First find the new screen-relative coordinates of the mouse
    ClientToScreen( _win, &coords );

    // Now set the new mouse cursor position...
    SetCursorPos( coords.x, coords.y );
}

void RenderSurface::_computeScreenSize( unsigned int &width, unsigned int &height ) const
{
    DEVMODE dm;
    memset(&dm,0,sizeof(dm));
    dm.dmSize = sizeof(dm);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    width  = dm.dmPelsWidth;
    height = dm.dmPelsHeight;
}

void RenderSurface::resizeToParent()
{
    RECT rect;

    if (_parent)
    {
        GetClientRect(_parent, &rect);

        _windowWidth = (rect.right - rect.left);
        _windowX = 0;

        _windowHeight = (rect.bottom - rect.top);
        _windowY = 0;

        _resizeWindow();
    }
}

void RenderSurface::mapWindow()
{
    fprintf( stderr, "RenderSurface::mapWindow() - Unimplemented under win32\n" );
}

void RenderSurface::unmapWindow()
{
    fprintf( stderr, "RenderSurface::unMapWindow() - Unimplemented under win32\n" );
}

void RenderSurface::_fini()
{
    if( _realized && _drawableType == DrawableType_Window )
    {
        if (!_ownWindow)
#ifdef __VS6_IMPLEMENTATION
            SetWindowLong(_win, GWL_WNDPROC, (LONG)_oldWndProc);
#else
            SetWindowLongPtr(_win, GWLP_WNDPROC, (LONG_PTR)_oldWndProc);
#endif

        PostThreadMessage(getThreadId(), WM_QUIT, 0, 0);
    }
}

void RenderSurface::bindPBufferToTexture(BufferType buffer) const
{
    WGLExtensions *ext = WGLExtensions::instance();
    if (!ext || !ext->isSupported(WGLExtensions::ARB_render_texture))
    {
        // RTT is not supported, fall back to good old copy-to-texture.
        glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, _windowWidth, _windowHeight);
        return;
    }

    switch (buffer)
    {
        case BackBuffer:
            ext->wglBindTexImage(reinterpret_cast<HPBUFFERARB>(_win), WGL_BACK_LEFT_ARB);
            break;
        case FrontBuffer:
            ext->wglBindTexImage(reinterpret_cast<HPBUFFERARB>(_win), WGL_FRONT_LEFT_ARB);
            break;
        default:
            ext->wglBindTexImage(reinterpret_cast<HPBUFFERARB>(_win), static_cast<GLenum>(buffer));
    }        
}

void RenderSurface::_useOverrideRedirect( bool )
{
    // Ignored on Windows.  Flag value remains false.
}

void RenderSurface::_initThreads()
{
    // Stub
}


#endif  // WIN32


