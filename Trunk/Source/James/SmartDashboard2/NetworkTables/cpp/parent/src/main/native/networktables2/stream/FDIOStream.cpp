//TODO implement if used

#include "stdafx.h"
/*
 * FDIOStream.cpp
 *
 *  Created on: Sep 27, 2012
 *      Author: Mitchell Wills
 */

#include "networktables2/stream/FDIOStream.h"
#include "networktables2/util/IOException.h"
#include "networktables2/util/EOFException.h"

#include <errno.h>
#include <stdlib.h>
//#ifdef _WRS_KERNEL
//#include <iolib.h>
//#else
//#include <unistd.h>
//#endif
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#include <wininet.h>
#include <ws2tcpip.h>


FDIOStream::FDIOStream(int _fd){
  fd = _fd;
  //	f = fdopen(_fd, "rbwb");
  //	if(f==NULL)
  //		throw IOException("Could not open stream from file descriptor", errno);
}
FDIOStream::~FDIOStream(){
	close();
	if (fd != INVALID_SOCKET)
	{
		closesocket( fd );
		fd = (int)INVALID_SOCKET;  //pedantic, in case we cache as a member variable
	}
}

int FDIOStream::read(void* ptr, int numbytes){
	if(numbytes==0)
		return 0;
	char* bufferPointer = (char*)ptr;
	int totalRead = 0;
	while (totalRead < numbytes) 
	{
		int numRead=recv(fd, bufferPointer, numbytes-totalRead, 0);
		if(numRead == 0){
			throw EOFException();
		}
		else if (numRead < 0) {
			perror("read error: ");
			fflush(stderr);
			throw IOException("Error on FDIO read");
		}
		bufferPointer += numRead;
		totalRead += numRead;
	}
	return totalRead;
}

int Send( int sockfd,char* Data, size_t sizeData )
{
	assert(sockfd!=INVALID_SOCKET);
	bool Result_ = true;

	WSABUF wsaBuf_;
	wsaBuf_.buf = Data;
	wsaBuf_.len = (ULONG) sizeData;
	DWORD BytesSent_;

	while (WSASend( sockfd, &wsaBuf_, 1, &BytesSent_, 0, NULL, NULL ) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			Result_ = false;
			break;
		}
		Sleep(1);
	}
	assert(Result_);  //TODO reproduce and implement
	return(int)BytesSent_;
}

int FDIOStream::write(const void* ptr, int numbytes)
{
	int numWrote = Send(fd,(char *)ptr,numbytes);
  //int numWrote = ::write(fd, (char*)ptr, numbytes);//TODO: this is bad
  //int numWrote = fwrite(ptr, 1, numbytes, f);
  if(numWrote==numbytes)
    return numWrote;
  perror("write error: ");
  fflush(stderr);
  throw IOException("Could not write all bytes to fd stream");
	
}
void FDIOStream::flush(){
  //if(fflush(f)==EOF)
  //  throw EOFException();
}
void FDIOStream::close()
{
	//I am not really sure why this is here... we don't own the socket so I do not wish to close it... however the shutdown may be
	//what was intended here
	//  [8/31/2013 Terminator]

  //fclose(f);//ignore any errors closing
	//assert(false);
	//::close(fd);
	if (fd != INVALID_SOCKET)
		shutdown( fd, SD_BOTH );
}

