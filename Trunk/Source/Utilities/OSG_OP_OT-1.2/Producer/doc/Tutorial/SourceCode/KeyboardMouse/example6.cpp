#include <stdio.h>
//C++ source file - Open Producer - Copyright (C) 2002 Don Burns
//Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
//as published by the Free Software Foundation.

// Simple example of use of Producer::RenderSurface
// The myGraphics class is a simple sample of how one would implement
// graphics drawing with Producer::RenderSurface

#include <string>
#include <Producer/CameraConfig>
#include <Producer/CameraGroup>
#include <Producer/InputArea>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include "MySceneHandler2"
#include "MyKeyboardMouseCallback"

int main( int argc, char **argv )
{
	if( argc < 2 )
	{
		std::cerr <<"Usage: " << argv[0] << " <configfile>\n";
		return -1;
	}

    Producer::ref_ptr<Producer::CameraConfig> cfg = new Producer::CameraConfig;
    if( cfg->parseFile( argv[1] ) == false )
    {
        std::cerr <<"Camera Config File parse failed" << std::endl;
	    return -1;
    }

    Producer::ref_ptr<Producer::CameraGroup> cg  = new Producer::CameraGroup( cfg.get() );

	for( int i = 0; i < cg->getNumberOfCameras(); i++ )
		cg->getCamera(i)->setSceneHandler( new MySceneHandler );
	
    Producer::ref_ptr<Producer::InputArea> ia = cfg->getInputArea();

    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(ia.get());

	bool done = false;
    Producer::ref_ptr<MyKeyboardMouseCallback> kbmcb = new MyKeyboardMouseCallback(done);
	kbm->setCallback( kbmcb.get() );
	kbm->startThread();

    Producer::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
	tb->setOrientation( Producer::Trackball::Y_UP );

	//cg->realize( Producer::CameraGroup::ThreadPerCamera);
	cg->realize( Producer::CameraGroup::SingleThreaded);

	while( !done )
	{
		tb->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );
		for( int i = 0; i < cg->getNumberOfCameras(); i++ )
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
}
