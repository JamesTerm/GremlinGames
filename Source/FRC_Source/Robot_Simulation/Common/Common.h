#pragma once

//#define __SetRobotRemote__

//project dependencies
#include "../../../GG_Framework/UI/GG_Framework.UI.h"
#include "../../../GG_Framework/Logic/Scripting/GG_Framework.Logic.Scripting.h"

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the COMMON_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// COMMON_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef COMMON_EXPORTS
#define COMMON_API __declspec(dllexport)
#else
#define COMMON_API __declspec(dllimport)
#endif

#define Robot_TesterCode  //used to branch AI test code from wind-river code

//#define __DisableSmartDashboard__ //used to quickly disable the smart dashboard
#ifndef __DisableSmartDashboard__
#include "../../SmartDashboard/SmartDashboard_import.h"
#else
class SmartDashboard //: public SensorBase
{
public:
	static void init() {}
	static void shutdown() {}

	//static void PutData(std::string key, Sendable *data) {}
	//static void PutData(NamedSendable *value){}
	//static Sendable* GetData(std::string keyName);

	static void PutBoolean(std::string keyName, bool value) {}
	static bool GetBoolean(std::string keyName) {return false;}

	static void PutNumber(std::string keyName, double value) {}
	static double GetNumber(std::string keyName) {return 0.0;}

	static void PutString(std::string keyName, std::string value) {}
	static int GetString(std::string keyName, char *value, unsigned int valueLen) {return 0;}
	static std::string GetString(std::string keyName) {return "";} 

	//static void PutValue(std::string keyName, ComplexData& value) {}
	//static void RetrieveValue(std::string keyName, ComplexData& value) {}
};
#endif

typedef osg::Vec2d Vec2D;
namespace Base=GG_Framework::Base;
namespace UI=GG_Framework::UI;
namespace Scripting=GG_Framework::Logic::Scripting;

//local includes
namespace Robot_Tester
{
	//Note like our game everything can reach the UI, but the UI knows nothing about the entity
	#include "Entity_Properties.h"
	//#include "FrameWork_UI.h"
	#include "Physics_2D.h"
	#include "Physics_1D.h"
	#include "Entity2D.h"
	#include "Goal.h"
	#include "Ship_1D.h"
	#include "Ship.h"
	#include "Vehicle_Drive.h"
	#include "AI_Base_Controller.h"
	#include "UI_Controller.h"
	#include "Poly.h"
	#include "Debug.h"
	#include "UDP_Listener.h"
	#include "Robot_Control_Common.h"
}
