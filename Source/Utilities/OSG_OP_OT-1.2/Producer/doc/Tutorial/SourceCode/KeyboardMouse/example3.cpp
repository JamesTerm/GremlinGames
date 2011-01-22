#include <stdio.h>
//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <Producer/Camera>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include "MySceneHandler2"
#include "MyKeyboardMouseCallback"

int main()
{
    Producer::ref_ptr<Producer::Camera> camera = new Producer::Camera;
    Producer::ref_ptr<Producer::RenderSurface> rs = camera->getRenderSurface();
    rs->setWindowRectangle( 100, 100, 640, 480 );
    rs->setWindowName( "Producer Example using Camera" );

    Producer::ref_ptr<MySceneHandler> sh = new MySceneHandler;
    camera->setSceneHandler( sh.get() );

    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(rs.get());
    bool done = false;

    Producer::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(done);
    kbm->setCallback( kbmcb.get() );
    kbm->startThread();

    Producer::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
    tb->setOrientation( Producer::Trackball::Y_UP );

    while( !done )
    {
        tb->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );
        sh->setMatrix( tb->getMatrix().ptr() );
        // Place the Camera into the world
        camera->setViewByLookat( 
                        0.0, 0.0, 8.0,   /// Position of the eye
                        0.0, 0.0, 0.0,   /// Point the eye is looking at
                        0.0, 1.0, 0.0 ); /// Up
        camera->frame();
    }
    return 0;
}
