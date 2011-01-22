// Fringe.Base.h
#pragma once

#include "../../CompilerSettings.h"

#ifdef FRINGE_BASE_EXPORTS
#define FRINGE_BASE_API __declspec(dllexport)
#else
#define FRINGE_BASE_API __declspec(dllimport)
#endif

//////////////////
// Other Libraries
#include "../../CompilerSettings.h"

#include "..\..\GG_Framework\Base\GG_Framework.Base.h"
#include "..\..\GG_Framework\Logic\GG_Framework.Logic.h"
#include "..\..\GG_Framework\AppReuse\GG_Framework.AppReuse.h"
#include "..\..\GG_Framework\Logic\Network\GG_Framework.Logic.Network.h"
#include "..\..\GG_Framework\Logic\Scripting\GG_Framework.Logic.Scripting.h"

#include "..\..\GG_Framework\UI\GG_Framework.UI.h"
#include "..\..\GG_Framework\UI\Audio\GG_Framework.UI.Audio.h"
#include "..\..\GG_Framework\UI\OSG\GG_Framework.UI.OSG.h"

///////////////////////
// Fringe.Base Includes
#include "FringeScript.h"
#include "ThrustShip2_TransmittedEntity.h"
#include "Ship.h"
#include "HUD_PDCB.h"
#include "ThrusterShip2.h"
#include "Mouse_ShipDriver.h"
#include "ThrusterShip_ControllerBase.h"
#include "ThrusterShip_Controller2.h"
#include "UI_GameClient.h"
#include "HelpScreen.h"
