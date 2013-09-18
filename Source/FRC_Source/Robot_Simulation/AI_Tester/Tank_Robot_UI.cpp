#include "stdafx.h"
#include "Robot_Tester.h"
namespace Robot_Tester
{
	#include "Tank_Robot_UI.h"
}

#ifdef Robot_TesterCode
using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi2=M_PI*2.0;
#else
using namespace Framework::Base;
using namespace std;
#endif


#ifdef Robot_TesterCode

  /***************************************************************************************************************/
 /*												Tank_Wheel_UI													*/
/***************************************************************************************************************/

void Tank_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	m_props=*props;
	m_Rotation=0.0;
}

void Tank_Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Tread= new osgText::Text;
	m_Tread->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
	m_Tread->setCharacterSize(m_UIParent->GetFontSize());
	m_Tread->setFontResolution(10,10);
	m_Tread->setPosition(position);
	m_Tread->setAlignment(osgText::Text::CENTER_CENTER);
	m_Tread->setText(L"_");
	m_Tread->setUpdateCallback(m_UIParent);
}

void Tank_Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Tread.valid()) geode->addDrawable(m_Tread);
	else
		if (m_Tread.valid()) geode->removeDrawable(m_Tread);
}

void Tank_Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	const double FS=m_UIParent->GetFontSize();
	//Vec2d TreadRotPos(0.0,cos(m_Rotation)-0.3);  //good for " font
	Vec2d TreadRotPos(sin(m_Rotation)*-0.25,(cos(m_Rotation)*.5)+0.4);
	const Vec2d TreadOffset(m_props.m_Offset[0]+TreadRotPos[0],m_props.m_Offset[1]+TreadRotPos[1]);
	const Vec2d TreadLocalOffset=GlobalToLocal(Heading,TreadOffset);
	const osg::Vec3 TreadPos (parent_pos[0]+( TreadLocalOffset[0]*FS),parent_pos[1]+( TreadLocalOffset[1]*FS),parent_pos[2]);

	const double TreadColor=((sin(-m_Rotation) + 1.0)/2.0) * 0.8 + 0.2;
	m_Tread->setColor(osg::Vec4(TreadColor,TreadColor,TreadColor,1.0));

	if (m_Tread.valid())
	{
		m_Tread->setPosition(TreadPos);
		m_Tread->setRotation(FromLW_Rot_Radians(Heading,0.0,0.0));
	}
}

void Tank_Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Tread.valid()) m_Tread->setCharacterSize(SizeToUse);
}

void Tank_Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}

  /***************************************************************************************************************/
 /*												Tank_Robot_UI													*/
/***************************************************************************************************************/
void Tank_Robot_UI::UI_Init(Actor_Text *parent)
{
	for (size_t i=0;i<6;i++)
		m_Wheel[i].UI_Init(parent);
}

void Tank_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	Vec2D Offsets[6]=
	{
		Vec2D(-1.6, 2.0),
		Vec2D( 1.6, 2.0),
		Vec2D(-1.6, 0.0),
		Vec2D( 1.6, 0.0),
		Vec2D(-1.6,-2.0),
		Vec2D( 1.6,-2.0),
	};
	for (size_t i=0;i<6;i++)
	{
		Tank_Wheel_UI::Wheel_Properties props;
		props.m_Offset=Offsets[i];
		props.m_Wheel_Diameter=m_TankRobot->GetTankRobotProps().WheelDiameter;
		m_Wheel[i].Initialize(em,&props);
	}
}

void Tank_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos)
{
	//just dispatch the update to the wheels (for now)
	for (size_t i=0;i<6;i++)
		m_Wheel[i].update(nv,draw,parent_pos,-m_TankRobot->GetAtt_r());
}

void Tank_Robot_UI::Text_SizeToUse(double SizeToUse)
{
	for (size_t i=0;i<6;i++)
		m_Wheel[i].Text_SizeToUse(SizeToUse);
}

void Tank_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	for (size_t i=0;i<6;i++)
		m_Wheel[i].UpdateScene(geode,AddOrRemove);
}

void Tank_Robot_UI::TimeChange(double dTime_s)
{
	Tank_Robot &_=*m_TankRobot;
	for (size_t i=0;i<6;i++)
	{
		//For the linear velocities we'll convert to angular velocity and then extract the delta of this slice of time
		const double LinearVelocity=(i&1)?_.GetRightVelocity():_.GetLeftVelocity();
		const double PixelHackScale=m_Wheel[i].GetFontSize()/10.0;  //scale the wheels to be pixel aesthetic
		//Note: for UI... to make it pixel friendly always use 6 inches with the hack and not _.GetTankRobotProps().WheelDiameter
		const double RPS=LinearVelocity /  (PI * Inches2Meters(6.0) * PixelHackScale);
		const double AngularVelocity=RPS * Pi2;
		m_Wheel[i].AddRotation(AngularVelocity*dTime_s);
	}
}

#endif
