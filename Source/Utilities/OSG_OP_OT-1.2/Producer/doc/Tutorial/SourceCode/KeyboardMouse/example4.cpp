#include <stdio.h>
//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <Producer/Camera>
#include <Producer/CameraGroup>
#include <Producer/InputArea>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include "MySceneHandler2"
#include "MyKeyboardMouseCallback"

int main()
{
    // Set up camera 1 with its own redners surface
    Producer::ref_ptr<Producer::Camera> camera1 = new Producer::Camera;
    Producer::ref_ptr<Producer::RenderSurface> rs1 = camera1->getRenderSurface();
	rs1->setWindowRectangle(10,10,620,480);
    rs1->setInputRectangle( Producer::RenderSurface::InputRectangle(0.0,0.5,0.0,1.0));
    // Use Shear offsets
	camera1->setOffset( 1.0, 0.0 );
	camera1->setSceneHandler(new MySceneHandler);

    // Set up camera 2 with its own redners surface
    Producer::ref_ptr<Producer::Camera> camera2 = new Producer::Camera;
    Producer::ref_ptr<Producer::RenderSurface> rs2 = camera2->getRenderSurface();
	rs2->setWindowRectangle(650,10,620,480);
    rs2->setInputRectangle( Producer::RenderSurface::InputRectangle(0.5,1.0,0.0,1.0));
    // Use Shear offsets
	camera2->setOffset( -1.0, 0.0 );
	camera2->setSceneHandler(new MySceneHandler);
	
    Producer::ref_ptr<Producer::InputArea> ia = new Producer::InputArea;
	ia->addRenderSurface( rs1.get() ); 
	ia->addRenderSurface( rs2.get() ); 

    Producer::ref_ptr<Producer::CameraConfig> cfg = new Producer::CameraConfig;
	cfg->addCamera("Camera 1", camera1.get());
	cfg->addCamera("Camera 2", camera2.get());

    Producer::ref_ptr<Producer::CameraGroup> cg = new Producer::CameraGroup(cfg.get());

    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(ia.get());

	bool done = false;
    Producer::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(done);
	kbm->setCallback( kbmcb.get() );
	kbm->startThread();

    Producer::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
	tb->setOrientation( Producer::Trackball::Y_UP );

	cg->realize(Producer::CameraGroup::ThreadPerCamera);
	//cg->realize(Producer::CameraGroup::SingleThreaded);

	while( !done )
	{
		tb->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );
		for( unsigned int i = 0; i < cg->getNumberOfCameras(); i++ )
		{
			MySceneHandler *sh = dynamic_cast<MySceneHandler *>(cg->getCamera(i)->getSceneHandler());
			sh->setMatrix( tb->getMatrix().ptr() );
		}
		// Place the Camera into the world
		cg->setViewByLookat( 
                        0.0f, 0.0f, 4.0f,   /// Position of the eye
						0.0f, 0.0f, 0.0f,   /// Point the eye is looking at
						0.0f, 1.0f, 0.0f ); /// Up
		cg->frame();
	}
	return 0;
}

