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

//const double PI=M_PI;
const double Pi2=M_PI*2.0;
const double Pi_Half=1.57079632679489661923;

  /***********************************************************************************************************/
 /*										Butterfly_Robot::DriveModeManager 									*/
/***********************************************************************************************************/
Butterfly_Robot::DriveModeManager::DriveModeManager(Butterfly_Robot *parent) : m_pParent(parent)
{
}

void Butterfly_Robot::DriveModeManager::SetMode(DriveMode Mode)
{
	const TractionModeProps *PropsToUse=(Mode==eTractionDrive)?&m_TractionModeProps:&m_OmniModeProps;
	m_pParent->UpdateShipProperties(PropsToUse->ShipProperties.GetShipProps());
	//init the props (more of a pedantic step to avoid corrupt data)
	Rotary_Props props=m_ButterflyProps.GetDriveProps().GetRoteryProps();
	props.InverseMaxForce=m_TractionModeProps.InverseMaxForce;
	props.LoopState=(m_TractionModeProps.IsOpen)?Rotary_Props::eOpen : Rotary_Props::eClosed;
	//Now for the hand-picked swerve properties
	for (size_t i=0;i<4;i++)
		m_pParent->UpdateDriveProps(props,i);
}

void Butterfly_Robot::DriveModeManager::Initialize(const Butterfly_Robot_Properties &props)
{
	m_ButterflyProps=props;
	m_TractionModeProps=props.GetTractionModeProps();
	m_OmniModeProps.ShipProperties=m_pParent->m_ShipProps;
	m_OmniModeProps.IsOpen=m_pParent->GetSwerveRobotProps().IsOpen_Wheel;
	m_OmniModeProps.InverseMaxForce=m_pParent->GetSwerveRobotProps().InverseMaxForce;
}

  /***********************************************************************************************************/
 /*												Butterfly_Robot												*/
/***********************************************************************************************************/

Butterfly_Robot::Butterfly_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous) : 
	Swerve_Robot(EntityName,robot_control,IsAutonomous),m_DriveModeManager(this)
{

}

Butterfly_Robot::~Butterfly_Robot()
{

}

void Butterfly_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	const Butterfly_Robot_Properties *RobotProps=dynamic_cast<const Butterfly_Robot_Properties *>(props);
	if (RobotProps)
		m_DriveModeManager.Initialize(*RobotProps);
}

  /***********************************************************************************************************/
 /*											Butterfly_Robot_Properties										*/
/***********************************************************************************************************/

Butterfly_Robot_Properties::Butterfly_Robot_Properties()
{
	memset(&m_TractionModePropsProps,0,sizeof(TractionModeProps));
}

void Butterfly_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	__super::LoadFromScript(script);
	m_TractionModePropsProps.ShipProperties=*this;
	const char* err=NULL;
	err = script.GetFieldTable("low_gear");
	if (!err)
	{
		m_TractionModePropsProps.ShipProperties.LoadFromScript(script);

		err = script.GetFieldTable("swerve_drive");
		if (!err) 
		{
			string sTest;
			err = script.GetField("is_closed",&sTest,NULL,NULL);
			if (!err)
			{
				if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
					m_TractionModePropsProps.IsOpen=true;
				else
					m_TractionModePropsProps.IsOpen=false;
			}
			err = script.GetField("inv_max_force", NULL, NULL, &m_TractionModePropsProps.InverseMaxForce);
			script.Pop();
		}
		script.Pop();
	}
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


  /***************************************************************************************************************/
 /*												Omni_Wheel_UI													*/
/***************************************************************************************************************/


  /***************************************************************************************************************/
 /*												Nona_Robot_UI													*/
/***************************************************************************************************************/
void Nona_Robot_UI::UI_Init(Actor_Text *parent)
{
	for (size_t i=0;i<5;i++)
		m_Wheel[i].UI_Init(parent);
}

void Nona_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	Vec2D Offsets[5]=
	{
		Vec2D(-1.6, 2.0),
		Vec2D( 1.6, 2.0),
		Vec2D(-1.6,-2.0),
		Vec2D( 1.6,-2.0),
		Vec2D( 0.0,0.0),
	};
	for (size_t i=0;i<5;i++)
	{
		Omni_Wheel_UI::Wheel_Properties props;
		props.m_Offset=Offsets[i];
		props.m_Wheel_Diameter=m_NonaRobot->GetSwerveRobotProps().WheelDiameter;
		m_Wheel[i].Initialize(em,&props);
	}
}

void Nona_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos)
{
	//just dispatch the update to the wheels (for now)
	for (size_t i=0;i<5;i++)
		m_Wheel[i].update(nv,draw,parent_pos,-(m_NonaRobot->GetAtt_r()));
}

void Nona_Robot_UI::Text_SizeToUse(double SizeToUse)
{
	for (size_t i=0;i<5;i++)
		m_Wheel[i].Text_SizeToUse(SizeToUse);
}

void Nona_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	for (size_t i=0;i<5;i++)
		m_Wheel[i].UpdateScene(geode,AddOrRemove);
}

void Nona_Robot_UI::TimeChange(double dTime_s)
{
	Nona_Robot &_=*m_NonaRobot;
	for (size_t i=0;i<4;i++)
	{
		//TODO GetIntendedVelocities for intended UI
		m_Wheel[i].SetSwivel(_.GetSwerveVelocitiesFromIndex(i+4));
		//For the linear velocities we'll convert to angular velocity and then extract the delta of this slice of time
		//const double LinearVelocity=_.GetSwerveVelocitiesFromIndex(i);
		const double LinearVelocity=const_cast<Swerve_Drive_Control_Interface &>(_.GetRobotControl()).GetRotaryCurrentPorV(i);
		const double PixelHackScale=m_Wheel[i].GetFontSize()/8.0;  //scale the wheels to be pixel aesthetic
		const double RPS=LinearVelocity /  (PI * _.GetSwerveRobotProps().WheelDiameter * PixelHackScale);
		const double AngularVelocity=RPS * Pi2;
		m_Wheel[i].AddRotation(AngularVelocity*dTime_s);
	}
	//Setup the kicker wheel
	m_Wheel[4].SetSwivel(Pi_Half);
	const double LinearVelocity=const_cast<Swerve_Drive_Control_Interface &>(_.GetRobotControl()).GetRotaryCurrentPorV(Nona_Robot::eWheel_Kicker);
	const double PixelHackScale=m_Wheel[4].GetFontSize()/8.0;  //scale the wheels to be pixel aesthetic
	const double RPS=LinearVelocity /  (PI * _.GetSwerveRobotProps().WheelDiameter * PixelHackScale);
	const double AngularVelocity=RPS * Pi2;
	m_Wheel[4].AddRotation(AngularVelocity*dTime_s);
}
