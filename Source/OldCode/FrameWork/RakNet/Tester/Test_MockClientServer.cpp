// FrameWork.RakNet.Tester Test_MockClientServer.cpp
#include "stdafx.h"
#include "../../../CompilerSettings.h"
#include "../FrameWork.RakNet.h"

namespace FrameWork
{
	namespace RakNetWrapper
	{
		namespace Tester
		{

			SUITE(Test_MockClientServer)
			{
				TEST(MainTest)
				{
					// Create the server
					MockServer* server = new MockServer();
					IServer* s = dynamic_cast<IServer*>(server);
					IPacketReceiver* pS = dynamic_cast<IPacketReceiver*>(server);
					CHECK(pS != NULL);
					CHECK(s != NULL);
					CHECK_EQUAL(IServer::Instance(), s);
					CHECK_EQUAL(false, IServer::Instance()->IsActive());

					// Create the client
					MockClient client(*server);
					IClient* c = dynamic_cast<IClient*>(&client);
					IPacketReceiver* pC = dynamic_cast<IPacketReceiver*>(&client);
					CHECK(pC != NULL);
					CHECK(c != NULL);
					CHECK_EQUAL(IClient::Instance(), c);

					// We should all be connected now
					CHECK_EQUAL(true, IClient::Instance()->IsConnected());
					CHECK_EQUAL(true, IServer::Instance()->IsActive());

					// But no-one has received messages
					CHECK(pS->Receive() == NULL);
					CHECK(pC->Receive() == NULL);

					// Sending NULL fails
					CHECK_EQUAL(false, IServer::Instance()->Send(0, 0, 
						HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true));
					CHECK_EQUAL(false, IClient::Instance()->Send(0, 0, 
						HIGH_PRIORITY, RELIABLE, 0));
					CHECK_EQUAL(false, IServer::Instance()->Send(0, 
						HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true));
					CHECK_EQUAL(false, IClient::Instance()->Send(0, 
						HIGH_PRIORITY, RELIABLE, 0));
					CHECK(pS->Receive() == NULL);
					CHECK(pC->Receive() == NULL);

					// Try sending a normal char* packet between the two peers
					const char* message = "Hello Mock PEERS!";
					CHECK(IServer::Instance()->Send(message, (int)strlen(message)+1, 
						HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true));
					CHECK(IClient::Instance()->Send(message, (int)strlen(message)+1, 
						HIGH_PRIORITY, RELIABLE, 0));

					// Send another message to the client, to show two in the QUEUE
					const char* message2 = "Hello Mock PEERS Again!";
					CHECK(IServer::Instance()->Send(message2, (int)strlen(message2)+1, 
						HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true));


					// The client should have received 2 messages
					Packet* clientPacket = pC->Receive();
					CHECK(clientPacket != NULL);
					CHECK_EQUAL(0, stricmp(message, (const char*)clientPacket->data));
					pC->DeallocatePacket(clientPacket);
					clientPacket = pC->Receive();
					CHECK(clientPacket != NULL);
					CHECK_EQUAL(0, stricmp(message2, (const char*)clientPacket->data));
					pC->DeallocatePacket(clientPacket);
					CHECK(pC->Receive() == NULL);

					// The server should receive one message
					Packet* serverPacket = pS->Receive();
					CHECK(serverPacket != NULL);
					CHECK_EQUAL(0, stricmp(message, (const char*)serverPacket->data));
					pS->DeallocatePacket(serverPacket);
					CHECK(pS->Receive() == NULL);

					// Try sending a bitstream
					RakNet::BitStream bitStream;
					int iValIn = 42;
					float fValIn = 3.14f;
					bitStream.Write(iValIn);
					bitStream.Write(fValIn);
					CHECK(IServer::Instance()->Send(&bitStream, 
						HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_PLAYER_ID, true));

					// Receive the packets and make sure we get back out what we put in
					clientPacket = pC->Receive();
					CHECK(clientPacket != NULL);
					RakNet::BitStream clientReadBitStream(clientPacket->data, BITS_TO_BYTES(clientPacket->bitSize), false);
					int iValOut;  
					float fValOut;
					CHECK(clientReadBitStream.Read(iValOut));
					CHECK_EQUAL(iValIn, iValOut);
					CHECK(clientReadBitStream.Read(fValOut));
					CHECK_CLOSE(fValIn, fValOut, 0.001);
					pC->DeallocatePacket(clientPacket);


					// If we delete the Server, the Client is no longer connected.
					// also make sure we can delete with an element still in the QUEUE
					CHECK(IClient::Instance()->Send(message, (int)strlen(message)+1, 
						HIGH_PRIORITY, RELIABLE, 0));
					delete server;
					CHECK_EQUAL(false, IClient::Instance()->IsConnected());

					// And now the Client should fail in its send
					CHECK_EQUAL(false, IClient::Instance()->Send(message, (int)strlen(message)+1, 
						HIGH_PRIORITY, RELIABLE, 0));
				}
			}

		}
	}
}