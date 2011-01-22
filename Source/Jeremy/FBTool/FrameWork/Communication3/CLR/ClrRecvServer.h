#pragma once

ref class CommunicationsServerInterface;

public ref class Server
{
public:

	Server( System::String ^ application_name,				// The name of the application.
			   CommunicationsServerInterface		^ ci);	// The interface that receives frames and xml.

	//Destructor
	~Server(void);

	//Finalizer (when the object is released & not explicitly disposed)
	!Server(void);

	property wchar_t * Name
	{
		wchar_t * get();
	}

	property int QueueDepth
	{
		int get();
	}

private:
	::unmanaged_FC3_xml::receive * m_pUnmanagedServer;
};


struct unmanagedXmlInterface : ::unmanaged_FC3_xml::receive::client
{
	// Constructor
	unmanagedXmlInterface(CommunicationsServerInterface ^ ci);

	virtual void deliver_xml( const unmanaged_FC3_xml::message *p_msg );

	gcroot<CommunicationsServerInterface^> m_ci;
};

public ref class CommunicationsServerInterface abstract
{
public:
	CommunicationsServerInterface();
	~CommunicationsServerInterface();
	!CommunicationsServerInterface();

	virtual void ProcessXml( FrameWork::Communication3::CLR::Xml ^ p_msg ) { assert( false ); }

	unmanagedXmlInterface * m_uxi;
};