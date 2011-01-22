/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */

#include <iostream>
#include <Producer/CameraConfig>
#include <osgProducer/OsgCameraGroup>
#include <osgDB/ReadFile>
#include <osg/Vec3>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/StateSet>

#include <Producer/Trackball>
#include <Producer/KeyboardMouse>
#include <Producer/InputArea>

enum MotionMode{
     MoveCamera,
     MoveTransformNode,
};

static void printKeyBindings()
{
    std::cout << 
        "Key Bindings: \n"
        "   ' ' <space> - Reset Trackball position\n"
        "   '1'  - Set Trackball operational mode to Default\n"
        "   '2'  - Set Trackball operational mode to Inventor-Like\n"
        "   '3'  - Set Trackball operational mode to Performer-Like\n"
        "   'h' or '?'  - This message\n"
        "   'm'  - Toggle between Move Camera and Move Transform Matrix\n"
        "   'r'  - Set Reference Position to current state\n"
        "   'z'  - Orient Trackball Z UP\n"
        "   'y'  - Orient Trackball Y UP\n"
        "   ESC  - Quit\n"
            ;
}

class KBMCB : public Producer::KeyboardMouseCallback
{
    public:

        KBMCB(Producer::Trackball &tb) : 
            _tb(tb), 
            _mx(0.0f),
            _my(0.0f),
            _mbutton(0),
            _toggleMotionMode(false),
            _setReference(false),
            _done(false) {}


        void specialKeyPress( Producer::KeyCharacter key )
        {
            switch( key )
            {
                case Producer::KeyChar_Escape : _done = true; break;
		default : break;
            }
        }


        void keyPress( Producer::KeyCharacter key )
        {
            switch( key )
            {
                case '1': 
                    _tb.setOperationalMode( Producer::Trackball::DefaultOperationalMode ); 
                    break;

                case '2': 
                    _tb.setOperationalMode( Producer::Trackball::InventorLike ); 
                    break;

                case '3': 
                    _tb.setOperationalMode( Producer::Trackball::PerformerLike ); 
                    break;

                case ' ': 
                    _tb.reset(); 
                    break;

                case 'h':
                case '?':
                    printKeyBindings(); 
                    break;

                case 'r':
                    _setReference = true;
                    break;

                case 'm': 
                    _toggleMotionMode = true;
                    break;

                case 'z': _tb.setOrientation( Producer::Trackball::Z_UP ); break;
                case 'y': _tb.setOrientation( Producer::Trackball::Y_UP ); break;

                default: break;
            }
        }

		void mouseScroll( Producer::KeyboardMouseCallback::ScrollingMotion s) 
		{
			if( s == Producer::KeyboardMouseCallback::ScrollUp )
			    _tb.setDistance(_tb.getDistance() + _tb.getScale());
			else
			    _tb.setDistance(_tb.getDistance() - _tb.getScale());
		}


        void mouseMotion( float mx, float my )
        {
            _mx = mx;
            _my = my;
        }

        void buttonPress( float mx, float my, unsigned int mbutton )
        {
            _mx = mx;
            _my = my;
            _mbutton |= (1<<(mbutton-1));
        }

        void buttonRelease( float mx, float my, unsigned int mbutton )
        {
            _mx = mx;
            _my = my;
            _mbutton &= ~(1<<(mbutton-1));
        }

		void shutdown(){ _done = true; }

        float mx() { return _mx; }
        float my() { return _my; }
        unsigned int mbutton() { return _mbutton; }

        bool done() { return _done; }

        bool toggleMotionMode() { return checkToggle( _toggleMotionMode ); }
        bool setReference() { return checkToggle(_setReference); }

    protected: 
        virtual ~KBMCB(){}

    private:
        Producer::Trackball &_tb;
        float _mx, _my;
        unsigned int _mbutton;
        bool _toggleMotionMode;
        bool _setReference;
        bool _done;
        

    bool checkToggle( bool &toggle )
    {
        if( toggle == true )
        {
            toggle = false;
            return true;
        }
        return false;
    }


};

int main()
{
    Producer::ref_ptr<Producer::CameraConfig> cfg  = new Producer::CameraConfig;
    if( cfg->parseFile( "./oneWindow.cfg" ) == false )
    {
        std::cerr << "This test program requires a camera configuration file named \"oneWindow.cfg\"\n";
        return -1;
    }

    Producer::ref_ptr<osgProducer::OsgCameraGroup> cg = new osgProducer::OsgCameraGroup(cfg.get());

    osg::ref_ptr<osg::Node> root = osgDB::readNodeFile("cube.osg");
    if( root == NULL )
    {
        std::cerr << "This test program requires an osg file named \"cube.osg\"\n";
        return -1;
    }

    osg::ref_ptr<osg::MatrixTransform> tx = new osg::MatrixTransform;
    tx->addChild(root.get());
    cg->setSceneData(tx.get());
    cg->realize( Producer::CameraGroup::SingleThreaded );

    osg::ref_ptr<osg::StateSet> sset = new osg::StateSet;
    sset->setMode( GL_LIGHTING, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF );
    tx->setStateSet( sset.get()  );

    osg::BoundingSphere bs = root->getBound();

    Producer::ref_ptr<Producer::Trackball> tb = new Producer::Trackball;
    tb->setOrientation(Producer::Trackball::Z_UP);
    tb->setDistance(root->getBound().radius() * 3);
    // This takes a snapshot of the current state to which it
    // will return when you hit the space bar (tb.reset()).
    tb->setReference();

    /*
     * Use with InputArea
	 */
    Producer::ref_ptr<Producer::InputArea>ia = cfg->getInputArea();
    if( !ia.valid() )
    {
        std::cerr << "Please define an InputArea in the config file\n";
        return 1;
    }
    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(ia.get());
    
    Producer::ref_ptr<KBMCB> kbmcb = new KBMCB(*tb.get());
    kbm->setCallback( kbmcb.get() );
    kbm->startThread();
    MotionMode motionMode = MoveCamera;

    while( !kbmcb->done() )
    {
        cg->sync();

        if( kbmcb->setReference() )
            tb->setReference();

        tb->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );

        if( kbmcb->toggleMotionMode() )
        {
            motionMode = motionMode == MoveCamera ?
                         motionMode = MoveTransformNode :  
                         motionMode = MoveCamera ;  
            std::cout << "Motion mode is now: " << 
                    (motionMode == MoveCamera ? "MoveCamera" : "Move Transform Mode") <<
                    std::endl;

            if( motionMode == MoveCamera )
                tb->setComputeOrientation(true);
            else if( motionMode == MoveTransformNode )
                tb->setComputeOrientation(false);
        }

        if( motionMode == MoveCamera )
        {
            cg->setView(osg::Matrix(tb->getMatrix().ptr()));
            tx->setMatrix( osg::Matrix::identity() );
        }
        else if( motionMode == MoveTransformNode )
        {
            tx->setMatrix(osg::Matrix(tb->getMatrix().ptr()));
            switch( tb->getOrientation() )
            {
                case Producer::Trackball::Z_UP :
                    cg->setViewByLookat( 0, 0, 0, 
                                         0, 1, 0, 
                                         0, 0, 1 );
                    break;
                case Producer::Trackball::Y_UP:
                    cg->setViewByLookat( 0, 0,  0, 
                                         0, 0, -1, 
                                         0, 1,  0 );
                    break;
            }
        }
        cg->frame();
    }
	return 0;
}
