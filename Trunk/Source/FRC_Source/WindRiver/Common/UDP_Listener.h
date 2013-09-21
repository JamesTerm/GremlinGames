#pragma once

class UDP_Listener_Interface
{
	public:
		virtual ~UDP_Listener_Interface() {}  
		virtual void ProcessPacket(char *pkt,size_t pkt_size)=0;
};

class coodinate_manager_Interface : public UDP_Listener_Interface
{
	protected:
		double m_Xpos,m_Ypos;
		bool m_Updated; //true if we received a packet for this slice of time
	public:
		static coodinate_manager_Interface *CreateInstance();
		static void DestroyInstance(coodinate_manager_Interface *instance);
		void ResetUpdate() {m_Updated=false;}

		__inline double GetXpos() const {return m_Xpos;}
		__inline double GetYpos() const {return m_Ypos;}
		__inline bool IsUpdated() const {return m_Updated;}
};
