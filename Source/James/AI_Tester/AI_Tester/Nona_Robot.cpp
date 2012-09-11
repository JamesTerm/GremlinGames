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


  /***************************************************************************************************************/
 /*												Omni_Wheel_UI													*/
/***************************************************************************************************************/

void Omni_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	m_props=*props;
	m_Swivel=0.0;
	m_Rotation=0.0;
}

void Omni_Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
	m_Front= new osgText::Text;
	m_Front->setColor(osg::Vec4(0.3,0.3,1.0,1.0));
	m_Front->setCharacterSize(m_UIParent->GetFontSize());
	m_Front->setFontResolution(10,10);
	m_Front->setPosition(position);
	m_Front->setAlignment(osgText::Text::CENTER_CENTER);
	m_Front->setText(L"U");
	m_Front->setUpdateCallback(m_UIParent);

	m_Back= new osgText::Text;
	m_Back->setColor(osg::Vec4(0.1,0.1,1.0,1.0));
	m_Back->setCharacterSize(m_UIParent->GetFontSize());
	m_Back->setFontResolution(10,10);
	m_Back->setPosition(position);
	m_Back->setAlignment(osgText::Text::CENTER_CENTER);
	m_Back->setText(L"U");
	m_Back->setUpdateCallback(m_UIParent);

	m_Tread= new osgText::Text;
	m_Tread->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
	m_Tread->setCharacterSize(m_UIParent->GetFontSize());
	m_Tread->setFontResolution(10,10);
	m_Tread->setPosition(position);
	m_Tread->setAlignment(osgText::Text::CENTER_CENTER);
	//m_Tread->setText(L"\"");
	m_Tread->setText(L"__");
	m_Tread->setUpdateCallback(m_UIParent);

}

void Omni_Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
	{
		if (m_Front.valid()) geode->addDrawable(m_Front);
		if (m_Back.valid()) geode->addDrawable(m_Back);
		if (m_Tread.valid()) geode->addDrawable(m_Tread);
	}
	else
	{
		if (m_Front.valid()) geode->removeDrawable(m_Front);
		if (m_Back.valid()) geode->removeDrawable(m_Back);
		if (m_Tread.valid()) geode->removeDrawable(m_Tread);
	}
}

void Omni_Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	const double FS=m_UIParent->GetFontSize();
	Vec2d FrontSwivel(0.0,0.5);
	Vec2d BackSwivel(0.0,-0.5);
	//Vec2d TreadRotPos(0.0,cos(m_Rotation)-0.3);
	Vec2d TreadRotPos(sin(m_Rotation)*((fabs(m_Swivel)>PI_2)?0.5:-0.5),(cos(m_Rotation)*.8)+0.5);
	FrontSwivel=GlobalToLocal(m_Swivel,FrontSwivel);
	BackSwivel=GlobalToLocal(m_Swivel,BackSwivel);
	TreadRotPos=GlobalToLocal(m_Swivel,TreadRotPos);

	const Vec2d frontOffset(m_props.m_Offset[0]+FrontSwivel[0],m_props.m_Offset[1]+FrontSwivel[1]);
	const Vec2d backOffset(m_props.m_Offset[0]+BackSwivel[0],m_props.m_Offset[1]+BackSwivel[1]);
	const Vec2d TreadOffset(m_props.m_Offset[0]+TreadRotPos[0],m_props.m_Offset[1]+TreadRotPos[1]);

	const Vec2d FrontLocalOffset=GlobalToLocal(Heading,frontOffset);
	const Vec2d BackLocalOffset=GlobalToLocal(Heading,backOffset);
	const Vec2d TreadLocalOffset=GlobalToLocal(Heading,TreadOffset);
	const osg::Vec3 frontPos(parent_pos[0]+(FrontLocalOffset[0]*FS),parent_pos[1]+(FrontLocalOffset[1]*FS),parent_pos[2]);
	const osg::Vec3 backPos (parent_pos[0]+( BackLocalOffset[0]*FS),parent_pos[1]+( BackLocalOffset[1]*FS),parent_pos[2]);
	const osg::Vec3 TreadPos (parent_pos[0]+( TreadLocalOffset[0]*FS),parent_pos[1]+( TreadLocalOffset[1]*FS),parent_pos[2]);

	const double TreadColor=((sin(-m_Rotation) + 1.0)/2.0) * 0.8 + 0.2;
	m_Tread->setColor(osg::Vec4(TreadColor,TreadColor,TreadColor,1.0));

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
	if (m_Tread.valid())
	{
		m_Tread->setPosition(TreadPos);
		m_Tread->setRotation(FromLW_Rot_Radians(Heading+m_Swivel,0.0,0.0));
	}
}

void Omni_Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Front.valid())	m_Front->setCharacterSize(SizeToUse);
	if (m_Back.valid()) m_Back->setCharacterSize(SizeToUse);
	if (m_Tread.valid()) m_Tread->setCharacterSize(SizeToUse);
}

void Omni_Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}

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
