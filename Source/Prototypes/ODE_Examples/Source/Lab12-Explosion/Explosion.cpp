#include <osgDB/WriteFile>
#include <osg/Group>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osg/NodeCallback>

#include <osgParticle/Particle>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/RadialShooter>
#include <osgParticle/AccelOperator>
#include <osgParticle/FluidFrictionOperator>

#include "Explosion.h"

Explosion::Explosion()
{
	osg::Geode *geode = new osg::Geode;
    osgParticle::ParticleSystemUpdater *psu = new osgParticle::ParticleSystemUpdater;
    // Fire
    {
	    osgParticle::Particle ptemplate;
	    ptemplate.setLifeTime(1.0);
        ptemplate.setShape( osgParticle::Particle::QUAD );
	    ptemplate.setSizeRange(osgParticle::rangef(1.0f, 8.0f));
	    ptemplate.setAlphaRange(osgParticle::rangef(1.0f, 0.0f));
	    ptemplate.setColorRange(osgParticle::rangev4( osg::Vec4(1.0, 0.5, 0.0, 1),
		                                              osg::Vec4(1, 1, 1, 1 )));
        ptemplate.setPosition( osg::Vec3(0,0,0));
        ptemplate.setVelocity( osg::Vec3(0,0,0));
	    ptemplate.setMass(0.1f);	
	    ptemplate.setRadius(0.2f);	

	    osgParticle::ParticleSystem *ps = new osgParticle::ParticleSystem;
	    ps->setDefaultAttributes("smoke.rgb", true, false);
        psu->addParticleSystem( ps );

	    geode->addDrawable(ps);

        _fireEmitter = new osgParticle::ModularEmitter;
        _fireEmitter->setParticleSystem(ps);
	    _fireEmitter->setParticleTemplate(ptemplate);
        _fireEmitter->setEndless( false );
        _fireEmitter->setLifeTime( 1.5 );

	    osgParticle::RandomRateCounter *counter = new osgParticle::RandomRateCounter;
	    counter->setRateRange(10, 200);
	    _fireEmitter->setCounter(counter);
        _fireEmitter->setEnabled( false );

	    osgParticle::RadialShooter *shooter = new osgParticle::RadialShooter;
	    shooter->setInitialSpeedRange(1.0, 3.0);
        shooter->setThetaRange(0, 2* osg::PI );
        shooter->setPhiRange(0, 2 * osg::PI);
	    _fireEmitter->setShooter(shooter);

	    addChild(_fireEmitter);

    }

    // Smoke
    {
	    osgParticle::Particle ptemplate;
	    ptemplate.setLifeTime(20.0);
        ptemplate.setShape( osgParticle::Particle::QUAD );
	    ptemplate.setSizeRange(osgParticle::rangef(3.0f, 12.0f));
	    ptemplate.setAlphaRange(osgParticle::rangef(1.0f, 0.0f));
	    ptemplate.setColorRange(osgParticle::rangev4( osg::Vec4(0.1, 0.1, 0.1, 0.5),
		                                              osg::Vec4(1, 1, 1, 1.5 )));
        ptemplate.setPosition( osg::Vec3(0,0,0));
        ptemplate.setVelocity( osg::Vec3(0,0,0));
	    ptemplate.setMass(0.1f);	
	    ptemplate.setRadius(0.2f);	

	    osgParticle::ParticleSystem *ps = new osgParticle::ParticleSystem;
	    ps->setDefaultAttributes("smoke.rgb", false, false);
        psu->addParticleSystem( ps );

	    geode->addDrawable(ps);

        _smokeEmitter = new osgParticle::ModularEmitter;
        _smokeEmitter->setParticleSystem(ps);
	    _smokeEmitter->setParticleTemplate(ptemplate);
        _smokeEmitter->setEndless( false );
        _smokeEmitter->setLifeTime( 1.5 );

	    osgParticle::RandomRateCounter *counter = new osgParticle::RandomRateCounter;
	    counter->setRateRange(10, 200);
	    _smokeEmitter->setCounter(counter);
        _smokeEmitter->setEnabled( false );

	    osgParticle::RadialShooter *shooter = new osgParticle::RadialShooter;
	    shooter->setInitialSpeedRange(2.0, 5.0);
        shooter->setThetaRange(0, 2* osg::PI );
        shooter->setPhiRange(0, 2 * osg::PI);
	    _smokeEmitter->setShooter(shooter);


        osgParticle::SectorPlacer *placer = new osgParticle::SectorPlacer;
        placer->setCenter(0, 0, 1);
        placer->setRadiusRange(0.5, 5);
        placer->setPhiRange(0, 2 * osg::PI);    // 360° angle to make a circle
        _smokeEmitter->setPlacer(placer);


	    addChild(_smokeEmitter);
    }

    addChild( psu );
    osg::MatrixTransform *tx = new osg::MatrixTransform;
    tx->setMatrix( osg::Matrix::translate( 0, 0, 1.0 ));
    tx->addChild( geode );
     addChild(tx);

}

void Explosion::reset()
{
    //ss->pushKey (SIM_KEYCODE_RESET);
}

void Explosion::fire( double startTime ) 
{ 
    setUpdateCallback( new ExplosionCallback(this, startTime ));
	
    //ss->control().lastExplosion = startTime;
}

void Explosion::startFireEmitter()
{
    _fireEmitter->setEnabled( true );
}

void Explosion::startSmokeEmitter()
{
    _smokeEmitter->setEnabled( true );
}

Explosion::ExplosionCallback::ExplosionCallback( Explosion *explosion, double startTime ):
    _state(Idle),
    _explosion(explosion),
    _startTime(startTime)
    {}

void Explosion::ExplosionCallback::operator()(osg::Node *node, osg::NodeVisitor *nv )
{
    const osg::FrameStamp *fs = nv->getFrameStamp();
    double now = fs->getReferenceTime();

    switch( _state )
    {
        case Idle:
            _explosion->startFireEmitter();
            _state = FireStarted;
            break;

        case FireStarted:

            if( now > _startTime + 0.15 )
            {
                system( "/usr/bin/esdplay explosion.wav" );
                _state = SimStarted;
            }

        case SimStarted:

            if( now > _startTime + 2.0 )
            {
                _explosion->startSmokeEmitter();
                _state = SmokeStarted;
            }
            break;

        case SmokeStarted:
            //_explosion->setUpdateCallback( 0L );
            break;
    }
    traverse( node, nv );
}


