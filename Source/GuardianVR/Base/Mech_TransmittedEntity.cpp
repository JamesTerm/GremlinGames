// Mech_TransmittedEntity.cpp
#include "stdafx.h"
#include "GuardianVR.Base.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GuardianVR::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::AppReuse;

ClassFactoryT<Mech_TransmittedEntity, TransmittedEntity> Mech_TE_ClassFactory("Mech", TransmittedEntity::ClassFactory);

//! Build a Transmitted entity from the LUA Script
void Mech_TransmittedEntity::LoadFromScript(
	GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
	GG_Framework::Logic::ITransmittedEntityOwner* entityOwner)
{
	const char* err;

	// Not worried about these failing, we have good defaults
	CanUserPilot = true; // Optional
	script.GetField("UserCanPilot", NULL, &CanUserPilot, NULL);

	// We will need the node name for the sake of the Cannons (I do not like this here, but it has to be done before I read myself)
	err = script.GetField("ID", &NAME);
	ASSERT_MSG(!err, err);

	err = script.GetFieldTable("Mech");
	ASSERT_MSG(!err, err);
	{
		err = script.GetField("dHeading", NULL, NULL, &dHeading);
		ASSERT_MSG(!err, err);
		err = script.GetField("dPitch", NULL, NULL, &dPitch);
		ASSERT_MSG(!err, err);

		// Get all of the Cannons (optional)
		err = script.GetFieldTable("Cannons");
		if (!err)
		{
			DEBUG_CANNON_FIRE("Mech_TransmittedEntity::LoadFromScript(%s) -- Start Cannon Loop\n", NAME.c_str());
			int index = 1;
			while (!(err = script.GetIndexTable(index++)))
			{
				DEBUG_CANNON_FIRE("Mech_TransmittedEntity::LoadFromScript(%s) -- In Cannon Loop\n", NAME.c_str());

				CannonDesc cannon;
				cannon.LoadFromScript(NAME, index, script, controllingPlayerID, entityOwner);

				Cannons.push_back(cannon);
				script.Pop();
			}
			script.Pop();	// Pop the error one too we get off NOt getting an index
			script.Pop();   // This one is only for the Cannons, only need to do it if there was one
		}
	}
	script.Pop();

	// Call the base class to finish up
	__super::LoadFromScript(script, controllingPlayerID, entityOwner);
}
//////////////////////////////////////////////////////////////////////////

//! Build a Transmitted entity from a received Packet data
//! Apply this AFTER any packet identifiers
void Mech_TransmittedEntity::LoadFromBitStream(RakNet::BitStream& bs)
{
	__super::LoadFromBitStream(bs);

	// ReadIn each of the strings as NULL terminated
	bs.Read(dHeading);
	bs.Read(dPitch);

	bs.Read(CanUserPilot);

	{
		unsigned numCannons;
		bs.Read(numCannons);
		for (unsigned i = 0; i < numCannons; ++i)
		{
			DEBUG_CANNON_FIRE("Mech_TransmittedEntity::LoadFromBitStream(%s) -- Cannon Loop\n", NAME.c_str());
			CannonDesc cannon;
			cannon.LoadFromBitStream(bs);
			Cannons.push_back(cannon);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

//! Write the entity into the BitStream to be sent, WITHOUT the header
void Mech_TransmittedEntity::WriteToBitStream(RakNet::BitStream& bs)
{
	__super::WriteToBitStream(bs);

	// Write out each of the strings as NULL terminated
	bs.Write(dHeading);
	bs.Write(dPitch);

	bs.Write(CanUserPilot);

	{
		unsigned numCannons = Cannons.size();
		bs.Write(numCannons);
		for (unsigned i = 0; i < numCannons; ++i)
		{
			Cannons[i].WriteToBitStream(bs);
		}
	}
}
//////////////////////////////////////////////////////////////////////////