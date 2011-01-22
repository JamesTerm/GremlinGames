#include "stdafx.h"
#include "GuardianVR.Base.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GuardianVR::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;
using namespace GG_Framework::AppReuse;

Mech_ControllerBase::Mech_ControllerBase(Mech& mech) : 
	m_mech(mech), m_firingCannons(false), m_numKills(0), m_numHits(0), m_target(NULL)
{
	// Listen to keyboard events for switching targets
	GG_Framework::UI::KeyboardMouse_CB& kbm = GG_Framework::UI::MainWindow::GetMainWindow()->Keyboard_Mouse;
	GG_Framework::Logic::Entity3D::EventMap* em = m_mech.GetEventMap();

	// Add some event handlers for destruction and cannon hits
	em->EventPt_Map["DESTROY"].Subscribe(ehl, *this, &Mech_ControllerBase::MechDestroyed);
	m_mech.CannonRoundHit.Subscribe(ehl, *this, &Mech_ControllerBase::OnCannonRoundHit);
}

	/*
void Mech_ControllerBase::ReadScriptParameters(Mech_TransmittedEntity& te)
{
	// TODO: Read in stuff here the AI and the UI might need
}
*/

void Mech_ControllerBase::UpdateController(double dTime_s)
{
	// If the mech is being destroyed, it should not still try to do AI stuff
	if (m_mech.IsBeingDestroyed() || !m_mech.IsShowing())
		return;

	// TODO: Place simple AI here for moving and shooting
}

void Mech_ControllerBase::MechDestroyed(const osg::Vec3d& collisionPt)
{
	// If I am destroyed from a collision, I can count the kill towards my target
	Mech* target = dynamic_cast<Mech*>(GetTarget());
	if (target)
		target->GetController()->IncrementKills();
}
//////////////////////////////////////////////////////////////////////////

void Mech_ControllerBase::OnCannonRoundHit(GG_Framework::Logic::Entity3D* otherEntity)
{
	// Only count hits on my target
	if (GetTarget() == otherEntity)
		IncrementHits();
}
//////////////////////////////////////////////////////////////////////////

void Mech_ControllerBase::SetTarget(DestroyableEntity* newTarget)
{
	if (newTarget == &m_mech)
		newTarget = NULL;

	if (m_target != newTarget)
	{
		if (m_target)
		{
			m_target->GetEventMap()->EventPt_Map["DESTROY"].Remove(*this, &Mech_ControllerBase::TargetDestroyed);
			m_target->GetEventMap()->EventPt_Map["SHOT_DOWN"].Remove(*this, &Mech_ControllerBase::TargetDestroyed);
		}
		m_target = newTarget;
		if (newTarget)
		{
			m_target->GetEventMap()->EventPt_Map["DESTROY"].Subscribe(ehl, *this, &Mech_ControllerBase::TargetDestroyed);
			m_target->GetEventMap()->EventPt_Map["SHOT_DOWN"].Subscribe(ehl, *this, &Mech_ControllerBase::TargetDestroyed);
		}
		m_mech.SetGunTarget(newTarget);
	}
}
//////////////////////////////////////////////////////////////////////////

