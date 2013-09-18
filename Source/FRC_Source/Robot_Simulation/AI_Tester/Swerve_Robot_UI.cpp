#include "stdafx.h"
#include "AI_Tester.h"

namespace AI_Tester
{
	#include "CommonUI.h"
	#include "Swerve_Robot_UI.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

namespace Scripting=GG_Framework::Logic::Scripting;
//namespace Scripting=Framework::Scripting;

const double Pi=M_PI;
const double Pi2=Pi*2.0;


  /***************************************************************************************************************/
 /*												Swerve_Robot_UI													*/
/***************************************************************************************************************/
Swerve_Robot_UI::Swerve_Robot_UI(Swerve_Robot *SwerveRobot) : m_SwerveRobot(SwerveRobot) 
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i]=NULL;

}

Swerve_Robot_UI::~Swerve_Robot_UI()
{
	for (size_t i=0;i<4;i++)
	{
		Destroy_WheelUI(m_Wheel[i]);
		m_Wheel[i]=NULL;
	}
}

void Swerve_Robot_UI::UI_Init(Actor_Text *parent)
{
	for (size_t i=0;i<4;i++)
	{
		m_Wheel[i]->UI_Init(parent);
	}
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
		Swivel_Wheel_UI::Wheel_Properties props;
		props.m_Offset=Offsets[i];
		props.m_Wheel_Diameter=m_SwerveRobot->GetSwerveRobotProps().WheelDiameter;
		m_Wheel[i]=Create_WheelUI();
		m_Wheel[i]->Initialize(em,&props);
	}
}

void Swerve_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos)
{
	//just dispatch the update to the wheels (for now)
	for (size_t i=0;i<4;i++)
		m_Wheel[i]->update(nv,draw,parent_pos,-(m_SwerveRobot->GetAtt_r()));
}

void Swerve_Robot_UI::Text_SizeToUse(double SizeToUse)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i]->Text_SizeToUse(SizeToUse);
}

void Swerve_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i]->UpdateScene(geode,AddOrRemove);
}

void Swerve_Robot_UI::TimeChange(double dTime_s)
{
	Swerve_Robot &_=*m_SwerveRobot;
	for (size_t i=0;i<4;i++)
	{
		//TODO GetIntendedVelocities for intended UI
		m_Wheel[i]->SetSwivel(_.GetSwerveVelocitiesFromIndex(i+4));
		//For the linear velocities we'll convert to angular velocity and then extract the delta of this slice of time
		const double LinearVelocity=_.GetSwerveVelocitiesFromIndex(i);
		const double PixelHackScale=m_Wheel[i]->GetFontSize()/8.0;  //scale the wheels to be pixel aesthetic
		const double RPS=LinearVelocity /  (PI * _.GetSwerveRobotProps().WheelDiameter * PixelHackScale);
		const double AngularVelocity=RPS * Pi2;
		m_Wheel[i]->AddRotation(AngularVelocity*dTime_s);
	}
}
