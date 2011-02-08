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




//This is the main robot class used for FRC 2011 
//The SimpleRobot class is the base of a robot application that will automatically call your
 //Autonomous and OperatorControl methods at the right time as controlled by the switches on the driver station or the field controls.
class Robot_Main : public SimpleRobot
{

public:
	//Drive left & right motors for 2 seconds then stop
	void Autonomous(void)
	{
		RobotDrive myRobot(1,2); // robot drive system
		myRobot.SetExpiration(0.1);
		
		myRobot.SetSafetyEnabled(false);
		myRobot.Drive(0.5, 0.0); 	// drive forwards half speed
		Wait(2.0); 				//    for 2 seconds
		myRobot.Drive(0.0, 0.0); 	// stop robot
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
			
			while (IsOperatorControl())
			{
				//TODO we may want to measure the actual time delta here... this however is safer for initial testing
				//I'll keep this around as a synthetic time option for debug purposes
				double time=0.016;
				joy_binder.UpdateJoyStick(time);
				robot.TimeChange(time);
				//60 FPS is well tested with the code.  Since there is more overhead to implement the physics, the idea is to
				//run at a pace that doesn't spike the CPU
				Wait(0.016);				
			}
		}
	}
};

START_ROBOT_CLASS(Robot_Main);

