// GG_Framework.UI Text_PDCB.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;

void Text_PDCB::operator () (const osg::Camera &cam ) const
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

	osg::Vec4 color = GetTextColor();
	glColor4f( color[0], color[1], color[2], color[3]);

	osg::Vec2 posn = GetPosition(width, height);
	glRasterPos2f( posn[0], posn[1] );

	std::string text = GetText();
	m_text.drawString(GG_Framework::UI::OSG::Text::BitmapFont, text.c_str());

	glPopAttrib();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string DebugOut_PDCB::TEXT1("DebugOut_PDCB::TEXT1");
std::string DebugOut_PDCB::TEXT2("DebugOut_PDCB::TEXT2");
std::string DebugOut_PDCB::TEXT3("DebugOut_PDCB::TEXT3");
std::string DebugOut_PDCB::TEXT4("DebugOut_PDCB::TEXT4");
std::string DebugOut_PDCB::TEXT5("DebugOut_PDCB::TEXT5");

// This is actually not used since we override the operator()
std::string DebugOut_PDCB::GetText() const
{
	return TEXT1 + "\n" + TEXT2 + "\n" + TEXT3 + "\n" + TEXT4 + "\n" + TEXT5;
}
//////////////////////////////////////////////////////////////////////////

void DebugOut_PDCB::operator () (const osg::Camera &cam ) const
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

	osg::Vec4 color = GetTextColor();
	glColor4f( color[0], color[1], color[2], color[3]);

	osg::Vec2 posn = GetPosition(width, height);
	int vs = 20; // vertical spacing
	int yPos = posn[1];

	glRasterPos2f( posn[0], yPos ); yPos-=vs;
	m_text.drawString(GG_Framework::UI::OSG::Text::BitmapFont, TEXT1.c_str());
	glRasterPos2f( posn[0], yPos ); yPos-=vs;
	m_text.drawString(GG_Framework::UI::OSG::Text::BitmapFont, TEXT2.c_str());
	glRasterPos2f( posn[0], yPos ); yPos-=vs;
	m_text.drawString(GG_Framework::UI::OSG::Text::BitmapFont, TEXT3.c_str());
	glRasterPos2f( posn[0], yPos ); yPos-=vs;
	m_text.drawString(GG_Framework::UI::OSG::Text::BitmapFont, TEXT4.c_str());
	glRasterPos2f( posn[0], yPos ); yPos-=vs;
	m_text.drawString(GG_Framework::UI::OSG::Text::BitmapFont, TEXT5.c_str());

	glPopAttrib();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////