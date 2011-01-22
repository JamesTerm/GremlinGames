#include "Projector.h"

Projector::Projector(unsigned int unit) 
{
    setTextureUnit( unit );
}

Projector::Projector( unsigned int unit, double fov )
{
    setTextureUnit( unit );
    setFOV( fov );
}

Projector::Projector( unsigned int unit, double hfov, double vfov )
{
    setTextureUnit( unit );
    setFOV( hfov, vfov );
}

void Projector::setFOV( double fov )
{
    setFOV(fov,fov);
}

void Projector::setFOV( double hfov, double vfov )
{
    osg::TexGen *texGen = getTexGen();
    texGen->setMode( osg::TexGen::EYE_LINEAR );

    double left   = -tan(hfov*0.5);
    double right  =  tan(hfov*0.5);
    double bottom = -tan(vfov*0.5);
    double top    =  tan(vfov*0.5);
    osg::Matrix P;
    //P.makeFrustum( left, right, bottom, top, 1.0, 100.0 );
    P.makePerspective( 90.0, 1.0, 30.0, 300.0 );


    osg::Matrix C(
        0.5, 0, 0, 0,
        0, 0.5, 0, 0,
        0, 0, 0.5, 0,
        0.5, 0.5, 0.5, 1
    );

    osg::Matrix PC = P * C;
    texGen->setPlanesFromMatrix( PC );
}
