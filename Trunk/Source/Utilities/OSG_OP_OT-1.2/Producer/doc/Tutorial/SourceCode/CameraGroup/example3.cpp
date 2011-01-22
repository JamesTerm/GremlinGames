#include <Producer/CameraConfig>
#include <Producer/CameraGroup>
#include "MySceneHandler"

//
// Build a configuration that has two projection rectangles
// within one render surface
Producer::ref_ptr<Producer::CameraConfig> BuildConfig(void)
{
    Producer::ref_ptr<Producer::RenderSurface> rs = new Producer::RenderSurface;
	rs->setScreenNum(0);
	rs->useBorder(false);
	rs->setWindowRectangle(0,0,1280,480);
    Producer::ref_ptr<Producer::Camera> camera1 = new Producer::Camera;
	camera1->setRenderSurface(rs.get());
	camera1->setOffset( 1.0, 0.0 );
	camera1->setProjectionRectangle( 0.0f, 0.5f, 0.0f, 1.0f );

    Producer::ref_ptr<Producer::Camera> camera2 = new Producer::Camera;
	camera2->setRenderSurface(rs.get());
	camera2->setOffset( -1.0, 0.0 );
	camera2->setProjectionRectangle( 0.5f, 1.0f, 0.0f, 1.0f );

    Producer::ref_ptr<Producer::CameraConfig> cfg = new Producer::CameraConfig;
	cfg->addCamera("Camera 1", camera1.get());
	cfg->addCamera("Camera 2", camera2.get());

	return cfg;
}

int main( int argc, char **argv )
{
    Producer::ref_ptr<Producer::CameraConfig> cfg = BuildConfig();
    Producer::ref_ptr<Producer::CameraGroup> cg  = new Producer::CameraGroup( cfg.get() );

	cg->setSceneHandler( new MySceneHandler );
	cg->realize();

	while( true )
	{
		cg->frame();
	}
	return 0;
}
