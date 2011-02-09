#include <WPILib.h>
#include "DriverStationLCD.h"

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
#include "Robot_Tank.h"
#include "AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "UI_Controller.h"
#include "InOut_Interface.h"

const bool c_UseDefaultControls=false;


class SetUp_Manager
{
	protected:
		//Note: The order of the members are critical, as they are instantiated in the constructor
		Driver_Station_Joystick m_Joystick;  
		Framework::UI::JoyStick_Binder m_JoyBinder;
		Ship_Properties m_RobotProps;  //This will be upgraded soon
		Robot_Control m_Control; // robot drive system
		FRC_2011_Robot *m_pRobot; //This is a scoped pointer with late binding
		Framework::Base::EventMap m_EventMap;
		UI_Controller *m_pUI;
	public:
		SetUp_Manager() : m_Joystick(1,0), //for now 1 joystick starting at port 0 (i.e. no offset)
			m_JoyBinder(m_Joystick),m_pRobot(NULL),m_pUI(NULL)
		{
			m_Control.Initialize(&m_RobotProps);
			m_pRobot = new FRC_2011_Robot("FRC2011_Robot",&m_Control);
			m_pRobot->Initialize(m_EventMap,&m_RobotProps);
			//Bind the ship's eventmap to the joystick
			m_JoyBinder.SetControlledEventMap(m_pRobot->GetEventMap());

			//To to bind the UI controller to the robot
			AI_Base_Controller *controller=m_pRobot->GetController();
			assert(controller);
			m_pUI=new UI_Controller(m_JoyBinder,controller); 
			if (controller->Try_SetUIController(m_pUI))
			{
				//Success... now to let the entity set things up
				m_pUI->HookUpUI(true);
			}
			else
			{
				m_pUI->Set_AI_Base_Controller(NULL);   //no luck... flush ship association
				assert(false);
			}
		}
		void TimeChange(double dTime_s)
		{
			m_JoyBinder.UpdateJoyStick(dTime_s);
			m_pRobot->TimeChange(dTime_s);
		}

		~SetUp_Manager()
		{
			//Note: in visual studio the delete pointer implicitly checks for NULL, but I do not want to assume this for wind river.
			if (m_pUI)
			{
				delete m_pUI;
				m_pUI=NULL;
			}
			if (m_pRobot)
			{
				delete m_pRobot;
				m_pRobot=NULL;
			}
		}
};

class SetUp_Autonomous : public SetUp_Manager
{
	public:
		bool IsStillRunning()
		{
			return true; //TODO
		}
};

class RobotDemo : public SimpleRobot
{
public:
	RobotDemo(void)
	{
		GetWatchdog().SetExpiration(0.1);
	}

	void UpdateDS_LCD()
	{
		DriverStationLCD * lcd = DriverStationLCD::GetInstance();

		lcd->PrintfLine(DriverStationLCD::kMain_Line6, "Press select button");

		lcd->PrintfLine(DriverStationLCD::kUser_Line2, 
			"%d %d %d %d %d %d %d %d",
			(int)m_ds->GetDigitalIn(1),
			(int)m_ds->GetDigitalIn(2),
			(int)m_ds->GetDigitalIn(3),
			(int)m_ds->GetDigitalIn(4),
			(int)m_ds->GetDigitalIn(5),
			(int)m_ds->GetDigitalIn(6),
			(int)m_ds->GetDigitalIn(7),
			(int)m_ds->GetDigitalIn(8)
			);		

		lcd->PrintfLine(DriverStationLCD::kUser_Line3, "1: %.1f", 
			m_ds->GetAnalogIn(1));
		lcd->PrintfLine(DriverStationLCD::kUser_Line4, "2: %.1f", 
			m_ds->GetAnalogIn(2));
		lcd->PrintfLine(DriverStationLCD::kUser_Line5, "3: %.1f", 
			m_ds->GetAnalogIn(3));
		lcd->PrintfLine(DriverStationLCD::kUser_Line6, "4: %.1f", 
			m_ds->GetAnalogIn(4));

		lcd->UpdateLCD();
	}

	//Drive left & right motors for 2 seconds then stop
	void Autonomous(void)
	{
		SetUp_Autonomous main_autonomous;
		double tm = GetTime();
		GetWatchdog().SetEnabled(true);
		while (main_autonomous.IsStillRunning())
		{
			GetWatchdog().Feed();
			//TODO find out why autonomous timer is not working!
			//I'll keep this around as a synthetic time option for debug purposes
			double time=0.020;
			//double time=GetTime() - tm;
			//tm=GetTime();
			//Framework::Base::DebugOutput("%f\n",time),
			main_autonomous.TimeChange(time);
			//60 FPS is well tested with the code.  Since there is more overhead to implement the physics, the idea is to
			//run at a pace that doesn't spike the CPU
			Wait(0.005);				
		}
	}

	void OperatorControl(void)
	{
#if 0
		double tm = GetTime();
		
		GetWatchdog().SetEnabled(true);
		while (IsOperatorControl())
		{
			GetWatchdog().Feed();
			
			if (GetTime() - tm > 0.1)
			{
				UpdateDS_LCD();								
				tm = GetTime();
			}
		}
#endif

		if (c_UseDefaultControls)
		{
			RobotDrive myRobot(1,2,3,4); // robot drive system
			Joystick stick(1); // only 1 joystick
			//myRobot.SetExpiration(0.1);

			// Runs the motors with arcade steering. 
			GetWatchdog().SetEnabled(true);
			while (IsOperatorControl())
			{
				GetWatchdog().Feed();
				myRobot.ArcadeDrive(stick); // drive with arcade style (use right stick)
				Wait(0.005);				// wait for a motor update time
			}
		}
		else
		{
			SetUp_Manager main;
			double tm = GetTime();
			GetWatchdog().SetEnabled(true);
			while (IsOperatorControl())
			{
				GetWatchdog().Feed();
				//I'll keep this around as a synthetic time option for debug purposes
				//double time=0.020;
				double time=GetTime() - tm;
				tm=GetTime();
				//Framework::Base::DebugOutput("%f\n",time),
				main.TimeChange(time);
				//60 FPS is well tested with the code.  Since there is more overhead to implement the physics, the idea is to
				//run at a pace that doesn't spike the CPU
				Wait(0.005);				
			}
		}
	}
};

START_ROBOT_CLASS(RobotDemo);

