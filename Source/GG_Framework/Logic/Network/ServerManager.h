// ServerManager.h
#pragma once

namespace GG_Framework
{
	namespace Logic
	{
		namespace Network
		{
			//! The Server Manager is not a server itself, but it manages one or two
			//! other Servers.  Override this class, particularly ShouldRebroadcastMessage,
			//! to handle which messages should be re-broadcast and which should be fired as
			//! an event.
			class FRAMEWORK_LOGIC_NETWORK_API ServerManager
			{
			public:
				//! Both server1 and server2 MUST also cast to IPacketReceiver
				ServerManager(IServer* server1, IServer* server2);
				virtual ~ServerManager(){}

				IServer* GetServer1(){return m_server1;}
				IServer* GetServer2(){return m_server2;}

				// This is the key server function in the loop that passes the messages to all
				// clients except for the one that sent it.  It will be the Key Game Loop
				// For the Server itself
				virtual void ProcessMessages();

			protected:

				// This message will help us know how to process the in-coming message.
				// Some messages should perhaps not be sent at all, but should be processed in 
				// an event.  Return true to simply rebroadcast.  Subclasses can fire events
				// if they want to before returning from this function.
				virtual bool ShouldRebroadcastMessage(Packet* packet,
					PacketPriority& priority, PacketReliability& reliability, char& orderingChannel, std::set<PlayerID>*& ignorePlayers);

			private:
				IServer* const m_server1;
				IServer* const m_server2;

				void ProcessMessages(IPacketReceiver* fromRecv);
			};
		}
	}
}
