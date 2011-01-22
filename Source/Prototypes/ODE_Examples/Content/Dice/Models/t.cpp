#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Image>

main()
{
    osg::Image *img = osgDB::readImageFile( "OpenSceneGraphBanner_o.sgi" );
    unsigned char *ptr = img->data();

    for( int i = 0; i < img->s() * img->t(); i++ )
    {
        if( ptr[0] > 150 && ptr[1] > 150 && ptr[2] > 150 )
        {
            ptr[0] = 0;
            ptr[1] = 0;
            ptr[2] = 0;
            ptr[3] = 0;
        }
        ptr += 4;
    }

    osgDB::writeImageFile( *img, "OpenSceneGraphBanner.sgi");
}
