#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "FRC2011_Robot.h"
	#include "Swerve_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);  //We may limit swerve to this
const double c_GearHeightOffset=1.397;  //55 inches
const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;

  /***********************************************************************************************************************************/
 /*															Swerve_Robot															*/
/***********************************************************************************************************************************/
Swerve_Robot::Swerve_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders) : 
	Swerve_Drive(EntityName), m_RobotControl(robot_control), 
	m_UsingEncoders(UseEncoders) //,m_VoltageOverride(false),m_UseDeadZoneSkip(true)
{
	//m_UsingEncoders=true; //testing
	//m_CalibratedScaler_Left=m_CalibratedScaler_Right=1.0;
}

void Swerve_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	//const FRC_2011_Robot_Properties *RobotProps=dynamic_cast<const FRC_2011_Robot_Properties *>(props);
	//m_Arm.Initialize(em,RobotProps?&RobotProps->GetArmProps():NULL);
	//m_Claw.Initialize(em,RobotProps?&RobotProps->GetClawProps():NULL);

	//const double OutputRange=MAX_SPEED*0.875;  //create a small range
	//const double InputRange=20.0;  //create a large enough number that can divide out the voltage and small enough to recover quickly
	//m_PIDController_Left.SetInputRange(-MAX_SPEED,MAX_SPEED);
	//m_PIDController_Left.SetOutputRange(-InputRange,OutputRange);
	//m_PIDController_Left.Enable();
	//m_PIDController_Right.SetInputRange(-MAX_SPEED,MAX_SPEED);
	//m_PIDController_Right.SetOutputRange(-InputRange,OutputRange);
	//m_PIDController_Right.Enable();
	//m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
}
void Swerve_Robot::ResetPos()
{
	__super::ResetPos();
	//m_Arm.ResetPos();
	//m_Claw.ResetPos();
	m_RobotControl->Reset_Encoders();
	//m_PIDController_Left.Reset(),m_PIDController_Right.Reset();
	//ensure teleop has these set properly
	//m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
	//m_UseDeadZoneSkip=true;
}

void Swerve_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->TimeChange(dTime_s);
	UI_TimeChange(dTime_s);
	//TODO add encoder support here
	//{
	//	//Display encoders without applying calibration
	//	double Encoder_LeftVelocity,Encoder_RightVelocity;
	//	m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
	//}
	__super::TimeChange(dTime_s);
}

bool Swerve_Robot::InjectDisplacement(double DeltaTime_s,Vec2d &PositionDisplacement,double &RotationDisplacement)
{
	bool ret=false;
	//if (m_UsingEncoders)
	//{
	//	Vec2d computedVelocity=m_Physics.GetLinearVelocity();
	//	//double computedAngularVelocity=m_Physics.GetAngularVelocity();
	//	m_Physics.SetLinearVelocity(m_EncoderGlobalVelocity);
	//	//m_Physics.SetAngularVelocity(m_EncoderHeading);
	//	m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
	//	//We must set this back so that the PID can compute the entire error
	//	m_Physics.SetLinearVelocity(computedVelocity);
	//	//m_Physics.SetAngularVelocity(computedAngularVelocity);
	//	ret=true;
	//}
	return ret;
}

double Swerve_Robot::RPS_To_LinearVelocity(double RPS)
{
	return RPS * c_MotorToWheelGearRatio * M_PI * c_WheelDiameter; 
}

//void Swerve_Robot::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
//{
//	m_VoltageOverride=false;
//	if ((m_UsingEncoders)&&(VelocityToUse==0.0)&&(m_rotDisplacement_rad==0.0))
//			m_VoltageOverride=true;
//}

void Swerve_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	//double LeftVelocity=GetLeftVelocity(),RightVelocity=GetRightVelocity();
	//m_RobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);
}

  /***************************************************************************************************************/
 /*													Wheel_UI													*/
/***************************************************************************************************************/

void Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	m_props=*props;
	m_Swivel=0.0;
	m_Rotation=0.0;
}

void Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
	m_Front= new osgText::Text;
	m_Front->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
	m_Front->setCharacterSize(m_UIParent->GetFontSize());
	m_Front->setFontResolution(10,10);
	m_Front->setPosition(position);
	m_Front->setAlignment(osgText::Text::CENTER_CENTER);
	m_Front->setText(L"U");
	m_Front->setUpdateCallback(m_UIParent);

	m_Back= new osgText::Text;
	m_Back->setColor(osg::Vec4(1.0,0.0,0.0,1.0));
	m_Back->setCharacterSize(m_UIParent->GetFontSize());
	m_Back->setFontResolution(10,10);
	m_Back->setPosition(position);
	m_Back->setAlignment(osgText::Text::CENTER_CENTER);
	m_Back->setText(L"U");
	m_Back->setUpdateCallback(m_UIParent);
}

void Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
	{
		if (m_Front.valid()) geode->addDrawable(m_Front);
		if (m_Back.valid()) geode->addDrawable(m_Back);
	}
	else
	{
		if (m_Front.valid()) geode->removeDrawable(m_Front);
		if (m_Back.valid()) geode->removeDrawable(m_Back);
	}
}

void Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	const double FS=m_UIParent->GetFontSize();
	Vec2d FrontSwivel(0.0,0.5);
	Vec2d BackSwivel(0.0,-0.5);
	FrontSwivel=GlobalToLocal(m_Swivel,FrontSwivel);
	BackSwivel=GlobalToLocal(m_Swivel,BackSwivel);

	const Vec2d frontOffset(m_props.m_Offset[0]+FrontSwivel[0],m_props.m_Offset[1]+FrontSwivel[1]);
	const Vec2d backOffset(m_props.m_Offset[0]+BackSwivel[0],m_props.m_Offset[1]+BackSwivel[1]);

	const Vec2d FrontLocalOffset=GlobalToLocal(Heading,frontOffset);
	const Vec2d BackLocalOffset=GlobalToLocal(Heading,backOffset);
	const osg::Vec3 frontPos(parent_pos[0]+(FrontLocalOffset[0]*FS),parent_pos[1]+(FrontLocalOffset[1]*FS),parent_pos[2]);
	const osg::Vec3 backPos (parent_pos[0]+( BackLocalOffset[0]*FS),parent_pos[1]+( BackLocalOffset[1]*FS),parent_pos[2]);

	if (m_Front.valid())
	{
		m_Front->setPosition(frontPos);
		m_Front->setRotation(FromLW_Rot_Radians(PI+Heading+m_Swivel,0.0,0.0));
	}
	if (m_Back.valid())
	{
		m_Back->setPosition(backPos);
		m_Back->setRotation(FromLW_Rot_Radians(Heading+m_Swivel,0.0,0.0));
	}

}

void Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Front.valid())	m_Front->setCharacterSize(SizeToUse);
	if (m_Back.valid()) m_Back->setCharacterSize(SizeToUse); 
}

  /***************************************************************************************************************/
 /*												Swerve_Robot_UI													*/
/***************************************************************************************************************/
void Swerve_Robot_UI::UI_Init(Actor_Text *parent)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i].UI_Init(parent);
}

void Swerve_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	Vec2D Offsets[4]=
	{
		Vec2D(-1.6, 2.0),
		Vec2D( 1.6, 2.0),
		Vec2D(-1.6,-2.0),
		Vec2D( 1.6,-2.0),
	};
	for (size_t i=0;i<4;i++)
	{
		Wheel_UI::Wheel_Properties props;
		props.m_Offset=Offsets[i];
		props.m_Wheel_Diameter=c_WheelDiameter;
		m_Wheel[i].Initialize(em,&props);
	}
	__super::Initialize(em,props);
}

void Swerve_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos)
{
	//just dispatch the update to the wheels (for now)
	for (size_t i=0;i<4;i++)
		m_Wheel[i].update(nv,draw,parent_pos,-GetAtt_r());
}

void Swerve_Robot_UI::Text_SizeToUse(double SizeToUse)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i].Text_SizeToUse(SizeToUse);
}

void Swerve_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i].UpdateScene(geode,AddOrRemove);
}

void Swerve_Robot_UI::UI_TimeChange(double dTime_s)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i].SetSwivel(GetVelocities(i));
}
