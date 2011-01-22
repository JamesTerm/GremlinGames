
#include "OdeWorld.h"


OdeWorld::OdeWorld()
{
    _world = dWorldCreate();
    _space = dHashSpaceCreate (0);
    _contactgroup = dJointGroupCreate (0);
	//dWorldSetGravity (_world,0,0,0);
	//Normal earth gravity
    dWorldSetGravity (_world,0,0,-9.80);
	//default 20... no of iterations to perform per step... the lower the more efficient but less accurate
	//It seems that the lowest 1... seems to be pretty good if using dt*1.0
	dWorldSetQuickStepNumIterations(_world,20);
    dWorldSetCFM( _world, 1e-8);

    _currentGroundPlane.set( 0, 0, 1, 0 );
    _groundPlaneID = dCreatePlane( _space,
            _currentGroundPlane[0],
            _currentGroundPlane[1],
            _currentGroundPlane[2],
            _currentGroundPlane[3] );
}

void OdeWorld::setGravity( const osg::Vec3 &g )
{
    dWorldSetGravity (_world,g[0], g[1], g[2]);
}

void OdeWorld::setCFM( double cfm )
{
    dWorldSetCFM( _world, cfm );
}

void OdeWorld::setGroundPlane( const osg::Plane& plane )
{
    if( plane != _currentGroundPlane )
    {
        _currentGroundPlane = plane;
        dGeomDestroy( _groundPlaneID );
        _groundPlaneID = dCreatePlane (_space,
                                   _currentGroundPlane[0],
                                   _currentGroundPlane[1],
                                   _currentGroundPlane[2],
                                   _currentGroundPlane[3] );
    }
}

void OdeWorld::update(double dt)
{
    //int i;

    std::vector< osg::ref_ptr<OdeObject> >::iterator p;
    for( p = _objects.begin(); p != _objects.end(); p++ )
        (*p)->preUpdate();

    dSpaceCollide(_space,this,&s_nearCallback);
	//use the more efficient one! -James
	dWorldQuickStep(_world,dt);
    //dWorldStep(_world,dt);
    // remove all contact joints
    dJointGroupEmpty (_contactgroup);

    for( p = _objects.begin(); p != _objects.end(); p++ )
        (*p)->postUpdate();
}

void OdeWorld::s_nearCallback( void *data, dGeomID g0, dGeomID g1 )
{
    OdeWorld *This = static_cast<OdeWorld *>(data);
    if( This != 0L )
        This->nearCallback( g0, g1 );
}

void OdeWorld::nearCallback( dGeomID o1, dGeomID o2)
{
    const int N = 10;
    dContact contact[N];
    int n = dCollide (o1,o2,N,&contact[0].geom,sizeof(dContact));
    if(n > 0) 
    {
        for(int i=0; i<n; i++) 
        {
            contact[i].surface.mode = //dContactSlip1 | dContactSlip2 |
	                                  dContactSoftERP | dContactSoftCFM | dContactApprox1;
            contact[i].surface.mu = dInfinity;
            //contact[i].surface.mu2 = dInfinity;
            contact[i].surface.mu = 1.0;
            //contact[i].surface.slip1 = .00001;
            //contact[i].surface.slip2 = .00001;
            //contact[i].surface.soft_erp = 0.5;
            //contact[i].surface.soft_cfm = 0.3;
            //contact[i].surface.slip1 = 0.00001;
            //contact[i].surface.slip2 = 0.00001;
            contact[i].surface.soft_erp = 0.5;
            //contact[i].surface.soft_cfm = 0.000001;
            contact[i].surface.soft_cfm = 0.0001;
            dJointID c = dJointCreateContact (_world,_contactgroup,&contact[i]);
            dJointAttach (c, dGeomGetBody(contact[i].geom.g1), dGeomGetBody(contact[i].geom.g2));
        }
    }
}


void OdeWorld::addObject( OdeObject *obj )
{
    _objects.push_back( obj );
}
