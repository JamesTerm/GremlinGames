#pragma once

//project dependencies
#include "../../../GG_Framework/UI/GG_Framework.UI.h"
#include "../../../GG_Framework/Logic/Scripting/GG_Framework.Logic.Scripting.h"

#define AI_TesterCode  //used to branch AI test code from wind-river code

//#define __DisableSmartDashboard__ //used to quickly disable the smart dashboard
#ifndef __DisableSmartDashboard__
#include "../../SmartDashboard2/SmartDashboard_import.h"
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
namespace AI_Tester
{
	//Note like our game everything can reach the UI, but the UI knows nothing about the entity
	#include "Entity_Properties.h"
	#include "FrameWork_UI.h"
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
}
