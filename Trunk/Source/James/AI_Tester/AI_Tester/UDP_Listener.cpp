#include "stdafx.h"
#include <stdlib.h>     /* for exit() */
#include <stdio.h>      /* for printf(), fprintf() */
#include <string>     /* for string functions  */
#include <iostream>
#include <winsock2.h>    /* for socket(),... */

#include "AI_Tester.h"
#include "UDP_Listener.h"

#pragma comment( lib, "Ws2_32" )


class UDP_Listener
{
public:
	//http://www.chiefdelphi.com/forums/showthread.php?p=1024930
	//has info about dashboard to robot using 1130 and robot to dashboard using 1140
	//also in FMS white paper
	UDP_Listener(UDP_Listener_Interface *client,unsigned short portno=1130) : m_Client(client),m_Error(false)
	{
		WORD wVersionRequested;          // Version of Winsock to load 
		WSADATA wsaData;                 // Winsock implementation details 

		try
		{
			/* Winsock DLL and library initialization  */
			wVersionRequested = MAKEWORD(2, 0);   /* Request Winsock v2.0 */
			if (WSAStartup(wVersionRequested, &wsaData) != 0) /* Load Winsock DLL */
				throw 0;
			int clilen;
			// 1. Create a socket. 
			// Create a best-effort datagram socket using UDP 
			struct sockaddr_in serv_addr, cli_addr;
			if ((m_sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
				throw 1;

			unsigned long mode = 1;
			int test=ioctlsocket(m_sockfd, FIONBIO, &mode);
			if (test!=0)
				printf("Warning unable to set socket to non-blocking");

			/* Construct the server address structure */
			memset(&serv_addr, 0, sizeof(serv_addr));    // Zero out structure 
			serv_addr.sin_family = AF_INET;                 // Internet address family 
			serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			serv_addr.sin_port   = htons(portno);		 // Server port 
			if (bind(m_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
				throw 2;

			//listen(m_sockfd,5);
			clilen = sizeof(cli_addr);
		}
		catch (int ErrorCode)
		{
			const char *ErrorMsg=NULL;
			switch (ErrorCode)
			{
			case 0:
				ErrorMsg="WSAStartup() failed";
				break;
			case 1:
				ErrorMsg="socket() failed";
				break;
			case 2:
				ErrorMsg="ERROR on binding";
				break;
			};
			if (ErrorMsg)
				printf("ErrorMsg=%s\n",ErrorMsg);
			m_Error=true;

		}
	}

	~UDP_Listener()
	{
		// 5. Close the socket. 
		// Winsock requires a special function for sockets 
		closesocket(m_sockfd);    // Close client socket 
		WSACleanup();  // Cleanup Winsock 
	}
	void TimeChange(double dTime_s)
	{
		if (m_Error) return;
		char buffer[256];
		memset(&buffer,0,256);
		struct sockaddr_in fromAddr; 
		int fromSize = sizeof(fromAddr);
		int n=recvfrom(m_sockfd, buffer, 255, 0, (struct sockaddr *) &fromAddr, &fromSize);

		if (n>0)
		{
			m_Client->ProcessPacket(buffer,n);
		}
	}

private:
	UDP_Listener_Interface *m_Client; //delegate packet to client code
	int m_sockfd;
	bool m_Error;
};


class coodinate_manager : public coodinate_manager_Interface
{
	public:
		coodinate_manager() : m_UDP(this)
		{
			
		}
		void TimeChange(double dTime_s)
		{
			m_Updated=false;
			m_UDP.TimeChange(dTime_s);
			//TODO smart dashboard display and servo manipulation
		}
	protected: //from UDP_Listener_Interface
		virtual void ProcessPacket(char *pkt,size_t pkt_size)
		{
			if (pkt_size==16)
			{
				typedef unsigned long DWORD;
				long *ptr=(long *)pkt;
				//apparently winsock doesn't have the big endian issue
				#if 0
				DWORD sync=_byteswap_ulong(ptr[0] );
				long XInt=(long)_byteswap_ulong(ptr[1]);
				long YInt=(long)_byteswap_ulong(ptr[2]);
				long checksum=(long)_byteswap_ulong(ptr[3]);
				#else
				DWORD sync=ptr[0];
				long XInt=(long)ptr[1];
				long YInt=(long)ptr[2];
				long checksum=(long)ptr[3];
				#endif

				if (sync==0xabacab)
				{
					if (checksum==XInt+YInt)
					{
						m_Updated=true;
						m_Xpos=(double)XInt / 10000000.0;
						m_Ypos=(double)YInt / 10000000.0;
						//printf("New coordinates %f , %f\n",m_Xpos,m_Ypos);
						//SmartDashboard::PutNumber("X Position",m_Xpos);
						//SmartDashboard::PutNumber("Y Position",m_Ypos);
					}
					else
						printf("%d + %d != %d\n",(int)XInt,(int)YInt,(int)checksum);
				}
			}
			else
				printf("warning packet size=%d\n",pkt_size);
		}
	private:
	 UDP_Listener m_UDP;
};

coodinate_manager_Interface *coodinate_manager_Interface::CreateInstance()
{
	return new coodinate_manager;
}

void coodinate_manager_Interface::DestroyInstance(coodinate_manager_Interface *instance)
{
	delete instance;
}
