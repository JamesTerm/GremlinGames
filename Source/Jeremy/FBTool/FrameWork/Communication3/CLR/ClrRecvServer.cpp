#include "StdAfx.h"
#include "FrameWork_Communication3_CLR.h"

namespace FrameWork { namespace Communication3 { namespace CLR {


Server::Server( System::String ^ application_name,			// The name of the application.
			   CommunicationsServerInterface		^ ci)	// The interface that receives frames and xml.
{
	pin_ptr<const wchar_t> ustr(PtrToStringChars(application_name));
	m_pUnmanagedServer = new ::unmanaged_FC3_xml::receive(ustr, ci->m_uxi);
}


//Destructor
Server::~Server()
{
	this->!Server();
}

//Finalizer
Server::!Server()
{
	if (m_pUnmanagedServer)
	{
		delete m_pUnmanagedServer;
		m_pUnmanagedServer = NULL;
	}
}


wchar_t * Server::Name::get( void )
{
	return (wchar_t *)(m_pUnmanagedServer->name());
}

int Server::QueueDepth::get( void )
{
	return m_pUnmanagedServer->queue_depth();
}

unmanagedXmlInterface::unmanagedXmlInterface(CommunicationsServerInterface ^ ci)
{
	m_ci = ci;
}

void unmanagedXmlInterface::deliver_xml( const unmanaged_FC3_xml::message *p_msg )
{
	Xml ^ tempxml = gcnew Xml(p_msg);
	m_ci->ProcessXml(tempxml);
}

CommunicationsServerInterface::CommunicationsServerInterface()
{
	m_uxi = new unmanagedXmlInterface(this);
}
CommunicationsServerInterface::~CommunicationsServerInterface()
{
	this->!CommunicationsServerInterface();
}

CommunicationsServerInterface::!CommunicationsServerInterface()
{
	delete m_uxi;
}

}}} // namespace