#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
#include "PIDController.h"
#include "Calibration_Testing.h"
#include "Tank_Robot.h"
#include "Robot_Control_Interface.h"
#include "Servo_System.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

namespace Base=GG_Framework::Base;
namespace Scripting=GG_Framework::Logic::Scripting;


  /***********************************************************************************************************************************/
 /*														Servo_Position_Control														*/
/***********************************************************************************************************************************/

Servo_Position_Control::Servo_Position_Control(const char EntityName[],Servo_Control_Interface *robot_control,size_t InstanceIndex) : Ship_1D(EntityName),
	m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
	m_LastPosition(0.0),m_MatchVelocity(0.0),
	m_LastTime(0.0),m_PreviousVelocity(0.0)
{
}

void Servo_Position_Control::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	__super::Initialize(em,props);
	const Servo_Properties *Props=dynamic_cast<const Servo_Properties *>(props);
	assert(Props);
	//This will copy all the props
	m_Servo_Props=Props->GetServoProps();

	const double MaxSpeedReference=Props->GetMaxSpeed();
}

void Servo_Position_Control::TimeChange(double dTime_s)
{
	const double CurrentVelocity=m_Physics.GetVelocity();

	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated
	if (!m_LastTime) 
	{
		m_LastTime=dTime_s;
		#ifdef AI_TesterCode
		assert(dTime_s!=0.0);
		#endif
	}

	//const double NewPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	//const double Displacement=NewPosition-m_LastPosition;
	//const double PotentiometerVelocity=Displacement/m_LastTime;

	//m_LastPosition=NewPosition;
	//m_LastTime=dTime_s;

	__super::TimeChange(dTime_s);


	//#ifdef __DebugLUA__
	//if ((m_Servo_Props.Console_Dump))
	//{
	//	double PosY=m_LastPosition;
	//	//double PosY=RAD_2_DEG(m_LastPosition);
	//	printf("y=%.2f p=%f\n",PosY,CurrentVelocity);
	//}
	//#endif

	//Note: the scaling of angle occurs in robot control
	m_RobotControl->SetAngle(m_InstanceIndex,GetPos_m());
}


void Servo_Position_Control::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first
	if (!GetBypassPos_Update())
	{
		m_RobotControl->Reset_Servo(m_InstanceIndex);
		double NewPosition=m_RobotControl->GetAngle(m_InstanceIndex);
		Stop();
		SetPos_m(NewPosition);
		m_LastPosition=NewPosition;
	}
}
