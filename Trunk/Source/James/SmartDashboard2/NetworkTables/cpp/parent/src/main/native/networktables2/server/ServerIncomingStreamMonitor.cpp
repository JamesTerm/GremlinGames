/*
 * ServerIncomingStreamMonitor.cpp
 *
 *  Created on: Sep 26, 2012
 *      Author: Mitchell Wills
 */

#include "networktables2/server/ServerIncomingStreamMonitor.h"
#include "networktables2/stream/IOStream.h"

ServerIncomingStreamMonitor::ServerIncomingStreamMonitor(IOStreamProvider& _streamProvider, ServerNetworkTableEntryStore& _entryStore,
		ServerIncomingConnectionListener& _incomingListener, ServerAdapterManager& _adapterListener, NetworkTableEntryTypeManager& _typeManager,
		NTThreadManager& _threadManager) :
	streamProvider(_streamProvider), entryStore(_entryStore), incomingListener(_incomingListener), adapterListener(_adapterListener),
			typeManager(_typeManager), threadManager(_threadManager), monitorThread(NULL)
{
}

ServerIncomingStreamMonitor::~ServerIncomingStreamMonitor()
{
	delete &typeManager;
}

/**
 * Start the monitor thread
 */
void ServerIncomingStreamMonitor::start()
{
	if (monitorThread != NULL)
		stop();
	monitorThread = threadManager.newBlockingPeriodicThread(this, "Server Incoming Stream Monitor Thread");
}
/**
 * Stop the monitor thread
 */
void ServerIncomingStreamMonitor::stop()
{
	if (monitorThread != NULL)
	{
		monitorThread->stop();
		delete monitorThread;
		monitorThread = NULL;
	}
}

void ServerIncomingStreamMonitor::run()
{
	try
	{
		IOStream* newStream = streamProvider.accept();
		//Note: monitorThread must be checked to avoid crash on exit
		//  [8/31/2013 Terminator]
		if ((monitorThread!=NULL)&&(newStream != NULL))
		{
			ServerConnectionAdapter* connectionAdapter = new ServerConnectionAdapter(newStream, entryStore, entryStore, adapterListener, typeManager, threadManager);
			incomingListener.OnNewConnection(*connectionAdapter);
		}
	}
	catch (IOException& e)
	{
		//could not get a new stream for some reason. ignore and continue
	}
}

