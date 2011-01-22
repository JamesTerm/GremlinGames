#include <Producer/Camera>
#include <osgProducer/OsgSceneHandler>

#include <osgDB/ReadFile>
#include <osg/Node>
#include <osg/Vec3>


int main(int argc, char **argv)
{
    // Load your OSG database
    osg::ref_ptr<osg::Node> cow = osgDB::readNodeFile( "cow.osg" );
    if( !cow.valid() )
    {
        std::cerr << "\n\tOoops, this application needs the \"cow.osg\" model.\n\n";
        return 1;
    }

    // Set up the Scene Handler
    Producer::ref_ptr<osgProducer::OsgSceneHandler> sceneHandler = new osgProducer::OsgSceneHandler;
    sceneHandler->getSceneView()->setDefaults();
    sceneHandler->getSceneView()->setSceneData( cow.get() );

    // Create a Producer Camera and pass it the scene handler
    Producer::ref_ptr<Producer::Camera> camera = new Producer::Camera;
    // Optional
    camera->getRenderSurface()->setWindowRectangle( 100, 100, 800, 600 );
    camera->setSceneHandler( sceneHandler.get() );

    // Camera manipulation stuff
    osg::BoundingSphere bs = cow->getBound();
    osg::Vec3 center = bs.center();
    float radius = bs.radius() * 3;
    float viewingAngle = 0.0;

    do {
        // Rotate around the cow...
        viewingAngle += osg::DegreesToRadians(1.0);
        osg::Vec3 eye = center + osg::Vec3( radius * cos(viewingAngle),
                                            radius * sin(viewingAngle),
                                            0);
        camera->setViewByLookat( eye[0], eye[1], eye[2],
                                 center[0], center[1], center[2],
                                 0, 0, 1 );

        // Click!
        camera->frame();
    } while( camera->getRenderSurface()->isRealized() );
}
