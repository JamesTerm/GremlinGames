#include <osg/MatrixTransform>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/WriteFile>

#include "Plane.h"


osg::Group *makeWalls()
{
    osg::Matrix mat[6];
    osg::Group *group = new osg::Group;


    mat[0] = osg::Matrix::translate( 0.0, 0.0, -40.0 );
    mat[1] =  osg::Matrix::rotate( M_PI, 0, 1, 0 ) * 
            osg::Matrix::translate( 0.0, 0.0,  40.0 );
    mat[2] = osg::Matrix::rotate( M_PI*0.5, 0, 1, 0 ) * 
             osg::Matrix::translate( -40.0, 0.0, 0.0 );
    mat[3] = osg::Matrix::rotate( -M_PI*0.5, 0, 1, 0 ) * 
             osg::Matrix::translate(  40.0, 0.0, 0.0 );

    mat[4] = osg::Matrix::rotate( -M_PI*0.5, 1, 0, 0 ) * 
             osg::Matrix::translate(  0.0, -40.0, 0.0 );

    mat[5] = osg::Matrix::rotate(  M_PI*0.5, 1, 0, 0 ) * 
             osg::Matrix::translate(  0.0,  40.0, 0.0 );


    for( int i = 0; i < 6; i++ )
    {
        Plane *p = new Plane(-40.0, 40.0, -40.0, 40.0, 0.0, 81.0 );
        osg::MatrixTransform *tx = new osg::MatrixTransform;
        tx->setMatrix( mat[i] );
        tx->addChild( p );
        group->addChild( tx );
    }



    return group;
}


int main()
{
    osg::Node *g = makeWalls();

    osgDB::writeNodeFile( *g, "walls.osg" );

    return 0;
}
