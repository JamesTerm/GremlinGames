#include "WPILib.h"
#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "UI_Controller.h"

const bool c_UseDefaultControls=true;

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
			RobotDrive myRobot(1,2); // robot drive system
			Joystick stick(1); // only 1 joystick
			//I'm giving a whole second before the timeout kicks in... I do not want false positives!
			myRobot.SetExpiration(1.0);
			
			// Runs the motors with arcade steering. 
			myRobot.SetSafetyEnabled(true);
			while (IsOperatorControl())
			{
				//TODO replace this with my code
				myRobot.ArcadeDrive(stick); // drive with arcade style (use right stick)
				//60 FPS is well tested with the code.  Since there is more overhead to implement the physics, the idea is to
				//run at a pace that doesn't spike the CPU
				Wait(0.016);				
			}
		}
	}
};

START_ROBOT_CLASS(Robot_Main);

