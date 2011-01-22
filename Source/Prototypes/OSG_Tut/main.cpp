#include "KeyboardMouseCallback.h"
#include "osg/Node"
#include "osgDB/ReadFile"
#include "osgProducer/OsgCameraGroup"
#include "Producer/Trackball"
#include "osg/PositionAttitudeTransform"
#include "osgUtil/UpdateVisitor"

osg::AnimationPath* createAnimationPath(osg::Vec3d StartPos)
{
	// set up the animation path 
	osg::AnimationPath* animationPath = new osg::AnimationPath;
	animationPath->setLoopMode(osg::AnimationPath::LOOP);

	float yaw = 0.0f;
	float roll = osg::inDegrees(0.0f);

	osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));
	animationPath->insert(0.0,osg::AnimationPath::ControlPoint(StartPos,rotation));
	roll = osg::inDegrees(180.0f);
	rotation=(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));
	//make it spin
	animationPath->insert(1000.0,osg::AnimationPath::ControlPoint(StartPos,rotation));
	return animationPath;
}



void main(int argc, const char** argv)
{
	// Load your OSG database
	osg::ref_ptr<osg::Node> childModel = osgDB::readNodeFile(argv[1]);

	osg::ref_ptr<osg::PositionAttitudeTransform> Node1 = new osg::PositionAttitudeTransform;
	Node1->addChild(childModel.get());

	osg::ref_ptr<osg::PositionAttitudeTransform> Node2 = new osg::PositionAttitudeTransform;
	Node2->addChild(childModel.get());

	//Root = the observable universe
	osg::ref_ptr<osg::Group> root = new osg::Group;
	root->addChild(Node1.get());
	root->addChild(Node2.get());

	float Childradius= childModel->getBound().radius();
	Node1->setPosition(osg::Vec3d(-2*Childradius,0,0));
	osg::AnimationPath* animPath = createAnimationPath(osg::Vec3d(+2*Childradius,0,0));
	Node2->setUpdateCallback(new osg::AnimationPathCallback(animPath));
	

	// Allocate a new configuration, parse a file if it is passed
	// on the command line, otherwise use the system default configuration.
	Producer::ref_ptr<osgProducer::OsgCameraGroup> osgCameraGroup = 
		new osgProducer::OsgCameraGroup;

	// Pass the camera group the scene 
	osgCameraGroup->setSceneData(root.get());

	//Render Surface is like HWND
	// Set up the Keyboard and Mouse class
	Producer::ref_ptr<Producer::KeyboardMouse> kbm = 
		new Producer::KeyboardMouse(osgCameraGroup->getCamera(0)->getRenderSurface());

	Producer::ref_ptr<ExampleKeyboardMouseCallback> kbmcb = new ExampleKeyboardMouseCallback;
	kbm->setCallback( kbmcb.get() );
	kbm->startThread();

	// Camera manipulation will now be done with the 
	// Producer::Trackball
	osg::BoundingSphere bs = root->getBound();
	
	//It is probably a good idea to have the Kbmcb own the trackball instead of here
	osg::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
	tb->setOrientation( Producer::Trackball::Z_UP );
	tb->setDistance(bs.radius()*3.0f);
	//This is like setting the "center handle" of where the center is
	tb->translate(-bs.center().x(),-bs.center().y(),-bs.center().z());
	// a fall back for when we wish to reset a default
	tb->setReference();


	// Default realize or single threaded
	// osgCameraGroup->realize(Producer::CameraGroup::SingleThreaded);
	osgCameraGroup->realize();

	// For the animation section
	osg::ref_ptr<osg::FrameStamp> frameStamp = new osg::FrameStamp;
	osgUtil::UpdateVisitor update;
	update.setFrameStamp( frameStamp.get() );
	osg::Timer_t start_tick = osg::Timer::instance()->tick();
	int frameNum = 0;
	
	bool IsFullScreen=false;
	osgCameraGroup->getCamera(0)->getRenderSurface()->fullScreen(IsFullScreen);
	osgCameraGroup->getCamera(0)->getRenderSurface()->setWindowRectangle(100,100, 600, 400);

	while( !kbmcb->done() )
	{
		if (IsFullScreen!=kbmcb->GetFullScreenMode())
		{
			IsFullScreen=kbmcb->GetFullScreenMode();
			osgCameraGroup->getCamera(0)->getRenderSurface()->fullScreen(IsFullScreen);
		}

		// synchronize to the when cull and draw threads have completed.
		//TODO see if this is where the sleep is handled
		osgCameraGroup->sync();

		//This may be handled within the callback
		if( kbmcb->resetTrackball() )
			tb->reset();

		tb->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );
		//adjusts the camera via matrix settings (controlled by the camera)
		osgCameraGroup->setViewByMatrix( tb->getMatrix());

		//Here's a cool animation, WITHOUT using the Animation Path
		osg::Vec3d Pos= Node1->getPosition();
		Pos[1]-=Childradius*0.03;
		Node1->setPosition(Pos);

		// Here is where we do animation path callback
		frameStamp->setReferenceTime(osg::Timer::instance()->tick() - start_tick);
		frameStamp->setFrameNumber(++frameNum);
		root->accept(update);

		// handles the actually drawing (e.g. bitblt)
		osgCameraGroup->frame();
	}

	// synchronize to the when cull and draw threads have completed.
	osgCameraGroup->sync();

}