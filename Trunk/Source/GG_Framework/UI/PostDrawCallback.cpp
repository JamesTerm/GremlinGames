// PostDrawCallback.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI::OSG;
using namespace GG_Framework::UI;

// Add all KB funcs to the help screen
static char *g_help[] = {
		"                           Help Screen                           ",
#ifdef _DEBUG
		"                              DEBUG                              ",
#endif
		"                                                                 ",
		" Key      Function                                       Default ",
		" ESC      Quit",
		" ALT+f    Toggle Full Screen                                on   ",
		" ALT+a    Toggle Animation Time Display                     on   ",
		" ALT+d    Toggle Trackball Distance Display                 on   ",
		" ALT+s    Toggle FrameRate Display                          on   ",
		" ALT+m    Toggle Centered Model Name Display                on   ",
		" h/?      Toggle help screen (this screen)                  off  ",
		" ALT+w    Toggles through polygon modes of centered node    solid",
		" ALT+b    Toggles bkgd to black/lt. gray/blue               black",
		" ALT+p    Save the current image to c:\\OSGV_ScreenCapture.bmp   ",
		"                                                                 ",
		" ARROWS   Select Centered Node                                   ",
		" (space)  Resets Camera position to last saved                   ",
		0L  
};

void PostDrawCallback::operator () (const Producer::Camera &cam)
{
	// If nothing is enabled, just pop out
	if (!_enableMask) return;

	int x, y;
	unsigned int width, height;
	cam.getProjectionRectangle( x, y, width, height );

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

	if( isEnabled( DisplayAnim ))
	{
		ViewPortReference* vpr = m_viewer.GetViewPortReference();
		Actor* actor = vpr ? vpr->GetActor() : NULL;
		const char* title = actor ? "Actor Local Time" : "Global Time";
		const char* actionName = actor ? actor->GetCurrActionName() : "No Actor";
		if (!actionName) actionName = "No Action";
		double time = actor ? actor->GetFrameStamp()->getReferenceTime() : m_timer.GetCurrTime_s();
		int frameNum = actor ? actor->GetFrameStamp()->getFrameNumber() : TIME_2_FRAME(time);

		glRasterPos2f( 10.0, 10.0 );
		m_text.drawString( Text::BitmapFont, title );

		glRasterPos2f( 10.0, 30.0 );
		// Show the Frame Number of the current Actor/Action OR The Global Time
		std::string buff = GG_Framework::Base::BuildString(
			"%s(%0.2f, %i)", actionName, time, frameNum);
		m_text.drawString( Text::BitmapFont, buff.c_str() );
	}

	if( isEnabled( DisplayCenterRef ))
	{
		const char* title = m_viewer.Get_VP_ActorTitle();
		glRasterPos2f( width - (strlen(title) * 10) - 30, height - 20.0 );
		m_text.drawString( Text::BitmapFont, title);
	}

	if( isEnabled( DisplayFrameRate ))
	{
		char buff[32];
		sprintf( buff, "%3.2f fps", m_viewer.GetFrameRate());
		glRasterPos2f( 10.0, height - 20.0 );
		m_text.drawString( Text::BitmapFont, buff );
	}

	if( isEnabled( DisplayDist ) )
	{
		char* trackBallLabel = "Trackball Fixed Axis Dist.";
		glRasterPos2f( width - (strlen(trackBallLabel) * 10) - 30, 30.0 );
		m_text.drawString( Text::BitmapFont, trackBallLabel );

		char buff[32];
		sprintf( buff, "%8.1f m.", m_viewer.GetTrackBall().getDistance());
		glRasterPos2f( width - (strlen(buff) * 10) - 30, 10.0 );
		m_text.drawString( Text::BitmapFont, buff );
	}

	if( isEnabled( DisplayHelp ))
	{
		char **ptr = g_help;
		int nl = sizeof(g_help)/sizeof(const char *);
		int lw = strlen(g_help[0]);
		int hs = 10; // horizontal spacing
		int vs = 20; // vertical spacing
		int x = (width - (lw*hs))>>1;
		int y = vs + height - ((height - (nl*vs))>>1);
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
			m_text.drawString( Text::BitmapFont, *ptr );
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