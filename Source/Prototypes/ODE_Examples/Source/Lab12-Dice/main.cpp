#include <iostream>
#include <Producer/Camera>
#include <Producer/KeyboardMouse>
#include <Producer/Trackball>
#include <osgProducer/OsgSceneHandler>
#include <osgDB/ReadFile>

#include <osg/Timer>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/NodeCallback>
#include <osg/LightModel>
#include <osg/Texture2D>
#include <osg/TexGenNode>
#include <osg/PolygonOffset>
#include <osg/CullFace>

#include <osg/Geometry>

#include <osgUtil/SceneView>
#include <osgUtil/RenderStage>


#include "KeyboardMouseCallback.h"
#include <osgUtil/UpdateVisitor>
#include "OdeWorld.h"
#include "RigidBody.h"
#include "PBufferTexture2D.h"
#include "Projector.h"

//The shadows are somewhat fudged since RenderToTextureStage is no longer a part of OSG... the shadows work but produce OpenGL error warnings, so I am
//disabling these by default to have a fully functional demo with no errors
// -James [10/21/2007]
#undef __UseShadows__

class DiceBody: public RigidBody
{
    public:
        DiceBody( OdeWorld &odeWorld ) : RigidBody(odeWorld) 
        {
            _body = dBodyCreate (odeWorld.getWorld());
            dBodySetPosition (_body, -35.0 + float(rand()%70), 
                                     -35.0 + float(rand()%70), 
                                     -35.0 + float(rand()%70));
                                                                                                      
            dMass m;
            dMassSetBox(&m, 1., 8.0, 8.0, 8.0);
            //dMassAdjust(&m,0.01);

            dBodySetMass(_body,&m);
                                                                                                      
            dGeomID box = dCreateBox(odeWorld.getSpace(),8.0, 8.0, 8.0 );
            dGeomSetBody (box,_body);
        }

        virtual void init(OdeWorld &) {}

};

class DiceUpdateCallback :  public osg::NodeCallback 
{
    public:
        DiceUpdateCallback( DiceBody &dice ): _dice(dice) {}

        void operator() (osg::Node *node, osg::NodeVisitor *nv )
        {
            osg::MatrixTransform *tx = dynamic_cast<osg::MatrixTransform *>(node);
            if( tx != 0L )
            {

                osg::Matrix m = _dice.getMatrix();
                tx->setMatrix( m );
            }
            traverse( node, nv );
        }

    private:
        DiceBody &_dice;
};

class Walls : public RigidBody
{
    public:
        Walls( OdeWorld &odeWorld ): RigidBody(odeWorld)
        {
            planes[0].set(  0.0,  0.0,  1.0, -40.0 );
            planes[1].set(  1.0,  0.0,  0.0, -40.0 );
            planes[2].set(  0.0,  0.0, -1.0, -40.0 );
            planes[3].set( -1.0,  0.0,  0.0, -40.0 );
            planes[4].set(  0.0, -1.0,  0.0, -40.0 );
            planes[5].set(  0.0,  1.0,  0.0, -40.0 );

            for( int i = 0; i < 6; i++ )
                _planeID[i] = dCreatePlane( odeWorld.getSpace(), planes[i][0], planes[i][1], planes[i][2], planes[i][3] );
        }

        void reOrient( osg::Matrix m )
        {
            for( int i = 0; i < 6; i++ )
            {
                osg::Vec3 v = osg::Vec3(planes[i][0], planes[i][1], planes[i][2]) * m;
                dGeomPlaneSetParams( _planeID[i], v[0], v[1], v[2], -40.0);
            }
        }

        virtual void init(OdeWorld &) {}

    private:

        osg::Plane planes[6];
        dGeomID _planeID[6];
};


class WallsUpdateCallback : public osg::NodeCallback
{
    public:
        WallsUpdateCallback( Walls *walls, Producer::Trackball *trackball ):
            _walls(walls),
            _trackball(trackball)
            {}

