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
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgProducer/OsgSceneHandler>

class CaptureImageCallback : public Producer::Camera::Callback
{
    public:
        void operator () (const Producer::Camera &cam )
        {
            int x, y;
            unsigned int w, h;
            cam.getProjectionRectangle(x,y,w,h);

            osg::ref_ptr<osg::Image>image = new osg::Image;
            image->allocateImage( w, h, 1, GL_RGB, GL_UNSIGNED_BYTE);
            image->readPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE);
            osgDB::writeImageFile( *(image.get()), "cow.jpg" );
        }
};

int main(int argc, char **argv)
{
    // Create the Camera intended for rendering into a Pbuffer
    Producer::ref_ptr<Producer::Camera> pBufferCamera = new Producer::Camera;

    // Set the RenderSurface's DrawableType to Pbuffer
    pBufferCamera->getRenderSurface()->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );
    pBufferCamera->getRenderSurface()->setWindowRectangle( 0, 0, 256, 256 );

    // Set the pbuffer camera up to render the cow.
    osg::ref_ptr<osg::Node> cow = osgDB::readNodeFile( "cow.osg" );
    osg::ref_ptr<osgProducer::OsgSceneHandler> pbufferSceneHandler = new osgProducer::OsgSceneHandler;
    pbufferSceneHandler->getSceneView()->setDefaults();
    pbufferSceneHandler->getSceneView()->setSceneData( cow.get() );
    pbufferSceneHandler->getSceneView()->setDrawBufferValue(GL_FRONT);

	pBufferCamera->setSceneHandler( pbufferSceneHandler.get());

    osg::BoundingSphere bs = cow->getBound();
    pBufferCamera->setViewByLookat( 
                bs.center()[0] + bs.radius() * 3,
                bs.center()[1],
                bs.center()[2],

                bs.center()[0],
                bs.center()[1],
                bs.center()[2],

                0, 0, 1 );

    pBufferCamera->addPostDrawCallback( new CaptureImageCallback );
    // ... take a snapshot
    pBufferCamera->frame();

	return 0;
}

