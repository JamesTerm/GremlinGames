// C++ source file - Open Scene Graph Training - Copyright (C) 2004 Don Burns
// Distributed under the terms of the GNU LIBRARY GENERAL PUBLIC LICENSE (LGPL)
// as published by the Free Software Foundation.

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexMat>
#include <osg/BlendFunc>
#include <osg/AlphaFunc>
#include "Cube.h"

Cube::Cube()
{
    // 8 Vertecies for each corner of the cube
    float vertex[][3] = {
        { -1.,  1.,  1. }, // 0
        { -1., -1.,  1. }, // 1
        {  1., -1.,  1. }, // 2
        {  1.,  1.,  1. }, // 3
        {  1.,  1., -1. }, // 4
        {  1., -1., -1. }, // 5
        { -1., -1., -1. }, // 6
        { -1.,  1., -1. }, // 7
    };

    // A set of indecies representing the six faces of the cube 
    // as quads
    int vertex_index[] = {
        0, 1, 2, 3, // A
        3, 2, 5, 4, // B
        4, 5, 6, 7, // C
        7, 6, 1, 0, // D
        0, 3, 4, 7, // E
        1, 6, 5, 2, // F
    };

    // A normal for each face of the cube
    float normal[][3] = {
        {  0.0f,  0.0f,  1.0f }, // A
        {  1.0f,  0.0f,  0.0f }, // B
        {  0.0f,  0.0f, -1.0f }, // C
        { -1.0f,  0.0f,  0.0f }, // D
        {  0.0f,  1.0f,  0.0f }, // E
        {  0.0f, -1.0f,  0.0f }, // F
    };

    // Texture coordinates
    float texcrds[][2] = {
        { 0.0, 1.0 },
        { 0.0, 0.0 },
        { 1.0, 0.0 },
        { 1.0, 1.0 }
    };

    osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> color   = new osg::Vec4Array;

    for( int i = 0; i < 24; i++ )
    {
        coords->push_back( osg::Vec3( vertex[vertex_index[i]][0],
                                      vertex[vertex_index[i]][1],
                                      vertex[vertex_index[i]][2]));

        texcoords->push_back( osg::Vec2( texcrds[i%4][0],
                                         texcrds[i%4][1] ));

        normals->push_back( osg::Vec3( normal[i/4][0],
                                       normal[i/4][1],
                                       normal[i/4][2]));
    }
    color->push_back( osg::Vec4(1.0, 1.0, 1.0, 1.0));

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray(coords.get());
    geometry->setTexCoordArray( 0, texcoords.get() );
    geometry->setNormalArray(normals.get());
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->setColorArray(color.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, coords->size()));

    addDrawable( geometry.get() );
}
