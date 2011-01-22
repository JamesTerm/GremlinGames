// C++ source file - Open Scene Graph Training - Copyright (C) 2004 Don Burns
// Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
// as published by the Free Software Foundation.

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include "MovieMaker.h"

int main(int argc, char **argv)
{
    // Parse command line arguments 
    osg::ArgumentParser args( &argc, argv );

    // Set up the viewer
    osgProducer::Viewer viewer(args);
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // Load up the models specified on the command line
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(args);
    if( !loadedModel.valid() )
    {
        std::cerr << argv[0] << ": No data loaded.  Exiting." << std::endl;
        return 1;
    }

    // Optionally tell the viewer what to display for a help message
    viewer.getUsage(*args.getApplicationUsage());

    // Optimize the scene
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // Realize the viewer
    viewer.realize();

	//*** Make a MovieMaker
	MovieMaker mm;
	int X, Y;
	unsigned W, H;
	
	// Get Window Rectangle really gives us the X,Y pos of window and width and Height
	viewer.getCamera(0)->getRenderSurface()->getWindowRectangle(X, Y, W, H);

	// This X and Y is if we want to get just a PART of a window, relative to this window
	X = Y = 0;

	mm.StartCapture("C:\TestMovie.avi", X, Y, W, H);

    while( !viewer.done() )
    {
        viewer.sync();
        viewer.update();

		//*** Snap the current frame after update, before frame()
		mm.Snap();

        viewer.frame();
    }
    viewer.sync();

	//*** End the capture
	mm.EndCapture();

    return 0;
}
