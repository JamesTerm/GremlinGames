#include <stdio.h>
#include <unistd.h>
#include <iostream>

#include <OpenThreads/Thread>

#include <Producer/Camera>
#include <Producer/Trackball>
#include <osgProducer/OsgSceneHandler>

#include <osg/Node>
#include <osg/MatrixTransform>
#include <osgUtil/UpdateVisitor>
#include <osgDB/ReadFile>

#include "gui.h"

using namespace Producer;
using namespace osg;
using namespace osgProducer;

class TransformCB : public NodeCallback
{
    public:
    TransformCB(Trackball &tb) : _tb(tb) {}
    void  operator()(Node *node, NodeVisitor *nv )
    {
        MatrixTransform *tx = dynamic_cast<MatrixTransform *>(node);

        osg::Matrix mat(_tb.getMatrix().ptr() );
        tx->setMatrix( mat );
        traverse(node,nv);
    }

    private :
        Trackball &_tb;
};

class renderThread : public OpenThreads::Thread
{
    public:
        renderThread( Gui *gui, osg::Node *scene ) : Thread(), _gui(gui), _scene(scene) 
        {
           _done = false;
        }

        virtual void run()
        {
            osg::ref_ptr<Producer::Camera> cam = new Producer::Camera;
            while( (_win=_gui->getRenderWindow()) == 0 )
                usleep(50000);
            cam->getRenderSurface()->setWindow( _win );
            cam->getRenderSurface()->realize();
            osg::ref_ptr<osgProducer::OsgSceneHandler> sh = new osgProducer::OsgSceneHandler;
            sh->getSceneView()->setDefaults();
            sh->getSceneView()->setSceneData( _scene );

            cam->setSceneHandler( sh.get() );

            osg::BoundingSphere bs = sh->getSceneView()->getSceneData()->getBound();
            cam->setViewByLookat( 
                        bs.center()[0], bs.center()[1] - (bs.radius()*3), bs.center()[2],
                        bs.center()[0], bs.center()[1], bs.center()[2],
                        0, 0, 1);
            osgUtil::UpdateVisitor update;

            osg::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
            tb->setComputeOrientation(false);

            MatrixTransform *tx = dynamic_cast<MatrixTransform *>(sh->getSceneView()->getSceneData());

            if( tx != NULL )
                tx->setUpdateCallback( new TransformCB(*tb) );
            while( !_done )
            {
                sh->getSceneView()->getSceneData()->accept( update );
                cam->frame();
                tb->input( _gui->mx(), _gui->my(), _gui->buttonState() );
            }
        }

        void setDone() { _done = true; }

    private:
        Producer::Window _win;
        Gui *_gui;
        osg::Node *_scene;
        bool _done;
};


Node *loadDatabase( int argc, char **argv )
{
    MatrixTransform *transform = new MatrixTransform;

    Group *group = new Group;
    transform->addChild( group );

    // Load files listed on the command line and populate
    // the Group node at the top of the scene graph
    for( int i = 1; i < argc; i++ )
    {
        Node *node = osgDB::readNodeFile( argv[i] );
        if( node != NULL )
            group->addChild( node );
        else
            std::cerr << "Unable to load \"" << argv[i] << "\"." << std::endl;
    }

    if( group->getNumChildren() == 0 )
    {
        std::cerr << "Usage : " << argv[0] << " <model.osg> ..." << std::endl;
        exit(1);
    }

    return transform;
}

main(int argc, char **argv )
{
    Gui *gui = new Gui;
    gui->init( &argc, argv);
    renderThread rt(gui, loadDatabase( argc, argv ));
    // Set the render thread to run in the background
    rt.startThread();

    // The GUI will run in the foreground
    gui->mainLoop();

    rt.setDone();
    rt.join();
}
