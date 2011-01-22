// ServerMain.cpp
// December 6, 2006
// Rick Pingry
// Gremlin Games
#include "..\RakWrapper\RakWrapper.h"
#include "Common.h"
#include <iostream>
#include <string>
#include <conio.h>
using namespace std;

void ProcessMessage(RakWrapperNS::RakServer& server)
{
	RakWrapperNS::RakWrapper::PacketPropertiesStruct packProp = server.GetNextPacketType();

	if (!packProp.length)
		return;

	int senderID = -1;
	switch(packProp.type) {
			case vector:
				{
					VectorStruct *va = new VectorStruct[packProp.length];
					server.GetNextPacket<VectorStruct>(va, packProp.length, senderID);

					cout << "Received vector packet from" << senderID << endl;
					for (int i=0; i<packProp.length; i++)
						cout << "va[" << i << "] = " << "(" << va[i].x << ", " << va[i].y << ", " << va[i].z << ")" << endl;

					// Send it back out
					cout << "Broadcasting vectors" << endl;
					server.SendPacket<VectorStruct>(packProp, va);
					delete[] va;
				}
				break;
			case integer:
				{
					int *ia = new int[packProp.length];
					server.GetNextPacket<int>(ia, packProp.length, senderID);

					cout << "Received integer array packet from" << senderID << endl;
					for (int i=0; i<packProp.length; i++)
						cout << "ia[" << i << "] = " << ia[i] << endl;

					// Send it back out
					cout << "Broadcasting ints" << endl;
					server.SendPacket<int>(packProp, ia);
					delete[] ia;
				}
				break;
			case str:
				{
					char *ca = new char[packProp.length];
					server.GetNextPacket<char>(ca, packProp.length, senderID);

					cout << "Received character array packet from" << senderID << endl;
					cout << ca << endl;

					// Send it back out
					cout << "Broadcasting string" << endl;
					server.SendPacket<char>(packProp, ca);
					delete[] ca;
				}
				break;
	}
}

int main () 
{
	char serverName[MAXIMUM_NAME_LENGTH];

	cout << "Starting Evolutions Server" << endl;
	cout << "==========================" << endl;
	cout << "Enter the name of this server: ";
	cin >> serverName;

	RakWrapperNS::RakServer server;
	server.Connect(serverName);

	string message;
	int senderID = -1;

	while (true) 
	{
		if (kbhit())
		{
			cin >> message;

			if (message == "quit") {
				cout << "Goodbye." << endl;
				break;
			}

			// Server only needs to wait for the quit
		}
		ProcessMessage(server);
	}

	return 0;
}