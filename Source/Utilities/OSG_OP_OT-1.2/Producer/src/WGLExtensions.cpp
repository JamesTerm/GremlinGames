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

#include "WGLExtensions.h"

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#include <sstream>


using namespace Producer;

namespace
{
    ref_ptr<TemporaryWindow> default_wnd__;
    static int               tempwnd_id__ = 0;
    OpenThreads::Mutex       wglext_critical__;
}


WGLExtensions::InstanceMap WGLExtensions::instances_;


WGLExtensions::WGLExtensions()
: supp_(0)
{
    // WGL_ARB_pbuffer
    wglCreatePBuffer     = (WGLCreatePBufferProc)wglGetProcAddress("wglCreatePbufferARB");
    wglGetPBufferDC      = (WGLGetPBufferDCProc)wglGetProcAddress("wglGetPbufferDCARB");
    wglReleasePBufferDC  = (WGLReleasePBufferDCProc)wglGetProcAddress("wglReleasePbufferDCARB");
    wglDestroyPBuffer    = (WGLDestroyPBufferProc)wglGetProcAddress("wglDestroyPbufferARB");
    wglQueryPBuffer      = (WGLQueryPBufferProc)wglGetProcAddress("wglQueryPbufferARB");
    if (wglCreatePBuffer && wglGetPBufferDC && wglReleasePBufferDC && wglDestroyPBuffer && wglQueryPBuffer)
        supp_ |= ARB_pbuffer;

    // WGL_ARB_render_texture
    wglBindTexImage      = (WGLBindTexImageProc)wglGetProcAddress("wglBindTexImageARB");
    wglReleaseTexImage   = (WGLReleaseTexImageProc)wglGetProcAddress("wglReleaseTexImageARB");
    wglSetPBufferAttrib  = (WGLSetPbufferAttribProc)wglGetProcAddress("wglSetPbufferAttribARB");
    if (wglBindTexImage && wglReleaseTexImage && wglSetPBufferAttrib)
        supp_ |= ARB_render_texture;

    // WGL_ARB_pixel_format
    wglChoosePixelFormat = (WGLChoosePixelFormatProc)wglGetProcAddress("wglChoosePixelFormatARB");
    if (wglChoosePixelFormat)
        supp_ |= ARB_pixel_format;

    // WGL_ARB_make_current_read
    wglMakeContextCurrent = (WGLMakeContextCurrentProc)wglGetProcAddress("wglMakeContextCurrentARB");
    if (!wglMakeContextCurrent)
    {
        wglMakeContextCurrent = (WGLMakeContextCurrentProc)wglGetProcAddress("wglMakeContextCurrentEXT");
    }
    if (wglMakeContextCurrent)
        supp_ |= ARB_make_current_read;
}

WGLExtensions::~WGLExtensions()
{
}

WGLExtensions *WGLExtensions::instance()
{
    // we must serialize access to this method because
    // the underlying implementation of the Standard Library
    // is not guaranted to be thread-safe, and therefore
    // the map<> container could raise multithreading issues.
    //OpenThreads::ScopedLock<OpenThreads::Mutex> slock(wglext_critical__);

    HGLRC context = wglGetCurrentContext();

    bool nocontext = (context == 0);

    if (nocontext || !default_wnd__.valid())
    {
        if (!default_wnd__.valid() || !default_wnd__->getHandle())
        {
            default_wnd__ = new TemporaryWindow;
            if (!default_wnd__->getHandle())
            {
                std::cerr << "Producer::WGLExtensions: could not create and initialize the temporary window" << std::endl;
                return 0;
            }
        }

        context = default_wnd__->getContext();
        if (!default_wnd__->makeCurrent())
        {
            std::cerr << "Producer::WGLExtensions: could not make the temporary window's context active" << std::endl;
        }
    }

    if (!instances_[context])
    {
        instances_[context] = new WGLExtensions;
    }

    if (nocontext)
        wglMakeCurrent(0, 0);

    return instances_[context].get();
}



TemporaryWindow::TemporaryWindow()
:    handle_(0),
    dc_(0),
    context_(0),
    instance_(0)
{
    create();
}

TemporaryWindow::TemporaryWindow(const TemporaryWindow &)
{
    throw "This is TemporaryWindow, please don't copy me!"; 
}

void TemporaryWindow::create()
{
    std::ostringstream oss;
    oss << "tempwnd" << (++tempwnd_id__);
    classname_ = oss.str();

    instance_ = GetModuleHandle(0);
        
    WNDCLASS wndclass;
            
    wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; 
    wndclass.lpfnWndProc   = DefWindowProc;
    wndclass.cbClsExtra    = 0;
    wndclass.cbWndExtra    = 0;
    wndclass.hInstance     = instance_;
    wndclass.hCursor       = 0;
    wndclass.hIcon         = 0;
    wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wndclass.lpszMenuName  = 0;                            
    wndclass.lpszClassName = classname_.c_str();

    if (!RegisterClass(&wndclass))
        return;

    if (!(handle_ = CreateWindowEx( 0,
                                    classname_.c_str(),
                                    TEXT(classname_.c_str()),
                                    WS_POPUP,
                                    0,
                                    0,
                                    100,
                                    100,
                                    0,
                                    0,
                                    instance_,
                                    0)))
    {
        kill();
        return;
    }

    if (!(dc_ = GetDC(handle_)))
    {
        kill();
        return;
    }

    PIXELFORMATDESCRIPTOR pfd = { 
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                     
        PFD_DRAW_TO_WINDOW |   
        PFD_SUPPORT_OPENGL,      
        PFD_TYPE_RGBA,         
        24,                    
        0, 0, 0, 0, 0, 0,      
        0,                     
        0,                     
        0,                     
        0, 0, 0, 0,            
        16,
        0,                     
        0,                     
        PFD_MAIN_PLANE,        
        0,                     
        0, 0, 0                
    }; 

    int visual_id = ChoosePixelFormat(dc_, &pfd); 

    if (!SetPixelFormat(dc_, visual_id, &pfd))
    {
        kill();
        return;
    }

    if (!(context_ = wglCreateContext(dc_)))
    {
        kill();
        return;
    }
}

TemporaryWindow::~TemporaryWindow()
{
    kill();
}

void TemporaryWindow::kill()
{
    if (context_)
    {
        // mew 2005-05-09 commented out due to crashes.
        // possible causes are unsafe destructor ordering, or context already
        // deleted by window deletion; see:
        // http://openscenegraph.org/pipermail/osg-users/2005-May/052753.html
        //wglDeleteContext(context_);
        context_ = 0;
    }

    if (dc_)
    {
        ReleaseDC(handle_, dc_);
        dc_ = 0;
    }

    if (handle_)
    {
        DestroyWindow(handle_);
        handle_ = 0;
    }

    UnregisterClass(classname_.c_str(), instance_);
    instance_ = 0;
}

bool TemporaryWindow::makeCurrent()
{
    return wglMakeCurrent(dc_, context_);
}

#endif


