#include <deque>

#include <Producer/Camera>
#include <osgProducer/OsgSceneHandler>

#include <osgDB/ReadFile>
#include <osg/Node>
#include <osg/Vec3>

Producer::Camera *makeACamera(Producer::RenderSurface *rs, unsigned int prindex , osg::Node *scene)
{
    float prVals[][4] = {
        // left, right, bottom, top
        { 0.05, 0.45, 0.05, 0.45 },
        { 0.55, 0.95, 0.05, 0.45 },
        { 0.05, 0.45, 0.55, 0.95 },
        { 0.55, 0.95, 0.55, 0.95 },
    };

    // Create a Producer Camera and pass it the scene handler
    Producer::Camera *camera = new Producer::Camera;

    // If a REnderSurface was passed, set the camera's rendersurface to this
    if( rs != 0L )
        camera->setRenderSurface( rs );

    // Set the projection rectangle by the index 'prindex' passed in 
    camera->setProjectionRectangle( prVals[prindex][0], prVals[prindex][1], prVals[prindex][2], prVals[prindex][3] );

    // Set up the Scene Handler
    Producer::ref_ptr<osgProducer::OsgSceneHandler> sceneHandler = new osgProducer::OsgSceneHandler;
    sceneHandler->getSceneView()->setDefaults();
    sceneHandler->getSceneView()->setSceneData( scene );
    camera->setSceneHandler( sceneHandler.get() );

    return camera;
}


main(int argc, char **argv)
{
    // Load your OSG database
    osg::ref_ptr<osg::Node> cow = osgDB::readNodeFile( "cow.osg" );
    if( !cow.valid() )
    {
        std::cerr << "\n\tOoops, this application needs the \"cow.osg\" model.\n\n";
        return 1;
    }

    // Keep a queue of camera's
    std::deque <Producer::ref_ptr<Producer::Camera> > cameras;

    // Create the first three.  Use the First camera's render surface for all 
    // other cameras.
    int prindex = 0;
    cameras.push_back( makeACamera( 0L, ((prindex++)%4), cow.get() ));
    Producer::ref_ptr< Producer::RenderSurface> rs = cameras.front()->getRenderSurface();

    cameras.push_back( makeACamera( rs.get(), ((prindex++)%4), cow.get() ));
    cameras.push_back( makeACamera( rs.get(), ((prindex++)%4), cow.get() ));

    // optional - so RenderSurface is not full screen
    rs->setWindowRectangle( 0, 0, 800, 600 );

    // Camera manipulation stuff
    osg::BoundingSphere bs = cow->getBound();
    osg::Vec3 center = bs.center();
    float radius = bs.radius() * 3;
    float viewingAngle = 0.0;

    // we need one more camera for just clearing the screen.
    // This is normally not necessary, but peculiar to this example.
    Producer::ref_ptr<Producer::Camera> clearCamera = new Producer::Camera;
    clearCamera->setRenderSurface(rs.get());
    clearCamera->setClearColor( 0, 0, 0, 1 );

    unsigned int frameCount = 0;
    do {
        // Clear screen... see note above regarding clear camera
        clearCamera->clear();

        // Rotate around the cow...
        viewingAngle += osg::DegreesToRadians(1.0);
        osg::Vec3 eye = center + osg::Vec3( radius * cos(viewingAngle),
                                            radius * sin(viewingAngle),
                                            0);

        std::deque <Producer::ref_ptr<Producer::Camera> >::iterator p;
        for( p = cameras.begin(); p != cameras.end(); p++ )
        {
            (*p)->setViewByLookat( eye[0], eye[1], eye[2],
                                   center[0], center[1], center[2],
                                   0, 0, 1 );

            // Send false so buffers aren't swapped
            (*p)->frame(false);
        }
        // Explicitely swap buffers here
        rs->swapBuffers();


        // Every 120 frames, delete one camera and add a new one.

        if( !(++frameCount % 120) )
        {
            cameras.pop_front();
            cameras.push_back( makeACamera( rs.get(), ((prindex++)%4), cow.get() ));
        }
    }while( rs->isRealized());
}
