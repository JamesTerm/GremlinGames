/*
 * ServerConnectionList.h
 *
 *  Created on: Sep 26, 2012
 *      Author: Mitchell Wills
 */

#ifndef SERVERCONNECTIONLIST_H_
#define SERVERCONNECTIONLIST_H_



class ServerConnectionList;
class ServerIncomingStreamMonitor;

#include "../FlushableOutgoingEntryReceiver.h"
#include "../NetworkTableEntry.h"
#include "ServerAdapterManager.h"
#include "ServerConnectionAdapter.h"
#include <vector>



/**
 * A list of connections that the server currently has
 * 
 * @author Mitchell
 *
 */
class ServerConnectionList : public FlushableOutgoingEntryReceiver, public ServerAdapterManager{
private:
	ReentrantSemaphore connectionsLock;
	std::vector<ServerConnectionAdapter*> connections;
	ServerIncomingStreamMonitor * const m_Factory; //make call to close connection
public:
	ServerConnectionList(ServerIncomingStreamMonitor *Factory);
	virtual ~ServerConnectionList();
	/**
	 * Add a connection to the list
	 * @param connection
	 */
	void add(ServerConnectionAdapter& connection);
	
	
	void close(ServerConnectionAdapter& connectionAdapter, bool closeStream);
	/**
	 * close all connections and remove them
	 */
	void closeAll();
	
	void offerOutgoingAssignment(NetworkTableEntry* entry);
	void offerOutgoingUpdate(NetworkTableEntry* entry);
	void flush();
	void ensureAlive();
};



#endif /* SERVERCONNECTIONLIST_H_ */
