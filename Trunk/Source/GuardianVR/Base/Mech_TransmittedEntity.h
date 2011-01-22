// GuardianVR.Base Mech_TransmittedEntity
#pragma once

namespace GuardianVR
{
	namespace Base
	{
		struct Mech_TransmittedEntity : public GG_Framework::AppReuse::DestroyableEntity_TransmittedEntity
		{
			//! Write the entity into the BitStream to be sent, WITHOUT the header
			virtual void WriteToBitStream(RakNet::BitStream& bs);


			//! This is used for KB turning (if we use them at all) and the max turning rate for the mouse
			double dHeading, dPitch;

			bool CanUserPilot;

			std::vector<GG_Framework::AppReuse::CannonDesc> Cannons;

			//! Build a Transmitted entity from the LUA Script
			//! This assumes that there is a Table on the top of the LUA stack
			virtual void LoadFromScript(
				GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
				GG_Framework::Logic::ITransmittedEntityOwner* entityOwner);

			//! Build a Transmitted entity from a received Packet data
			//! Apply this AFTER any packet identifiers
			virtual void LoadFromBitStream(RakNet::BitStream& bs);
		};
	}
}

