/**********************************************************************
 *
 *    FILE:            LightPointNode.cpp
 *
 *    DESCRIPTION:    Read/Write osgSim::LightPointNode in binary format to disk.
 *
 *    CREATED BY:        Auto generated by iveGenerator.exe
 *                    and later modified by Rune Schmidt Jensen.
 *
 *    HISTORY:        Created 14.9.2003
 *
 **********************************************************************/

#include "Exception.h"
#include "LightPointNode.h"
#include "LightPoint.h"
#include "Node.h"

using namespace ive;

void LightPointNode::write(DataOutputStream* out){
    // Write LightPointNode's identification.
    out->writeInt(IVELIGHTPOINTNODE);
    // If the osgSim class is inherited by any other class we should also write this to file.
    osg::Node*  node = dynamic_cast<osg::Node*>(this);
    if(node){
        ((ive::Node*)(node))->write(out);
    }
    else
        throw Exception("LightPointNode::write(): Could not cast this osgSim::LightPointNode to an osg::Node.");

    // Write LightPointNode's properties.
    out->writeFloat(getMinPixelSize());
    out->writeFloat(getMaxPixelSize());
    out->writeFloat(getMaxVisibleDistance2());

    // Write out LightPoints.
    unsigned int size = getNumLightPoints();
    out->writeUInt(size);
    for(unsigned int i=0; i<size; i++){
        ((ive::LightPoint*)(&getLightPoint(i)))->write(out);
    }
}

void LightPointNode::read(DataInputStream* in){
    // Peek on LightPointNode's identification.
    int id = in->peekInt();
    if(id == IVELIGHTPOINTNODE){
        // Read LightPointNode's identification.
        id = in->readInt();
        // If the osgSim class is inherited by any other class we should also read this from file.
        osg::Node*  node = dynamic_cast<osg::Node*>(this);
        if(node){
            ((ive::Node*)(node))->read(in);
        }
        else
            throw Exception("LightPointNode::read(): Could not cast this osgSim::LightPointNode to an osg::Object.");
        // Read LightPointNode's properties
        setMinPixelSize(in->readFloat());
        setMaxPixelSize(in->readFloat());
        setMaxVisibleDistance2(in->readFloat());

        // Read in lightpoints.
        unsigned int size = in->readUInt();
        for(unsigned int i=0; i<size; i++ ){
            osgSim::LightPoint lightPoint;
            ((ive::LightPoint*)(&lightPoint))->read(in);
            addLightPoint(lightPoint);
        }
    }
    else{
        throw Exception("LightPointNode::read(): Expected LightPointNode identification.");
    }
}