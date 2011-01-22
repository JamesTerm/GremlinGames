//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::Camera
// The MySceneHandler class is a simple sample of a Camera::SceneHandler

#include <Producer/Camera>
#include "MySceneHandler"

int main(int argc, char **argv)
{
    // Declare the camera
    Producer::ref_ptr<Producer::Camera> camera = new Producer::Camera;

    // Optional.  Configure the size of the camera's render
    // surface.  Without these lines, the RenderSurface would
    // fill the whole screen and have no border
    Producer::ref_ptr<Producer::RenderSurface> rs = camera->getRenderSurface();
    rs->setWindowRectangle( 100, 100, 640, 480 );
    rs->setWindowName( "Producer Example using Camera" );

    // Tell the camera about the Scene Handler.  See notes in MySceneHandler
    camera->setSceneHandler( new MySceneHandler );

    // Main loop.  Note that the while() statement comes after camera->frame()
    // because the RenderSurface is not realized until the first call to camera->frame().
    do {
        camera->frame();
    } while( rs->isRealized() );

    return 0;
}
