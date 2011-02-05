#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;

Entity_Properties::Entity_Properties()
{
	m_EntityName="Entity";
	//m_NAME="default";
	m_Mass=10000.0;
	m_Dimensions[0]=12.0;
	m_Dimensions[1]=12.0;
};

void Entity_Properties::LoadFromScript(GG_Framework::Logic::Scripting::Script& script)
{
	const char* err;

	err = script.GetGlobalTable(m_EntityName.c_str());
	ASSERT_MSG(!err, err);
	{
		err = script.GetField("Mass", NULL, NULL, &m_Mass);
		ASSERT_MSG(!err, err);

		//Get the ship dimensions
		err = script.GetFieldTable("Dimensions");
		if (!err)
		{
			//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
			err = script.GetField("Length", NULL, NULL,&m_Dimensions[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("Width", NULL, NULL,&m_Dimensions[0]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		else
			m_Dimensions[0]=m_Dimensions[1]=2.0; //using 2.0 here means 1.0 radius default which is a great default for torque radius of mass

	}
	script.Pop();
}

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

void Ship_Properties::LoadFromScript(GG_Framework::Logic::Scripting::Script& script)
{
	const char* err;
	m_ShipType=eDefault;
	m_EntityName="Ship";
	err = script.GetGlobalTable("Ship");
	if (err)
	{
		err = script.GetGlobalTable("RobotTank");
		if (!err)
		{
			//m_ShipType=eRobotTank;	//keep around for less stress
			m_ShipType=eFRC2011_Robot;
			m_EntityName="RobotTank";
		}
	}
	ASSERT_MSG(!err, err);
	{
		double dHeading;
		err = script.GetField("dHeading", NULL, NULL, &dHeading);
		m_dHeading=DEG_2_RAD(dHeading);
		ASSERT_MSG(!err, err);

		err = script.GetField("ACCEL", NULL, NULL, &m_ACCEL);
		ASSERT_MSG(!err, err);
		err = script.GetField("BRAKE", NULL, NULL, &m_BRAKE);
		ASSERT_MSG(!err, err);
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
		script.GetField("MaxAccelReverse", NULL, NULL, &m_MaxAccelReverse);
		script.GetField("MaxTorqueYaw", NULL, NULL, &m_MaxTorqueYaw);

		err = script.GetField("MAX_SPEED", NULL, NULL, &m_MAX_SPEED);
		ASSERT_MSG(!err, err);
		err = script.GetField("ENGAGED_MAX_SPEED", NULL, NULL, &m_ENGAGED_MAX_SPEED);
		if (err)
			m_ENGAGED_MAX_SPEED=m_MAX_SPEED;

	}
	script.Pop();

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
	NewShip->MaxAccelForward=m_MaxAccelForward;
	NewShip->MaxAccelReverse=m_MaxAccelReverse;
	NewShip->MaxTorqueYaw=m_MaxTorqueYaw;
}

UI_Ship_Properties::UI_Ship_Properties()
{
	m_TextImage="*";
	m_UI_Dimensions[0]=1,m_UI_Dimensions[1]=1;
};

void UI_Ship_Properties::Initialize(const char **TextImage,osg::Vec2d &Dimension) const
{
	*TextImage=m_TextImage.c_str();
	Dimension[0]=m_UI_Dimensions[0];
	Dimension[1]=m_UI_Dimensions[1];
}

void UI_Ship_Properties::LoadFromScript(GG_Framework::Logic::Scripting::Script& script)
{
	const char* err;
	err = script.GetGlobalTable("Ship");
	if (err)
		err = script.GetGlobalTable("RobotTank");
	ASSERT_MSG(!err, err);
	{
		//Get the ships UI
		err = script.GetFieldTable("UI");
		if (!err)
		{
			err = script.GetField("Length", NULL, NULL,&m_UI_Dimensions[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("Width", NULL, NULL,&m_UI_Dimensions[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("TextImage",&m_TextImage,NULL,NULL);
			ASSERT_MSG(!err, err);
			script.Pop();
		}

	}
	script.Pop();

	__super::LoadFromScript(script);
}
