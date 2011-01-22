#include <iostream>
#include "../ScratchDLL/ScratchDLL.h"
#include "Junk.h"
#include "Junk2.h"
using namespace std;
using namespace Scratch;

#include <osgProducer/Viewer>
#include <osgDB/ReadFile>

void main () {
	Scratch::ScratchClass scratch;

	// construct the viewer.
	osgProducer::Viewer viewer;

	// set up the value with sensible default event handlers.
	viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

	// pass the loaded scene graph to the viewer.
	viewer.setSceneData(scratch.GetScene());

	// create the windows and run the threads.
	viewer.realize();

	while( !viewer.done() )
	{
		// wait for all cull and draw threads to complete.
		viewer.sync();

		// update the scene by traversing it with the the update visitor which will
		// call all node update callbacks and animations.
		viewer.update();

		// fire off the cull and draw traversals of the scene.
		viewer.frame();

		osg::Vec3d trans = scratch.GetLittleShipTrans().getPosition();
		trans[1]+= 0.5;
		scratch.GetLittleShipTrans().setPosition(trans);
	}

	// wait for all cull and draw threads to complete before exit.
	viewer.sync();

	// run a clean up frame to delete all OpenGL objects.
	viewer.cleanup_frame();

	// wait for all the clean up frame to complete.
	viewer.sync();
}