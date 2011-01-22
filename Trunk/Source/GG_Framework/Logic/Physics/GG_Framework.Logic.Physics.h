// GG_Framework.Logic.Physics.h
#pragma once

#ifdef FRAMEWORK_LOGIC_PHYSICS_EXPORTS
#define FRAMEWORK_LOGIC_PHYSICS_API __declspec(dllexport)
#else
#define FRAMEWORK_LOGIC_PHYSICS_API __declspec(dllimport)
#endif

#undef DISABLE_BLACKOUT

//Not sure yet if I want these public
//#include "OdeWorld.h"
//#include "RigidBody.h"
#include "../../Base/GG_Framework.Base.h"
#include "PhysicsEntity.h"

static const double EARTH_G = 9.80665;
