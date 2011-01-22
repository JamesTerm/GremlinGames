// GG_Framework.Logic.Network.h
#pragma once

#ifdef FRAMEWORK_LOGIC_NETWORK_EXPORTS
#define FRAMEWORK_LOGIC_NETWORK_API __declspec(dllexport)
#else
#define FRAMEWORK_LOGIC_NETWORK_API __declspec(dllimport)
#endif

///////////////////
// Useful Constants

//////////////////
// Other Libraries
#include "BitStream.h"
#include "PacketPriority.h"
#include "..\..\..\CompilerSettings.h"
#include "..\..\Base\GG_Framework.Base.h"

//////////////////
// Useful Global Functions
struct Packet;
namespace GG_Framework
{
	namespace Logic
	{
		namespace Network
		{
			FRAMEWORK_LOGIC_NETWORK_API unsigned char GetPacketIdentifier(const unsigned char* data);
			FRAMEWORK_LOGIC_NETWORK_API const unsigned char* GetPacketIdentifierPtr(const unsigned char* data);
			FRAMEWORK_LOGIC_NETWORK_API void DebugOutputPacket(Packet& packet);

			FRAMEWORK_LOGIC_NETWORK_API unsigned char CreateRebroadcastInstructions(
				PacketPriority priority, PacketReliability reliability);
			FRAMEWORK_LOGIC_NETWORK_API void FindRebroadcastInstructions(unsigned char info,
				PacketPriority& priority, PacketReliability& reliability);

			// Some helper functions for working with strings, reads/writes the length first
			FRAMEWORK_LOGIC_NETWORK_API void WriteString(RakNet::BitStream& bs, const std::string& s);
			FRAMEWORK_LOGIC_NETWORK_API void ReadString(RakNet::BitStream& bs, std::string& s);

			// Writing out PlayerID's are not thread safe, and use a static string, so this wraps that
			FRAMEWORK_LOGIC_NETWORK_API std::string PlayID2Str(const PlayerID& pID);
		}
	}
}


//////////////////
// My headers
#include "SynchronizedTimer.h"
#include "Interfaces.h"
#include "PacketEnums.h"
#include "ClientServer.h"
#include "EventManager.h"
#include "ServerManager.h"
#include "MockClientServer.h"