#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;

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

void Entity1D_Properties::LoadFromScript(GG_Framework::Logic::Scripting::Script& script)
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
			err = script.GetField("Length", NULL, NULL,&m_Dimension);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		else
			m_Dimension=2.0;

	}
	script.Pop();
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

void Ship_1D_Properties::LoadFromScript(GG_Framework::Logic::Scripting::Script& script)
{
	const char* err;
	m_ShipType=eDefault;
	m_EntityName="Ship1D";
	err = script.GetGlobalTable("Ship1D");
	ASSERT_MSG(!err, err);
	{
		err = script.GetField("ACCEL", NULL, NULL, &m_ACCEL);
		ASSERT_MSG(!err, err);
		err = script.GetField("BRAKE", NULL, NULL, &m_BRAKE);

		script.GetField("MaxAccelForward", NULL, NULL, &m_MaxAccelForward);
		script.GetField("MaxAccelReverse", NULL, NULL, &m_MaxAccelReverse);

		err = script.GetField("MAX_SPEED", NULL, NULL, &m_MAX_SPEED);
		ASSERT_MSG(!err, err);
	}
	script.Pop();

	//TODO support range in script

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

	m_MaxAccelLeft=40.0 * Scale;
	m_MaxAccelRight=40.0 * Scale;
	m_MaxAccelForward=87.0 * Scale;
	m_MaxAccelReverse=70.0 * Scale;
	m_MaxTorqueYaw=2.5;
};

void Ship_Properties::LoadFromScript(GG_Framework::Logic::Scripting::Script& script)
{
	const char* err;
	m_ShipType=eDefault;
	m_EntityName="Ship";
	err = script.GetGlobalTable("Ship");
	if (err)
	{
		for (size_t i=0;i<_countof(csz_RobotNames);i++)
		{
			err = script.GetGlobalTable(csz_RobotNames[i]);
			if (!err)
			{
				m_ShipType=(Ship_Type)(i+1);
				m_EntityName=csz_RobotNames[i];
				break;
			}
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

  /***********************************************************************************************************************************/
 /*														UI_Ship_Properties															*/
/***********************************************************************************************************************************/

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
	{
		for (size_t i=0;i<_countof(csz_RobotNames);i++)
		{
			err = script.GetGlobalTable(csz_RobotNames[i]);
			if (!err)
				break;
		}
	}
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
