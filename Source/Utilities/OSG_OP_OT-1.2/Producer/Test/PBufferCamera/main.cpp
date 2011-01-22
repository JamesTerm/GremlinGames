// C++ source file - Open Scene Graph Training - Copyright (C) 2004 Don Burns
// Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
// as published by the Free Software Foundation.

#include <iostream>
#include <osgDB/ReadFile>
#include <osgUtil/SceneView>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include "PBufferTexture2D.h"
#include "Cube.h"


class CameraUpdateCallback : public Producer::Camera::UpdateCallback
{
    public:
        CameraUpdateCallback( osg::Node *scene ):
            _scene(scene),_angle(0.0) {}

        void operator () (Producer::Camera &camera )
        {
            osg::BoundingSphere bs = _scene->getBound();
            double l = bs.radius() * 3.0;

            camera.setViewByLookat( 
                    l * cos(_angle),
                    l * sin(_angle),
                    bs.center()[2],
                    bs.center()[0],
                    bs.center()[1],
                    bs.center()[2],
                    0,0,1);
            _angle += osg::PI/180.0;
        }

    private:
        osg::ref_ptr<osg::Node>_scene;
        double _angle;
};

void setTraversalMask( Producer::Camera *camera, unsigned int mask )
{
    osgProducer::OsgSceneHandler *sh = dynamic_cast<osgProducer::OsgSceneHandler *>(camera->getSceneHandler() );
    if( sh == 0L )
    {
        std::cerr << "DOH!" << std::endl;
    }
    else
    {
        osgUtil::SceneView *sceneView = sh->getSceneView();
        int inheritanceMask = sceneView->getInheritanceMask();
        inheritanceMask &= ~(osg::CullSettings::CULL_MASK);
        sceneView->setInheritanceMask( inheritanceMask );
        sceneView->setCullMask( mask );
    }
}

int main(int argc, char **argv)
{
    // Parse command line arguments 
    osg::ArgumentParser args( &argc, argv );

    // Set up the viewer
    osgProducer::Viewer viewer(args);
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // Set up a root node
    osg::ref_ptr<osg::Group> root = new osg::Group;

    // Load up the models specified on the command line
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(args);
    if( !loadedModel.valid() )
    {
        std::cerr << argv[0] << ": No data loaded.  Exiting." << std::endl;
        return 1;
    }
    loadedModel->setNodeMask( 0x1 );  // This node mask will match the  PBuffers cull node mask

    osg::ref_ptr<Cube> cube = new Cube;
    cube->setNodeMask( 0x02 );         // This node mask will match the Main camera's cull node mask

    root->addChild( loadedModel.get() );
    root->addChild( cube.get() );

    // Optionally tell the viewer what to display for a help message
    viewer.getUsage(*args.getApplicationUsage());

    // Optimize the scene
    osgUtil::Optimizer optimizer;
    optimizer.optimize(root.get());

    // set the scene to render
    viewer.setSceneData(root.get());

    // Set the texture for the cube as the pbuffer
    Producer::CameraConfig *cameraConfig = viewer.getCameraConfig();
    Producer::RenderSurface *pbuffer = cameraConfig->findRenderSurface( "PbufferRenderSurface" );
    // Create the Pbuffer Texture
    if( pbuffer != 0L )
    {
        // Create the Texture attached to the PBuffer
        osg::ref_ptr<PBufferTexture2D> texture = new PBufferTexture2D( pbuffer );
        texture->setInternalFormat(GL_RGB);
        texture->setTextureSize(pbuffer->getWindowWidth(),pbuffer->getWindowHeight());
        texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP);
        texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP);

        cube->getOrCreateStateSet()->setTextureAttributeAndModes( 0, texture.get() );
    }

    // Realize the viewer
    viewer.realize(Producer::CameraGroup::SingleThreaded);

    // set the traversal masks on each of the cameras
    // Note this has to be done after realize
    Producer::Camera *pbufferCamera = cameraConfig->findCamera( "A_PBufferCamera" );
    if( pbufferCamera != 0L )
    {
        setTraversalMask( pbufferCamera, 0x01 );
        pbufferCamera->setUpdateCallback( new CameraUpdateCallback( loadedModel.get() ) );
    }

    Producer::Camera *mainCamera = cameraConfig->findCamera( "B_MainCamera" );
    if( mainCamera != 0L )
    {
        setTraversalMask( mainCamera, 0x02 );
        // We'll set the clear color to black on the main Camera to distinguish
        // the blue background of the pbuffer.
        //
        // You have to call this here because osgProducer clobbers 
        // the ClearColor directive in the config file
        mainCamera->setClearColor( 0.0, 0.0, 0.0, 1.0 );
    }

    while( !viewer.done() )
    {
        viewer.sync();
        viewer.update();
        viewer.frame();
    }
    viewer.sync();
    return 0;
}
