//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

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

    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(rs.get());

	bool done = false;
	kbm->setCallback( new MyKeyboardMouseCallback(done));
	kbm->startThread();

	while( !done )
	{
		camera->frame();
	}
	return 0;
}
