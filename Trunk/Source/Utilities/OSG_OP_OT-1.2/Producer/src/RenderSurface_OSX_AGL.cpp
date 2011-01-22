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

#include <Producer/Export>
#include <Producer/RenderSurface>

#ifdef _OSX_AGL_IMPLEMENTATION

#include <Carbon/Carbon.h>
#include <OpenGL/OpenGL.h>
#include <AGL/agl.h>


using namespace Producer;

static OpenThreads::Mutex s_init_mutex;

// get a double value from a dictionary
static double getDictDouble (CFDictionaryRef refDict, CFStringRef key)
{
   double double_value;
   CFNumberRef number_value = (CFNumberRef) CFDictionaryGetValue(refDict, key);
   if (!number_value) // if can't get a number for the dictionary
       return -1;  // fail
   if (!CFNumberGetValue(number_value, kCFNumberDoubleType, &double_value)) // or if cant convert it
        return -1; // fail
    return double_value; // otherwise return the long value
}

bool RenderSurface::realize(Producer::VisualChooser *vc, GLContext shared_context)
{
    // we are already realized, so simply return
    
    if (_realized) return _realized;
        

    if( shared_context != 0L )
        _sharedGLContext = shared_context;
    else
        _sharedGLContext = _globallySharedGLContext;

    if ( _init() == false )
        return false;
    
    _realized = true;
    _realizeBlock->release();

    std::vector <Producer::ref_ptr<Callback> >::iterator p;
    for( p = _realizeCallbacks.begin(); p != _realizeCallbacks.end(); p++ )
    {
        if( (*p).valid() )
            (*(*p).get())( *this );
    }
    _realizeCallbacks.clear();

    return _realized;

}

bool RenderSurface::makeCurrent() const
{
    //s_init_mutex.lock();
    aglSetCurrentContext(_glcontext);
    //s_init_mutex.unlock();
    return true;
}

void RenderSurface::swapBuffers()
{
    //s_init_mutex.lock();
    aglSwapBuffers(_glcontext);
    //s_init_mutex.unlock();
}

void RenderSurface::run()
{
    _init();

    if( _threadReady != 0L)
        _threadReady->block();
}

void RenderSurface::sync(int)
{
}

void RenderSurface::fullScreen( bool flag )
{
    if (flag == _isFullScreen) 
        return;
    
    if (!_realized) {
        _isFullScreen = flag;
        return;
    }
    
    s_init_mutex.lock();
    
    aglSetCurrentContext (_glcontext);
    aglSetDrawable(_glcontext, NULL);
    aglSetFullScreen(NULL, 0,0,0,0);
    _releaseDisplayOrWindow();
    
    _isFullScreen = flag;
    
    _captureDisplayOrWindow();
    
    // bind context
    if (_isFullScreen) {
        _glcontext = _fullscreenGlcontext;
        aglSetCurrentContext (_glcontext);
        aglEnable (_glcontext, AGL_FS_CAPTURE_SINGLE);
        if (!aglSetFullScreen (_glcontext, 0, 0, 0, 0)) {
            std::cerr << "Rendersurface: could not switch to fullscreen" << std::endl;
        }
    } else {
        _glcontext = _windowGlcontext;
        aglSetCurrentContext (_glcontext);
        aglSetDrawable(_glcontext, GetWindowPort(_win));
        ShowWindow(_win);
    }
    
    s_init_mutex.unlock();

}

unsigned int RenderSurface::getNumberOfScreens()
{
    if( _numScreens == UnknownAmount )
    {
        if( CGGetActiveDisplayList( 0, NULL, &_numScreens ) != CGDisplayNoErr )
        {
            std::cerr << "CGGetActiveDisplayList() falied\n";
            _numScreens = UnknownAmount;
            return 0;
        }
    }
//    _numScreens = 1;
    return _numScreens;
}


static Display *_openDisplay(unsigned int num)
{
    static Display *displays = NULL;
    static CGDisplayCount numDisplays = 0;

    if( displays == NULL )
    {
        if( CGGetActiveDisplayList( 0, NULL, &numDisplays ) != CGDisplayNoErr )
        {
            std::cerr << "CGGetActiveDisplayList() falied\n";
            return NULL;
        }

        if( numDisplays == 0 )
        {
            std::cerr << "openDisplay: No Displays!\n";
            return NULL;
        }

        displays = new Display[numDisplays];
        if( CGGetActiveDisplayList( numDisplays, displays, &numDisplays ) != CGDisplayNoErr )
        {
            std::cerr << "CGGetActiveDisplayList() falied\n";
            return NULL;
        }
    }

    if( num > numDisplays )
    {
        std::cerr << "OpenDisplay(): " << num << " is out of range.\n";
        return NULL;
    }

    return &displays[num];
}

