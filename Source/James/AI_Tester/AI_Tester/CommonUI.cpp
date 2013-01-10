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
