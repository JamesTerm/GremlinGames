#include "ScratchClass.h"

#include <osgDB/ReadFile>

Scratch::ScratchClass::ScratchClass()
{
	_scene = new osg::Group();
	_bigShipTrans = new osg::PositionAttitudeTransform();
	_bigShipTrans->addChild(osgDB::readNodeFile("../Prototypes/TestData/Union-Crusader.lwo"));
	_bigShipTrans->setPosition(osg::Vec3d(100,0,0));
	_scene->addChild(_bigShipTrans.get());

	_littleShipTrans = new osg::PositionAttitudeTransform();
	_littleShipTrans->addChild(osgDB::readNodeFile("../Prototypes/TestData/Q-22A.lwo"));
	_littleShipTrans->setPosition(osg::Vec3d(0,-200,0));
	_scene->addChild(_littleShipTrans.get());
}