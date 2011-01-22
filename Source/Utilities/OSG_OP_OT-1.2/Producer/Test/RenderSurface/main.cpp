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

#ifdef WIN32
#include <windows.h>
#include <iostream>
#define usleep(x)	Sleep((x)/1000)
#define sleep(x)	Sleep((x)*1000)
#else
#include <unistd.h>
#endif

#ifdef __DARWIN_OSX__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <Producer/RenderSurface>
#include <Producer/KeyboardMouse>

#include "Text.h"

using namespace Producer;

class KBMCB : public KeyboardMouseCallback
{
  public:

	KBMCB(): 
			_toggleFullScreen(false),
			_toggleResize(false) ,
			_toggleCursor(false) ,
			_toggleBorder(false) ,
			_positionCursor(false),
			_quit(false)
			{}

	void specialKeyPress( KeyCharacter key )
	{
		switch( key )
		{
			case KeyChar_Escape :
				_quit = true;
				break;

			default :
				break;
		}
	}

	void keyPress( KeyCharacter key )
	{
		switch( key )
		{
			case 'b':
				_toggleBorder = true;
				break;

			case 'c':
				_toggleCursor = true;
				break;


			case 'f' :
				_toggleFullScreen = true;
				break;

			case 'p' :
				_positionCursor = true;
				break;

			case 'r' :
				_toggleResize = true;
				break;

			default :
				break;
		}
	}

	bool toggleFullScreen() { return checkToggle( _toggleFullScreen ); }
	bool toggleResize() { return checkToggle(_toggleResize); }
	bool toggleCursor() { return checkToggle(_toggleCursor); }
	bool toggleBorder() { return checkToggle(_toggleBorder); }
	bool positionCursor() { return checkToggle(_positionCursor); }
	bool quit() { return _quit; }

  protected:
    virtual ~KBMCB() {}

  private:
	bool _toggleFullScreen;
	bool _toggleResize;
	bool _toggleCursor;
	bool _toggleBorder;
	bool _positionCursor;
	bool _quit;

	bool checkToggle( bool &toggle )
	{
		if( toggle == true )
		{
			toggle = false;
			return true;
		}
		return false;
	}

};

static void drawText(RenderSurface &rs)
{
    Text *text = Text::getSingleton();

    char *strings[] = {
			"===========================================",
			"Interactive test of Producer::RenderSurface",
			"  Key bindings:",
			"     'b' : Toggle window border ON/OFF",
			"     'c' : Toggle cursor ON/OFF",
			"     'f' : Toggle full screen ON/OFF",
			"     'p' : Position pointer at 320,240",
			"     'r' : Resize window",
            0L
            };

    glViewport( 0, 0, rs.getWindowWidth(), rs.getWindowHeight() );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glOrtho( 0.0, double(rs.getWindowWidth()), 0.0, double(rs.getWindowHeight()), -1.0, 1.0); 
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    int charwidth = 10;
    int charheight = 30;
    float x = (rs.getWindowWidth() - (42*charwidth))*0.5;
    float y = (rs.getWindowHeight() + (8*charheight))*0.5;
    char **ptr = strings;

    glColor4f( 1, 1, 0, 1 );
    while( *ptr )
    {
        glRasterPos2f(x,y);
        y -= 30.0;
        text->drawString( Text::BitmapFont, *ptr );
        ptr++;
    }
}

