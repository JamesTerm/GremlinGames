#include "WPILib.h"
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

using namespace Framework::Base;

  /***********************************************************************************************************************************/
 /*														Driver_Station_Joystick														*/
/***********************************************************************************************************************************/

size_t Driver_Station_Joystick::GetNoJoysticksFound() 
{
	return m_NoJoysticks;
}

bool Driver_Station_Joystick::read_joystick (size_t nr, JoyState &Info)
{
	//First weed out numbers not in range
	int Number=(int)nr;
	Number-=m_StartingPort;
	bool ret=false;
	nr++;  //DOH the number selection is cardinal!  :(
	if ((Number>=0) && (Number<m_NoJoysticks))
	{
		memset(&Info,0,sizeof(JoyState));  //zero the memory
		//The axis selection is also ordinal
		Info.lX=m_ds->GetStickAxis(nr,1);
		Info.lY=m_ds->GetStickAxis(nr,2);
		Info.lZ=m_ds->GetStickAxis(nr,3);
		Info.lRx=m_ds->GetStickAxis(nr,4);
		Info.lRy=m_ds->GetStickAxis(nr,5);
		Info.ButtonBank[0]=m_ds->GetStickButtons(nr);
		ret=true;
	}
	return ret;
}

Driver_Station_Joystick::Driver_Station_Joystick(int NoJoysticks,int StartingPort) : m_NoJoysticks(NoJoysticks), m_StartingPort(StartingPort)
{
	m_ds = DriverStation::GetInstance();
	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName="Joystick_1";
	common.InstanceName="Driver_Station";
	common.JoyCapFlags=
		JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
		JoystickInfo::fX_Rot|JoystickInfo::fY_Rot;
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=true;
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	common.ProductName="Joystick_2";
	m_JoyInfo.push_back(common);
	common.ProductName="Joystick_3";
	m_JoyInfo.push_back(common);
	common.ProductName="Joystick_4";
	m_JoyInfo.push_back(common);
}

Driver_Station_Joystick::~Driver_Station_Joystick()
{
}

  /***********************************************************************************************************************************/
 /*															Robot_Control															*/
/***********************************************************************************************************************************/

void Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2011_Robot_Properties *robot_props=static_cast<const FRC_2011_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
}

void Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	LeftVelocity=0.0,RightVelocity=0.0;
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->PrintfLine(DriverStationLCD::kUser_Line4, "l=%.1f r=%.1f", m_LeftEncoder.Get(),m_RightEncoder.Get());	
}

void Robot_Control::UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity)
{
	//DOUT2("left=%f right=%f \n",LeftVelocity/m_RobotMaxSpeed,RightVelocity/m_RobotMaxSpeed);
	m_RobotDrive.SetLeftRightMotorSpeeds((float)(LeftVelocity/m_RobotMaxSpeed),(float)(RightVelocity/m_RobotMaxSpeed));
}
void Robot_Control::UpdateArmVelocity(double Velocity)
{
	//DOUT4("Arm=%f",Velocity/m_ArmMaxSpeed);
	float VelocityToUse=(float)(Velocity/m_ArmMaxSpeed);
	m_ArmMotor.SetLeftRightMotorSpeeds(VelocityToUse,VelocityToUse);  //always the same velocity for both!
}

double Robot_Control::GetArmCurrentPosition()
{
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	DriverStation *ds=DriverStation::GetInstance();
	double raw_value = (double) ds->GetAnalogIn(1); //TODO determine what channel this is
	double ret=FRC_2011_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(raw_value);
	//I may keep these on as they should be useful feedback
	#if 1
	double height=FRC_2011_Robot::Robot_Arm::Arm_AngleToHeight_m(ret);
	lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %.1fft %.1fin", RAD_2_DEG(ret),height*3.2808399,height*39.3700787);
	//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "1: Pot=%.1f ", raw_value);
	#endif
	return ret;
}
