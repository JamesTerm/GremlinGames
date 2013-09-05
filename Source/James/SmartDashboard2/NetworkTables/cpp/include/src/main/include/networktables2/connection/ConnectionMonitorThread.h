/*
 * ConnectionMonitorThread.h
 *
 *  Created on: Sep 22, 2012
 *      Author: Mitchell Wills
 */

#ifndef CONNECTIONMONITORTHREAD_H_
#define CONNECTIONMONITORTHREAD_H_


class ConnectionMonitorThread;

#include "networktables2/connection/ConnectionAdapter.h"
#include "networktables2/connection/NetworkTableConnection.h"
#include "networktables2/thread/PeriodicRunnable.h"




/**
 * A periodic thread that repeatedly reads from a connection
 * @author Mitchell
 *
 */
class ConnectionMonitorThread : public PeriodicRunnable{
private:
	ConnectionAdapter& adapter;
	NetworkTableConnection& connection;

public:
	/**
	 * create a new monitor thread
	 * @param adapter
	 * @param connection
	 */
	ConnectionMonitorThread(ConnectionAdapter& adapter, NetworkTableConnection& connection);

	void run();
};


#endif /* CONNECTIONMONITORTHREAD_H_ */
