// Fringe.Base HUD_PDCB.cpp
#include "stdafx.h"
#include "Fringe.Base.h"

#include "..\..\DebugPrintMacros.hpp"

using namespace Fringe::Base;
using namespace GG_Framework::AppReuse;

#undef SKIP_HUD_BOXES

const int hs = 10; // horizontal text spacing
const int vs = 20; // vertical text spacing
double MAX_FADE = 1.0; // How much of a black/red fade out do we have?  0 is none, 1.0 is complete

osg::Vec3 POS_G_VECTOR(0.0, 0.6, 0.8);

osg::Vec4 OK_COLOR(0.3, 0.8, 0.1, 0.4);
osg::Vec4 NOMINAL_COLOR(0.8, 0.8, 0.1, 0.5);
osg::Vec4 WARNING_COLOR(0.8, 0.2, 0.1, 0.65);
osg::Vec4 DANGER_COLOR(1.0, 0.1, 0.1, 0.8);

void DrawBox(int x1, int y1, int x2, int y2)
{
	glVertex2i( x1, y1 );
	glVertex2i( x1, y2 );
	glVertex2i( x2, y2 );
	glVertex2i( x2, y1 );
}

void TextRasterPos(double X, double Y, unsigned winWidth, unsigned winHeight)
{
	int x1 = X*winWidth;
	int y1 = winHeight - (Y*winHeight);
	glRasterPos2i( x1, y1 );
}

void DrawBoxWin(double X, double Y, double width, double height, unsigned winWidth, unsigned winHeight)
{
	int x1 = X*winWidth;
	int x2 = (X+width)*winWidth;
	int y1 = winHeight - (Y*winHeight);
	int y2 = winHeight - ((Y+height)*winHeight);
	DrawBox(x1,y1,x2,y2);
}

void DrawLineBox(int x1, int y1, int x2, int y2)
{
	glVertex2i( x1, y1 );
	glVertex2i( x1, y2 );

	glVertex2i( x1, y2 );
	glVertex2i( x2, y2 );

	glVertex2i( x2, y2 );
	glVertex2i( x2, y1 );

	glVertex2i( x2, y1 );
	glVertex2i( x1, y1 );
}

void DrawLineBoxWin(double X, double Y, double width, double height, unsigned winWidth, unsigned winHeight)
{
	int x1 = X*winWidth;
	int x2 = (X+width)*winWidth;
	int y1 = winHeight - (Y*winHeight);
	int y2 = winHeight - ((Y+height)*winHeight);
	DrawLineBox(x1,y1,x2,y2);
}

HUD_PDCB::HUD_PDCB(osg::Vec3 localHeadPos) : 
m_enabled(true), m_speed(0.0), m_Gs(INITIALIZE_Gs+2.0), m_lineG(6.5), m_maxG(10.0), 
m_blackOUTredOUT(0.0), m_flightMode(""), m_addnText("ADDN TEXT"), 
m_velocityAvg(5), m_headVelocityAvg(5),m_headAccelerationAvg(5),m_lockedOff(false)
{
}

