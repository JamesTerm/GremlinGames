// GG_Framework.UI.OSG.h
#pragma once

#ifdef FRAMEWORK_UI_OSG_EXPORTS
#define FRAMEWORK_UI_OSG_API __declspec(dllexport)
#else
#define FRAMEWORK_UI_OSG_API __declspec(dllimport)
#endif

//////////////////
// Other Libraries
#include "..\..\..\CompilerSettings.h"
#include "..\..\Base\GG_Framework.Base.h"

///////////////////////
// Fringe.UI.OSG Includes
#include "Producer_Trackball.h"
#include "Trackball.h"
#include "Misc.h"
#include "SnapImageDrawCallback.h"
#include "Text.h"
#include "ICamera.h"
#include "Timer.h"
#include "PickVisitor.h"
#include "VectorDerivativeOverTimeAverager.h"
#include "LoadStatus.h"



