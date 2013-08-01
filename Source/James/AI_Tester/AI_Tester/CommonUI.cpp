#include "stdafx.h"
#include "AI_Tester.h"
#include "Debug.h"
namespace AI_Tester
{
	#include "PIDController.h"
	#include "Calibration_Testing.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
	#include "CommonUI.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

namespace Base=GG_Framework::Base;
namespace Scripting=GG_Framework::Logic::Scripting;

  /***************************************************************************************************************/
 /*												Side_Wheel_UI													*/
/***************************************************************************************************************/

void Side_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	if (props)
		m_props=*props;
	else
	{
		m_props.m_Offset=Vec2d(0,0);
		m_props.m_Color=osg::Vec4(1.0,0.0,0.5,1.0);
		m_props.m_TextDisplay=L"|";
	}
}

void Side_Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Wheel= new osgText::Text;
	m_Wheel->setColor(m_props.m_Color);
	m_Wheel->setCharacterSize(m_UIParent->GetFontSize());
	m_Wheel->setFontResolution(10,10);
	m_Wheel->setPosition(position);
	m_Wheel->setAlignment(osgText::Text::CENTER_CENTER);
	m_Wheel->setText(m_props.m_TextDisplay);
	m_Wheel->setUpdateCallback(m_UIParent);
}

void Side_Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Wheel.valid()) geode->addDrawable(m_Wheel);
	else
		if (m_Wheel.valid()) geode->removeDrawable(m_Wheel);
}

void Side_Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	double HeadingToUse=Heading+m_Rotation;
	const double FS=m_UIParent->GetFontSize();
	const Vec2d WheelOffset(m_props.m_Offset[0],m_props.m_Offset[1]);
	const Vec2d WheelLocalOffset=GlobalToLocal(Heading,WheelOffset);
	const osg::Vec3 WheelPos (parent_pos[0]+( WheelLocalOffset[0]*FS),parent_pos[1]+( WheelLocalOffset[1]*FS),parent_pos[2]);

	if (m_Wheel.valid())
	{
		m_Wheel->setPosition(WheelPos);
		m_Wheel->setRotation(FromLW_Rot_Radians(HeadingToUse,0.0,0.0));
	}
}

void Side_Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Wheel.valid()) m_Wheel->setCharacterSize(SizeToUse);
}

void Side_Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}

  /***************************************************************************************************************/
 /*												Swivel_Wheel_UI													*/
/***************************************************************************************************************/

void Swivel_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	m_props=*props;
	m_Swivel=0.0;
	m_Rotation=0.0;
}

void Swivel_Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
	m_Front= new osgText::Text;
	m_Front->setColor(GetFrontWheelColor());
	m_Front->setCharacterSize(m_UIParent->GetFontSize());
	m_Front->setFontResolution(10,10);
	m_Front->setPosition(position);
	m_Front->setAlignment(osgText::Text::CENTER_CENTER);
	m_Front->setText(L"U");
	m_Front->setUpdateCallback(m_UIParent);

	m_Back= new osgText::Text;
	m_Back->setColor(GetBackWheelColor());
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

void Swivel_Wheel_UI::SetWheelColor(osg::Vec4 Color, WheelEnum Wheel)
{
	switch (Wheel)
	{
	case eFront:
		m_Front->setColor(Color);
		break;
	case eBack:
		m_Back->setColor(Color);
		break;
	case eTread:
		//all though this is in-effective its added for completion
		m_Tread->setColor(Color);
		break;
	}
}

void Swivel_Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
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

void Swivel_Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
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

void Swivel_Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Front.valid())	m_Front->setCharacterSize(SizeToUse);
	if (m_Back.valid()) m_Back->setCharacterSize(SizeToUse);
	if (m_Tread.valid()) m_Tread->setCharacterSize(SizeToUse);
}

void Swivel_Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}
