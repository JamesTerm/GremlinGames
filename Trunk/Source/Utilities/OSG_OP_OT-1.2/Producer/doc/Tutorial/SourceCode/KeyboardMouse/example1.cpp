//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::KeyboardMouse
// MySceneHandler is a simple example of a Camera::SceneHandler

#include <Producer/Camera>
#include <Producer/KeyboardMouse>

#include "MySceneHandler"
#include "MyKeyboardMouseCallback"

int main()
{
    Producer::ref_ptr<Producer::Camera> camera = new Producer::Camera;
    Producer::ref_ptr<Producer::RenderSurface> rs = camera->getRenderSurface();
	rs->setWindowRectangle( 100, 100, 640, 480 );
	rs->setWindowName( "Producer Example using Camera" );

	camera->setSceneHandler( new MySceneHandler );

    Producer::ref_ptr<Producer::KeyboardMouse>  kbm = new Producer::KeyboardMouse(rs.get());

	bool done = false;
    Producer::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(done);

	while( !done )
	{
		camera->frame();
		kbm->update( *kbmcb.get() );
	}
	return 0;
}
