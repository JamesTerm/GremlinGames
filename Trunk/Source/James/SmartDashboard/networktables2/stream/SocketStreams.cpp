#include "stdafx.h"
/*
 * SocketStreams.cpp
 *
 *  Created on: Sep 27, 2012
 *      Author: Mitchell Wills
 */

#include "SocketStreams.h"
#include "SocketStreamFactory.h"
#include "SocketServerStreamProvider.h"



IOStreamFactory& SocketStreams::newStreamFactory(const char* host, int port){
	return *new SocketStreamFactory(host, port);
}

IOStreamProvider& SocketStreams::newStreamProvider(int port){
	return *new SocketServerStreamProvider(port);
}

