#include "StdAfx.h"
#include "FrameWork_Communication3_CLR.h"

namespace FrameWork { namespace Communication3 { namespace CLR {


Xml::Xml( const char xml_data[] )
: m_ownsUnmanagedXml(true)
{
	m_pUnmanagedXml = new unmanaged_FC3_xml::message(xml_data);
}

Xml::Xml( const wchar_t xml_data[] )
: m_ownsUnmanagedXml(true)
{
	m_pUnmanagedXml = new unmanaged_FC3_xml::message(xml_data);
}

Xml::Xml( const size_t max_size )
: m_ownsUnmanagedXml(true)
{
	m_pUnmanagedXml = new unmanaged_FC3_xml::message((const DWORD)max_size);
}

Xml::Xml( const unmanaged_FC3_xml::message * uf )
: m_ownsUnmanagedXml(false)
{
	m_pUnmanagedXml = (unmanaged_FC3_xml::message *)uf;
}

Xml::Xml( System::String ^ xml_data )
: m_ownsUnmanagedXml(true)
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(xml_data));
	m_pUnmanagedXml = new unmanaged_FC3_xml::message(xstr);
}

Xml::~Xml()
{
	this->!Xml();
}

Xml::!Xml( void )
{
	if (m_ownsUnmanagedXml)
		delete m_pUnmanagedXml;
}

long Xml::AddRef( void )
{
	return m_pUnmanagedXml->addref();
}

long Xml::Release( void )
{
	return m_pUnmanagedXml->release();
}

bool Xml::Send( const System::String ^ dst, const bool async )
{
	pin_ptr<const wchar_t> dstr(PtrToStringChars(dst));
	return m_pUnmanagedXml->send(dstr);
}

int Xml::Broadcast( const bool async )
{
	//return m_pUnmanagedXml->broadcast(async);
	return 0;
}

const size_t Xml::Size( void )
{
//	return m_pUnmanagedXml->size();
	return 0;
}

int Xml::NoOutstandingPendingProcessings( void )
{
	//return m_pUnmanagedXml->get_no_outstanding_sends();
	return 0;
}

bool Xml::Parse(FrameWork::xml::node *p_node)
{
	return m_pUnmanagedXml->parse(p_node);
}

char* Xml::GetDataA( void )
{
	return (char *)(m_pUnmanagedXml);
}

wchar_t* Xml::GetDataW( void )
{
	return (wchar_t *)(m_pUnmanagedXml);
}

System::String ^ Xml::GetDataString( void )
{
//  Cast Operators don't seem to work in managed code?
//	wchar_t * twp = (wchar_t*)(m_pUnmanagedXml);
//	wchar_t * twp = m_pUnmanagedXml->get_wchar_ptr();
	const wchar_t * twp = m_pUnmanagedXml->operator const wchar_t * ();
	System::String ^ tstring = gcnew System::String(twp);
	return tstring;
}



}}} // closing namespace