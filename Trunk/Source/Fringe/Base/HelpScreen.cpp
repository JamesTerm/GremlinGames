// Fringe.Base HelpScreen.cpp
#include "stdafx.h"
#include "Fringe.Base.h"

using namespace Fringe::Base;

// Add all KB funcs to the help screen
static char *g_help[] = {
	    "                     VIP Demo - Help Screen                      ",
#ifdef _DEBUG
		"                              DEBUG                              ",
#endif
		"             Press F1 to show/hide this Help Screen              ",
		"            Press ESC to Dismiss this Screen or Quit             ",
		"                                                                 ",
		" w        Fly forward, double-tap for afterburners               ",
        " s        Brake, Good to do if blacking out                      ",
		" q        Match Target Speed                                     ",
		" x        Brake to a stop                                        ",
		" a,d      Strafe Left and Right                                  ",
		" mouse    Turn, Left button Fires, Right Button Rolls            ",
		" ]        Switch Ships (Some ships cannot be piloted)            ",
		" -,=      Switch Targets, BACKSPACE Targets enemy after YOU      ",
		" 0        Target ship in front of you                            ",
		" g        Toggle SLIDE off and on, SLIDE is ON after blackout    ",
		" p        Hold the p key down and move mouse to admire your ship ",
		" z        Toggle Auto-Pilot on/off and just let it fly!          ",
		"                                                                 ",
		"      When you have a target, point your ship in the direction of  ",
		"      the yellow line.  The targeting reticle takes lead into      ",
		"      account.  You are out of range if the reticle is red.        ",
		0L  
};
//////////////////////////////////////////////////////////////////////////

HelpScreen::HelpScreen() : m_enabled(true)
{
	GG_Framework::UI::MainWindow* mainWindow = GG_Framework::UI::MainWindow::GetMainWindow();
	if (mainWindow)
	{
		mainWindow->EscapeQuit.Subscribe(ehl, *this, &HelpScreen::OnMainWindowEscape);
	}
}
//////////////////////////////////////////////////////////////////////////

/// Called when the user presses ESCAPE, we may want to dismiss this window rather than letting the main window go away
void HelpScreen::OnMainWindowEscape(GG_Framework::UI::MainWindow* mainWindow, bool& okToQuit)
{
	// Only if we are already enabled
	if (m_enabled)
	{
		Enable(false);
		okToQuit = false;
	}
}
//////////////////////////////////////////////////////////////////////////
void HelpScreen::operator () (const osg::Camera &cam ) const
{
	// If nothing is enabled, just pop out
	if (!m_enabled) return;

	double width = cam.getViewport()->width();
	double height = cam.getViewport()->height();

	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0.0, double(width), 0.0, double(height), -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib( GL_ENABLE_BIT );
	glDisable( GL_TEXTURE_2D );
	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );

	glColor4f( 1, 1, 0, 1 );

	{
		char **ptr = g_help;
		int nl = sizeof(g_help)/sizeof(const char *);
		int lw = strlen(g_help[0]);
		int hs = 10; // horizontal spacing
		int vs = 20; // vertical spacing
		int x = ((int)width - (lw*hs))>>1;
		int ht23 = (int)(height*0.7);	// Move it up a little
		int y = height*0.3 + vs + ht23 - ((ht23 - (nl*vs))>>1);
		glPushAttrib( GL_ENABLE_BIT );
		glEnable( GL_BLEND );

		int x1 = x - 35;
		int x2 = x + (lw*hs) + 35;
		int y1 = y + 35;
		int y2 = y-(nl*vs) - 35;

		glColor4f( 0.3, 0.3, 0.6, 0.4 );
		glBegin( GL_QUADS );
		glVertex2i( x1, y1 );
		glVertex2i( x1, y2 );
		glVertex2i( x2, y2 );
		glVertex2i( x2, y1 );
		glEnd();

		glColor4f( 1, 1, 1, 0.7 );
		glBegin( GL_LINES );
		glVertex2i( x1, y1 );
		glVertex2i( x1, y2 );

		glVertex2i( x1, y2 );
		glVertex2i( x2, y2 );

		glVertex2i( x2, y2 );
		glVertex2i( x2, y1 );

		glVertex2i( x2, y1 );
		glVertex2i( x1, y1 );

		glEnd();

		glDisable( GL_BLEND );

		glColor4f( 1, 1, 0, 1 );
		while( *ptr )
		{
			y -= vs;
			glRasterPos2i( x, y );
			m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, *ptr );
			ptr++;
		}
		glPopAttrib();
	}


	glPopAttrib();

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}
//////////////////////////////////////////////////////////////////////////