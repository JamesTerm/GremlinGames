#include <iostream>
#include <string>
#include <Producer/CameraConfig>
#include <Producer/CameraGroup>
#include "MySceneHandler"

int main( int argc, char **argv )
{
    Producer::ref_ptr<Producer::CameraGroup> cg;
	std::string configFileName;

	if( argc > 1 )
		configFileName = argv[1];

	if( configFileName.empty() )
	{
		cg = new Producer::CameraGroup;
	}
	else
	{
        Producer::ref_ptr<Producer::CameraConfig> cfg = new Producer::CameraConfig;
		if( cfg->parseFile( configFileName.c_str() ) == false )
        {
            std::cerr << argv[0] << ": Failed to parse config file " << configFileName << std::endl;
			return 1;
        }
		cg  = new Producer::CameraGroup( cfg.get() );
	}

	for( unsigned int i = 0; i < cg->getNumberOfCameras(); i++ )
        cg->getCamera(i)->setSceneHandler(new MySceneHandler);

//	cg->realize( Producer::CameraGroup::ThreadPerCamera );
	cg->realize( Producer::CameraGroup::SingleThreaded );

	while( cg->validForRendering() )
	{
		cg->sync();
		cg->frame();
	}
}
