// GuardianVR.Base.h
#pragma once

#include "../../CompilerSettings.h"

#ifdef GUARDIANVR_BASE_EXPORTS
#define GUARDIANVR_BASE_API __declspec(dllexport)
#else
#define GUARDIANVR_BASE_API __declspec(dllimport)
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
// GuardianVR.Base Includes
#include "HelpScreen.h"
#include "GuardianVR_Script.h"
#include "Mech_TransmittedEntity.h"
#include "Mech.h"
#include "Mech_ControllerBase.h"
#include "Mouse_MechDriver.h"
#include "Mech_Controller.h"



