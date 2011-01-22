#ifndef PROJECT_H
#define PROJECT_H

#include <osg/TexGenNode>
#include <osg/Matrix>

class Projector : public osg::TexGenNode
{
    public:
        Projector(unsigned int unit) ;
        Projector( unsigned int unit, double fov );
        Projector( unsigned int unit, double hfov, double vfov );

        void setFOV( double fov );
        void setFOV( double hfov, double vfov );
};

#endif
