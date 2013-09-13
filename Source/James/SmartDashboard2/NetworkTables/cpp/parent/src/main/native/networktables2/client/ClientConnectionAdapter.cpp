/*
 * ClientConnectionAdapter.cpp
 *
 *  Created on: Nov 2, 2012
 *      Author: Mitchell Wills
 */
#include <algorithm>
#include "networktables2/client/ClientConnectionAdapter.h"
#include "networktables2/util/System.h"

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
			//TODO find better way to manage memory leak
			ClientConnectionState_Error *temp=dynamic_cast<ClientConnectionState_Error *>(connectionState);
			connectionState = newState;
			if (temp)
				delete temp;
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
	m_IsClosing(false),
	m_IsReconnecting(false){
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
	//TODO find better way to manage memory leak
	ClientConnectionState_Error *temp=dynamic_cast<ClientConnectionState_Error *>(connectionState);
	if (temp)
	{
		delete temp;
		connectionState=NULL;
	}

  delete &typeManager;
  delete &streamFactory;
}


void ClientConnectionAdapter::PurgeOldConnections()
{
	bool IsThisRightConnection=monitor?connection==monitor->GetNetworkTableConnection():false;
	if ((!IsThisRightConnection)&&(monitor))
	{
		typedef std::vector<DeletionPacket>::iterator DeletionListIter;
		//mark this thread to delete
		DeletionListIter iter = std::find(m_DeletionList.begin(),m_DeletionList.end(),monitor);
		if (iter!=m_DeletionList.end())
		{
			DeletionPacket &Element=(*iter);
			Element.CanDelete=true;  //we can now delete as it is in a sleep state upon this return
		}
		else
		{
			char Buffer[128];
			sprintf(Buffer,"Warning: %p not yet removed\n",monitor);
			throw BadMessageException(Buffer);  //keep track of this... it may be a few iterations at most
		}
	}

	//Clean out threads
	NTSynchronized sync(BlockDeletionList);
	typedef std::vector<DeletionPacket>::iterator DeletionListIter;
	for (DeletionListIter iter=m_DeletionList.begin();iter!=m_DeletionList.end();iter++)
	{
		const DeletionPacket &Element=(*iter);
		if (Element.CanDelete)
		{
			if(connection!=NULL)
				delete connection;
			if(Element.readThread!=NULL)
				delete Element.readThread;
			if(Element.monitor!=NULL)
				delete Element.monitor;
			m_DeletionList.erase(iter);
		}
	}
}


/*
 * Connection management
 */
/**
 * Reconnect the client to the server (even if the client is not currently connected)
 */
void ClientConnectionAdapter::reconnect() {
	{
		PurgeOldConnections();
		bool IsReconnecting=true;
		{	//This block needs to be atomic
			NTSynchronized sync(LOCK);
			if (!m_IsReconnecting)
			{
				m_IsReconnecting=true;
				IsReconnecting=false;
			}
		}
		if (!IsReconnecting)
		{
			{
				//Note: this cannot be within the critical section of the LOCK of connection adapter the write manager has a lock on the entry store and will call here
				entryStore.clearIds();

				NTSynchronized sync(LOCK);
				close();//close the existing stream and monitor thread if needed
				if (!m_IsClosing)
				{
					try{
						IOStream* stream = streamFactory.createStream();
						if(stream!=NULL)
						{
							connection = new NetworkTableConnection(stream, typeManager);
							monitor = new ConnectionMonitorThread(*this, *connection);
							readThread = threadManager.newBlockingPeriodicThread(monitor, "Client Connection Reader Thread");
							connection->sendClientHello();
							gotoState(&ClientConnectionState::CONNECTED_TO_SERVER);
						}
					} catch(IOException& e){
						close();//make sure to clean everything up if we fail to connect
					}
				}
			}
			//Outside of the critical section... reset the valve 
			m_IsReconnecting=false;
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
	NTSynchronized sync(LOCK);
	gotoState(newState);
	//instead of stopping the thread it will auto sleep for deletion once a new connection is assigned
	//if(readThread!=NULL){
	//	readThread->stop();
	//}
	if(connection!=NULL)
		connection->close();

	//defer this for the FlushableOutgoingEntryReceiver::ensureAlive method from the write manager thread
	if(monitor !=NULL)
	{
		assert(monitor && readThread);  //sanity check... if we have a connection we have the read and monitor
		NTSynchronized sync(BlockDeletionList);
		DeletionPacket newPacket;
		newPacket.monitor=monitor;
		newPacket.readThread=readThread;
		newPacket.connection=connection;
		newPacket.CanDelete=false;
		m_DeletionList.push_back(newPacket);
		monitor=NULL;
		readThread=NULL;
		connection=NULL;
	}
}



void ClientConnectionAdapter::badMessage(BadMessageException& e) {
	close(new ClientConnectionState_Error(e));
}

void ClientConnectionAdapter::ioException(IOException& e) {
	PurgeOldConnections();
	if(connectionState!=&ClientConnectionState::DISCONNECTED_FROM_SERVER)//will get io exception when on read thread connection is closed
	{
		reconnect();
		sleep_ms(500);
	}
	else
	{
		sleep_ms(33);  //avoid busy wait
	}
}

NetworkTableEntry* ClientConnectionAdapter::GetEntry(EntryId id) {
	return entryStore.GetEntry(id);
}


bool ClientConnectionAdapter::keepAlive() {
	//we keep alive if the connection the monitor thread holds matches this connection
	bool ret=monitor?connection==monitor->GetNetworkTableConnection():false;
	if (!ret)
		PurgeOldConnections();
	return ret;
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
	PurgeOldConnections();
}
