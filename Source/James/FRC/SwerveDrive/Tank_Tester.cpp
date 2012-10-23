#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "../Common/Entity_Properties.h"
#include "../Common/Physics_1D.h"
#include "../Common/Physics_2D.h"
#include "../Common/Entity2D.h"
#include "../Common/Goal.h"
#include "../Common/Ship.h"
#include "../Common/AI_Base_Controller.h"
#include "../Common/Vehicle_Drive.h"
#include "../Common/PIDController.h"
#include "Tank_Tester.h"

using namespace Framework::Base;
using namespace std;

//namespace Scripting=GG_Framework::Logic::Scripting;
namespace Scripting=Framework::Scripting;



  /***********************************************************************************************************************************/
 /*													Tank_Robot_Properties															*/
/***********************************************************************************************************************************/

Tank_Robot_Properties::Tank_Robot_Properties()
{
	Tank_Robot_Props props;
	memset(&props,0,sizeof(Tank_Robot_Props));

	//Late assign this to override the initial default
	props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side
	const double c_WheelDiameter=0.1524;  //6 inches
	props.WheelDiameter=c_WheelDiameter;
	props.LeftPID[0]=props.RightPID[0]=1.0; //set PIDs to a safe default of 1,0,0
	props.InputLatency=0.0;
	props.HeadingLatency=0.0;
	props.MotorToWheelGearRatio=1.0;  //most-likely this will be overridden
	props.VoltageScalar=1.0;  //May need to be reversed
	props.Feedback_DiplayRow=(size_t)-1;  //Only assigned to a row during calibration of feedback sensor
	props.IsOpen=true;  //Always true by default until control is fully functional
	props.PID_Console_Dump=false;  //Always false unless you want to analyze PID (only one system at a time!)
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	props.LeftMaxSpeedOffset=props.RightMaxSpeedOffset=0.0;
	props.ReverseSteering=false;
	props.Polynomial[0]=0.0;
	props.Polynomial[1]=1.0;
	props.Polynomial[2]=0.0;
	props.Polynomial[3]=0.0;
	props.Polynomial[4]=0.0;
	props.LeftEncoderReversed=false;
	props.RightEncoderReversed=false;
	props.DriveTo_ForceDegradeScalar=Vec2d(1.0,1.0);
	props.TankSteering_Tolerance=0.05;
	props.InverseMaxAccel=0.0;
	m_TankRobotProps=props;
}

void Tank_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("tank_drive");
	if (!err) 
	{
		//Quick snap shot of all the properties
		//Vec2D WheelDimensions;
		//double WheelDiameter;
		//double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
		//double LeftPID[3]; //p,i,d
		//double RightPID[3]; //p,i,d
		//Get the ship dimensions
		err = script.GetFieldTable("wheel_base_dimensions");
		if (!err)
		{
			double length_in, width_in;	
			//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
			err = script.GetField("length_in", NULL, NULL,&length_in);
			ASSERT_MSG(!err, err);
			err = script.GetField("width_in", NULL, NULL,&width_in);
			ASSERT_MSG(!err, err);
			m_TankRobotProps.WheelDimensions=Vec2D(Inches2Meters(width_in),Inches2Meters(length_in));  //x,y  where x=width
			script.Pop();
		}

		double wheel_diameter;
		err=script.GetField("wheel_diameter_in", NULL, NULL, &wheel_diameter);
		if (!err)
			m_TankRobotProps.WheelDiameter=Inches2Meters(wheel_diameter);
		script.GetField("encoder_to_wheel_ratio", NULL, NULL, &m_TankRobotProps.MotorToWheelGearRatio);
		script.GetField("voltage_multiply", NULL, NULL, &m_TankRobotProps.VoltageScalar);
		err = script.GetFieldTable("left_pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_TankRobotProps.LeftPID[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_TankRobotProps.LeftPID[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_TankRobotProps.LeftPID[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		err = script.GetFieldTable("right_pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_TankRobotProps.RightPID[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_TankRobotProps.RightPID[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_TankRobotProps.RightPID[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		script.GetField("tolerance", NULL, NULL, &m_TankRobotProps.PrecisionTolerance);
		script.GetField("latency", NULL, NULL, &m_TankRobotProps.InputLatency);
		err = script.GetField("heading_latency", NULL, NULL, &m_TankRobotProps.HeadingLatency);
		if (err)
			m_TankRobotProps.HeadingLatency=m_TankRobotProps.InputLatency+0.100;  //Give a good default without needing to add this property
		script.GetField("left_max_offset", NULL, NULL, &m_TankRobotProps.LeftMaxSpeedOffset);
		script.GetField("right_max_offset", NULL, NULL, &m_TankRobotProps.RightMaxSpeedOffset);
		script.GetField("drive_to_scale", NULL, NULL, &m_TankRobotProps.DriveTo_ForceDegradeScalar[1]);
		script.GetField("strafe_to_scale", NULL, NULL, &m_TankRobotProps.DriveTo_ForceDegradeScalar[0]);

		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_TankRobotProps.Feedback_DiplayRow=(size_t)fDisplayRow;

		string sTest;
		err = script.GetField("is_closed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
				m_TankRobotProps.IsOpen=true;
			else
				m_TankRobotProps.IsOpen=false;
		}
		err = script.GetField("show_pid_dump",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.PID_Console_Dump=true;
		}
		err = script.GetField("reverse_steering",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.ReverseSteering=true;
		}
		err = script.GetField("left_encoder_reversed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.LeftEncoderReversed=true;
		}
		err = script.GetField("right_encoder_reversed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.RightEncoderReversed=true;
		}

		err = script.GetFieldTable("curve_voltage");
		if (!err)
		{
			err = script.GetField("c", NULL, NULL,&m_TankRobotProps.Polynomial[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t1", NULL, NULL,&m_TankRobotProps.Polynomial[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t2", NULL, NULL,&m_TankRobotProps.Polynomial[2]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t3", NULL, NULL,&m_TankRobotProps.Polynomial[3]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t4", NULL, NULL,&m_TankRobotProps.Polynomial[4]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}

		script.GetField("inv_max_accel", NULL, NULL, &m_TankRobotProps.InverseMaxAccel);
		script.Pop(); 
	}
	err = script.GetFieldTable("controls");
	if (!err)
	{
		script.GetField("tank_steering_tolerance", NULL, NULL,&m_TankRobotProps.TankSteering_Tolerance);
		script.Pop();
	}

	__super::LoadFromScript(script);
}

