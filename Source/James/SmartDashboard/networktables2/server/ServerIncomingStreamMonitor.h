/*
 * ServerIncomingStreamMonitor.h
 *
 *  Created on: Sep 26, 2012
 *      Author: Mitchell Wills
 */

#ifndef SERVERINCOMINGSTREAMMONITOR_H_
#define SERVERINCOMINGSTREAMMONITOR_H_


class ServerIncomingStreamMonitor;


#include "../thread/PeriodicRunnable.h"
#include "../thread/NTThreadManager.h"
#include "../thread/NTThread.h"
#include "../stream/IOStreamProvider.h"
#include "../server/ServerIncomingConnectionListener.h"
#include "../server/ServerNetworkTableEntryStore.h"
#include "../server/ServerAdapterManager.h"
#include "../server/ServerConnectionAdapter.h"



/**
 * Thread that monitors for incoming connections
 * 
 * @author Mitchell
 *
 */
class ServerIncomingStreamMonitor : PeriodicRunnable{
private:
	IOStreamProvider& streamProvider;
	ServerNetworkTableEntryStore& entryStore;
	ServerIncomingConnectionListener& incomingListener;

	ServerAdapterManager& adapterListener;
	NetworkTableEntryTypeManager& typeManager;
	NTThreadManager& threadManager;
	NTThread* monitorThread;
	
public:
	/**
	 * Create a new incoming stream monitor
	 * @param streamProvider the stream provider to retrieve streams from
	 * @param entryStore the entry store for the server
	 * @param transactionPool transaction pool for the server
	 * @param incomingListener the listener that is notified of new connections
	 * @param adapterListener the listener that will listen to adapter events
	 * @param threadManager the thread manager used to create the incoming thread and provided to the Connection Adapters
	 */
	ServerIncomingStreamMonitor(IOStreamProvider& streamProvider, ServerNetworkTableEntryStore& entryStore,
			ServerIncomingConnectionListener& incomingListener,
			ServerAdapterManager& adapterListener,
			NetworkTableEntryTypeManager& typeManager, NTThreadManager& threadManager);
	
	/**
	 * Start the monitor thread
	 */
	void start();
	/**
	 * Stop the monitor thread
	 */
	void stop();
	
	void run();
	
	
	
};




#endif /* SERVERINCOMINGSTREAMMONITOR_H_ */
