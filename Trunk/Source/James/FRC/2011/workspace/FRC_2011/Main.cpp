#include "WPILib.h"
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
		SetUp_Manager(bool UseSafety) : m_Joystick(2,0), //2 joysticks starting at port 0
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

Goal *Get_TestLengthGoal(Ship_Tester *ship)
{
	//Construct a way point
	WayPoint wp;
	wp.Position[0]=0.0;
	wp.Position[1]=5.0;  //five meters
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal=new Goal_Ship_MoveToPosition(ship->GetController(),wp);
	return goal;
}

Goal *Get_TestRotationGoal(Ship_Tester *ship)
{
	//Rotate 180 degrees.  (Note: I skipped adding 180 to current heading since we assume it starts at 0)
	Goal_Ship_RotateToPosition *goal=new Goal_Ship_RotateToPosition(ship->GetController(),DEG_2_RAD(180.0));
	return goal;
}

Goal *Get_UberTubeGoal(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal
	Goal_OperateClaw *goal_CloseClaw=new Goal_OperateClaw(*Robot,true);

	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	const double starting_line=5.49656;  //18.03333
	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=starting_line; 
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);

	wp.Position[1]=starting_line+0.5;
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	Goal_OperateClaw *goal_OpenClaw=new Goal_OperateClaw(*Robot,false);
	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second
	wp.Position[1]=0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.0);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	MultitaskGoal *End_Goal=new MultitaskGoal;
	End_Goal->AddGoal(goal_arm2);
	End_Goal->AddGoal(goal_drive4);

	//wrap the goal in a notify goal
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete"); //will fire Complete once it is done
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_drive3);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_OpenClaw);
	MainGoal->AddSubgoal(goal_drive2);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	MainGoal->AddSubgoal(goal_CloseClaw);
	return MainGoal;
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
			assert(ship);
			{
				Goal *oldgoal=ship->ClearGoal();
				if (oldgoal)
					delete oldgoal;

				//Goal *goal=Get_TestLengthGoal(ship);
				//Goal *goal=Get_TestRotationGoal(ship);
				Goal *goal=Get_UberTubeGoal(m_pRobot);

				//If the goal above can cast to a notify goal then we can use it
				Goal_NotifyWhenComplete *notify_goal=dynamic_cast<Goal_NotifyWhenComplete *>(goal);
				//otherwise wrap the goal in to a notify goal instantiated here
				if (!notify_goal)
				{
					notify_goal=new Goal_NotifyWhenComplete(m_EventMap,"Complete"); //will fire Complete once it is done
					notify_goal->AddSubgoal(goal);  //add the non-notify goal here (may be composite)
				}

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
			RobotDrive myRobot(1,2,3,4); // robot drive system
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
			DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			while (IsOperatorControl())
			{
				//I'll keep this around as a synthetic time option for debug purposes
				//double time=0.020;
				double time=GetTime() - tm;
				tm=GetTime();
				//Framework::Base::DebugOutput("%f\n",time),
				main.TimeChange(time);
				//TODO see how fast the loop runs (if possible)
				lcd->UpdateLCD();
				Wait(0.005);				
			}
		}
	}
};

START_ROBOT_CLASS(Robot_Main);

