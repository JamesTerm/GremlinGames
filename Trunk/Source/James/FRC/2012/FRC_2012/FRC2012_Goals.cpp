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

namespace FRC_2011_Goals
{

float GetDS_Distance()
{
	DriverStation *ds = DriverStation::GetInstance();
	float Multiplier=ds->GetDigitalIn(8)?2:1;
	float position=ds->GetAnalogIn(1);
	position*=Multiplier;
	return position;
}

Goal *Get_TestLengthGoal(FRC_2011_Robot *Robot)
{
	//float position=DriverStation::GetInstance()->GetAnalogIn(1);
	float position=GetDS_Distance();
	//Construct a way point
	WayPoint wp;
	wp.Position[0]=0.0;
	wp.Position[1]=position;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_move1=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	Goal_Wait *goal_wait=new Goal_Wait(2.0); //wait
	wp.Position[1]=0;
	Goal_Ship_MoveToPosition *goal_move2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");

	MainGoal->AddSubgoal(goal_move2);
	MainGoal->AddSubgoal(goal_wait);
	MainGoal->AddSubgoal(goal_move1);
	return MainGoal;
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

	//This must happen first to ensure the elbow starts to open at the correct angle
	Goal_Ship1D_MoveToPosition *goal_arm_initialrest=new Goal_Ship1D_MoveToPosition(Arm,FRC_2011_Robot::Robot_Arm::GetPosRest());
	Goal_OperateSolenoid *goal_CloseClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,true);

	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);  //9 feet
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.7018);   //67 inches
	//give ability to tweak the correct height
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r((double)DriverStation::GetInstance()->GetAnalogIn(2));
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	//Note: full length is 232 inches or 5.89 meters
	//const double starting_line=5.49656;  //18.03333
	const double starting_line=(double)GetDS_Distance();

	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=starting_line;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);
	//We are going to score on 6 foot peg with elbow left close
	//Goal_OperateSolenoid *goal_OpenElbow=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eElbow,false);
	//Initial_Start_Goal->AddGoal(goal_OpenElbow);

	wp.Position[1]=starting_line+0.4572; //roughly 18 inches (Ideal would be half the distance of the pegs length)
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	
	Goal_OperateSolenoid *goal_OpenClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,false);
	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second
	
	//Note: Move the robot back enough to clear the peg without lowering the arm here (we will score by this point) ;)
	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	//Note: the last position could be back at start, but for now to be safe lets just go half the distance, unless its ability
	//to drive straight is doing better
	wp.Position[1]=starting_line/2.0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(-0.120);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *goal_arm_initialrest2=new Goal_Ship1D_MoveToPosition(Arm,0.06);

	MultitaskGoal *End_Goal=new MultitaskGoal;
	End_Goal->AddGoal(goal_arm_initialrest2);
	End_Goal->AddGoal(goal_drive4);

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_drive3);
	MainGoal->AddSubgoal(goal_arm2);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_OpenClaw);
	MainGoal->AddSubgoal(goal_drive2);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	//Note: I need not bother multi-task these since the close claw is so quick
	MainGoal->AddSubgoal(goal_arm_initialrest);
	MainGoal->AddSubgoal(goal_CloseClaw);
	return MainGoal;
};