void interactive()
{
	ref_ptr<RenderSurface> rs = new RenderSurface;
	rs->setWindowRectangle( 100, 100, 640, 480 );
    rs->setWindowName( "Producer RenderSurface test" );
	rs->useBorder( false );
	rs->realize();

    ref_ptr<KBMCB> kbmcb = new KBMCB;
	ref_ptr<KeyboardMouse> kbm = new KeyboardMouse(rs.get());
	kbm->setCallback( kbmcb.get() );
	kbm->startThread();

	bool resizeToggle = false;
	bool cursorToggle = true;

	glClearColor( 0.2f, 0.2f, 0.4f, 1.0f );
	//while( !kbmcb.quit() && rs->isRealized() )
	while( !kbmcb->quit() )
	{
		usleep(25000);
		if( kbmcb->toggleFullScreen() )
		{
			if( rs->isFullScreen() )
			{
				std::cout << "Unsetting full screen\n";
				rs->fullScreen(false);
			}
			else
			{
				std::cout <<"Setting full screen.\n";
				rs->fullScreen(true);
			}
		}

		if( kbmcb->toggleResize() )
		{
			resizeToggle = !resizeToggle;
			if( resizeToggle )
			{
				std::cout << "Resizing window to 800x600 at 300,300\n";
			    rs->setWindowRectangle( 300, 300, 800, 600 );
			}
			else
			{
				std::cout << "Resizing window to 640x408 at 100,100\n";
			    rs->setWindowRectangle( 100, 100, 640, 480 );
			}
		}

		if( kbmcb->toggleCursor() )
		{
			cursorToggle = !cursorToggle;
			std::cout << "Cursor : " << cursorToggle << std::endl;
			rs->useCursor( cursorToggle );
		}

		if( kbmcb->positionCursor() )
		{
			std::cout << "Positioning Pointer at 320,240\n";
			rs->positionPointer(320,240);
		}

		if( kbmcb->toggleBorder() )
		{
			if( rs->usesBorder() )
			{
				std::cout << "Toggling window border OFF\n";
				rs->useBorder(false);
			}
			else
			{
				std::cout << "Toggling window border ON\n";
				rs->useBorder(true);
			}
		}
		glClear( GL_COLOR_BUFFER_BIT );
        drawText(*rs.get());
		rs->swapBuffers();
	}
}

void noninteractive()
{
	ref_ptr<RenderSurface> rs = new RenderSurface;
	rs->setWindowRectangle( 100, 100, 640, 480 );
    rs->setWindowName( "Producer RenderSurface test" );
	rs->realize();

	glClearColor( 0.2f, 0.2f, 0.4f, 1.0f );

	glClear( GL_COLOR_BUFFER_BIT );
	rs->swapBuffers();
	printf( "Bordered window at 100, 100, 640 x 480...\n" );

	sleep(1);
	rs->useBorder(false);
	printf( "Non-bordered window...\n" );
	glClear( GL_COLOR_BUFFER_BIT );
	rs->swapBuffers();

	sleep(1);
	rs->useBorder(true);
	printf( "Bordered window...\n" );
	glClear( GL_COLOR_BUFFER_BIT );
	rs->swapBuffers();

	sleep(1);
	rs->fullScreen(true);
	printf( "Full screen...\n" );
	glClear( GL_COLOR_BUFFER_BIT );
	rs->swapBuffers();

	sleep(1);
	rs->fullScreen(false);
	printf( "Back to Window...\n" );
	glClear( GL_COLOR_BUFFER_BIT );
	rs->swapBuffers();

	sleep(1);
	printf( "Window resize to 300,300, 512 x 512 ... \n" ); 
	rs->setWindowRectangle( 300,300,512,512);
	glClear( GL_COLOR_BUFFER_BIT );
	rs->swapBuffers();

	sleep(1);
	printf( "Window resize back to 100,100, 640 x 480 ... \n" ); 
	rs->setWindowRectangle( 100,100,640,480);
	glClear( GL_COLOR_BUFFER_BIT );
	rs->swapBuffers();

	sleep(1);
	printf( "PositionCursor 320,240 ...\n" );
	rs->positionPointer(320,240);

	sleep(1);
	printf( "Cursor OFF\n" );
	rs->useCursor(false);

	sleep(1);
	rs->positionPointer(320,240);
	printf( "Cursor ON\n" );
	rs->useCursor(true);

	sleep(3);
}

int main( int argc, char **argv )
{
	if( argc > 1 && !strncmp(argv[1], "-n", 2) )
		noninteractive();
	else
		interactive();
	return 0;

}
