// Fringe.Base HUD_PDCB.h
#pragma once

#include <osg\Camera>

namespace Fringe
{
	namespace Base
	{
		class Speed_PDCB : public GG_Framework::UI::Text_PDCB
		{
		public:
			Speed_PDCB(Ship& ship) : Text_PDCB(), m_ship(ship) {}

			// Use this handler to tie events to this callback to toggle
			IEvent::HandlerList ehl;

			virtual std::string GetText() const {return GG_Framework::Base::BuildString("%3.0f mps", m_ship.GetVelocity_m_s().length());}
			virtual osg::Vec2 GetPosition(double winWidth, double winHeight) const {return osg::Vec2(winWidth - 70.0, winHeight - 20.0);}

		private:
			Ship& m_ship;
		};


		class ThrustShip2;

		//! THe HUD_PDCB will only be used for the VIP investors demo short-term.  Ultimately, 
		//! We will want for Justin to be able to build his HUD's completely in LightWave
		class HUD_PDCB 
			: public osg::Camera::DrawCallback
		{
		private:
			enum {INITIALIZE_Gs = 1000};
		public:
			// Uses the head position relative to the center of the ship to calculate G forces
			HUD_PDCB(osg::Vec3 localHeadPos);

			void ToggleEnabled(){m_enabled = !m_enabled;}
			bool IsEnabled() const {return m_enabled && !m_lockedOff;}
			void Enable(bool enable){m_enabled = enable;}

			virtual void operator () (const osg::Camera &cam ) const;

			// Use this handler to tie events to this callback to toggle
			IEvent::HandlerList ehl;

			// Call this each frame to determine G forces and current velocities
			void Update(ThrustShip2& ship, double dTime_s);

			void Reset();
			void SetLocked(bool lock);

			// Details for the flight mode
			std::string m_flightMode;

			// Nice for quick output of any debug text
			std::string m_addnText;

			Event1<int> Blackout;	// 0 for none, +1 for black-out, -1 for red-out

		private:
			void DrawHUD(unsigned winWidth, unsigned winHeight) const;
			bool m_enabled;
			osg::Vec3 m_localHeadPos;
			mutable GG_Framework::UI::OSG::Text m_text;

			GG_Framework::UI::OSG::VectorDerivativeOverTimeAverager m_velocityAvg;
			GG_Framework::UI::OSG::VectorDerivativeOverTimeAverager m_headVelocityAvg;
			GG_Framework::UI::OSG::VectorDerivativeOverTimeAverager m_headAccelerationAvg;

			// Do we want to turn it off
			bool m_lockedOff;

			// Speed
			double m_speed;
			double m_maxSpeed;
			double m_speedLine;
			osg::Vec4 m_speedColor;

			// Damage
			double m_damageRatio;
			osg::Vec4 m_damageColor;

			// Weapons
			double m_weaponsRatio;
			double m_weaponsLine;
			osg::Vec4 m_weaponsColor;

			// Hits, Kills, Deaths
			std::string m_HitsKills;
			std::string m_deaths;

			// Target String shows target and range
			std::string m_targetInfo;

			// Target Damage
			double m_tgtDamageRatio;
			osg::Vec4 m_tgtDamageColor;

			// Tracking G's
			double m_Gs, m_maxG, m_lineG;
			Averager<double, 45> m_G_Averager;
			double m_blackOUTredOUT;	// value between -1(red) and +1(black) most of the time 0.0
			double m_prevBlackOut;
		};
	}
}