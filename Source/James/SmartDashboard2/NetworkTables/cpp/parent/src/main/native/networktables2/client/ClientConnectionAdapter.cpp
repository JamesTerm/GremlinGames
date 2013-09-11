/*
 * ClientConnectionAdapter.cpp
 *
 *  Created on: Nov 2, 2012
 *      Author: Mitchell Wills
 */

#include "networktables2/client/ClientConnectionAdapter.h"

void ClientConnectionAdapter::gotoState(ClientConnectionState* newState){
	{
		NTSynchronized sync(LOCK);
		if(connectionState!=newState){
		        fprintf(stdout, "[NT] %p entered connection state: %s\n", (void*)this, newState->toString());
			fflush(stdout);
			if(newState==&ClientConnectionState::IN_SYNC_WITH_SERVER)
				connectionListenerManager.FireConnectedEvent();
			if(connectionState==&ClientConnectionState::IN_SYNC_WITH_SERVER)
				connectionListenerManager.FireDisconnectedEvent();
			connectionState = newState;
		}
	}
}
/**
 * @return the state of the connection
 */
ClientConnectionState* ClientConnectionAdapter::getConnectionState(){
	return connectionState;
}
/**
 * @return if the client is connected to the server
 */
bool ClientConnectionAdapter::isConnected() {
	return getConnectionState()==&ClientConnectionState::IN_SYNC_WITH_SERVER;
}

/**
 * Create a new ClientConnectionAdapter
 * @param entryStore
 * @param threadManager
 * @param streamFactory
 * @param transactionPool
 * @param connectionListenerManager
 */
ClientConnectionAdapter::ClientConnectionAdapter(ClientNetworkTableEntryStore& _entryStore, NTThreadManager& _threadManager, IOStreamFactory& _streamFactory, ClientConnectionListenerManager& _connectionListenerManager, NetworkTableEntryTypeManager& _typeManager):
	entryStore(_entryStore),
	streamFactory(_streamFactory),
	threadManager(_threadManager),
	connectionListenerManager(_connectionListenerManager),
	typeManager(_typeManager),
	readThread(NULL),
	monitor(NULL),
	connection(NULL),
	m_IsClosing(false){
	connectionState = &ClientConnectionState::DISCONNECTED_FROM_SERVER;
}
ClientConnectionAdapter::~ClientConnectionAdapter()
{
	m_IsClosing=true;
	//close all resources here since calling close will defer deletion of the thread (This is the UI thread and we must not defer it)
	if(connection!=NULL)
		connection->close();
	if(readThread!=NULL)
	{
	    delete readThread;
		readThread = NULL;
	}
	if(monitor!=NULL)
	{
	        delete monitor;
		monitor = NULL;
	}	
	if(connection!=NULL)
	{
		delete connection;
		connection = NULL;
	}	

  delete &typeManager;
  delete &streamFactory;
}


/*
 * Connection management
 */
/**
 * Reconnect the client to the server (even if the client is not currently connected)
 */
void ClientConnectionAdapter::reconnect() {
	{
		NTSynchronized sync(LOCK);
		close();//close the existing stream and monitor thread if needed
		if (!m_IsClosing)
		{
			try{
				IOStream* stream = streamFactory.createStream();
				if(stream==NULL)
					return;
				connection = new NetworkTableConnection(stream, typeManager);
				monitor = new ConnectionMonitorThread(*this, *connection);
				readThread = threadManager.newBlockingPeriodicThread(monitor, "Client Connection Reader Thread");
				connection->sendClientHello();
				gotoState(&ClientConnectionState::CONNECTED_TO_SERVER);
			} catch(IOException& e){
				close();//make sure to clean everything up if we fail to connect
			}
		}
	}
}

/**
 * Close the client connection
 */
void ClientConnectionAdapter::close() {
	close(&ClientConnectionState::DISCONNECTED_FROM_SERVER);
}
/**
 * Close the connection to the server and enter the given state
 * @param newState
 */
