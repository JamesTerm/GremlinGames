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
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"
#include "Ship.h"
#include "Robot_Tank.h"
#include "AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "UI_Controller.h"
#include "InOut_Interface.h"
#include "FRC2011_Robot.h"

const bool c_UseDefaultControls=false;


class SetUp_Manager
{
	protected:
		//Note: The order of the members are critical, as they are instantiated in the constructor
		Driver_Station_Joystick m_Joystick;  
		Framework::UI::JoyStick_Binder m_JoyBinder;
		FRC_2011_Robot_Properties m_RobotProps;
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
			if (dTime_s==0.0) return; //avoid division by zero errors
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
	private:
		void StopLoop()
		{
			m_StillRunning=false;
		}
		bool m_StillRunning;
		IEvent::HandlerList ehl;
	public:
		SetUp_Autonomous() : m_StillRunning(true)
		{
			m_pUI->SetAutoPilot(true);  //we are not driving the robot
			//Now to set up our goal
			Ship_Tester *ship=m_pRobot;  //we can always cast down
			assert(ship);
			{
				Goal *oldgoal=ship->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				//Matt change this to 1 to test length instead of rotation
				#if 0
				//Construct a way point
				WayPoint wp;
				wp.Position[0]=0.0;
				wp.Position[1]=5.0;  //five meters
				wp.Power=1.0;
				//Now to setup the goal
				Goal_Ship_MoveToPosition *goal=new Goal_Ship_MoveToPosition(ship->GetController(),wp);
				#else
				//Rotate 180 degrees.  (Note: I skipped adding 180 to current heading since we assume it starts at 0)
				Goal_Ship_RotateToPosition *goal=new Goal_Ship_RotateToPosition(ship->GetController(),DEG_2_RAD(180.0));
				#endif

				//wrap the goal in a notify goal
				Goal_NotifyWhenComplete *notify_goal=new Goal_NotifyWhenComplete(m_EventMap,"Complete"); //will fire Complete once it is done
				notify_goal->AddSubgoal(goal);  //only one goal (for now)
				notify_goal->Activate(); //now with the goal(s) loaded activate it
				//Now to subscribe to this event... it will call Stop Loop when the goal is finished
				m_EventMap.Event_Map["Complete"].Subscribe(ehl,*this,&SetUp_Autonomous::StopLoop);
				ship->SetGoal(notify_goal);
			}

		}
		bool IsStillRunning()
		{
			return m_StillRunning;
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
			double time=GetTime() - tm;
			tm=GetTime();
			//Framework::Base::DebugOutput("%f\n",time),
			//I'll keep this around as a synthetic time option for debug purposes
			//time=0.020;
			main_autonomous.TimeChange(time);
			//This is to simulate a 20ms interval... we'll need to measure the amount of overhead in the real robot
			Wait(0.002);				
			Wait(0.002);				
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
				Wait(0.004);				
			}
		}
	}
};

START_ROBOT_CLASS(RobotDemo);

