#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"
#include "Ship.h"

using namespace Framework::Base;

//namespace Scripting=GG_Framework::Logic::Scripting;
namespace Scripting=Framework::Scripting;

  /***********************************************************************************************************************************/
 /*															Entity1D_Properties														*/
/***********************************************************************************************************************************/

Entity1D_Properties::Entity1D_Properties()
{
	m_EntityName="Entity1D";
	m_Mass=10000.0;
	m_Dimension=12.0;
	m_IsAngular=false;
};

Entity1D_Properties::Entity1D_Properties(const char EntityName[],double Mass,double Dimension,bool IsAngular)
{
	m_EntityName=EntityName;
	m_Mass=Mass;
	m_Dimension=Dimension;
	m_IsAngular=IsAngular;
}

void Entity1D_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;

	//err = script.GetGlobalTable(m_EntityName.c_str());
	//ASSERT_MSG(!err, err);
	{
		script.GetField("mass_kg", NULL, NULL, &m_Mass);
		//At this level I do not know if I am dealing with a ship or robot, so I offer all units of measurement
		err=script.GetField("length_m", NULL, NULL,&m_Dimension);
		if (err)
		{
			double dimension;
			err=script.GetField("length_in", NULL, NULL,&dimension);
			if (!err)
				m_Dimension=Inches2Meters(dimension);
			else
			{
				err=script.GetField("length_ft", NULL, NULL,&dimension);
				if (!err)
					m_Dimension=Feet2Meters(dimension);
			}
		}
	
	}
}

void Entity1D_Properties::Initialize(Entity1D *NewEntity) const
{
	NewEntity->m_Dimension=m_Dimension;
	NewEntity->GetPhysics().SetMass(m_Mass);
	NewEntity->m_IsAngular=m_IsAngular;
}

  /***********************************************************************************************************************************/
 /*														Ship_1D_Properties															*/
/***********************************************************************************************************************************/

Ship_1D_Properties::Ship_1D_Properties()
{
	double Scale=0.2;  //we must scale everything down to see on the view
	m_MAX_SPEED = 400.0 * Scale;
	m_ACCEL = 60.0 * Scale;
	m_BRAKE = 50.0 * Scale;

	m_MaxAccelForward=87.0 * Scale;
	m_MaxAccelReverse=70.0 * Scale;
	m_MinRange=m_MaxRange=0.0;
	m_UsingRange=false;
};

Ship_1D_Properties::Ship_1D_Properties(const char EntityName[], double Mass,double Dimension,
				   double MAX_SPEED,
				   double ACCEL,double BRAKE,
				   double MaxAccelForward, double MaxAccelReverse,
				   Ship_Type ShipType,bool UsingRange,
				   double MinRange, double MaxRange,bool IsAngular
				   ) : Entity1D_Properties(EntityName,Mass,Dimension,IsAngular)
{
	m_MAX_SPEED = MAX_SPEED;
	m_ACCEL = ACCEL;
	m_BRAKE = BRAKE;
	m_MaxAccelForward=MaxAccelForward;
	m_MaxAccelReverse=MaxAccelReverse;
	m_ShipType=ShipType;
	m_MinRange=MinRange;
	m_MaxRange=MaxRange;
	m_UsingRange=UsingRange;
}

