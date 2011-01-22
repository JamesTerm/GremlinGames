#ifndef __APPLICATION_H
#define __APPLICATION_H

#include <osg/Switch>
#include "Explosion.h"

extern osg::Node *makeExplodingCow( bool &trigger);

class Application {
	public:

		enum Switches {
				PlaneSwitch,
				CowSwitch
		};

		Application():
			_trigger(false)
		{
			_root = new osg::Group;
			_explosion = new Explosion;
			_root->addChild( _explosion.get());
			_switch = new osg::Switch;

			osg::ref_ptr<osg::Node> plane = osgDB::readNodeFile( "plane.osg" );
			_switch->insertChild( PlaneSwitch, plane.get() );
			osg::ref_ptr<osg::Node> cow = makeExplodingCow(_trigger);
			_switch->insertChild( CowSwitch, cow.get() );

			_switch->setAllChildrenOff();

			_root->addChild( _switch.get());

		}

		osg::Group *getRootNode() { return _root.get(); }

		void togglePlane()
		{
			if( _switch->getValue( PlaneSwitch ) )
				_switch->setValue( PlaneSwitch, false );
			else
				_switch->setValue( PlaneSwitch, true );
		}

		void toggleCow()
		{
			if( _switch->getValue( CowSwitch ) )
				_switch->setValue( CowSwitch, true );
			else
				_switch->setValue( CowSwitch, true );
		}

		void trigger( bool sw, double t )
		{
			_trigger = sw;
			if( _trigger == true )
				_explosion->fire(t);
			else
				_explosion->reset();
		}

	private:

		osg::ref_ptr<osg::Group> _root;
		osg::ref_ptr<osg::Switch> _switch;
		osg::ref_ptr<Explosion> _explosion;
		bool _trigger;
};

#endif
