#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
	#include "FRC2012_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*														FRC_2012_Robot::Turret														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::Turret::Turret(Rotary_Control_Interface *robot_control) : Rotary_Linear("Turret",robot_control,eTurret)
{
}

void FRC_2012_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	GG_Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::Turret::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}

}


  /***********************************************************************************************************************************/
 /*															FRC_2012_Robot															*/
/***********************************************************************************************************************************/
FRC_2012_Robot::FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders) : 
	Tank_Robot(EntityName,robot_control,UseEncoders), m_RobotControl(robot_control), m_Turret(robot_control)
{
}

void FRC_2012_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2012_Robot_Properties *RobotProps=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	m_Turret.Initialize(em,RobotProps?&RobotProps->GetTurretProps():NULL);
	//m_Claw.Initialize(em,RobotProps?&RobotProps->GetClawProps():NULL);
}
void FRC_2012_Robot::ResetPos()
{
	__super::ResetPos();
	//m_Arm.ResetPos();
	//m_Claw.ResetPos();
}

void FRC_2012_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	Entity1D &turret_entity=m_Turret;  //This gets around keeping time change protected in derived classes
	turret_entity.TimeChange(dTime_s);
	//Entity1D &claw_entity=m_Claw;  //This gets around keeping time change protected in derived classes
	//claw_entity.TimeChange(dTime_s);
}

const double c_rMotorDriveForward_DeadZone=0.02;
const double c_rMotorDriveReverse_DeadZone=0.02;
const double c_lMotorDriveForward_DeadZone=0.02;
const double c_lMotorDriveReverse_DeadZone=0.02;

const double c_rMotorDriveForward_Range=1.0-c_rMotorDriveForward_DeadZone;
const double c_rMotorDriveReverse_Range=1.0-c_rMotorDriveReverse_DeadZone;
const double c_lMotorDriveForward_Range=1.0-c_lMotorDriveForward_DeadZone;
const double c_lMotorDriveReverse_Range=1.0-c_lMotorDriveReverse_DeadZone;

void FRC_2012_Robot::ComputeDeadZone(double &LeftVoltage,double &RightVoltage)
{
	//Eliminate the deadzone
	if (LeftVoltage>0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveForward_Range) + c_lMotorDriveForward_DeadZone;
	else if (LeftVoltage < 0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveReverse_Range) - c_lMotorDriveReverse_DeadZone;

	if (RightVoltage>0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveForward_Range) + c_rMotorDriveForward_DeadZone;
	else if (RightVoltage < 0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveReverse_Range) - c_rMotorDriveReverse_DeadZone;
}


void FRC_2012_Robot::BindAdditionalEventControls(bool Bind)
{
	//Entity2D::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	//if (Bind)
	//	em->EventOnOff_Map["Robot_CloseDoor"].Subscribe(ehl, *this, &FRC_2012_Robot::CloseDeploymentDoor);
	//else
	//	em->EventOnOff_Map["Robot_CloseDoor"]  .Remove(*this, &FRC_2012_Robot::CloseDeploymentDoor);

	Ship_1D &TurretEntity_Access=m_Turret;
	TurretEntity_Access.BindAdditionalEventControls(Bind);
	//Ship_1D &ClawShip_Access=m_Claw;
	//ClawShip_Access.BindAdditionalEventControls(Bind);
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2012_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	switch (index)
	{
		case FRC_2012_Robot::eTurret:
		{
			//	printf("Arm=%f\n",Voltage);
			//DOUT3("Arm Voltage=%f",Voltage);
			m_TurretVoltage=Voltage;
			m_Potentiometer.UpdatePotentiometerVoltage(Voltage);
			m_Potentiometer.TimeChange();  //have this velocity immediately take effect
		}
			break;
		case FRC_2012_Robot::ePowerWheels:
			m_RollerVoltage=Voltage;
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
	}
}

FRC_2012_Robot_Control::FRC_2012_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_TurretVoltage(0.0),m_RollerVoltage(0.0)
{
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
}