        void operator () (osg::Node *node, osg::NodeVisitor *nv )
        {
            osg::MatrixTransform *tx = dynamic_cast<osg::MatrixTransform *>(node);
            if( tx != 0L )
            {
                osg::Matrix mat = osg::Matrix(_trackball->getMatrix().ptr());
                tx->setMatrix( mat );
                _walls->reOrient(mat);
            }
            traverse( node, nv );
        }

    private:
        osg::ref_ptr<Walls> _walls;
        Producer::ref_ptr<Producer::Trackball> _trackball;
};


osg::Group *justTheDice;
osg::Node *justTheWalls;

osg::Node *makeScene( OdeWorld &odeWorld, Producer::Trackball *tb )
{
    char *diceNames[] = {
        "Models/redDice.osg",
        "Models/greenDice.osg",
        "Models/magentaDice.osg",
        "Models/yellowDice.osg",
        "Models/whiteDice.osg",
        "Models/blueDice.osg",
        "Models/cyanDice.osg",
        "Models/blackDice.osg",
        0L
    };
    osg::ref_ptr<osg::Group> dice = new osg::Group;

justTheDice = dice.get();

    char **ptr = diceNames;
    while( *ptr )
    {
        osg::ref_ptr<osg::MatrixTransform>tx = new osg::MatrixTransform;
        osg::ref_ptr<osg::Node> node = osgDB::readNodeFile( *ptr );
        if( node.valid() )
        {
            tx->addChild( node.get() );
            tx->setUpdateCallback( new DiceUpdateCallback(*(new DiceBody( odeWorld ))));
            dice->addChild( tx.get() );
        }
        else
        {
            std::cerr << "Ooops.  This demo need the model \"" << (*ptr) << "\"." << std::endl;
            return 0L;
        }
        ptr++;
    }

    osg::ref_ptr<Walls> walls = new Walls(odeWorld);
    osg::ref_ptr<osg::MatrixTransform> wallsTx = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile( "Models/walls.osg" );
justTheWalls = node.get();
    if( node.valid() )
    {
        wallsTx->addChild( node.get());
        wallsTx->setUpdateCallback( new WallsUpdateCallback( walls.get(), tb ));
    }
    else
    {
        std::cerr << "Ooops.  This demo need the model \"Models/walls.osg\"." << std::endl;
        return 0L;
    }

    osg::Group *group = new osg::Group;

    group->addChild( dice.get() );

    group->addChild( wallsTx.get() );

    osg::StateSet *sset = group->getOrCreateStateSet();
    osg::LightModel *lm = new osg::LightModel;
    lm->setAmbientIntensity( osg::Vec4( 0.2, 0.2, 0.2, 1));
    sset->setAttributeAndModes( lm );

    return group;
}

class LightTXCallback : public osg::NodeCallback
{
    public:
        LightTXCallback(): 
            a(0.0), 
            r(0.0),
            ainc(0.15)
            {}

        virtual void operator()( osg::Node *node, osg::NodeVisitor *nv )
        {
            osg::MatrixTransform *tx = dynamic_cast<osg::MatrixTransform *>(node);
            if( tx != 0L )
            {
                if( a < -45.0 || a > 45.0 ) 
                    ainc *= -1;
                a += ainc;
                r += osg::DegreesToRadians(1.0);

                float x = 40.0 * cos(r);
                float y = 40.0 * sin(r);

//a = r = 0.0;
                tx->setMatrix(  osg::Matrix::rotate( osg::DegreesToRadians(a), 0, 1, 0 ) *
                                osg::Matrix::rotate( osg::DegreesToRadians(r), 0, 0, 1 ) *
                                osg::Matrix::translate( 0.0, 0.0, 40.0 ));
            }
            traverse( node, nv );
        }
    private:
        float a;
        float r;
        float ainc;
};



class TextureDebugger: public osg::Projection 
{
    public:

