#pragma once

//project dependencies
#include "../../../GG_Framework/UI/GG_Framework.UI.h"

//local includes
namespace AI_Tester
{
	//Note like our game everything can reach the UI, but the UI knows nothing about the entity
	#include "FrameWork_UI.h"
	#include "Physics_2D.h"
	#include "Physics_1D.h"
	#include "Entity2D.h"
	#include "Goal.h"
	#include "Ship.h"
	#include "AI_Base_Controller.h"
	#include "UI_Controller.h"
}