void FRC_2012_Robot_Control::Reset_Rotary(size_t index)
{
	m_KalFilter_Arm.Reset();
}

void FRC_2012_Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2012_Robot_Properties *robot_props=dynamic_cast<const FRC_2012_Robot_Properties *>(props);

	assert(robot_props);

	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
	m_Potentiometer.Initialize(&robot_props->GetTurretProps());
}

void FRC_2012_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Potentiometer.SetTimeDelta(dTime_s);
	//display voltages
	DOUT2("l=%f r=%f t=%f\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),m_TurretVoltage);
}


double FRC_2012_Robot_Control::GetRotaryCurrentPosition(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2012_Robot::eTurret:
		
			result=m_Potentiometer.GetPotentiometerCurrentPosition();
			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			DOUT4 ("pot=%f",result);
			break;
		case FRC_2012_Robot::ePowerWheels:
			break;
	}
	return result;
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2012_Robot_Properties::FRC_2012_Robot_Properties()  : m_TurretProps(
	"Arm",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Properties::eSwivel,
	true,	//Using the range
	-Pi,Pi
	)
{
	{
		Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

		//Late assign this to override the initial default
		props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side
		props.WheelDiameter=c_WheelDiameter;
		props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
		props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
		m_TankRobotProps=props;
	}
	{
		Rotary_Props props=m_TurretProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_TurretProps.RoteryProps()=props;
	}
}


  /***********************************************************************************************************************************/
 /*														FRC_2012_Turret_UI															*/
/***********************************************************************************************************************************/

void FRC_2012_Turret_UI::Initialize(Entity2D::EventMap& em, const Turret_Properties *props)
{
	if (props)
		m_props=*props;
	else
		m_props.YOffset=2.0;
}
void FRC_2012_Turret_UI::UI_Init(Actor_Text *parent)
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Turret= new osgText::Text;
	m_Turret->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
	m_Turret->setCharacterSize(m_UIParent->GetFontSize());
	m_Turret->setFontResolution(10,10);
	m_Turret->setPosition(position);
	m_Turret->setAlignment(osgText::Text::CENTER_CENTER);
	m_Turret->setText(L"\\/\n|\n||\n||\n( )");
	m_Turret->setUpdateCallback(m_UIParent);
}
void FRC_2012_Turret_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	FRC_2012_Control_Interface *turret_access=m_RobotControl;
	double HeadingToUse=Heading+(-turret_access->GetRotaryCurrentPosition(FRC_2012_Robot::eTurret));
	const double FS=m_UIParent->GetFontSize();
	const Vec2d TurretOffset(0,m_props.YOffset);
	const Vec2d TurretLocalOffset=GlobalToLocal(HeadingToUse ,TurretOffset);
	const osg::Vec3 TurretPos (parent_pos[0]+( TurretLocalOffset[0]*FS),parent_pos[1]+( TurretLocalOffset[1]*FS),parent_pos[2]);

	//const char *TeamName=m_UIParent->GetEntityProperties_Interface()->GetTeamName();
	//if (strcmp(TeamName,"red")==0)
	//	m_Turret->setColor(osg::Vec4(1.0f,0.0f,0.5f,1.0f));  //This is almost magenta (easier to see)
	//else if (strcmp(TeamName,"blue")==0)
	//	m_Turret->setColor(osg::Vec4(0.0f,0.5f,1.0f,1.0f));  //This is almost cyan (easier to see too)

	if (m_Turret.valid())
	{
		m_Turret->setPosition(TurretPos);
		m_Turret->setRotation(FromLW_Rot_Radians(HeadingToUse,0.0,0.0));
	}

}
void FRC_2012_Turret_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Turret.valid()) m_Turret->setCharacterSize(SizeToUse);
}
void FRC_2012_Turret_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Turret.valid()) geode->addDrawable(m_Turret);
	else
		if (m_Turret.valid()) geode->removeDrawable(m_Turret);
}
