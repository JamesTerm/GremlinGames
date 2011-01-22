#include <osg/Geometry>
#include "Plane.h"

Plane::Plane( float west, float east, float south, float north, float alt, float dens )
{
    osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors  = new osg::Vec4Array;

	int nstrips = 0;
	int striplen = 0;
	float step = (east - west)/dens;

	for( float x = west; x < east - (step*0.25); x += step )
	{
		for( float y = south; y <= (north+(step*0.25)); y += step )
		{
            double s = (x - west)/(east - west);
            double t = (y - south)/(north - south);
			coords->push_back( osg::Vec3(x,y,alt));
            tcoords->push_back( osg::Vec2( s, t ));
			coords->push_back( osg::Vec3(x+step,y,alt));

            s = ((x+step) - west)/(east - west);
            tcoords->push_back( osg::Vec2( s, t ));
			if( x == west )
				striplen+=2;
		}
		nstrips++;
	}

    normals->push_back( osg::Vec3( 0, 0, 1 ));
    colors->push_back( osg::Vec4( 1, 1, 1, 1 ));

	osg::Geometry *geom = new osg::Geometry;
	geom->setVertexArray( coords.get() );
    geom->setTexCoordArray( 0, tcoords.get() );
    geom->setTexCoordArray( 1, tcoords.get() );
    geom->setNormalArray(normals.get());
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    geom->setColorArray(colors.get());
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);


	for( int i = 0; i < nstrips; i++ )
	{
		geom->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::TRIANGLE_STRIP, (i*striplen), striplen));
	}

	addDrawable( geom );
}
