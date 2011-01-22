#pragma once

namespace GG_Framework
{
	namespace Logic
	{
		namespace Physics
		{

class RigidBody : public OdeWorld::OdeObject
{
    public:

        RigidBody(OdeWorld &world);
        ~RigidBody();

        void setPosition( const osg::Vec3 & );
        osg::Vec3 getPosition();
        void setOrientation( const osg::Matrix & );
        void setLinearVelocity( const osg::Vec3 &);
        osg::Vec3 getLinearVelocity();
        void setAngularVelocity( const osg::Vec3 &);
        osg::Vec3 getAngularVelocity();

        void applyForce( const osg::Vec3 &force, const osg::Vec3 &point );
        void applyTorque( const osg::Vec3 &torque );

        osg::Matrix getMatrix();
        osg::Matrix getR();
        osg::Matrix getRi();

        virtual void preUpdate(); 
        virtual void postUpdate();

        virtual void reset( const osg::Vec3 &position,
                            const osg::Matrix &rotation,
                            double velocity );

    protected:
        dBodyID     _body;
        osg::Matrix _R;
        osg::Matrix _Ri;
        osg::Vec3  _position;
        osg::Matrix _Tx;
};

		}
	}
}
