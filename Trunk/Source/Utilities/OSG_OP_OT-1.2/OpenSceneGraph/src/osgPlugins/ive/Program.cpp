/**********************************************************************
 *
 *    FILE:            Program.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Program in binary format to disk.
 *
 *    CREATED BY:        Auto generated by iveGenerated
 *                    and later modified by Rune Schmidt Jensen.
 *
 *    HISTORY:        Created 20.3.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "Program.h"
#include "Object.h"

using namespace ive;

void Program::write(DataOutputStream* out){
    // Write Program's identification.
    out->writeInt(IVEPROGRAM);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Object*  obj = dynamic_cast<osg::Object*>(this);
    if(obj)
    {
        ((ive::Object*)(obj))->write(out);
    }
    else
        throw Exception("Program::write(): Could not cast this osg::Program to an osg::Object.");

    const AttribBindingList& abl = getAttribBindingList();
    out->writeUInt(abl.size());
    for(AttribBindingList::const_iterator itr = abl.begin();
        itr != abl.end();
        ++itr)
    {
        out->writeString(itr->first);
        out->writeUInt(itr->second);
    }

    // Write 
    out->writeUInt(getNumShaders());
    for(unsigned int si=0; si<getNumShaders(); ++si)
    {
        out->writeShader(getShader(si));
    }
}

void Program::read(DataInputStream* in)
{
    // Read Program's identification.
    int id = in->peekInt();
    if(id == IVEPROGRAM)
    {
        // Read Program's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Object*  obj = dynamic_cast<osg::Object*>(this);
        if(obj)
        {
            ((ive::Object*)(obj))->read(in);
        }
        else
            throw Exception("Program::read(): Could not cast this osg::Program to an osg::Object.");

    }
    else
    {
        throw Exception("Program::read(): Expected Program identification.");
    }

    // reading in shaders.
    unsigned int size = in->readUInt();
    for(unsigned int ai=0; ai<size; ++ai)
    {
        std::string name = in->readString();
        unsigned int index = in->readUInt();
        addBindAttribLocation(name, index);
    }

    // reading in shaders.
    size = in->readUInt();
    for(unsigned int si=0; si<size; ++si)
    {
        osg::Shader* shader = in->readShader();
        addShader(shader);
    }

}