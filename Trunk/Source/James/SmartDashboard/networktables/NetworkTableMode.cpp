/*
 * NetworkTableMode.cpp
 *
 *  Created on: Oct 16, 2012
 *      Author: Mitchell Wills
 */
#include "../stdafx.h"
#include <string>
#include "../networktables2/thread/NTThreadManager.h"
#include "../networktables2/NetworkTableNode.h"
#include "../networktables2/server/NetworkTableServer.h"
#include "../networktables2/client/NetworkTableClient.h"
#include "../networktables2/stream/SocketServerStreamProvider.h"
#include "../networktables2/stream/SocketStreamFactory.h"
#include "../networktables/NetworkTableMode.h"


NetworkTableServerMode NetworkTableMode::Server;
NetworkTableClientMode NetworkTableMode::Client;

NetworkTableServerMode::NetworkTableServerMode(){}
NetworkTableClientMode::NetworkTableClientMode(){}


//TODO this needs to be handled where these are not needed... most likely these should be instantiated within the constructor
//  [8/30/2013 Terminator]
static IOStreamProvider* gServer_streamProvider=NULL;
static NetworkTableEntryTypeManager* gServer_NetworkTableEntryTypeManager=NULL;

NetworkTableNode* NetworkTableServerMode::CreateNode(const char* ipAddress, int port, NTThreadManager& threadManager){
	IOStreamProvider* streamProvider = new SocketServerStreamProvider(port);
	gServer_streamProvider=streamProvider;
	gServer_NetworkTableEntryTypeManager=new NetworkTableEntryTypeManager;
	return new NetworkTableServer(*streamProvider, *gServer_NetworkTableEntryTypeManager, threadManager);
}

void NetworkTableServerMode::DestroyNode(NetworkTableNode *Node)
{
	if (Node!=NULL)
	{
		delete Node;
		delete gServer_streamProvider;
		delete gServer_NetworkTableEntryTypeManager;
	}
}


NetworkTableNode* NetworkTableClientMode::CreateNode(const char* ipAddress, int port, NTThreadManager& threadManager){
	IOStreamFactory* streamFactory = new SocketStreamFactory(ipAddress, port);
	return new NetworkTableClient(*streamFactory, *new NetworkTableEntryTypeManager(), threadManager);
}

void NetworkTableClientMode::DestroyNode(NetworkTableNode *Node)
{
	delete Node;
}
