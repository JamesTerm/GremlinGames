#pragma once

//project dependencies
#include "../../../GG_Framework/UI/GG_Framework.UI.h"
#include "../../../GG_Framework/Logic/Scripting/GG_Framework.Logic.Scripting.h"

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
	#include "Ship.h"
	#include "Robot_Tank.h"
	#include "AI_Base_Controller.h"
	#include "UI_Controller.h"
}
