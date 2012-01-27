#include "WPILib.h"

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Common/Entity_Properties.h"
#include "Common/Physics_1D.h"
#include "Common/Physics_2D.h"
#include "Common/Entity2D.h"
#include "Common/Goal.h"
#include "Common/Ship_1D.h"
#include "Common/Ship.h"
#include "Common/Vehicle_Drive.h"
#include "Common/PIDController.h"
#include "Common/Tank_Robot.h"
#include "Common/AI_Base_Controller.h"
#include "Common/Robot_Control_Interface.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2012_Robot.h"
#include "InOut_Interface.h"
#include "FRC2012_Goals.h"

const bool c_UseDefaultControls=false;


class SetUp_Manager
{
	protected:
		//Note: The order of the members are critical, as they are instantiated in the constructor
		Driver_Station_Joystick m_Joystick;  
		Framework::UI::JoyStick_Binder m_JoyBinder;
		FRC_2011_Robot_Properties m_RobotProps;
		FRC_2011_Robot_Control m_Control; // robot drive system
		FRC_2011_Robot *m_pRobot; //This is a scoped pointer with late binding
		Framework::Base::EventMap m_EventMap;
		IEvent::HandlerList ehl;
		FRC_2011_UI_Controller *m_pUI;
	public:
		
		void GoalComplete()
		{
			printf("Goals completed!\n");
		}
		
		SetUp_Manager(bool UseSafety,bool UseEncoders=false) : m_Joystick(2,0), //2 joysticks starting at port 0
			m_JoyBinder(m_Joystick),m_Control(UseSafety),m_pRobot(NULL),m_pUI(NULL)
		{
			m_Control.AsControlInterface().Initialize(&m_RobotProps);
			m_pRobot = new FRC_2011_Robot("FRC2011_Robot",&m_Control,UseEncoders);
			m_pRobot->Initialize(m_EventMap,&m_RobotProps);
			//Bind the ship's eventmap to the joystick
			m_JoyBinder.SetControlledEventMap(m_pRobot->GetEventMap());

			//To to bind the UI controller to the robot
			AI_Base_Controller *controller=m_pRobot->GetController();
			assert(controller);
			m_pUI=new FRC_2011_UI_Controller(m_JoyBinder,controller); 
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
			//This is for testing purposes
			m_EventMap.Event_Map["Complete"].Subscribe(ehl,*this,&SetUp_Manager::GoalComplete);
		}
		void TimeChange(double dTime_s)
		{
			if (dTime_s==0.0) return; //avoid division by zero errors
			m_JoyBinder.UpdateJoyStick(dTime_s);
			//This is called implicitly within the robot (for ease of compatability with AI)
			//m_Control.TimeChange(dTime_s);
			m_pRobot->TimeChange(dTime_s);
		}

		~SetUp_Manager()
		{
			m_EventMap.Event_Map["Complete"].Remove(*this,&SetUp_Manager::GoalComplete);

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

		void SetAutoPilot(bool autoPilot) {m_pUI->SetAutoPilot(autoPilot);}
		FRC_2011_Robot *GetRobot() const {return m_pRobot;}
		void SetSafety(bool UseSafety) {m_Control.SetSafety(UseSafety);}
		void ResetPos() 
		{	
			m_pRobot->ResetPos();
			m_Control.ResetPos();
		}
};


//This is the main robot class used for FRC 2011 
//The SimpleRobot class is the base of a robot application that will automatically call your
 //Autonomous and OperatorControl methods at the right time as controlled by the switches on the driver station or the field controls.
class Robot_Main : public SimpleRobot
{
	SetUp_Manager m_Manager;

public:
	Robot_Main(void) : m_Manager(false) //disable safety by default
	{
	}
	
	void Autonomous(void)
	{
		m_Manager.ResetPos();  //We must reset the position to ensure the distance is measured properly
		//autonomous mode cannot have safety on
		m_Manager.SetSafety(false);
		m_Manager.SetAutoPilot(true);  //we are not driving the robot
		//Now to set up our goal
		Ship_Tester *ship=m_Manager.GetRobot();  //we can always cast down
		m_Manager.GetRobot()->SetUseEncoders(true);
		//m_Manager.GetRobot()->SetUseEncoders(false);

		//assert(ship);
		size_t AutonomousValue=0;
		DriverStation *ds = DriverStation::GetInstance();
		AutonomousValue+=ds->GetDigitalIn(1)? 0x01 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(2)? 0x02 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(3)? 0x04 : 0x00;
		AutonomousValue+=ds->GetDigitalIn(4)? 0x08 : 0x00;
		printf("Autonomous mode= %d \n",AutonomousValue);
		const bool DoAutonomous=AutonomousValue!=0;  //set to false as safety override
		Goal *oldgoal=ship->ClearGoal();
		if (oldgoal)
			delete oldgoal;

		if (DoAutonomous)
		{
			using namespace FRC_2011_Goals;
			Goal *goal=NULL;
			switch (AutonomousValue)
			{
				case 1:		goal=Test_Arm(m_Manager.GetRobot());				break;
				case 2:		goal=Get_TestLengthGoal(m_Manager.GetRobot());		break;
				//case 3:		goal=Get_TestRotationGoal(ship);				break;
				case 3:		goal=Get_TestArmElbowClaw(m_Manager.GetRobot());	break;
				case 4:		goal=Get_UberTubeGoal(m_Manager.GetRobot());		break;
			}
			if (goal)
				goal->Activate(); //now with the goal(s) loaded activate it
			ship->SetGoal(goal);
		}
		
		double tm = GetTime();
		
		while (IsAutonomous() && !IsDisabled())
		{
			double time=GetTime() - tm;
			tm=GetTime();
			//Framework::Base::DebugOutput("%f\n",time),
			//I'll keep this around as a synthetic time option for debug purposes
			//time=0.020;
			m_Manager.TimeChange(time);
			//using this from test runs from robo wranglers code
			Wait(0.010);				
		}
		printf("Autonomouse loop end IsA=%d IsD=%d \n",IsAutonomous(),IsDisabled());
		oldgoal=ship->ClearGoal();
		if (oldgoal)
			delete oldgoal;
		ship->SetGoal(NULL);

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
			printf("Starting TeleOp Session\n");
			m_Manager.ResetPos();  //This should avoid errors like the arm swinging backwards
			m_Manager.GetRobot()->SetUseEncoders(false);
			m_Manager.SetAutoPilot(false);  //we are driving the robot
			double tm = GetTime();
			m_Manager.SetSafety(true);
			while (IsOperatorControl() && !IsDisabled())
			{
				//I'll keep this around as a synthetic time option for debug purposes
				//double time=0.020;
				double time=GetTime() - tm;
				tm=GetTime();
				//Framework::Base::DebugOutput("%f\n",time),
				m_Manager.TimeChange(time);
				//using this from test runs from robo wranglers code
				Wait(0.010);
			}
		}
	}
};

START_ROBOT_CLASS(Robot_Main);