        TextureDebugger(osg::Texture2D *tex)
        {
            setMatrix(osg::Matrix::ortho2D(0.0, 1.0, 0.0, 1.0 ));
            osg::ref_ptr<osg::MatrixTransform> mva = new osg::MatrixTransform;
            mva->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
            mva->setMatrix(osg::Matrix::identity());
            addChild( mva.get());

            osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
            osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;

            coords->push_back( osg::Vec3( 0.0, 0.0, 0.0 ));
            coords->push_back( osg::Vec3( 0.2, 0.0, 0.0 ));
            coords->push_back( osg::Vec3( 0.2, 0.2, 0.0 ));
            coords->push_back( osg::Vec3( 0.0, 0.2, 0.0 ));

            tcoords->push_back( osg::Vec2( 0.0, 0.0 ));
            tcoords->push_back( osg::Vec2( 1.0, 0.0 ));
            tcoords->push_back( osg::Vec2( 1.0, 1.0 ));
            tcoords->push_back( osg::Vec2( 0.0, 1.0 ));

            osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
            geom->setVertexArray(coords.get());
            geom->setTexCoordArray( 0, tcoords.get() );
            geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, coords->size()));


            osg::ref_ptr<osg::StateSet> sset = new osg::StateSet;
            sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );

            sset->setTextureAttributeAndModes( 0, tex );
            sset->setTextureMode( 1, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            sset->setTextureMode( 1, GL_TEXTURE_GEN_S, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            sset->setTextureMode( 1, GL_TEXTURE_GEN_T, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            sset->setTextureMode( 2, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            sset->setTextureMode( 2, GL_TEXTURE_GEN_S, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            sset->setTextureMode( 2, GL_TEXTURE_GEN_T, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
            sset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );

            sset->setRenderBinDetails(110,"RenderBin");
            geom->setStateSet( sset.get() );

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable( geom.get() );

            mva->addChild( geode.get() );
        }
};

class PreDrawRangeInverterCB : public Producer::Camera::Callback
{
    public:
        PreDrawRangeInverterCB(bool flag):_flag(flag) {}

        virtual void operator()( const Producer::Camera & )
        {
            if( _flag )
                glDepthRange( 0.7, 0.3 );
            else
                glDepthRange( 0.0, 1.0 );
        }

    private:
        bool _flag;

};


int main( int argc, char **argv )
{
    srand(time(0L));

    OdeWorld *odeWorld = new OdeWorld;
    odeWorld->setGroundPlane( osg::Plane(0.0, 0.0, 1.0, -100.0) );

    // Set up the main camera
    Producer::ref_ptr<Producer::Camera> camera = new Producer::Camera;
    camera->getRenderSurface()->setWindowRectangle( 0, 0, 1280, 1024 );
    camera->getRenderSurface()->setWindowName( "OpenSceneGraph & Open Dynamics Engine" );

    // Set up the visual
    Producer::ref_ptr<Producer::VisualChooser> vc = new Producer::VisualChooser;
    vc->addAttribute(Producer::VisualChooser::RGBA);
    vc->addAttribute(Producer::VisualChooser::DoubleBuffer);
    vc->addAttribute(Producer::VisualChooser::DepthSize, 24);
    vc->addAttribute(Producer::VisualChooser::RedSize, 8);
    vc->addAttribute(Producer::VisualChooser::GreenSize, 8);
    vc->addAttribute(Producer::VisualChooser::BlueSize, 8);
    //vc->addAttribute(Producer::VisualChooser::AlphaSize, 8);
    //vc->addAttribute(VisualChooser::StencilSize, 8);
    //vc->addAttribute(VisualChooser::Stereo);
#if defined( GLX_SAMPLES_ARB )
   vc->addExtendedAttribute(GLX_SAMPLES_ARB, 4);
#endif
#if defined( GLX_SAMPLES_BUFFER_ARB )
    vc->addExtendedAttribute(GLX_SAMPLES_BUFFER_ARB, 1);
#endif
    camera->getRenderSurface()->setVisualChooser( vc.get() );


    // Set up the Keyboard and  Mouse
    Producer::ref_ptr<Producer::KeyboardMouse> kbm = new Producer::KeyboardMouse(camera->getRenderSurface());
    Producer::ref_ptr<ExampleKeyboardMouseCallback> kbmcb = new ExampleKeyboardMouseCallback;
    kbm->setCallback( kbmcb.get());
    kbm->startThread();

    // Set up the trackball
    Producer::ref_ptr<Producer::Trackball> trackball = new Producer::Trackball;
    trackball->setComputeOrientation(false);
    trackball->disableDistancing();
    trackball->disablePanning();


    // Set up the scene hander and scene
    osg::ref_ptr<osgProducer::OsgSceneHandler> sceneHandler = new osgProducer::OsgSceneHandler;
    //sceneHandler->getSceneView()->setComputeNearFarMode( osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR );
    sceneHandler->getSceneView()->setDefaults();


    /////////////////////////////////////////////////////////
    //                   root
    //                    |
    //    --------------------------------
    //    |                              |
    //   scene                        lightx
    //                                   |
    //                                texgenNode

    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osg::Node> scene = makeScene(*odeWorld, trackball.get());
    if( !scene.valid() )
        return 1;

    root->addChild( scene.get() );

    // 0. Add light model
    osg::ref_ptr<osg::MatrixTransform> lightx = new osg::MatrixTransform;
    osg::ref_ptr<osg::Node> lightModel = osgDB::readNodeFile( "Models/light.osg" );
    lightx->addChild( lightModel.get() );

    ///////////////////////////////////////////////////////////////////////////////////////
    lightx->setCullingActive( false );
    //lightx->setUpdateCallback( new LightTXCallback(&lightMatrix) );
    lightx->setUpdateCallback( new LightTXCallback );
    root->addChild( lightx.get() );


#if 1 // 1. add OpenGL Spot Light
    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
    lightx->addChild( ls.get() );

    osg::ref_ptr<osg::Light> light = ls->getLight();
	const int LN=1;
    light->setLightNum( LN );
    light->setPosition(osg::Vec4( 0.0, 0.0, 0.0, 1.0f));
    light->setDirection(osg::Vec3( 0.0f, 0.0f, -1.0f));
    light->setAmbient(osg::Vec4(0.4f,0.4f,0.4f,1.0f));
    light->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    light->setSpotCutoff(40.0f);
    light->setSpotExponent(10.0f);
	if (LN!=0)
	{
		//Turn on the lights!
		ls->setLocalStateSetModes(osg::StateAttribute::ON);
		//This light source needs the root node's state set, which by default is not created.  
		//This way all children know that this light source will affect them
		osg::StateSet *ss=root->getOrCreateStateSet();
		ls->setStateSetModes(*ss,osg::StateAttribute::ON);
	}

#endif



#if 1 // 2. add projected Texture Light
    lightx->addChild( new Projector( 2, osg::DegreesToRadians(90.0) ));

    osg::ref_ptr<osg::Image> spotImage = osgDB::readImageFile( "spot.rgba" );
    osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
    tex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP);
    tex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP);
    tex->setImage( spotImage.get() );

    osg::StateSet *sset = scene->getOrCreateStateSet();
    sset->setTextureAttributeAndModes( 1, tex.get() ); 
    sset->setTextureMode( 1, GL_TEXTURE_2D,    osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 1, GL_TEXTURE_GEN_S, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 1, GL_TEXTURE_GEN_T, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 1, GL_TEXTURE_GEN_R, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 1, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
#endif


    osg::Group *theDice = new osg::Group;
    theDice->addChild( justTheDice );

#if 0
    {


        osg::ref_ptr<osg::PolygonOffset> poff = new osg::PolygonOffset;
        poff->setFactor(-1.1f);
        poff->setUnits(-4.0f);
        osg::StateSet *ss = theDice->getOrCreateStateSet();
        ss->setAttribute(poff.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        ss->setMode(GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

        osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
        cf->setMode(osg::CullFace::FRONT);
        ss->setAttribute(cf.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        ss->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    }
#endif

#ifdef __UseShadows__ // Add shadow - Create Pbuffer
    // PBuffer
    //////////////////////////////////////////////////////////////
    Producer::ref_ptr<Producer::Camera> pbufferCamera = new Producer::Camera;
    pbufferCamera->getRenderSurface()->setDrawableType( Producer::RenderSurface::DrawableType_PBuffer );
    pbufferCamera->getRenderSurface()->setRenderToTextureMode(Producer::RenderSurface::RenderToRGBATexture);
    pbufferCamera->getRenderSurface()->setWindowRectangle( 0, 0, 512, 512 );
    osg::ref_ptr<osgProducer::OsgSceneHandler> pbSceneHandler = new osgProducer::OsgSceneHandler;
    pbSceneHandler->getSceneView()->setDefaults();
    pbSceneHandler->getSceneView()->setSceneData( theDice );
    pbSceneHandler->getSceneView()->setComputeNearFarMode( osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR );
    pbufferCamera->setSceneHandler( pbSceneHandler.get() );
    pbufferCamera->setLensPerspective( 90.0, 90.0, 30.0, 300.0 );
    pbufferCamera->setClearColor( 1.0, 1.0, 1.0, 1.0 );

    pbufferCamera->addPreDrawCallback( new PreDrawRangeInverterCB( true ) );
    //////////////////////////////////////////////////////////////

    // Create shadow texture
    osg::ref_ptr<PBufferTexture2D> shadowTexture = new PBufferTexture2D(pbufferCamera->getRenderSurface());

    shadowTexture->setInternalFormat(GL_DEPTH_COMPONENT);
    shadowTexture->setShadowComparison(false);
    shadowTexture->setShadowTextureMode(osg::Texture::LUMINANCE);

    shadowTexture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    shadowTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    shadowTexture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    shadowTexture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    shadowTexture->setBorderColor( osg::Vec4( 1, 1, 1, 1 ));

    // Set the shadow texture to the walls' stateset
    sset = justTheWalls->getOrCreateStateSet();
    //sset = root->getOrCreateStateSet();
    sset->setTextureAttributeAndModes( 2, shadowTexture.get() ); 
    sset->setTextureMode( 2, GL_TEXTURE_2D,    osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 2, GL_TEXTURE_GEN_S, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 2, GL_TEXTURE_GEN_T, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 2, GL_TEXTURE_GEN_R, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    sset->setTextureMode( 2, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

    camera->getRenderSurface()->setReadDrawable( pbufferCamera->getRenderSurface());
    //camera->addPreDrawCallback( new PreDrawRangeInverterCB( false ) );
#endif


    // FOR DEBUG
    //root->addChild( new TextureDebugger( shadowTexture.get() ));

    /////////////////////////////////////////////////

    sceneHandler->getSceneView()->setSceneData( root.get() );
    camera->setSceneHandler( sceneHandler.get() );
    //camera->setLensPerspective( 50.0, 50.0, 30.0, 300.0 );
    camera->setViewByLookat( 0.0, -220.0, 0.0,
                             0.0, 0.0, 0.0,
                             0.0, 0.0, 1.0 );

#ifdef __UseShadows__
    // Shadow - PBuffer
    pbufferCamera->frame();
#endif
    camera->frame();

    // Finally, set up the update visitor and FrameStamp
    osgUtil::UpdateVisitor update;
    osg::Timer timer;
    osg::Timer_t t0  = timer.tick();

    // Hack for a bug in OSG.
	//This appears no longer be an issue (but causes it to look odd, so I've removed the hack)
	// James [10/21/2007]
/*
    {
        //pbufferCamera->frame();
        pbSceneHandler->getSceneView()->setDrawBufferValue(GL_FRONT);
        //camera->frame();
        sceneHandler->getSceneView()->setDrawBufferValue(GL_FRONT);
    }
*/
    while( !kbmcb->done() )
    {
        osg::Timer_t t1 = timer.tick();
        double dt = timer.delta_s(t0, t1 );
        t0 = t1;

		//While the spec shows using time in seconds here... using all correct settings everything appears slow... so I have tweaked it to look the
		//right speed
        odeWorld->update( dt*10.0 );
        root->accept( update );
        
        if( kbmcb->resetTrackball() )
            trackball->reset();
        trackball->input( kbmcb->mx(), kbmcb->my(), kbmcb->mbutton() );

#ifdef __UseShadows__
        // PBuffer
        ///////////////////////////////////////////
        {
            osg::Matrix mi;
            mi.invert( lightx->getMatrix() );
            pbufferCamera->setViewByMatrix( Producer::Matrix( mi.ptr()));
            pbufferCamera->frame();
        }
        ///////////////////////////////////////////
#endif
        camera->frame();
    }
}