void HUD_PDCB::DrawHUD(unsigned winWidth, unsigned winHeight) const
{
	// Build the speed String, but we do not want to draw it yet
	double speed = ((int)(m_speed*10.0+0.5)/10.0);	// Get it to the first decimal place
	char speedString[16];
	sprintf(speedString, "%6.1f", (float)speed);
	if (true)	// Convert all leading spaces to '-'
	{
		unsigned i = 0;
		while (speedString[i] == ' ') 
		{
			speedString[i] = '_';
			++i;
		}
	}

	// The damage line
	double damageX = .379;
	double damageY = .871;
	double damageW = .020;
	double damageH = -.151;
	double damageBar = m_damageRatio * damageH;

	// Find the Speed Line
	double speedX = .403;
	double speedY = .839;
	double speedW = .020;
	double speedH = -.151;
	double speedRatio = speed / m_maxSpeed;
	if (speedRatio > 1.0) speedRatio = 1.0;
	double speedBar = speedH * speedRatio;
	ASSERT(m_maxSpeed > 0.0);
	double speedLine = speedY + (speedH * (m_speedLine / m_maxSpeed));

	// And the G bar line
	double G_X = .579;
	double G_Y = .839;
	double G_W = .020;
	double G_H = -.151;
	double G_H2 = G_H / 2.0;
	double G_Mid = G_Y + G_H2;
	
	ASSERT(m_maxG > 0.0);
	double G_plus = G_Mid + (m_lineG/m_maxG)*G_H2;
	double G_minus = G_Mid - (m_lineG/m_maxG)*G_H2;
	double Gratio = (m_Gs >= INITIALIZE_Gs) ? 0.0 : m_Gs / m_maxG;
	if (Gratio > 1.0) Gratio = 1.0;
	if (Gratio < -1.0) Gratio = -1.0;
	double G_lineH = G_H2 * -Gratio;
	char G_txt[32];
	sprintf(G_txt, "%i", (int)m_Gs);

	// The target tgtDamage line
	double tgtDamageX = .520;
	double tgtDamageY = .832;
	double tgtDamageW = .051;
	double tgtDamageH = .027;
	double tgtDamageBar = m_tgtDamageRatio * tgtDamageW;

	// Weapons
	double weaponsX = .603;
	double weaponsY = .871;
	double weaponsW = .020;
	double weaponsH = -.151;
	double weaponsBar = weaponsH * m_weaponsRatio;
	double weaponsLine = weaponsY + (weaponsH * m_weaponsLine);

	double AlphaFade = 0.5;



	glPushAttrib( GL_ENABLE_BIT );

	// Move the entire thing down just a bit
	glPushMatrix();
	glTranslatef(0.0f, -50.0f, 0.0f);

	glEnable( GL_BLEND );
	{
#ifndef SKIP_HUD_BOXES
		// Boxes
		glColor4f( 0.3, 0.3, 0.6, 0.3 * AlphaFade );
		glBegin( GL_QUADS );
		{
			DrawBoxWin(.379, .720, .020, .151, winWidth, winHeight);
			DrawBoxWin(speedX, speedY, speedW, speedH, winWidth, winHeight);

			DrawBoxWin(.429, .736, .144, .092, winWidth, winHeight);

			DrawBoxWin(G_X, G_Y, G_W, G_H, winWidth, winHeight);
			DrawBoxWin(.603, .720, .020, .151, winWidth, winHeight);

			DrawBoxWin(.520, .832, .051, .027, winWidth, winHeight);
			DrawBoxWin(.531, .864, .040, .028, winWidth, winHeight);
			DrawBoxWin(.531, .896, .040, .028, winWidth, winHeight);
		}
		glEnd();

		// Damage Bar
		glColor4f(m_damageColor[0], m_damageColor[1], m_damageColor[2], m_damageColor[3]*AlphaFade);
		glBegin( GL_QUADS );
		{
			DrawBoxWin(damageX, damageY, damageW, damageBar, winWidth, winHeight);
		}
		glEnd();

		// Speed Bar
		glColor4f(m_speedColor[0], m_speedColor[1], m_speedColor[2], m_speedColor[3]*AlphaFade);
		glBegin( GL_QUADS );
		{
			DrawBoxWin(speedX, speedY, speedW, speedBar, winWidth, winHeight);
		}
		glEnd();

		// G Bar
		glColor4f( ((Gratio > 0.0) ? 0.0 : 0.6), 0.0, 0.0, 0.5 * AlphaFade );
		glBegin( GL_QUADS );
		{
			DrawBoxWin(G_X, G_Mid, G_W, G_lineH, winWidth, winHeight);
		}
		glEnd();

		// weapons Bar
		glColor4f(m_weaponsColor[0], m_weaponsColor[1], m_weaponsColor[2], m_weaponsColor[3] * AlphaFade);
		glBegin( GL_QUADS );
		{
			DrawBoxWin(weaponsX, weaponsY, weaponsW, weaponsBar, winWidth, winHeight);
		}
		glEnd();

		// tgtDamage Bar
		glColor4f(m_tgtDamageColor[0], m_tgtDamageColor[1], m_tgtDamageColor[2], m_tgtDamageColor[3]*AlphaFade);
		glBegin( GL_QUADS );
		{
			DrawBoxWin(tgtDamageX, tgtDamageY, tgtDamageBar, tgtDamageH, winWidth, winHeight);
		}
		glEnd();

		// Lines
		glColor4f( 1, 1, 1, 0.7 * AlphaFade );
		glBegin( GL_LINES );
		{
			DrawLineBoxWin(damageX, damageY, damageW, damageH, winWidth, winHeight);
			
			DrawLineBoxWin(speedX, speedY, speedW, speedH, winWidth, winHeight);
			DrawLineBoxWin(speedX, speedLine, speedW, 0.001, winWidth, winHeight);

			DrawLineBoxWin(.429, .736, .144, .092, winWidth, winHeight);
			DrawLineBoxWin(.435, .740, .131, .041, winWidth, winHeight);
			DrawLineBoxWin(.435, .785, .131, .041, winWidth, winHeight);

			DrawLineBoxWin(G_X, G_Y, G_W, G_H, winWidth, winHeight);
			DrawLineBoxWin(G_X, G_Mid, G_W, 0.001, winWidth, winHeight);	// Mid Line
			DrawLineBoxWin(G_X, G_plus, G_W, 0.001, winWidth, winHeight);	// plus Line
			DrawLineBoxWin(G_X, G_minus, G_W, 0.001, winWidth, winHeight);	// minus Line
			
			DrawLineBoxWin(weaponsX, weaponsY, weaponsW, weaponsH, winWidth, winHeight);
			DrawLineBoxWin(weaponsX, weaponsLine, weaponsW, 0.001, winWidth, winHeight);

			DrawLineBoxWin(tgtDamageX, tgtDamageY, tgtDamageW, tgtDamageH, winWidth, winHeight);
			DrawLineBoxWin(.531, .864, .040, .028, winWidth, winHeight);
			DrawLineBoxWin(.531, .896, .040, .028, winWidth, winHeight);
		}
		glEnd();
#endif

		// White text (same as box color)
		// glColor4f( 1, 1, 0, 1 );
		TextRasterPos(0.582, 0.685, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, G_txt );
		TextRasterPos(0.582, 0.860, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "G+" );
		TextRasterPos(0.385, 0.680, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "Velocity" );
		TextRasterPos(0.365, 0.885, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "Damage" );
		TextRasterPos(0.595, 0.885, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "Weapons" );

		// Yellow text
		glColor4f( 1, 1, 0, 1*AlphaFade );

		TextRasterPos(0.435, 0.710, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, m_addnText );
		TextRasterPos(0.435, 0.730, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, m_targetInfo );

		TextRasterPos(0.445, 0.770, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, speedString );
		TextRasterPos(0.540, 0.770, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "m/s" );
		TextRasterPos(0.445, 0.817, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, m_flightMode );

		TextRasterPos(0.435, 0.850, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "Tgt DMG" );

		TextRasterPos(0.435, 0.885, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "Deaths" );
		TextRasterPos(0.545, 0.885, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, m_deaths );
		
		TextRasterPos(0.435, 0.920, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, "Hits/Kills" );
		TextRasterPos(0.540, 0.920, winWidth, winHeight);
		m_text.drawString( GG_Framework::UI::OSG::Text::BitmapFont, m_HitsKills );

		glDisable( GL_BLEND );
	}
	glPopMatrix();
	glPopAttrib();
}
//////////////////////////////////////////////////////////////////////////

