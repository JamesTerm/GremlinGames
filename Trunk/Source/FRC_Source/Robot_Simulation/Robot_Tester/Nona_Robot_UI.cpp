#include "stdafx.h"
#include "Robot_Tester.h"
namespace Robot_Tester
{
	#include "CommonUI.h"
	#include "Swerve_Robot_UI.h"
	#include "Nona_Robot_UI.h"
}

using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

namespace Scripting=GG_Framework::Logic::Scripting;
//namespace Scripting=Framework::Scripting;

//const double PI=M_PI;
const double Pi2=M_PI*2.0;
const double Pi_Half=1.57079632679489661923;


  /***********************************************************************************************************/
 /*												Butterfly_Robot_UI											*/
/***********************************************************************************************************/

void Butterfly_Robot_UI::DriveModeManager_SetMode_Callback(DriveMode Mode)
{
	for (size_t i=0;i<4;i++)
	{
		if (Mode==Butterfly_Robot::eTractionDrive)
		{
			m_Wheel[i]->SetWheelColor(osg::Vec4(1.0,0.0,0.0,1.0),Swivel_Wheel_UI::eFront);
			m_Wheel[i]->SetWheelColor(osg::Vec4(0.5,0.1,0.1,1.0),Swivel_Wheel_UI::eBack);
		}
		else
		{
			m_Wheel[i]->SetWheelColor(m_Wheel[i]->GetFrontWheelColor(),Swivel_Wheel_UI::eFront);
			m_Wheel[i]->SetWheelColor(m_Wheel[i]->GetBackWheelColor(),Swivel_Wheel_UI::eBack);
		}
	}
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

void Nona_Robot_UI::DriveModeManager_SetMode_Callback(DriveMode Mode)
{
	for (size_t i=0;i<5;i++)
	{
		if (Mode==Butterfly_Robot::eTractionDrive)
		{
			if (i!=4)
			{
				m_Wheel[i].SetWheelColor(osg::Vec4(1.0,0.0,0.0,1.0),Swivel_Wheel_UI::eFront);
				m_Wheel[i].SetWheelColor(osg::Vec4(0.5,0.1,0.1,1.0),Swivel_Wheel_UI::eBack);
			}
			else
			{
				m_Wheel[i].SetWheelColor(osg::Vec4(0.2,0.2,0.2,1.0),Swivel_Wheel_UI::eFront);
				m_Wheel[i].SetWheelColor(osg::Vec4(0.2,0.2,0.2,1.0),Swivel_Wheel_UI::eBack);
			}
		}
		else
		{
			m_Wheel[i].SetWheelColor(m_Wheel[i].GetFrontWheelColor(),Swivel_Wheel_UI::eFront);
			m_Wheel[i].SetWheelColor(m_Wheel[i].GetBackWheelColor(),Swivel_Wheel_UI::eBack);
		}
	}
}
