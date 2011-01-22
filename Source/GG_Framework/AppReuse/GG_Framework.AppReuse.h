// GG_Framework.AppReuse.h
#pragma once

#ifdef FRAMEWORK_APPREUSE_EXPORTS
#define FRAMEWORK_APPREUSE_API __declspec(dllexport)
#else
#define FRAMEWORK_APPREUSE_API __declspec(dllimport)
#endif

#define DEBUG_CANNON_FIRE // printf

//////////////////
// Other Libraries
#include "../../CompilerSettings.h"

#include "..\..\GG_Framework\Logic\GG_Framework.Logic.h"

///////////////////////
// GG_Framework.AppReuse Includes
#include "Cannon.h"
#include "CollisionEntity.h"
#include "DestructionProxyEntity.h"
#include "DestroyableEntity.h"
#include "ChasePlane_CamManipulator.h"