void HUD_PDCB::SetLocked(bool lock)
{
	if (m_lockedOff != lock)
	{
		m_lockedOff = lock;
		Reset();
	}
}
//////////////////////////////////////////////////////////////////////////

void HUD_PDCB::Update(ThrustShip2& ship, double dTime_s)
{
	osg::Quat currOrient = ship.GetAtt_quat();
	osg::Vec3 currGlobalPos = ship.GetPos_m();
	osg::Vec3 vel = ship.GetLinearVelocity();


	if ((dTime_s > 0.0) && !m_lockedOff)
	{
		////// TODO:  I am not sure that this should be here.  
		// Perhaps there needs to be a "Pilot" class that feels G-Forces?  Or in Physics and the Pilot class
		// Determines the limits.
		// This is particularly true when we apply G forces to AI pilots

		// NOTE that this method of determining G forces breaks down at velocities higher than
		// > 4000 mps.  (It might also be based on the position from the origin, floating point precision error?)
		// James thinks it might also be related to the timer not being accurate and consistent.
		// (See Case 16).  In the short term, we will just manually limit the maximum velocity.
		// 
		// Future builds will make these adjustments:
		// - 1.  Move these calculations to the physics code, Make a Pilot class for the limits
		// - 2.  Place everything related to the timer on a separate thread to keep the timer offsets consistent.
		
		// Find the current speed
		m_speed = vel.length();
		m_maxSpeed = ship.GetMaxSpeed();
		if (m_maxSpeed <= 0.0)
			m_maxSpeed = 1000.0;
		m_speedLine = ship.GetEngaged_Max_Speed();
		if (m_speedLine < 0.0)
			m_speedLine = 0.0;
		if (m_speedLine > m_maxSpeed)
			m_speedLine = m_maxSpeed;
		m_speedColor = ship.GetIsAfterBurnerOn() ? DANGER_COLOR : ((m_speed > m_speedLine) ? WARNING_COLOR : OK_COLOR);

		// Damage Levels
		m_damageRatio = 1.0 - (ship.GetRemainingHitPoints() / ship.GetRespawnPoints());
		if (m_damageRatio > 1.0)
			m_damageRatio = 1.0;
		if (m_damageRatio < 0.0)
			m_damageRatio = 0.0;
		if (m_damageRatio < 0.25)
			m_damageColor = OK_COLOR;
		else if (m_damageRatio < 0.5)
			m_damageColor = NOMINAL_COLOR;
		else if (m_damageRatio < 0.75)
			m_damageColor = WARNING_COLOR;
		else
			m_damageColor = DANGER_COLOR;

		// Deaths, Hits, Kills
		m_deaths = GG_Framework::Base::BuildString("%i", ship.GetNumDestructions());
		unsigned numHits = ship.GetController()->GetNumHits();
		unsigned numKills = ship.GetController()->GetNumKills();
		m_HitsKills = GG_Framework::Base::BuildString("%i/%i", numHits, numKills);

		// Target Info
		DestroyableEntity* target = ship.GetController()->GetTarget();
		if (target)
		{
			unsigned range = (unsigned)(ship.GetPos_m() - target->GetPos_m()).length();
			m_targetInfo = GG_Framework::Base::BuildString("%s (%i m)", target->GetName().c_str(), range);
			if (m_targetInfo.length() > 18)	// Use KM for long strings
				m_targetInfo = GG_Framework::Base::BuildString("%s (%i km)", target->GetName().c_str(), range/1000);

			m_tgtDamageRatio = 1.0 - (target->GetRemainingHitPoints() / target->GetRespawnPoints());
			if (m_tgtDamageRatio > 1.0)
				m_tgtDamageRatio = 1.0;
			if (m_tgtDamageRatio < 0.0)
				m_tgtDamageRatio = 0.0;
			if (m_tgtDamageRatio < 0.25)
				m_tgtDamageColor = OK_COLOR;
			else if (m_tgtDamageRatio < 0.5)
				m_tgtDamageColor = NOMINAL_COLOR;
			else if (m_tgtDamageRatio < 0.75)
				m_tgtDamageColor = WARNING_COLOR;
			else
				m_tgtDamageColor = DANGER_COLOR;
		}
		else
		{
			m_tgtDamageRatio = 0.0;
			m_tgtDamageColor = OK_COLOR;
			m_targetInfo = "NO TARGET";
		}

		// Weapons status
		Cannon::TempStatus tempStatus;
		ship.GetWeaponStatus(tempStatus, m_weaponsRatio, m_weaponsLine);
		switch(tempStatus)
		{
			case Cannon::CANNON_Overheated: m_weaponsColor = DANGER_COLOR; break;
			case Cannon::CANNON_Warning: m_weaponsColor = WARNING_COLOR; break;
			case Cannon::CANNON_Nominal: m_weaponsColor = NOMINAL_COLOR; break;
			case Cannon::CANNON_Cold: m_weaponsColor = OK_COLOR; break;
		}

		// Get the current G's and the limits calculated from Physics and 
		m_maxG = ship.GetPhysics().StructuralDmgGLimit;
		m_Gs=ship.GetPhysics().GetPilotInfo().Get_Gs();
		m_lineG = ship.GetPhysics().GetPilotInfo().GLimit;
		m_blackOUTredOUT=ship.GetPhysics().GetPilotInfo().Get_blackOUTredOUT();
		if (m_maxG < m_lineG)
			m_maxG = m_lineG;	// Not that this SHOULD happen, but it might, so we will check

		// If we just blacked out or red out, or recovered back to 0, fire an event that lets us know about it.	
		if ((m_blackOUTredOUT > 1.0) && (m_prevBlackOut < 1.0))
			ship.GetEventMap()->EventValue_Map["BLACKOUT"].Fire(1);
		else if ((m_blackOUTredOUT < -1.0) && (m_prevBlackOut > -1.0))
			ship.GetEventMap()->EventValue_Map["BLACKOUT"].Fire(-1);
		else if ((m_blackOUTredOUT == 0.0) && (m_prevBlackOut != 0.0))
			ship.GetEventMap()->EventValue_Map["BLACKOUT"].Fire(0);
		m_prevBlackOut=m_blackOUTredOUT;
	}
}
//////////////////////////////////////////////////////////////////////////
void HUD_PDCB::operator () (const osg::Camera &cam ) const
{
	// If nothing is enabled, just pop out
	if (m_Gs >= INITIALIZE_Gs)
		return;
	if (IsEnabled() || (m_blackOUTredOUT > 0.01) || (m_blackOUTredOUT < -0.01))
	{
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

		if (IsEnabled())
			DrawHUD(width, height);

		if ( (m_blackOUTredOUT > 0.01) || (m_blackOUTredOUT < -0.01))
		{
			glPushAttrib( GL_ENABLE_BIT );
			glEnable( GL_BLEND );
			{
				// Big Box
				double fade = m_blackOUTredOUT;
				if (fade < 0.0) fade *= -1.0;
				if (fade > 1.0) fade = 1.0;
				glColor4f( ((m_blackOUTredOUT > 0.0) ? 0.0 : 0.6), 0.0, 0.0, fade*MAX_FADE );
				glBegin( GL_QUADS );
				{
					DrawBox(0, 0, width, height);
				}
				glEnd();
			}
			glPopAttrib();
		}

		glPopAttrib();
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
	}
}

//////////////////////////////////////////////////////////////////////////

void HUD_PDCB::Reset()
{
	m_speed = 0.0;
	m_Gs = INITIALIZE_Gs+2.0;
	m_blackOUTredOUT = m_prevBlackOut=0.0;
	m_velocityAvg.Reset();
	m_headVelocityAvg.Reset();
	m_headAccelerationAvg.Reset();
}
//////////////////////////////////////////////////////////////////////////