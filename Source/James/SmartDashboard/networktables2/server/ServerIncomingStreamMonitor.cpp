#include "stdafx.h"
/*
 * ServerIncomingStreamMonitor.cpp
 *
 *  Created on: Sep 26, 2012
 *      Author: Mitchell Wills
 */

#include "ServerIncomingStreamMonitor.h"
#include "../stream/IOStream.h"

ServerIncomingStreamMonitor::ServerIncomingStreamMonitor(IOStreamProvider& _streamProvider, ServerNetworkTableEntryStore& _entryStore,
		ServerIncomingConnectionListener& _incomingListener, ServerAdapterManager& _adapterListener, NetworkTableEntryTypeManager& _typeManager,
		NTThreadManager& _threadManager) :
	streamProvider(_streamProvider), entryStore(_entryStore), incomingListener(_incomingListener), adapterListener(_adapterListener),
			typeManager(_typeManager), threadManager(_threadManager), monitorThread(NULL)
{
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
		NTThread *temp=monitorThread;
		monitorThread = NULL;  //call this before stop for the check below to ensur a new server connection adapter will not happen
		temp->stop();
		delete temp;
	}
}

void ServerIncomingStreamMonitor::run()
{
	try
	{
		while (monitorThread!=NULL)
		{
			IOStream* newStream = streamProvider.accept();
			{
				Synchronized sync(BlockDeletionList);
				for (size_t i=0;i<m_DeletionList.size();i++)
				{
					ServerConnectionAdapter *Element=m_DeletionList[i];
					Element->shutdown(true);  //TODO assume to always close stream
					delete Element;
				}
				m_DeletionList.clear();
			}
			//Note: monitorThread must be checked to avoid crash on exit
			//  [8/31/2013 Terminator]
			if ((monitorThread!=NULL)&&(newStream != NULL))
			{
				ServerConnectionAdapter* connectionAdapter = new ServerConnectionAdapter(newStream, entryStore, entryStore, adapterListener, typeManager, threadManager);
				incomingListener.OnNewConnection(*connectionAdapter);
			}
		}
	}
	catch (IOException& e)
	{
		//could not get a new stream for some reason. ignore and continue
	}
}

void ServerIncomingStreamMonitor::close(ServerConnectionAdapter *Adapter)
{
	Synchronized sync(BlockDeletionList);
	m_DeletionList.push_back(Adapter);
}