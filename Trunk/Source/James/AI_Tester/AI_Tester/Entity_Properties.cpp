#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;

namespace Scripting=GG_Framework::Logic::Scripting;
//namespace Scripting=Framework::Scripting;

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

void Ship_1D_Properties::SetFromShip_Properties (const Ship_Props & NewValue)
{
	m_MAX_SPEED=NewValue.MAX_SPEED;
	m_ACCEL=NewValue.ACCEL;
	m_BRAKE=NewValue.BRAKE;
	m_MaxAccelForward=NewValue.MaxAccelForward_High;
	m_MaxAccelReverse=NewValue.MaxAccelReverse_High;
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

