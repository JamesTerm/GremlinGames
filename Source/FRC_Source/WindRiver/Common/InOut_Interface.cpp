
#include "WPILib.h"

#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "../Base/Script.h"

//#include "Entity_Properties.h"
//#include "Physics_1D.h"
//#include "Physics_2D.h"
//#include "Entity2D.h"
//#include "Goal.h"
//#include "Ship_1D.h"
//#include "Ship.h"
//#include "Vehicle_Drive.h"
//#include "PIDController.h"
//#include "AI_Base_Controller.h"
//#include "Robot_Control_Interface.h"

#include "../Base/Joystick.h"
#include "../Base/JoystickBinder.h"

//#include "UI_Controller.h"
//#include "PIDController.h"

#include "InOut_Interface.h"
#include "Debug.h"

using namespace Framework::Base;
#undef __DisableTankDrive__

  /***********************************************************************************************************************************/
 /*																Encoder2															*/
/***********************************************************************************************************************************/


Encoder2::Encoder2(UINT32 aChannel, UINT32 bChannel, bool reverseDirection, EncodingType encodingType) : Encoder(aChannel,bChannel,reverseDirection,encodingType),
		m_LastDistance(0.0)
{
}

void Encoder2::Reset2()
{
	m_LastDistance=0.0;
	Reset();
}

double Encoder2::GetRate2(double dTime_s)
{
	//Using distance will yield the same rate as GetRate, without precision loss to GetRaw()
	const double CurrentDistance=GetDistance();
	const double delta=CurrentDistance - m_LastDistance;
	m_LastDistance=CurrentDistance;
	return delta/dTime_s;
}

  /***********************************************************************************************************************************/
 /*														Driver_Station_Joystick														*/
/***********************************************************************************************************************************/

size_t Driver_Station_Joystick::GetNoJoysticksFound() 
{
	return m_NoJoysticks;
}

void Driver_Station_Joystick::SetSlotList(const Driver_Station_SlotList &list)
{
	//if this is empty then we'll leave it as it was
	if (list.size()==0)
		return;
	
	m_SlotList=list;
	m_NoJoysticks=list.size();
	m_JoyInfo.clear();  //we'll repopulate with the slot names
	
	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName=list[0];
	common.InstanceName="Driver_Station";
	common.JoyCapFlags=
		JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
		JoystickInfo::fX_Rot|JoystickInfo::fY_Rot|JoystickInfo::fZ_Rot;
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=strcmp(common.ProductName.c_str(),"none")!=0;  //make use of this bool instead of doing a strcmp each iteration
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	for (size_t i=1;i<list.size();i++)
	{
		common.ProductName=list[i];
		common.bPresent=strcmp(common.ProductName.c_str(),"none")!=0;  //make use of this bool instead of doing a strcmp each iteration
		m_JoyInfo.push_back(common);
	}
}

bool Driver_Station_Joystick::read_joystick (size_t nr, JoyState &Info)
{
	//First weed out numbers not in range
	int Number=(int)nr;
	Number-=m_StartingPort;
	bool ret=false;
	nr++;  //DOH the number selection is cardinal!  :(
	if ((Number>=0) && (Number<m_NoJoysticks) && (m_JoyInfo[nr-1].bPresent))
	{
		memset(&Info,0,sizeof(JoyState));  //zero the memory
		//The axis selection is also ordinal
		Info.lX=m_ds->GetStickAxis(nr,1);
		Info.lY=m_ds->GetStickAxis(nr,2);
		Info.lZ=m_ds->GetStickAxis(nr,3);
		Info.lRx=m_ds->GetStickAxis(nr,4);
		Info.lRy=m_ds->GetStickAxis(nr,5);
		Info.lRz=m_ds->GetStickAxis(nr,6);
		Info.ButtonBank[0]=m_ds->GetStickButtons(nr);
		ret=true;
	}
	return ret;
}

Driver_Station_Joystick::Driver_Station_Joystick(int NoJoysticks,int StartingPort) : m_NoJoysticks(NoJoysticks), m_StartingPort(StartingPort)
{
	m_ds = DriverStation::GetInstance();
	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName="joystick_1";
	common.InstanceName="Driver_Station";
	common.JoyCapFlags=
		JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
		JoystickInfo::fX_Rot|JoystickInfo::fY_Rot|JoystickInfo::fZ_Rot;
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=true;
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	common.ProductName="joystick_2";
	m_JoyInfo.push_back(common);
	common.ProductName="joystick_3";
	m_JoyInfo.push_back(common);
	common.ProductName="joystick_4";
	m_JoyInfo.push_back(common);
}

Driver_Station_Joystick::~Driver_Station_Joystick()
{
}





