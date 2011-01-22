// RakWrapper.h
// December 2, 2006
// Jared C Jones
// Gremlin Games
#ifndef __RakWrapper_Included_
#define __RakWrapper_Included_

#include "ImportsExports.h" // DLL-specific imports/exports
// RakNet Classes
#include "PacketEnumerations.h"
#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include "RakClientInterface.h"
#include "RakServerInterface.h"
#include "NetworkTypes.h"
#include "BitStream.h"

const static int MAXIMUM_NAME_LENGTH = 40;

namespace RakWrapperNS
{

class RAKWRAPPER_API RakWrapper {
protected:
	struct EnumerationDataStruct { char serverName[MAXIMUM_NAME_LENGTH]; };
	struct StaticClientDataStruct { char name[MAXIMUM_NAME_LENGTH]; };
	RakNetStatisticsStruct* rss;
	Packet* m_currPacket;
	unsigned char packetIdentifier;
	virtual bool Send( const char *data, const int length) = 0;
	virtual void ReleaseCurrentPacket() = 0;
	virtual Packet* ReceivePacket() = 0;

public:
	RakWrapper();
	~RakWrapper();

	// Need something that can wrap up type and length of a packet
	struct RAKWRAPPER_API PacketPropertiesStruct {
		short type;
		short length;
		PacketPropertiesStruct(short t = 0, short l = 0): type(t), length(l){};
	};
	PacketPropertiesStruct GetNextPacketType();
	template <class T> void GetNextPacket(T* array, int numElements, int& senderID) 
	{
		memcpy(array, m_currPacket->data + sizeof(PacketPropertiesStruct)+1, numElements*sizeof(T));
		senderID = m_currPacket->playerIndex;
	}

	template <class T> void SendPacket(PacketPropertiesStruct info, const T* array) 
	{
		static char buff[4096];
		buff[0] = (char)ID_USER_PACKET_ENUM;
		int totalLen = sizeof(PacketPropertiesStruct)+info.length*sizeof(T)+1;
		// ASSERT(totalLen < 4096);
		memcpy(buff+1, &info, sizeof(PacketPropertiesStruct));
		memcpy(buff+1+sizeof(PacketPropertiesStruct), array, info.length*sizeof(T));
		Send(buff, totalLen);
	}
};

class RAKWRAPPER_API RakClient: public RakWrapper {
private:
	RakClientInterface* m_client;

protected:
	virtual bool Send( const char *data, const int length);
	virtual void ReleaseCurrentPacket();
	virtual Packet* ReceivePacket();

public:
	// Constructor/Destructor
	RakClient();
	~RakClient();

	// Make Connection
	void Connect(const char* playerName, const char* serverIP);
};

class RAKWRAPPER_API RakServer: public RakWrapper {
private:
	RakServerInterface* m_server;

protected:
	virtual bool Send( const char *data, const int length);
	virtual void ReleaseCurrentPacket();
	virtual Packet* ReceivePacket();

public:
	// Constructor/Destructor
	RakServer();
	~RakServer();

	// Make Connection
	void Connect(const char* serverName);
};

};

#endif __RakWrapper_Included_