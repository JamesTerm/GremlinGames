// C++ source file - Open Scene Graph Training - Copyright (C) 2004 Don Burns
// Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
// as published by the Free Software Foundation.

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include "Application.h"
#include "XManipulator.h"



int main(int argc, char **argv)
{
    // Parse command line arguments 
    osg::ArgumentParser args( &argc, argv );


	Application app;

    // Set up the viewer
    osgProducer::Viewer viewer(args);
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
	viewer.getEventHandlerList().push_back( new XManipulator(app) );

	/*
	osg::ref_ptr<Explosion> explosion = new Explosion;
	osg::ref_ptr<osg::Group> root = new osg::Group;
    // Load up the models specified on the command line
	root->addChild( makeExplodingCow( go ) );
	root->addChild( osgDB::readNodeFile( "plane.osg" ));
	root->addChild( explosion.get());
	*/

    // Optionally tell the viewer what to display for a help message
    viewer.getUsage(*args.getApplicationUsage());

    // Optimize the scene
    osgUtil::Optimizer optimizer;
    optimizer.optimize(app.getRootNode());

    // set the scene to render
    viewer.setSceneData(app.getRootNode());

    // Realize the viewer
    viewer.realize();
    while( !viewer.done() )
    {
        viewer.sync();
        viewer.update();
        viewer.frame();
    }
    viewer.sync();
    return 0;
}
