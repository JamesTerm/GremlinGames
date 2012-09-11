#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "PIDController.h"
	#include "Calibration_Testing.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
	#include "Swerve_Robot.h"
	#include "Nona_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

namespace Scripting=GG_Framework::Logic::Scripting;
//namespace Scripting=Framework::Scripting;

const double Pi2=M_PI*2.0;


  /***********************************************************************************************************/
 /*												Butterfly_Robot												*/
/***********************************************************************************************************/

Butterfly_Robot::Butterfly_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous) : 
	Swerve_Robot(EntityName,robot_control,IsAutonomous)
{

}

Butterfly_Robot::~Butterfly_Robot()
{

}

  /***********************************************************************************************************/
 /*													Nona_Robot												*/
/***********************************************************************************************************/

Nona_Robot::Nona_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous) : 
Butterfly_Robot(EntityName,robot_control,IsAutonomous),m_KickerWheel("KickerWheel",robot_control,eWheel_Kicker),m_NonaDrive(NULL)
{
}

Swerve_Drive *Nona_Robot::CreateDrive() 
{
	const_cast<Nona_Drive *>(m_NonaDrive)=new Nona_Drive(this);
	return m_NonaDrive;
}

Nona_Robot::~Nona_Robot()
{
}

void Nona_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	const Nona_Robot_Properties *RobotProps=dynamic_cast<const Nona_Robot_Properties *>(props);
	if (RobotProps)
	{
		const Rotary_Properties &kickerwheel=RobotProps->GetKickerWheelProps();
		m_KickerWheel.Initialize(em,&kickerwheel);
	}
}

void Nona_Robot::InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s)
{
	double encoderVelocity=m_RobotControl->GetRotaryCurrentPorV(eWheel_Kicker);
	const double IntendedVelocity=m_NonaDrive->GetKickerWheelIntendedVelocity();
	m_NonaDrive->SetKickerWheelVelocity(encoderVelocity);
	__super::InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
	m_NonaDrive->SetKickerWheelVelocity(IntendedVelocity);
	m_KickerWheel.SetRequestedVelocity(IntendedVelocity);
	m_KickerWheel.AsEntity1D().TimeChange(dTime_s);
}

  /***********************************************************************************************************/
 /*												Nona_Robot_Properties										*/
/***********************************************************************************************************/

Nona_Robot_Properties::Nona_Robot_Properties() : m_KickerWheelProps(
	"Drive",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//These should match the settings in the script
	2.450592,   //Max Speed (This is linear movement speed)
	//10, //TODO find out why I need 8 for turns
	10.0,10.0, //ACCEL, BRAKE  (These can be ignored)
	//Make these as fast as possible without damaging chain or motor
	//These must be fast enough to handle rotation angular acceleration
	4.0,4.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Properties::eSimpleMotor,
	false	//No limit ever!
	)
{
	//Always use aggressive stop for driving
	m_KickerWheelProps.RoteryProps().UseAggressiveStop=true;
}

void Nona_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("kicker");
	if (!err)
	{
		m_KickerWheelProps.LoadFromScript(script);
		script.Pop();
	}
	__super::LoadFromScript(script);
}

  /***********************************************************************************************************/
 /*												Nona_Robot_Control											*/
/***********************************************************************************************************/

Nona_Robot_Control::Nona_Robot_Control() : m_KickerWheelVoltage(0.0)
{
	SetDisplayVoltage(false);  //override so we can display it here
}

void Nona_Robot_Control::Reset_Rotary(size_t index)
{
	if (index==Nona_Robot::eWheel_Kicker)
		m_KickerWheelEncoder.ResetPos();
	else
		__super::Reset_Rotary(index);
}

double Nona_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result;
	if (index==Nona_Robot::eWheel_Kicker)
	{
		result=m_KickerWheelEncoder.GetEncoderVelocity();
		//DOUT5("%f",result);
	}
	else
		result=__super::GetRotaryCurrentPorV(index);
	return result;
}

void Nona_Robot_Control::UpdateRotaryVoltage(size_t index,double Voltage)
{
	if (index==Nona_Robot::eWheel_Kicker)
	{
		//DOUT5("%f",Voltage);
		m_KickerWheelVoltage=Voltage;
		m_KickerWheelEncoder.UpdateEncoderVoltage(Voltage);
		m_KickerWheelEncoder.TimeChange();
	}
	else
		__super::UpdateRotaryVoltage(index,Voltage);
}

void Nona_Robot_Control::Initialize(const Entity_Properties *props)
{
	const Nona_Robot_Properties *robot_props=dynamic_cast<const Nona_Robot_Properties *>(props);

	//For now robot_props can be NULL since the swerve robot is borrowing it
	if (robot_props)
	{
		m_KickerWheelEncoder.Initialize(&robot_props->GetKickerWheelProps());
	}
	__super::Initialize(props);
}

void Nona_Robot_Control::Reset_Encoders()
{
	Reset_Rotary(Nona_Robot::eWheel_Kicker);
	__super::Reset_Encoders();
}

void Nona_Robot_Control::Swerve_Drive_Control_TimeChange(double dTime_s)
{
	m_KickerWheelEncoder.SetTimeDelta(dTime_s);
	//would need to have a condition override here if this class is subclassed
	if (true)
	{
		//display voltages
		DOUT2("fl=%.2f fr=%.2f rl=%.2f rr=%.2f kw=%.2f\n",m_EncoderVoltage[Swerve_Robot::eWheel_FL],m_EncoderVoltage[Swerve_Robot::eWheel_FR],
			m_EncoderVoltage[Swerve_Robot::eWheel_RL],m_EncoderVoltage[Swerve_Robot::eWheel_RR],m_KickerWheelVoltage);
	}
	__super::Swerve_Drive_Control_TimeChange(dTime_s);
}
