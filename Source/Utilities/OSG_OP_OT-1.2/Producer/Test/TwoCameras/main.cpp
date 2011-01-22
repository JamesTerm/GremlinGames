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

// Simple example of use of Producer::Camera
// The MySceneHandler class is a simple sample of a Camera::SceneHandler

#include <Producer/CameraGroup>
#include <Producer/CameraConfig>
#include "MySceneHandler"
#include "MySceneHandler2"

int main()
{
    Producer::ref_ptr<Producer::Camera> camera1 = new Producer::Camera;
    Producer::ref_ptr<Producer::Camera> camera2 = new Producer::Camera;

    Producer::ref_ptr<Producer::RenderSurface> rs = camera1->getRenderSurface();
	rs->setWindowRectangle( 100, 100, 640, 480 );
	rs->setWindowName( "Producer example using two Cameras, one ProjectionRectangle" );

	camera2->setRenderSurface(rs.get());


	camera1->setSceneHandler( new MySceneHandler );
    camera1->setViewByLookat( 0.0f, 0.0f, 15.0f,  /// Position of the eye
                              0.0f, 0.0f, 0.0f,   /// Point the eye is looking at
                              0.0f, 1.0f, 0.0f ); /// Up

	camera2->setSceneHandler( new MySceneHandler2 );
    camera2->setViewByLookat( 0.0f, 0.0f, 45.0f,   /// Position of the eye
                     0.0f, 0.0f, 0.0f,   /// Point the eye is looking at
                     0.0f, 1.0f, 0.0f ); /// Up

	// If using camera group, use this code
    Producer::ref_ptr<Producer::CameraConfig> cfg = new Producer::CameraConfig;
	cfg->addCamera("Camera 1", camera1.get());
	cfg->addCamera("Camera 2", camera2.get());

    Producer::ref_ptr<Producer::CameraGroup> cg = new Producer::CameraGroup(cfg.get());

	cg->realize( Producer::CameraGroup::SingleThreaded );
	//cg->realize( Producer::CameraGroup::ThreadPerCamera );
	// end if using camera group
	
	while( cg->validForRendering() )
	{
		// if using camera group
		cg->frame();
		// else
		/*
		camera1.frame(false);
		camera2.frame(false);
		// We only need to advance on camera as that will swap buffers
		// on a single shared render surface
		camera1.advance();
		*/
		// end if using camera group
	}
	return 0;
}
