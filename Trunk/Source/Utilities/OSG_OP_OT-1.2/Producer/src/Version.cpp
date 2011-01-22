/* -*-c++-*- Producer - Copyright (C) 2001-2004  Don Burns
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 */

#include <string>
#include <Producer/Version>

using namespace Producer;

// fallback for windows where the project file doesn't provide the version string
#ifndef PRODUCER_VERSION
#define PRODUCER_VERSION "1.1"
#endif

Version::Version()
{
    std::string str(PRODUCER_VERSION);
    size_t b = 0, e;

    e = str.find(".", b);
    _major = atoi(str.substr(b,b+e).c_str()); 
    b = e+1;
    e = str.find(".", b);
    _minor = atoi(str.substr(b,b+e).c_str()); 
    b = e+1;
    _release = atoi(str.substr(b,str.length()-b).c_str()); 
}

