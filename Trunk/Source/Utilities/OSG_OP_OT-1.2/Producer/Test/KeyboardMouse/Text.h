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

#ifndef BMV_TEXT_H
#define BMV_TEXT_H

#ifdef WIN32

#include <windows.h>

#endif

#include <string>
#include <map>

#include <GL/gl.h>


class Text {
    public:
	enum Font {
	    BitmapFont,
	    FilledFont,
	    OutlineFont,
	    StrokeFont,
		LastFont
	};	
	Text();
	void drawString( Font, std::string );
	static Text *getSingleton();
	void *operator new( size_t size );

    protected:

	static Text *theText;

    private :

	GLuint fontmap[LastFont];
	GLuint loadFont(Font font);

	bool createStrokeFont( GLuint fontBase );
	bool createOutlineFont( GLuint fontBase );
	bool createFilledFont( GLuint fontBase );
	bool createBitmapFont( GLuint fontBase );

};

#endif