bool RenderSurface::_captureDisplayOrWindow()
{
    
    _dpy = _openDisplay(_screen);    
    
    if (_isFullScreen)
    {
        
        CGDisplayCapture(*_dpy);
        
        if( _screenWidth == UnknownDimension )
            _screenWidth = CGDisplayPixelsWide(*_dpy);

        if( _screenHeight == UnknownDimension )
            _screenHeight = CGDisplayPixelsHigh(*_dpy);

        
        // add next line and on following line replace hard coded depth and refresh rate
        CGRefreshRate refresh =  getDictDouble (CGDisplayCurrentMode (*_dpy), kCGDisplayRefreshRate);  
        CFDictionaryRef display_mode_values =
            CGDisplayBestModeForParametersAndRefreshRate(*_dpy, CGDisplayBitsPerPixel (*_dpy), 
                            _screenWidth,_screenHeight,  
                            refresh,  
                            NULL);

        // save current display settings
        _oldDisplayMode = CGDisplayCurrentMode(*_dpy);  
                                  
        CGDisplaySwitchToMode(*_dpy, display_mode_values);                             

    }
    else
    {
    
        if( _windowWidth == UnknownDimension )
            _windowWidth = (unsigned int)((float)CGDisplayPixelsWide(*_dpy) * 1.0f);

        if( _windowHeight == UnknownDimension )
            _windowHeight = (unsigned int)((float)CGDisplayPixelsHigh(*_dpy) * 1.0f);
    
        // create window
        if (_win == NULL) {
            Rect bounds = {_windowY, _windowX, _windowY + _windowHeight, _windowX + _windowWidth};
            OSStatus err = CreateNewWindow(kDocumentWindowClass,  kWindowLiveResizeAttribute | kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute, &bounds, &_win);
            if (err != 0) {
                std::cerr << "Producer: could not create window: " << err << std::endl;
                _win = NULL;
                return false;
            }
        }
    }
    return true;
}

void RenderSurface::_releaseDisplayOrWindow() {

    if (_isFullScreen) {
        CGDisplaySwitchToMode(*_dpy, _oldDisplayMode);
        CGDisplayRelease(*_dpy);
    } else
        HideWindow(_win);
}

bool RenderSurface::_init()
{
    s_init_mutex.lock();
    
    // capture display / window
    if (!_captureDisplayOrWindow())
        return false;
 
    // Set up the VisualChooser 
    VisualChooser *vc; 
    if(_visualChooser.valid()) vc = _visualChooser.get();
    else
    {
      vc = new VisualChooser;
      vc->setSimpleConfiguration(_drawableType != DrawableType_PBuffer);
    }
       
    // Create context for window
    VisualInfo *vinfo = vc->choose(_dpy, 0);
    if(vinfo)
    {
      _windowGlcontext = aglCreateContext (*vinfo, _sharedGLContext);
       aglDestroyPixelFormat( *vinfo );
       *vinfo = 0;
    }

    // Create context for fullscreen
    vinfo = vc->choose(_dpy, 1);
    if(vinfo)
    {
      _fullscreenGlcontext = aglCreateContext (*vinfo, _windowGlcontext);
      aglDestroyPixelFormat( *vinfo );
      *vinfo = 0;
    }

    // Initialize globallySharedGLContext if global sharing is requested and 
    // the global variable is still NULL.
    // Caveat is if the user attempted to share GL contexts already through realize()
    if( _shareAllGLContexts == true && 
            _globallySharedGLContext == 0L &&
            _sharedGLContext == 0L )
    {
        _globallySharedGLContext = _sharedGLContext = _windowGlcontext;
    }

    // attaching context to window/display
    if (_isFullScreen) {
        _glcontext = _fullscreenGlcontext;
        makeCurrent();
        aglSetFullScreen(_glcontext, 0, 0, 0, 0);
    } else {
        _glcontext = _windowGlcontext;
        makeCurrent();
        aglSetDrawable(_glcontext, GetWindowPort(_win));
        ShowWindow(_win);
    }

    // Add sync support by default:
    GLint swap = 1;
    aglSetInteger (_glcontext, AGL_SWAP_INTERVAL, &swap);
    
    s_init_mutex.unlock();
    
    return true;
}

void RenderSurface::_fini()
{
    if (_realized) {
        
        s_init_mutex.lock();

               
        aglSetDrawable(_glcontext, NULL);
        aglSetCurrentContext(NULL);
        aglDestroyContext(_windowGlcontext);
        aglDestroyContext(_fullscreenGlcontext);     
        
        _releaseDisplayOrWindow();
        
        if (_win) DisposeWindow(_win);
                                            
        
        s_init_mutex.unlock();

    }                                                
                                        
}

void RenderSurface::_setBorder(bool)
{
}

void RenderSurface::_useCursor(bool flag)
{
    if (_useCursorFlag==flag) return;
    
    _useCursorFlag = flag;

    if( _realized )
    {
        if (_useCursorFlag)
        {
            // switching on cursor
            CGDisplayShowCursor(*_dpy);
        }
        else
        {
            // switching off cursor
            CGDisplayHideCursor(*_dpy);
        }
    }
}

void RenderSurface::_setCursor(Cursor)
{
}

void RenderSurface::_useOverrideRedirect( bool )
{
    // Ignored on OSX CGL.  Flag value remains false.
}

void RenderSurface::_setCursorToDefault()
{
}

void RenderSurface::_resizeWindow()
{
    if (_isFullScreen) {
        _windowWidth =  CGDisplayPixelsWide(*_dpy);
        _windowHeight =  CGDisplayPixelsHigh(*_dpy);
    }
}

void RenderSurface::_positionPointer(int x, int y)
{
    if( _realized )
    {
        CGPoint point;
        point.x = x;
        point.y = y;
        CGDisplayMoveCursorToPoint(*_dpy, point);
    }
}


void RenderSurface::_computeScreenSize(unsigned int &width, unsigned int &height) const
{
    width =  CGDisplayPixelsWide(*_dpy);
    height =  CGDisplayPixelsHigh(*_dpy);
}


void RenderSurface::bindPBufferToTexture(BufferType /*buffer*/) const
{
    // Ok... a bit of a hack.
    glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, _windowWidth, _windowHeight);
}

void _setCursor()
{
}

void _positionPointer(int x, int y)
{
}

void _resizeWindow()
{
}

void _fini()
{
}

void RenderSurface::_setWindowName( const std::string &name )
{
    if( !_realized )
        return;
}

void RenderSurface::_initThreads()
{
    // Ignored...
}

#endif
