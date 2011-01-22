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

#include <Producer/Camera>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>

#include <osg/StateSet>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgProducer/OsgSceneHandler>

#include "KeyboardMouseCallback.h"
#include "Cube.h"


int main(int argc, char **argv)
{
    // Create the Camera intended for rendering into a Pbuffer
    Producer::ref_ptr<Producer::Camera> pBufferCamera = new Producer::Camera;

    // Set the RenderSurface's DrawableType to Pbuffer
    pBufferCamera->getRenderSurface()->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );	
    pBufferCamera->getRenderSurface()->setWindowRectangle( 0, 0, 256, 256 );

#if !defined(GLX_VERSION_1_1)
	pBufferCamera->getRenderSurface()->setRenderToTextureMode(Producer::RenderSurface::RenderToRGBTexture);
#endif

    // Set the pbuffer camera up to render the cow.
    osg::ref_ptr<osg::Node> cow = osgDB::readNodeFile( "cow.osg" );
    osg::ref_ptr<osgProducer::OsgSceneHandler> pbufferSceneHandler = new osgProducer::OsgSceneHandler;
    pbufferSceneHandler->getSceneView()->setDefaults();
    pbufferSceneHandler->getSceneView()->setSceneData( cow.get() );
    pBufferCamera->setSceneHandler( pbufferSceneHandler.get());

    // This line is not necessary on glX pbuffer examples, but Windows
    // seems to  not like rendering to the back buffer on pbuffers
    pbufferSceneHandler->getSceneView()->setDrawBufferValue( GL_FRONT );

    // Cube is a cube, with a texture, with a texture subload callback, 
    // which is hard-coded to read a 256 x 256 image from 0,0 of the
    // current read buffer
    osg::ref_ptr<osg::Node> cube = new Cube(pBufferCamera->getRenderSurface()); 
    osg::ref_ptr<osgProducer::OsgSceneHandler> osgSceneHandler = new osgProducer::OsgSceneHandler;
    osgSceneHandler->getSceneView()->setDefaults();
    osgSceneHandler->getSceneView()->setSceneData( cube.get() );

    // Set up the main camera
    Producer::ref_ptr<Producer::Camera> camera = new Producer::Camera;
    camera->getRenderSurface()->setWindowRectangle( 100, 100, 800, 600 );
    camera->getRenderSurface()->setWindowName( "Producer PBuffer Test" );
    camera->setSceneHandler( osgSceneHandler.get() );
    camera->setClearColor( 0.4, 0.4, 0.4, 1.0 );

#if defined(GLX_VERSION_1_1)
	// This determins where Pixel reads occur from.  The main Camera will
    // set the pBuffer camera's rendersurface as the buffer to read from.
	camera->getRenderSurface()->setReadDrawable( pBufferCamera->getRenderSurface());
#endif

    // We'll rotate the pBuffer camera around the cow
    osg::BoundingSphere bs = cow->getBound();
    osg::Vec3 center = bs.center();
    float radius = bs.radius() * 4;
    float viewingAngle = 0.0;

    // Add keyboard mouse and trackball
    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(camera->getRenderSurface());
    Producer::ref_ptr<ExampleKeyboardMouseCallback> kbmcb = new ExampleKeyboardMouseCallback;
    kbm->setCallback( kbmcb.get() );
    kbm->startThread();
                                                                                                                            
    // Camera manipulation will now be done with the
    // Producer::Trackball
    bs = cube->getBound();
    osg::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
    tb->setOrientation( Producer::Trackball::Z_UP );
    tb->setDistance(bs.radius()*3.0f);
    tb->translate(-bs.center().x(),-bs.center().y(),-bs.center().z());
    tb->setReference();

    while( !kbmcb->done() )
    {
        if( kbmcb->resetTrackball() )
            tb->reset();

        // Rotate the pBuffer camera and ...
        viewingAngle += osg::DegreesToRadians(1.0);
        osg::Vec3 eye = center + osg::Vec3( radius * cos(viewingAngle),
                                            radius * sin(viewingAngle),
                                            0);
        pBufferCamera->setViewByLookat( eye[0], eye[1], eye[2],
                                        center[0], center[1], center[2],
                                        0, 0, 1 );
        // ... take a snapshot
        pBufferCamera->frame();

        tb->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );
        camera->setViewByMatrix( tb->getMatrix());

        // Take a snapshot in the main camera.
        camera->frame();
    } 
    return 0;
}

