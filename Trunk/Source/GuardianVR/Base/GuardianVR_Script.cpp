// GuardianVR.Base GuardianVR_Script.cpp
#include "stdafx.h"
#include "GuardianVR.Base.h"

using namespace GuardianVR::Base;

GuardianVR_Script::GuardianVR_Script()
{
	NameMap["EXISTING_ENTITIES"] = "EXISTING_MECHS";
	NameMap["GetNewPlayerEntity"] = "GetNewPlayerMech";
	NameMap["Entity"] = "Mech";
	NameMap["COLLISION_INDEX"] = "MECH_CLASS";
}
//////////////////////////////////////////////////////////////////////////
