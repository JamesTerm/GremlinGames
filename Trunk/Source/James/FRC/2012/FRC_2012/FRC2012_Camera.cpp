#include "WPILib.h"

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Common/Entity_Properties.h"
#include "Common/Physics_1D.h"
#include "Common/Physics_2D.h"
#include "Common/Entity2D.h"
#include "Common/Goal.h"
#include "Common/Ship_1D.h"
#include "Common/Ship.h"
#include "Common/Vehicle_Drive.h"
#include "Common/PIDController.h"
#include "Common/Tank_Robot.h"
#include "Common/AI_Base_Controller.h"
#include "Common/Robot_Control_Interface.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2012_Robot.h"
#include "FRC2012_Camera.h"
#include "InOut_Interface.h"
#include "FRC2012_Goals.h"

  /***********************************************************************************************************************************/
 /*													FRC_2012_CameraProcessing														*/
/***********************************************************************************************************************************/

FRC_2012_CameraProcessing::FRC_2012_CameraProcessing() : m_Camera(NULL),m_LastTime(0.0)
{
	//Default will use the team ip 10.34.81.11  with 11
	m_Camera=&AxisCamera::GetInstance();
}

FRC_2012_CameraProcessing::~FRC_2012_CameraProcessing()
{
	m_Camera=NULL;  //We don't own this, but I do wish to treat it like we do
}

void FRC_2012_CameraProcessing::CameraProcessing_TimeChange(double dTime_s)
{
	if (m_Camera->IsFreshImage())
	{
		double time=GetTime() - m_LastTime;
		//Throttle down the iterations to a reasonable workload
		if (time>=1001.0/30000.0)
		{
			m_LastTime=GetTime();
			printf("\rCamera Time %f           ",time);
		}
	}
}

