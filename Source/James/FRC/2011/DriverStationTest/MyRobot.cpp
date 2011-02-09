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
			//TODO all of this will be moved to a manager class when starting the autonomous implementation

			//Set up input
			Driver_Station_Joystick joystick(1,0);  //for now 1 joystick starting at port 0 (i.e. no offset)
			Framework::UI::JoyStick_Binder joy_binder(joystick);

			Ship_Properties robot_props;  //This will be upgraded soon
			Robot_Control control; // robot drive system
			control.Initialize(&robot_props);
			FRC_2011_Robot robot("FRC2011_Robot",&control);
			Framework::Base::EventMap eventMap;
			robot.Initialize(eventMap,&robot_props);
			//Bind the ship's eventmap to the joystick
			joy_binder.SetControlledEventMap(robot.GetEventMap());

			//To to bind the UI controller to the robot
			AI_Base_Controller *controller=robot.GetController();
			assert(controller);
			UI_Controller UI(joy_binder,controller); 
			if (controller->Try_SetUIController(&UI))
			{
				//Success... now to let the entity set things up
				UI.HookUpUI(true);
			}
			else
			{
				UI.Set_AI_Base_Controller(NULL);   //no luck... flush ship association
				assert(false);
			}

			double tm = GetTime();
			GetWatchdog().SetEnabled(true);
			while (IsOperatorControl())
			{
				GetWatchdog().Feed();
				//TODO we may want to measure the actual time delta here... this however is safer for initial testing
				//I'll keep this around as a synthetic time option for debug purposes
				//double time=0.016;
				double time=GetTime() - tm;
				//Framework::Base::DebugOutput("%f\n",tm),
				joy_binder.UpdateJoyStick(time);
				robot.TimeChange(time);
				//60 FPS is well tested with the code.  Since there is more overhead to implement the physics, the idea is to
				//run at a pace that doesn't spike the CPU
				Wait(0.005);				
			}
		}
	}
};

START_ROBOT_CLASS(RobotDemo);

