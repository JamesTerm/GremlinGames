#include "WPILib.h"
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
		SetUp_Manager(bool UseSafety) : m_Joystick(1,0), //for now 1 joystick starting at port 0 (i.e. no offset)
			m_JoyBinder(m_Joystick),m_Control(UseSafety),m_pRobot(NULL),m_pUI(NULL)
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
	   //autonomous mode cannot have safety on
		SetUp_Autonomous() : SetUp_Manager(false),m_StillRunning(true)
		{
			m_pUI->SetAutoPilot(true);  //we are not driving the robot
			//Now to set up our goal
			Ship_Tester *ship=m_pRobot;  //we can always cast down
			double x=0.0;
			double y=5.0;  //five meters
			assert(ship);
			{
				Goal *oldgoal=ship->ClearGoal();
				if (oldgoal)
					delete oldgoal;
				//Construct a way point
				WayPoint wp;
				wp.Position[0]=x;
				wp.Position[1]=y;
				wp.Power=1.0;
				//Now to setup the goal
				Goal_Ship_MoveToPosition *goal=new Goal_Ship_MoveToPosition(ship->GetController(),wp);

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




//This is the main robot class used for FRC 2011 
//The SimpleRobot class is the base of a robot application that will automatically call your
 //Autonomous and OperatorControl methods at the right time as controlled by the switches on the driver station or the field controls.
class Robot_Main : public SimpleRobot
{

public:
	//Drive left & right motors for 2 seconds then stop
	void Autonomous(void)
	{
		SetUp_Autonomous main_autonomous;
		double tm = GetTime();
		while (main_autonomous.IsStillRunning())
		{
			double time=GetTime() - tm;
			tm=GetTime();
			//Framework::Base::DebugOutput("%f\n",time),
			//I'll keep this around as a synthetic time option for debug purposes
			//time=0.020;
			main_autonomous.TimeChange(time);
			//TODO see how fast the loop runs (if possible)
			Wait(0.005);				
		}

	}
	void OperatorControl(void)
	{
		if (c_UseDefaultControls)
		{
			RobotDrive myRobot(1,2); // robot drive system
			Joystick stick(1); // only 1 joystick
			myRobot.SetExpiration(0.1);
			
			// Runs the motors with arcade steering. 
			myRobot.SetSafetyEnabled(true);
			while (IsOperatorControl())
			{
				myRobot.ArcadeDrive(stick); // drive with arcade style (use right stick)
				Wait(0.005);				// wait for a motor update time
			}
		}
		else
		{
			SetUp_Manager main(true);  //use false to disable safety
			double tm = GetTime();
			while (IsOperatorControl())
			{
				//I'll keep this around as a synthetic time option for debug purposes
				//double time=0.020;
				double time=GetTime() - tm;
				tm=GetTime();
				//Framework::Base::DebugOutput("%f\n",time),
				main.TimeChange(time);
				//TODO see how fast the loop runs (if possible)
				Wait(0.005);				
			}

		}
	}
};

START_ROBOT_CLASS(Robot_Main);

