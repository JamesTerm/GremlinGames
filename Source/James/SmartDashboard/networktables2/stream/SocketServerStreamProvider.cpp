#include "stdafx.h"
/*
 * SocketServerStreamProvider.cpp
 *
 *  Created on: Sep 27, 2012
 *      Author: Mitchell Wills
 */

#include "SocketServerStreamProvider.h"
#include "FDIOStream.h"
#include "../util/IOException.h"

//#include <strings.h>
#include <cstring>
#include <errno.h>
#ifdef _WRS_KERNEL
#include <inetLib.h>
#include <selectLib.h>
#include <sockLib.h>
#include <taskLib.h>
#include <usrLib.h>
#include <ioLib.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
//#include <unistd.h>
#ifdef WIN32
#include <windows.h>
//#include <winsock.h>
#include <winsock2.h>
#include <wininet.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif
#endif

#ifndef ERROR
#define ERROR -1
#endif

#if defined(WIN32) || defined(_WRS_KERNEL)
typedef int addrlen_t;
#else
typedef socklen_t addrlen_t;
#endif

void load_tcpip(void)
{
	WSAData wsaData_;
	WORD wVersionRequested_ = MAKEWORD( 2, 2 );

	int result=WSAStartup( wVersionRequested_, &wsaData_ );
	assert(result==0);
}

bool unload_tcpip(void)
{
	WSACleanup();
	return true;
}


SocketServerStreamProvider::SocketServerStreamProvider(int port)
{
	load_tcpip();

	struct sockaddr_in serverAddr;
	int sockAddrSize = sizeof(serverAddr);
	memset(&serverAddr, 0, sockAddrSize);

#ifdef _WRS_KERNEL
	serverAddr.sin_len = (u_char)sockAddrSize;
#endif
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		throw IOException("Error creating server socket", errno);
	}

	// Set the TCP socket so that it can be reused if it is in the wait state.
	int reuseAddr = 1;
	setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuseAddr, sizeof(reuseAddr));

	// Bind socket to local address.
	if (bind(serverSocket, (struct sockaddr *)&serverAddr, sockAddrSize) != 0)
	{
		close();
		throw IOException("Could not bind server socket", errno);
	}

	if (listen(serverSocket, 1) == SOCKET_ERROR)
	{
		close();
		throw IOException("Could not listen on server socket", errno);
	}
}

SocketServerStreamProvider::~SocketServerStreamProvider()
{
	close();
	unload_tcpip();
}


IOStream* SocketServerStreamProvider::accept(){
	struct timeval timeout;
	// Check for a shutdown once per second
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	while (true)
	{
		fd_set fdSet;

		FD_ZERO(&fdSet);
		FD_SET(serverSocket, &fdSet);
		if (select(FD_SETSIZE, &fdSet, NULL, NULL, &timeout) > 0)
		{
			if (FD_ISSET(serverSocket, &fdSet))
			{
				struct sockaddr clientAddr = {0};
				addrlen_t clientAddrSize = 0;
				int connectedSocket = ::accept(serverSocket, &clientAddr, &clientAddrSize);
				if (connectedSocket == ERROR)
					return NULL;
				
				int on = 1;
				setsockopt(connectedSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
				
				return new FDIOStream(connectedSocket);
			}
		}
	}
	return NULL;
}

void SocketServerStreamProvider::close()
{
	if (serverSocket!=INVALID_SOCKET)
	{
		//::close(serverSocket);
		shutdown( serverSocket, SD_BOTH );
		closesocket( serverSocket );
		serverSocket = (int)INVALID_SOCKET;

	}
}
