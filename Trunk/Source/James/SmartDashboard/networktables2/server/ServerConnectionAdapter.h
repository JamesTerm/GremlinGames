/*
 * ServerConnectionAdapter.h
 *
 *  Created on: Sep 26, 2012
 *      Author: Mitchell Wills
 */

#ifndef SERVERCONNECTIONADAPTER_H_
#define SERVERCONNECTIONADAPTER_H_

class ServerConnectionAdapter;

#include "../connection/ConnectionMonitorThread.h"
#include "../NetworkTableEntry.h"
#include "../connection/ConnectionAdapter.h"
#include "../stream/IOStream.h"
#include "../IncomingEntryReceiver.h"
#include "../FlushableOutgoingEntryReceiver.h"
#include "../server/ServerNetworkTableEntryStore.h"
#include "../server/ServerAdapterManager.h"
#include "../server/ServerConnectionState.h"
#include "../thread/NTThread.h"
#include "../thread/NTThreadManager.h"



/**
 * Object that adapts messages from a client to the server
 * 
 * @author Mitchell
 *
 */
class ServerConnectionAdapter : ConnectionAdapter, IncomingEntryReceiver, FlushableOutgoingEntryReceiver{
private:
	ServerNetworkTableEntryStore& entryStore;
	IncomingEntryReceiver& transactionReceiver;
	ServerAdapterManager& adapterListener;
public:
	/**
	 * the connection this adapter uses
	 */
	NetworkTableConnection connection;
private:
	NTThread* readThread;
	ConnectionMonitorThread monitorThread;
private:

	ServerConnectionState* connectionState;

	void gotoState(ServerConnectionState* newState);

public:
	/**
	 * Create a server connection adapter for a given stream
	 * 
	 * @param stream
	 * @param transactionPool
	 * @param entryStore
	 * @param transactionReceiver
	 * @param adapterListener
	 * @param threadManager
	 */
	ServerConnectionAdapter(IOStream* stream, ServerNetworkTableEntryStore& entryStore, IncomingEntryReceiver& transactionReceiver, ServerAdapterManager& adapterListener, NetworkTableEntryTypeManager& typeManager, NTThreadManager& threadManager);
	virtual ~ServerConnectionAdapter();

	void badMessage(BadMessageException& e);
	
	void ioException(IOException& e);
	
	
	/**
	 * stop the read thread and close the stream
	 */
	void shutdown(bool closeStream);

	void keepAlive();

	void clientHello(ProtocolVersion protocolRevision);

	void protocolVersionUnsupported(ProtocolVersion protocolRevision);

	void serverHelloComplete();

	void offerIncomingAssignment(NetworkTableEntry* entry);

	void offerIncomingUpdate(NetworkTableEntry* entry, SequenceNumber sequenceNumber, EntryValue value);

	NetworkTableEntry* GetEntry(EntryId id);

	void offerOutgoingAssignment(NetworkTableEntry* entry);
	
	void offerOutgoingUpdate(NetworkTableEntry* entry);


	void flush();

	/**
	 * @return the state of the connection
	 */
	ServerConnectionState* getConnectionState();

	void ensureAlive();

};


#endif /* SERVERCONNECTIONADAPTER_H_ */