void Ship_1D_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	m_ShipType=eDefault;

	//I shouldn't need this
	//m_EntityName="Ship1D";
	//err = script.GetGlobalTable("Ship1D");
	//ASSERT_MSG(!err, err);

	{
		//double m_MAX_SPEED;
		//double m_ACCEL, m_BRAKE;
		//double m_MaxAccelForward,m_MaxAccelReverse;
		//double m_MinRange,m_MaxRange;
		//bool m_UsingRange;

		script.GetField("max_speed", NULL, NULL, &m_MAX_SPEED);
		script.GetField("accel", NULL, NULL, &m_ACCEL);
		script.GetField("brake", NULL, NULL, &m_BRAKE);
		script.GetField("max_accel_forward", NULL, NULL, &m_MaxAccelForward);
		script.GetField("max_accel_reverse", NULL, NULL, &m_MaxAccelReverse);
		double range;
		err=script.GetField("min_range_deg", NULL, NULL, &range);
		if (!err) m_MinRange=DEG_2_RAD(range);
		else
		{
			err=script.GetField("min_range", NULL, NULL, &range);
			if (!err) m_MinRange=range;
		}
		err=script.GetField("max_range_deg", NULL, NULL, &range);
		if (!err) m_MaxRange=DEG_2_RAD(range);
		else
		{
			err=script.GetField("max_range", NULL, NULL, &range);
			if (!err) m_MaxRange=range;
		}
		std::string sTest;
		//TODO determine why the bool type fails
		//script.GetField("using_range", NULL, &m_UsingRange, NULL);
		err=script.GetField("using_range", &sTest, NULL, NULL);
		if (!err)
			m_UsingRange=!((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'));

	}
	// Let the base class finish things up
	__super::LoadFromScript(script);
}

void Ship_1D_Properties::Initialize(Ship_1D *NewShip) const
{
	NewShip->MAX_SPEED=m_MAX_SPEED;
	NewShip->ACCEL=m_ACCEL;
	NewShip->BRAKE=m_BRAKE;
	NewShip->MaxAccelForward=m_MaxAccelForward;
	NewShip->MaxAccelReverse=m_MaxAccelReverse;
	NewShip->m_UsingRange=m_UsingRange;
	NewShip->m_MinRange=m_MinRange;
	NewShip->m_MaxRange=m_MaxRange;
}

  /***********************************************************************************************************************************/
 /*															Entity_Properties														*/
/***********************************************************************************************************************************/

Entity_Properties::Entity_Properties()
{
	m_EntityName="Entity";
	//m_NAME="default";
	m_Mass=25.0;
	m_Dimensions[0]=0.6477;
	m_Dimensions[1]=0.9525;
};

void Entity_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	{
		err = script.GetField("Mass", NULL, NULL, &m_Mass);

		//Get the ship dimensions
		err = script.GetFieldTable("Dimensions");
		if (!err)
		{
			//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
			err = script.GetField("Length", NULL, NULL,&m_Dimensions[1]);
			err = script.GetField("Width", NULL, NULL,&m_Dimensions[0]);
			script.Pop();
		}
	}
}

void Entity_Properties::Initialize(Entity2D *NewEntity) const
{
	NewEntity->m_Dimensions[0]=m_Dimensions[0];
	NewEntity->m_Dimensions[1]=m_Dimensions[1];
	NewEntity->GetPhysics().SetMass(m_Mass);
}

  /***********************************************************************************************************************************/
 /*															Ship_Properties															*/
/***********************************************************************************************************************************/
//These must be in the same order as they are in Ship_Properties::Ship_Type
const char * const csz_RobotNames[] =
{
	"RobotTank",
	"RobotSwerve",
	"Robot2011",
	"Robot2012",
};

Ship_Properties::Ship_Properties()
{
	m_dHeading = DEG_2_RAD(514.0);

	m_MAX_SPEED = 2.916;
	m_ENGAGED_MAX_SPEED = 2.916;
	m_ACCEL = m_ENGAGED_MAX_SPEED;
	m_BRAKE = m_ENGAGED_MAX_SPEED;
	m_STRAFE = m_BRAKE;
	m_AFTERBURNER_ACCEL = 60.0;    //we could use these, but I don't think it is necessary 
	m_AFTERBURNER_BRAKE = m_BRAKE;

	//These are the most important that setup the force restraints
	m_MaxAccelLeft=5.0;		//The left and right apply to strafe (are ignored for 2011 robot)
	m_MaxAccelRight=5.0;
	m_MaxAccelForward=5.0;
	m_MaxAccelReverse=5.0;
	m_MaxTorqueYaw=25.0;

	//I'm leaving these in event though they are not going to be used
	double RAMP_UP_DUR = 1.0;
	double RAMP_DOWN_DUR = 1.0;
	m_EngineRampAfterBurner= m_AFTERBURNER_ACCEL/RAMP_UP_DUR;
	m_EngineRampForward= m_ACCEL/RAMP_UP_DUR;
	m_EngineRampReverse= m_BRAKE/RAMP_UP_DUR;
	m_EngineRampStrafe= m_STRAFE/RAMP_UP_DUR;
	m_EngineDeceleration= m_ACCEL/RAMP_DOWN_DUR;
};

const char *Ship_Properties::SetUpGlobalTable(Scripting::Script& script)
{
	const char* err;
	//m_ShipType=eDefault;
	m_EntityName="Ship";
	err = script.GetGlobalTable("Ship");
	if (err)
	{
		for (size_t i=0;i<_countof(csz_RobotNames);i++)
		{
			err = script.GetGlobalTable(csz_RobotNames[i]);
			if (!err)
			{
				//m_ShipType=(Ship_Type)(i+1);
				m_EntityName=csz_RobotNames[i];
				break;
			}
		}
	}
	return err;
}

