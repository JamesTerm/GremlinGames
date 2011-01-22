// GG_Framework.Logic TransmittedEntity.h
#pragma once

namespace GG_Framework
{
	namespace Logic
	{
		struct TransmittedEntity;

		// ServerManager will implement this
		class ITransmittedEntityOwner
		{
		public:
			// Returns the index of the new entity
			virtual unsigned AddTransmittedEntity(TransmittedEntity* newEntity) = 0;
		};
		//////////////////////////////////////////////////////////////////////////

		//! This object is read from a script or passed across a network so its
		//! Classes can be created.
		struct FRAMEWORK_LOGIC_API TransmittedEntity
		{
			TransmittedEntity();

			static TransmittedEntity* CreateFromServerScript(
				GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, ITransmittedEntityOwner* entityOwner);
			static TransmittedEntity* CreateFromBitStream(RakNet::BitStream& bs);

			//! Write the entity into the BitStream to be sent, WITHOUT the header
			virtual void WriteToBitStream(RakNet::BitStream& bs);

			std::string NAME;
			std::string OSGV;
			std::string CPP_CLASS;
			PlayerID CONTROLLING_PLAYER_ID;
			unsigned NETWORK_ID;

			//Stuff needed for physics
			double Mass;

			double Dimensions[3]; //Dimensions- Length Width and Height

			// COLLISION_INDEX is used to determine collision ordering
			int COLLISION_INDEX;

			//! Positions in meters, rotations in degrees
			double X, Y, Z, Heading, Pitch, Roll;

			static FactoryMapT<TransmittedEntity> ClassFactory;

			//! Build a Transmitted entity from the LUA Script
			//! This assumes that there is a Table on the top of the LUA stack
			virtual void LoadFromScript(
				GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, ITransmittedEntityOwner* entityOwner);

			//! Build a Transmitted entity from a received Packet data
			//! Apply this AFTER any packet identifiers
			virtual void LoadFromBitStream(RakNet::BitStream& bs);
		};
		//////////////////////////////////////////////////////////////////////////

		
	}	
}
