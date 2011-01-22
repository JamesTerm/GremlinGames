#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <GL/gl.h>
#include <iostream>

#include <OpenThreads/Thread>

#include <Producer/Types>
#include <Producer/Trackball>
#include <Producer/CameraConfig>
#include <Producer/VisualChooser>
#include <Producer/KeyboardMouse>

#include <osg/Node>
#include <osg/Group>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osgUtil/UpdateVisitor>
#include <osgDB/ReadFile>
#include <osgProducer/OsgSceneHandler>
//#include "osgSceneHandler.h"
#include "gui.h"
//#include "Thread.h"

using namespace Producer;
using namespace osg;
using namespace osgDB;
using namespace osgUtil;
using namespace osgProducer;

class TransformCB : public NodeCallback
{
    public:
	    TransformCB(Trackball &tb) : _tb(tb),a(0.0) {}
	    void  operator()(Node *node, NodeVisitor *nv )
	    {
            MatrixTransform *tx = dynamic_cast<MatrixTransform *>(node);

	        osg::Matrix mat(_tb.getMatrix().ptr() );

            tx->setMatrix( mat );
	        traverse(node,nv);
	    }

    private :
	    Trackball &_tb;
	    float a;
};


class guiThread: public OpenThreads::Thread
{
    public:
	    guiThread(int argc, char **argv, VisualChooser *vc):Thread() 
	    {
	        _argc = argc;
	        _argv = argv;
	        _vc = vc;
	        _done = false;
	    }

        class myGinitCallback : public Gui::GinitCallback
        {
    	    public :
		        myGinitCallback(): Gui::GinitCallback(), _win(0) {}
		        void operator () (Producer::Window win) { _win = win; }
                Producer::Window getWindow() { return _win; }
    	    private :
                Producer::Window _win;
	    };

	    virtual void run()
	    {
	        _mgi = new myGinitCallback;
	        _gui = new Gui;
    	    _gui->setVisualChooser( _vc );

	        for( int i = 0; i < _argc; i++ )
	        {
		        if( std::string(_argv[i]) == "--detached" )
		            _gui->setMode( Gui::Detached );
	        }
    	    _gui->init( &_argc, _argv, _mgi );
    	    _gui->mainLoop();
	        _done = true;
	    }

        Producer::Window getWindow() { return _mgi->getWindow(); }
	    bool done() { return _done; }

	    float mx() { return _gui->mx(); }
	    float my() { return _gui->my(); }
	    unsigned int buttonState() { return _gui->buttonState(); }

    private:

	    Gui *_gui;
	    VisualChooser *_vc;
	    int _argc;
	    char **_argv;
	    myGinitCallback *_mgi;
	    bool _done;
};

class myKbdMouseCallback : public KeyboardMouseCallback
{
    public :
	myKbdMouseCallback(Trackball &tb) :  KeyboardMouseCallback(), _done(false), _tb(tb) 
	{
	  _mx = _my = 0.0;
	  _button_state = 0;
	}

	virtual void keyPress( unsigned int key ) 
	{
	    if(( key & 0xFF ) == 0x1B )
		_done = true;

	    switch( key & 0xFF )
	    {
		case ' ':
		    _tb.reset();
		    break;
		case '1' :
		    _tb.setOperationalMode( Trackball::DefaultOperationalMode );
		    break;

		case '2' :
		    _tb.setOperationalMode( Trackball::InventorLike );
		    _tb.setOrientation( Trackball::Z_UP );
		    break;

		case '3' :
		    _tb.setOperationalMode( Trackball::PerformerLike );
		    break;
	    }
	}

	virtual void mouseMotion( float mx, float my )
	{
	    _mx = mx;
	    _my = my;
	}

	virtual void buttonPress( float mx, float my, unsigned int button )
	{
	    _mx = mx;
	    _my = my;
 	
	    _button_state |= (1<<(button-1));
	    _tb.input(_mx, _my, _button_state );
	}

	virtual void buttonRelease( float mx, float my, unsigned int button )
	{
	    mx = mx;
	    my = my;
	    _button_state &= ~(1<<(button-1));
	    _tb.input(_mx, _my, _button_state );
	}

	virtual bool idle(void)
	{
	    _tb.input(_mx, _my, _button_state );
	}

	bool done() { return _done; }
	void setDone( bool flag ) { _done = flag; }


    private :
	    bool _done;
	    unsigned int _button_state;
	    float _mx, _my;
	    Trackball &_tb;
};



int main(int argc, char **argv)
{
    osg::ref_ptr<CameraConfig> cfg = new CameraConfig;
    cfg->parseFile( "./test.cfg" );

    guiThread guit( argc, argv, cfg->getRenderSurface(0)->getVisualChooser());
    guit.startThread();

    osg::ref_ptr<Trackball> tb = new Trackball;
    tb->setComputeOrientation(false);

    MatrixTransform *transform = new MatrixTransform;
    transform->setUpdateCallback( new TransformCB(*tb) );

    Group *group = new Group;
    transform->addChild( group );

    Gui::Mode mode = Gui::Embedded;
    // Load files listed on the command line and populate
    // the Group node at the top of the scene graph
    for( int i = 1; i < argc; i++ )
    {
	    if( std::string(argv[i]) == "--detached" )
	    {
	        mode = Gui::Detached;
	        continue;
	    }
        Node *node = osgDB::readNodeFile( argv[i] );
	    if( node != NULL )
	        group->addChild( node );
	    else
	        std::cerr << "Unable to load \"" << argv[i] << "\"." << std::endl;
    }

    if( group->getNumChildren() == 0 )
    {
        std::cerr << "Usage : " << argv[0] << " [--detached] <model.osg> ..." << std::endl;
	    return -1;
    }

    KeyboardMouse *km;
    myKbdMouseCallback *cb;

    Producer::Camera *cam = cfg->getCamera(0);
    if( mode == Gui::Embedded )
    {
        Producer::Window win;
        while( (win = guit.getWindow()) == 0 )
	    usleep(50000);
        cam->getRenderSurface()->setWindow( win );
    }
    else
    {
	    cam->getRenderSurface()->realize();
        km = new KeyboardMouse( cam->getRenderSurface() ); 
	    cb = new myKbdMouseCallback(*tb);
    }

    OsgSceneHandler *sh = new OsgSceneHandler;
    sh->getSceneView()->setDefaults();
    sh->getSceneView()->setSceneData( transform );

    cam->setSceneHandler( sh );

    BoundingSphere bs = transform->getBound();
    cam->setViewByLookat( 
            bs.center()[0], bs.center()[1] - (bs.radius()*2), bs.center()[2],
		    bs.center()[0], bs.center()[1], bs.center()[2],
		    0, 0, 1);
    UpdateVisitor update;
    while( !guit.done() )
    {
	    if( mode == Gui::Embedded )
	        tb->input( guit.mx(), guit.my(), guit.buttonState());
	    else
	        km->update( *cb );
	    transform->accept( update );
	    cam->frame();
    }
    guit.join();
    return 0;
}