void Ship_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	{
		double dHeading;
		err = script.GetField("dHeading", NULL, NULL, &dHeading);
		if (!err)
			m_dHeading=DEG_2_RAD(dHeading);
		else
			script.GetField("heading_rad", NULL, NULL, &m_dHeading);

		err = script.GetField("ACCEL", NULL, NULL, &m_ACCEL);
		err = script.GetField("BRAKE", NULL, NULL, &m_BRAKE);
		err = script.GetField("STRAFE", NULL, NULL, &m_STRAFE);
		if (err)
			m_STRAFE=m_BRAKE; 		//Give strafe the default of the brake

		m_AFTERBURNER_ACCEL=m_ACCEL;  //Give afterburner acceleration rate the same as thrusters for default
		err = script.GetField("AFTERBURNER_ACCEL", NULL, NULL, &m_AFTERBURNER_ACCEL);

		m_AFTERBURNER_BRAKE=m_BRAKE;  //Give afterburner brake rate the same as brakes for default
		err = script.GetField("AFTERBURNER_ACCEL", NULL, NULL, &m_AFTERBURNER_BRAKE);

		// By defaults, we want all of the engines to ramp up to full in this amount of time
		double RAMP_UP_DUR = 1.0;
		double RAMP_DOWN_DUR = 1.0;

		err = script.GetField("RAMP_UP_DUR", NULL, NULL, &RAMP_UP_DUR);
		err = script.GetField("RAMP_DOWN_DUR", NULL, NULL, &RAMP_DOWN_DUR);
		const double MIN_RAMP = 0.00001;	// Avoid stupid numbers, div by 0 errors, etc.
		if (RAMP_UP_DUR < MIN_RAMP) RAMP_UP_DUR = MIN_RAMP;	
		if (RAMP_DOWN_DUR < MIN_RAMP) RAMP_DOWN_DUR = MIN_RAMP;

		err = script.GetField("EngineDeceleration", NULL, NULL, &m_EngineDeceleration);
		if (err) m_EngineDeceleration= m_ACCEL/RAMP_DOWN_DUR;
		err = script.GetField("EngineRampStrafe", NULL, NULL, &m_EngineRampStrafe);
		if (err) m_EngineRampStrafe= m_STRAFE/RAMP_UP_DUR;
		err = script.GetField("EngineRampForward", NULL, NULL, &m_EngineRampForward);
		if (err) m_EngineRampForward= m_ACCEL/RAMP_UP_DUR;
		err = script.GetField("EngineRampReverse", NULL, NULL, &m_EngineRampReverse);
		if (err) m_EngineRampReverse= m_BRAKE/RAMP_UP_DUR;
		err = script.GetField("EngineRampAfterBurner", NULL, NULL, &m_EngineRampAfterBurner);
		if (err) m_EngineRampAfterBurner= m_AFTERBURNER_ACCEL/RAMP_UP_DUR;


		script.GetField("MaxAccelLeft", NULL, NULL, &m_MaxAccelLeft);
		script.GetField("MaxAccelRight", NULL, NULL, &m_MaxAccelRight);
		script.GetField("MaxAccelForward", NULL, NULL, &m_MaxAccelForward);
		err=script.GetField("MaxAccelForward_High", NULL, NULL, &m_MaxAccelForward_High);
		if (err)
			m_MaxAccelForward_High=m_MaxAccelForward;

		script.GetField("MaxAccelReverse", NULL, NULL, &m_MaxAccelReverse);
		err=script.GetField("MaxAccelReverse_High", NULL, NULL, &m_MaxAccelReverse_High);
		if (err)
			m_MaxAccelReverse_High=m_MaxAccelReverse;

		script.GetField("MaxTorqueYaw", NULL, NULL, &m_MaxTorqueYaw);

		err = script.GetField("MAX_SPEED", NULL, NULL, &m_MAX_SPEED);
		err = script.GetField("ENGAGED_MAX_SPEED", NULL, NULL, &m_ENGAGED_MAX_SPEED);
		if (err)
			m_ENGAGED_MAX_SPEED=m_MAX_SPEED;

	}

	// Let the base class finish things up
	__super::LoadFromScript(script);
}

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
	//NewShip->MaxAccelForward=m_MaxAccelForward;
	//NewShip->MaxAccelReverse=m_MaxAccelReverse;
	NewShip->MaxTorqueYaw=m_MaxTorqueYaw;
}


double Ship_Properties::GetMaxAccelForward(double Velocity) const
{
	const double ratio = fabs(Velocity)/m_MAX_SPEED;
	const double  &Low=m_MaxAccelForward;
	const double &High=m_MaxAccelForward_High;
	return (ratio * High) + ((1.0-ratio) * Low);
}

double Ship_Properties::GetMaxAccelReverse(double Velocity) const
{
	const double ratio = fabs(Velocity)/m_MAX_SPEED;
	const double  &Low=m_MaxAccelReverse;
	const double &High=m_MaxAccelReverse_High;
	return (ratio * High) + ((1.0-ratio) * Low);
}

