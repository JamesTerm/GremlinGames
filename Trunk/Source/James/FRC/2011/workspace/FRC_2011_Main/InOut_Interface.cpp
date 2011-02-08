#include "WPILib.h"
#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Entity_Properties.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"
#include "Robot_Tank.h"
#include "AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "UI_Controller.h"
#include "InOut_Interface.h"

using namespace Framework::Base;

size_t Driver_Station_Joystick::GetNoJoysticksFound() 
{
	return 1;    //TODO see if this is detectable... we can hard code for now
}

bool Driver_Station_Joystick::read_joystick (size_t nr, JoyState &Info)
{
	//First weed out numbers not in range
	int Number=(int)nr;
	Number-=m_StartingPort;
	bool ret=false;
	if ((Number>=0) && (Number<m_NoJoysticks))
	{
		memset(&Info,0,sizeof(JoyState));  //zero the memory
		Info.lX=m_ds->GetStickAxis(nr,0);
		Info.lY=m_ds->GetStickAxis(nr,1);
		Info.lZ=m_ds->GetStickAxis(nr,2);
		Info.lRx=m_ds->GetStickAxis(nr,3);
		Info.lRy=m_ds->GetStickAxis(nr,4);
		Info.ButtonBank[0]=m_ds->GetStickButtons(nr);
		ret=true;
	}
	return ret;
}

Driver_Station_Joystick::Driver_Station_Joystick(int NoJoysticks,int StartingPort) : m_NoJoysticks(NoJoysticks), m_StartingPort(StartingPort)
{
	m_ds = DriverStation::GetInstance();
	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName="Driver_Station";
	common.InstanceName="Joystick_1";
	common.JoyCapFlags=
		JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
		JoystickInfo::fX_Rot|JoystickInfo::fY_Rot;
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=true;
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	common.InstanceName="Joystick_2";
	m_JoyInfo.push_back(common);
	common.InstanceName="Joystick_3";
	m_JoyInfo.push_back(common);
	common.InstanceName="Joystick_4";
	m_JoyInfo.push_back(common);
}

Driver_Station_Joystick::~Driver_Station_Joystick()
{
}