void ClientConnectionAdapter::close(ClientConnectionState* newState) {
	{
		NTSynchronized sync(LOCK);
		gotoState(newState);
		//instead of stopping the thread it will auto sleep for deletion once a new connection is assigned
		//if(readThread!=NULL){
		//	readThread->stop();
		//}
		if(connection!=NULL){
			connection->close();
		}
		//clear connection before issuing the deferred delete to avoid race condition
		if(connection!=NULL){
			delete connection;
			connection = NULL;
		}	
		entryStore.clearIds();//TODO maybe move this to reconnect so that the entry store doesn't have to be valid when this object is deleted

		//defer this for the FlushableOutgoingEntryReceiver::ensureAlive method from the write manager thread
		if(monitor !=NULL)
		{
			assert(monitor && readThread);  //sanity check... if we have a connection we have the read and monitor
			NTSynchronized sync(BlockDeletionList);
			DeletionPacket newPacket;
			newPacket.monitor=monitor;
			newPacket.readThread=readThread;
			m_DeletionList.push_back(newPacket);
			monitor=NULL;
			readThread=NULL;
		}
	}
}



void ClientConnectionAdapter::badMessage(BadMessageException& e) {
	close(new ClientConnectionState_Error(e));
}

void ClientConnectionAdapter::ioException(IOException& e) {
	if(connectionState!=&ClientConnectionState::DISCONNECTED_FROM_SERVER)//will get io exception when on read thread connection is closed
		reconnect();
	//gotoState(new ClientConnectionState.Error(e));
}

NetworkTableEntry* ClientConnectionAdapter::GetEntry(EntryId id) {
	return entryStore.GetEntry(id);
}


bool ClientConnectionAdapter::keepAlive() {
	//we keep alive if the connection the monitor thread holds matches this connection
	return monitor?connection==monitor->GetNetworkTableConnection():false;
}

void ClientConnectionAdapter::clientHello(ProtocolVersion protocolRevision) {
	throw BadMessageException("A client should not receive a client hello message");
}

void ClientConnectionAdapter::protocolVersionUnsupported(ProtocolVersion protocolRevision) {
	close();
	gotoState(new ClientConnectionState_ProtocolUnsuppotedByServer(protocolRevision));
}

void ClientConnectionAdapter::serverHelloComplete() {
	if (connectionState==&ClientConnectionState::CONNECTED_TO_SERVER) {
		try {
			gotoState(&ClientConnectionState::IN_SYNC_WITH_SERVER);
			entryStore.sendUnknownEntries(*connection);
		} catch (IOException& e) {
			ioException(e);
		}
	}
	else
		throw BadMessageException("A client should only receive a server hello complete once and only after it has connected to the server");
}


void ClientConnectionAdapter::offerIncomingAssignment(NetworkTableEntry* entry) {
	entryStore.offerIncomingAssignment(entry);
}
void ClientConnectionAdapter::offerIncomingUpdate(NetworkTableEntry* entry, SequenceNumber sequenceNumber, EntryValue value) {
	entryStore.offerIncomingUpdate(entry, sequenceNumber, value);
}

void ClientConnectionAdapter::offerOutgoingAssignment(NetworkTableEntry* entry) {
	try {
		{
			NTSynchronized sync(LOCK);
			if(connection!=NULL && connectionState==&ClientConnectionState::IN_SYNC_WITH_SERVER)
				connection->sendEntryAssignment(*entry);
		}
	} catch(IOException& e){
		ioException(e);
	}
}

void ClientConnectionAdapter::offerOutgoingUpdate(NetworkTableEntry* entry) {
	try {
		{
			NTSynchronized sync(LOCK);
			if(connection!=NULL && connectionState==&ClientConnectionState::IN_SYNC_WITH_SERVER)
				connection->sendEntryUpdate(*entry);
		}
	} catch(IOException& e){
		ioException(e);
	}
}
void ClientConnectionAdapter::flush() {
	{
		NTSynchronized sync(LOCK);
		if(connection!=NULL) {
			try {
				connection->flush();
			} catch (IOException& e) {
				ioException(e);
			}
		}
	}
}
void ClientConnectionAdapter::ensureAlive() {
	{
		NTSynchronized sync(LOCK);
		if(connection!=NULL) {
			try {
			  connection->sendKeepAlive();
			} catch (IOException& e) {
				ioException(e);
			}
		}
		else
			reconnect();//try to reconnect if not connected
	}
	{
		NTSynchronized sync(BlockDeletionList);
		for (size_t i=0;i<m_DeletionList.size();i++)
		{
			const DeletionPacket &Element=m_DeletionList[i];
			if(Element.readThread!=NULL)
				delete Element.readThread;
			if(Element.monitor!=NULL)
				delete Element.monitor;
		}
		m_DeletionList.clear();
	}

}
