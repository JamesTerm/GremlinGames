#include "WPILib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ioLib.h>
#include "UDP_Listener.h"

unsigned long _byteswap_ulong(unsigned long i)
{
    unsigned int j;
    j =  (i << 24);
    j += (i <<  8) & 0x00FF0000;
    j += (i >>  8) & 0x0000FF00;
    j += (i >> 24);
    return j;
}

class UDP_Listener
{
	public: 
	//http://www.chiefdelphi.com/forums/showthread.php?p=1024930
	//has info about dashboard to robot using 1130 and robot to dashboard using 1140
	UDP_Listener(UDP_Listener_Interface *client,int portno=1130) : m_Client(client),m_Error(false)
	{
		try 
		{
			 //socklen_t clilen;
			int clilen;
			 struct sockaddr_in serv_addr, cli_addr;
			 m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			 if (m_sockfd < 0)	
				 throw 0;
			 
			unsigned long mode = 1;
			int test=ioctl(m_sockfd, FIONBIO,(int) &mode);
			if (test!=0)
				printf("Warning unable to set socket to non-blocking");

			 bzero((char *) &serv_addr, sizeof(serv_addr));
			 serv_addr.sin_family = AF_INET;
			 serv_addr.sin_addr.s_addr = INADDR_ANY;
			 serv_addr.sin_port = htons(portno);
			 if (bind(m_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
				 throw 1;
			 listen(m_sockfd,5);
			 clilen = sizeof(cli_addr);
		}
		catch (int ErrorCode)
		{
			const char *ErrorMsg=NULL;
			switch (ErrorCode)
			{
			case 0:
				ErrorMsg="ERROR opening socket";
				break;
			case 1:
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
	     close(m_sockfd);
	}
	void TimeChange(double dTime_s)
	{
		if (m_Error) return;
	     char buffer[256];
	     bzero(buffer,256);
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
				DWORD sync=_byteswap_ulong(ptr[0] );
				long XInt=(long)_byteswap_ulong(ptr[1]);
				long YInt=(long)_byteswap_ulong(ptr[2]);
				long checksum=(long)_byteswap_ulong(ptr[3]);
				if (sync==0xabacab)
				{
					if (checksum==XInt+YInt)
					{
						m_Updated=true;
						m_Xpos=(double)XInt / 10000000.0;
						m_Ypos=(double)YInt / 10000000.0;
						//printf("New coordinates %f , %f\n",m_Xpos,m_Ypos);
						SmartDashboard::PutNumber("X Position",m_Xpos);
						SmartDashboard::PutNumber("Y Position",m_Ypos);
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
