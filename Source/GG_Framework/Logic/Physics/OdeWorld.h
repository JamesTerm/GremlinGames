#pragma once

namespace GG_Framework
{
	namespace Logic
	{
		namespace Physics
		{

class OdeWorld {

    public:

        class OdeObject : public osg::Referenced
        { 
            public:
                OdeObject() {}
                virtual void init(OdeWorld &) = 0L;
                virtual void preUpdate() {}
                virtual void postUpdate() {}
        };

        OdeWorld();
        void setGravity( const osg::Vec3 & );
        void setCFM( double cfm );
        void setGroundPlane( const osg::Plane& );
        void update(double);
        void addObject( OdeObject * );
        dWorldID getWorld() { return _world; }
        dSpaceID getSpace() { return _space; }

        static inline void OsgToOdeMat( const osg::Matrix &R, dMatrix3 &r )
        {
            r[ 0] = R(0,0);
            r[ 1] = R(1,0);
            r[ 2] = R(2,0);
            r[ 3] = 0.0;
            r[ 4] = R(0,1);
            r[ 5] = R(1,1);
            r[ 6] = R(2,1);
            r[ 7] = 0.0;
            r[ 8] = R(0,2);
            r[ 9] = R(1,2);
            r[10] = R(2,2);
            r[11] = 0.0;
        }

    private:
        dWorldID _world;
        dSpaceID _space;
        dJointGroupID _contactgroup;
        dGeomID _groundPlaneID;
        osg::Plane _currentGroundPlane;
        std::vector<osg::ref_ptr<OdeObject> >_objects;

        static const osg::Vec3 _defaultGravity;
        static const osg::Plane _defaultGroundPlane;

        static void s_nearCallback (void *, dGeomID, dGeomID);
        void nearCallback( dGeomID, dGeomID );
};

		}
	}
}