Goal *Get_UberTubeGoal_V2(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(2.7432);  //9 feet
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.7018);   //67 inches
	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.08712);   //42.8 inches
	//give ability to tweak the correct height
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(
			(double)DriverStation::GetInstance()->GetAnalogIn(2));
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	//Construct a way point
	//Note: full length is 232 inches or 5.89 meters
	//const double starting_line=5.49656;  //18.03333
	//const double starting_line=2.3; //hack not calibrated
	//give ability to tweak on driver station
	//const double starting_line=(double)DriverStation::GetInstance()->GetAnalogIn(1);
	const double starting_line=(double)GetDS_Distance();
	
	WayPoint wp;
	wp.Position[0]=0;
	wp.Position[1]=starting_line;
	wp.Power=1.0;
	//Now to setup the goal
	Goal_Ship_MoveToPosition *goal_drive=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	Initial_Start_Goal->AddGoal(goal_drive);

	//This is a hack only needed if we cannot use encoders.  This gives robot time to slow down.
	//Goal_Wait *goal_waitforstop=new Goal_Wait(1.0); 

	wp.Position[1]=starting_line+0.1;
	Goal_Ship_MoveToPosition *goal_drive2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.83312);  //32.8 TODO find how much to lower
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second

	wp.Position[1]=starting_line;
	Goal_Ship_MoveToPosition *goal_drive3=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);

	//wp.Position[1]=0;
	//instead of going all the way back to start we'll go half the distance (since the robot is having hard time going straight)
	wp.Position[1]=starting_line/2.0;
	Goal_Ship_MoveToPosition *goal_drive4=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(0.0);
	Goal_Ship1D_MoveToPosition *goal_arm3=new Goal_Ship1D_MoveToPosition(Arm,position);

	MultitaskGoal *End_Goal=new MultitaskGoal;
	End_Goal->AddGoal(goal_arm3);
	End_Goal->AddGoal(goal_drive4);

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_drive3);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_arm2);
	MainGoal->AddSubgoal(goal_drive2);
	//MainGoal->AddSubgoal(goal_waitforstop);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	return MainGoal;
};

Goal *Test_Arm(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();
	//Now to setup the goal

	//double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(1.08712);   //42.8 inches
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(
			(double)DriverStation::GetInstance()->GetAnalogIn(2));

	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *Initial_Start_Goal=goal_arm;  //using the same variable name

	Goal_Wait *goal_waitfordrop=new Goal_Wait(5.0); //wait a half a second
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(-0.120);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);
	Goal_Wait *goal_waitfordrive=new Goal_Wait(2.0); //wait a half a second

	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(FRC_2011_Robot::Robot_Arm::GetPosRest());
	Goal_Ship1D_MoveToPosition *goal_arm3=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *End_Goal=goal_arm3;

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_waitfordrive);
	MainGoal->AddSubgoal(goal_arm2);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	return MainGoal;
};


Goal *Get_TestArmElbowClaw(FRC_2011_Robot *Robot)
{
	Ship_1D &Arm=Robot->GetArm();

	//Now to setup the goal

	//This must happen first to ensure the elbow starts to open at the correct angle
	Goal_Ship1D_MoveToPosition *goal_arm_initialrest=new Goal_Ship1D_MoveToPosition(Arm,FRC_2011_Robot::Robot_Arm::GetPosRest());
	Goal_OperateSolenoid *goal_CloseClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,true);

	//give ability to tweak the correct height
	double position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r((double)DriverStation::GetInstance()->GetAnalogIn(2));
	Goal_Ship1D_MoveToPosition *goal_arm=new Goal_Ship1D_MoveToPosition(Arm,position);


	MultitaskGoal *Initial_Start_Goal=new MultitaskGoal;
	Initial_Start_Goal->AddGoal(goal_arm);
	//Goal_OperateSolenoid *goal_OpenElbow=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eElbow,false);
	//Initial_Start_Goal->AddGoal(goal_OpenElbow);

	Goal_OperateSolenoid *goal_OpenClaw=new Goal_OperateSolenoid(*Robot,FRC_2011_Robot::eClaw,false);
	Goal_Wait *goal_waitfordrop=new Goal_Wait(0.5); //wait a half a second
	
	position=FRC_2011_Robot::Robot_Arm::HeightToAngle_r(-0.120);
	Goal_Ship1D_MoveToPosition *goal_arm2=new Goal_Ship1D_MoveToPosition(Arm,position);

	Goal_Ship1D_MoveToPosition *End_Goal=goal_arm2;

	//wrap the goal in a notify goal (Note: we don't need the notify, but we need a composite goal that is prepped properly)
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(End_Goal);
	MainGoal->AddSubgoal(goal_waitfordrop);
	MainGoal->AddSubgoal(goal_OpenClaw);
	MainGoal->AddSubgoal(Initial_Start_Goal);
	//Note: I need not bother multi-task these since the close claw is so quick
	MainGoal->AddSubgoal(goal_arm_initialrest);
	MainGoal->AddSubgoal(goal_CloseClaw);
	return MainGoal;
};

}
