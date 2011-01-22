#ifndef __ScratchClass_Included_
#define __ScratchClass_Included_

#include <osg/Group>
#include <osg/PositionAttitudeTransform>

#include "ImportsExports.h"
namespace Scratch{

	class SCRATCHDLL_API ScratchClass {
	public:
		ScratchClass();
		
		osg::Group* GetScene(){return _scene.get();}
		osg::PositionAttitudeTransform& GetBigShipTrans(){return *(_bigShipTrans.get());}
		osg::PositionAttitudeTransform& GetLittleShipTrans(){return *(_littleShipTrans.get());}

	private:
		osg::ref_ptr<osg::Group> _scene;
		osg::ref_ptr<osg::PositionAttitudeTransform> _bigShipTrans;
		osg::ref_ptr<osg::PositionAttitudeTransform> _littleShipTrans;

	};
}


#endif __ScratchClass_Included_