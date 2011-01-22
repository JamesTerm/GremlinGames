#include <Producer/CameraConfig>
#include <Producer/CameraGroup>
#include "MySceneHandler"

Producer::ref_ptr<Producer::CameraConfig> BuildConfig(void)
{
    Producer::ref_ptr<Producer::RenderSurface> rs1 = new Producer::RenderSurface;
	rs1->setScreenNum(0);
	rs1->useBorder(false);
	rs1->setWindowRectangle(0,0,640,480);
    Producer::ref_ptr<Producer::Camera> camera1 = new Producer::Camera;
	camera1->setRenderSurface(rs1.get());
	camera1->setOffset( 1.0, 0.0 );

    Producer::ref_ptr<Producer::RenderSurface> rs2 = new Producer::RenderSurface;
	rs2->setScreenNum(0);
	rs2->useBorder(false);
	rs2->setWindowRectangle(640,0,640,480);
    Producer::ref_ptr<Producer::Camera> camera2 = new Producer::Camera;
	camera2->setRenderSurface(rs2.get());
	camera2->setOffset( -1.0, 0.0 );

    Producer::ref_ptr<Producer::CameraConfig> cfg = new Producer::CameraConfig;
	cfg->addCamera("Camera 1", camera1.get());
	cfg->addCamera("Camera 2", camera2.get());

	return cfg;
}

int main( int argc, char **argv )
{
    Producer::ref_ptr<Producer::CameraConfig> cfg = BuildConfig();
    Producer::ref_ptr<Producer::CameraGroup> cg  = new Producer::CameraGroup( cfg.get() );

	for( int i = 0; i < cg->getNumberOfCameras(); i++ )
		cg->getCamera(i)->setSceneHandler( new MySceneHandler );

	cg->realize( Producer::CameraGroup::ThreadPerCamera );

	while( cg->validForRendering() )
	{
		cg->frame();
	}
	return 0;
}
