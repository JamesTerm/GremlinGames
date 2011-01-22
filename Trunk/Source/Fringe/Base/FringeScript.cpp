// Fringe.Base FringeScript.cpp
#include "stdafx.h"
#include "Fringe.Base.h"

using namespace Fringe::Base;

FringeScript::FringeScript()
{
	NameMap["EXISTING_ENTITIES"] = "EXISTING_SHIPS";
	NameMap["GetNewPlayerEntity"] = "GetNewPlayerShip";
	NameMap["Entity"] = "Ship";
	NameMap["COLLISION_INDEX"] = "SHIP_CLASS";
}
//////////////////////////////////////////////////////////////////////////
