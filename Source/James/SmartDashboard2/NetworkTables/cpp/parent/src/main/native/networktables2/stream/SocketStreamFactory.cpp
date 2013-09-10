#include "stdafx.h"
/*
* SocketStreamFactory.cpp
*
*  Created on: Nov 3, 2012
*      Author: Mitchell Wills
*/


#include <cstring>
#ifdef _WRS_KERNEL
#else
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
//#include <unistd.h>
#ifdef WIN32
//#include <winsock.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#endif
#endif
#include "networktables2/stream/FDIOStream.h"
#include "networktables2/stream/SocketStreamFactory.h"


SocketStreamFactory::SocketStreamFactory(const char* _host, int _port):host(_host), port(_port){}

SocketStreamFactory::~SocketStreamFactory(){}

IOStream *SocketStreamFactory::createStream(){
#ifdef _WRS_KERNEL
	//crio client not supported
	return NULL;
#else
	IOStream *ret=NULL;
	int sockfd = INVALID_SOCKET;
	try
	{
		struct sockaddr_in serv_addr;
		struct hostent *server;

		sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sockfd < 0) throw 0;

		server = gethostbyname(host);

		if (server == NULL) 
			throw 1;

		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
		serv_addr.sin_port = htons(port);

		//We are outgoing so setup the socket options before making the connection
		//Setup for TCP_NODELAY for nice crisp response time...
		int on = 1;
		setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
		// Set the linger options
		const LINGER linger = { 1, 0 };
		setsockopt( sockfd, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger) );

		if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
			throw 2;

		ret=new FDIOStream(sockfd);
	}
	catch (int ErrorCode)
	{
		//Close resources if we cannot meet all the preconditions
		if (sockfd != INVALID_SOCKET)
		{
			//We must shut down socket before issuing the close to avoid zombie ports (Ask Kirk)
			shutdown( sockfd, SD_BOTH );
			closesocket( sockfd );
			sockfd = (int)INVALID_SOCKET;  //pedantic, in case we cache as a member variable
		}
		const char *ErrorMsg=NULL;
		switch (ErrorCode)
		{
		case 0:
			ErrorMsg="ERROR opening socket";
			break;
		case 1:
			ErrorMsg="ERROR, no such host";
			break;
		case 2:
			ErrorMsg="ERROR on connect";
			break;
		};
		if (ErrorMsg)
			printf("ErrorMsg=%s WSA error=%d\n",ErrorMsg,WSAGetLastError());
		Sleep(1000); //avoid flooding to connect... it doesn't need to occur every 20ms
	}
	return ret;
#endif
}
