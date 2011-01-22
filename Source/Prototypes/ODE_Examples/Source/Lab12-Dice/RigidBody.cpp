#include <assert.h>
#include "RigidBody.h"


RigidBody::RigidBody(OdeWorld &world ):
    _body(0L)
{
    world.addObject( this );
}

RigidBody::~RigidBody()
{
}

void RigidBody::setOrientation( const osg::Matrix &R )
{
    if( _body == 0 ) return;
    dMatrix3 r;

    r[0] = R(0,0);
    r[1] = R(1,0);
    r[2] = R(2,0);
    r[3] = 0.0;
    r[4] = R(0,1);
    r[5] = R(1,1);
    r[6] = R(2,1);
    r[7] = 0.0;
    r[8] = R(0,2);
    r[9] = R(1,2);
    r[10] = R(2,2);
    r[11] = 0.0;

    dBodySetRotation( _body, r );
}

void RigidBody::setPosition( const osg::Vec3 &pos )
{
    if( _body == 0 ) return;
    dBodySetPosition(_body, pos[0], pos[1], pos[2] );
}

void RigidBody::setLinearVelocity( const osg::Vec3 &v )
{
    if( _body == 0 ) return;
    dBodySetLinearVel( _body, v[0], v[1], v[2] );
}

osg::Vec3 RigidBody::getLinearVelocity()
{
    if( _body == 0 ) return osg::Vec3(0,0,0);
    const dReal *d = dBodyGetLinearVel(_body);
    return osg::Vec3(d[0], d[1], d[2]);
}

osg::Vec3 RigidBody::getPosition()
{
    if( _body == 0 ) return osg::Vec3(0,0,0);

    const dReal *d = dBodyGetPosition(_body);
    return osg::Vec3(d[0],d[1],d[2]);
}

void RigidBody::setAngularVelocity( const osg::Vec3 &v )
{
    if( _body == 0 ) return;
    dBodySetAngularVel( _body, v[0], v[1], v[2] );
}

osg::Vec3 RigidBody::getAngularVelocity()
{
    if( _body == 0 ) return osg::Vec3(0,0,0);

    const dReal *d = dBodyGetAngularVel(_body);
    return osg::Vec3(d[0], d[1], d[2]);
}


void RigidBody::preUpdate()
{
}

void RigidBody::postUpdate()
{
    if( _body == 0 ) return;
    const dReal *d = dBodyGetPosition(_body);
    const dReal *r = dBodyGetRotation(_body);

    _R.set( r[0], r[4], r[8], 0,
            r[1], r[5], r[9], 0,
            r[2], r[6], r[10], 0,
            0, 0, 0, 1 );

    _Ri.set(r[0], r[1], r[2], 0,
            r[4], r[5], r[6], 0,
            r[8], r[9], r[10], 0,
            0, 0, 0, 1 );

    _Tx.set( r[0], r[4], r[8], 0,
             r[1], r[5], r[9], 0,
             r[2], r[6], r[10], 0,
             d[0], d[1], d[2], 1.0 );
}


osg::Matrix RigidBody::getMatrix()
{
    return _Tx;
}

osg::Matrix RigidBody::getR()
{
    return _R;
}

osg::Matrix RigidBody::getRi()
{
    return _Ri;
}

void RigidBody::applyForce( const osg::Vec3 &force, const osg::Vec3 &point )
{
    if( _body == 0L ) return;
    dBodyAddRelForceAtRelPos( _body,
            force[0], force[1], force[2],
            point[0], point[1], point[2] );
}

void RigidBody::applyTorque( const osg::Vec3 &torque )
{
    if( _body == 0L ) return;

    dBodyAddRelTorque( _body, torque[0], torque[1], torque[2] );
}


void RigidBody::reset( const osg::Vec3 &position, 
                       const osg::Matrix &rotation, 
                       double velocity )
{
    if( _body == 0L ) return;

    setPosition( position );
    setOrientation(rotation);
    osg::Vec3 v = (osg::Vec3(0,1,0) * rotation) * velocity;
    setLinearVelocity(v);
    setAngularVelocity(osg::Vec3(0,0,0));
}
