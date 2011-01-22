/**********************************************************************
 *
 *  FILE:           Object.cpp
 *
 *  DESCRIPTION:    Read/Write osg::Object in binary format to disk.
 *
 *  CREATED BY:     Auto generated by iveGenerated
 *                  and later modified by Rune Schmidt Jensen.
 *
 *  HISTORY:        Created 17.3.2003
 *
 *    Copyright 2003 VR-C
 **********************************************************************/

#include "Exception.h"
#include "Object.h"

using namespace ive;

void Object::write(DataOutputStream* out)
{
    // Write Object's identification.
    out->writeInt(IVEOBJECT);

    if ( out->getVersion() >= VERSION_0012 )
    {
        // Write Name
        out->writeString(getName());
    }

    // Write Object's properties.
    switch(getDataVariance())
    {
        case(osg::Object::STATIC): out->writeChar((char)0); break;
        case(osg::Object::DYNAMIC): out->writeChar((char)1); break;
    }
}

void Object::read(DataInputStream* in){
    // Read Object's identification.
    int id = in->peekInt();
    if(id == IVEOBJECT)
    {
        // Code to read Object's properties.
        id = in->readInt();

        if ( in->getVersion() >= VERSION_0012 )
        {
            // Read name
            setName(in->readString());
        }

        char c = in->readChar();
        switch((int)c)
        {
            case 0: setDataVariance(osg::Object::STATIC);break;
            case 1: setDataVariance(osg::Object::DYNAMIC);break;
        }
    }
    else{
        throw Exception("Object::read(): Expected Object identification");
    }
}
