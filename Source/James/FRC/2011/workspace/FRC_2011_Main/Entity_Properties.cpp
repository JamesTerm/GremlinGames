#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Entity_Properties.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"

using namespace Framework::Base;

Entity_Properties::Entity_Properties()
{
	m_EntityName="Entity";
	//m_NAME="default";
	m_Mass=10000.0;
	m_Dimensions[0]=12.0;
	m_Dimensions[1]=12.0;
};


void Entity_Properties::Initialize(Entity2D *NewEntity) const
{
	NewEntity->m_Dimensions[0]=m_Dimensions[0];
	NewEntity->m_Dimensions[1]=m_Dimensions[1];
	NewEntity->GetPhysics().SetMass(m_Mass);
}

Ship_Properties::Ship_Properties()
{
	m_dHeading = DEG_2_RAD(270.0);

	double Scale=0.2;  //we must scale everything down to see on the view
	m_MAX_SPEED = 2000.0 * Scale;
	m_ENGAGED_MAX_SPEED = 400.0 * Scale;
	m_ACCEL = 60.0 * Scale;
	m_BRAKE = 50.0 * Scale;
	m_STRAFE = m_BRAKE; //could not find this one
	m_AFTERBURNER_ACCEL = 107.0 * Scale;
	m_AFTERBURNER_BRAKE = m_BRAKE;

	double RAMP_UP_DUR = 1.0;
	double RAMP_DOWN_DUR = 1.0;
	m_EngineRampAfterBurner= m_AFTERBURNER_ACCEL/RAMP_UP_DUR;
	m_EngineRampForward= m_ACCEL/RAMP_UP_DUR;
	m_EngineRampReverse= m_BRAKE/RAMP_UP_DUR;
	m_EngineRampStrafe= m_STRAFE/RAMP_UP_DUR;
	m_EngineDeceleration= m_ACCEL/RAMP_DOWN_DUR;
};


void Ship_Properties::Initialize(Ship_2D *NewShip) const
{
	NewShip->dHeading=m_dHeading;
	NewShip->MAX_SPEED=m_MAX_SPEED;
	NewShip->ENGAGED_MAX_SPEED=m_ENGAGED_MAX_SPEED;
	NewShip->ACCEL=m_ACCEL;
	NewShip->BRAKE=m_BRAKE;
	NewShip->STRAFE=m_STRAFE;
	NewShip->AFTERBURNER_ACCEL=m_AFTERBURNER_ACCEL;
	NewShip->AFTERBURNER_BRAKE=m_AFTERBURNER_BRAKE;

	NewShip->EngineRampAfterBurner=m_EngineRampAfterBurner;
	NewShip->EngineRampForward=m_EngineRampForward;
	NewShip->EngineRampReverse=m_EngineRampReverse;
	NewShip->EngineRampStrafe=m_EngineRampStrafe;
	NewShip->EngineDeceleration=m_EngineDeceleration;

	NewShip->MaxAccelLeft=m_MaxAccelLeft;
	NewShip->MaxAccelRight=m_MaxAccelRight;
	NewShip->MaxAccelForward=m_MaxAccelForward;
	NewShip->MaxAccelReverse=m_MaxAccelReverse;
	NewShip->MaxTorqueYaw=m_MaxTorqueYaw;
}
