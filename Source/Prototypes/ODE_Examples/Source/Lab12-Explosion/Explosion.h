#ifndef SGI_DEMO_EXPLOSION_H
#define SGI_DEMO_EXPLOSION_H

#include <osg/Group>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <osgParticle/ModularEmitter>


class Explosion : public osg::Group
{
    public:
        Explosion();

        void reset();
        void fire( double startTime ) ;
        void startFireEmitter();
        void startSmokeEmitter();

    protected:

        osgParticle::ModularEmitter *_fireEmitter;
        osgParticle::ModularEmitter *_smokeEmitter;

        class ExplosionCallback : public osg::NodeCallback
        {
            public:
                ExplosionCallback( Explosion *explosion, double startTime );
                void operator()(osg::Node *node, osg::NodeVisitor *nv );
            private:
                enum State {
                    Idle,
                    FireStarted,
                    SimStarted,
                    SmokeStarted
                };

                State _state;
                osg::ref_ptr<Explosion> _explosion;
                double _startTime;
        };
};
#endif 
