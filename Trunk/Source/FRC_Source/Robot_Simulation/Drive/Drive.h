#pragma  once

//project dependencies
#include "../Common/Common.h"

#ifdef DRIVE_EXPORTS
#define DRIVE_API __declspec(dllexport)
#else
#define DRIVE_API __declspec(dllimport)
#endif

namespace AI_Tester
{
	#include "../Common/PIDController.h"
	#include "../Common/Calibration_Testing.h"
	#include "../Common/Robot_Control_Interface.h"
	#include "../Common/Rotary_System.h"
	#include "../Common/Servo_System.h"
	#include "Tank_Robot.h"
	#include "Swerve_Robot.h"
	#include "Nona_Robot.h"
}
