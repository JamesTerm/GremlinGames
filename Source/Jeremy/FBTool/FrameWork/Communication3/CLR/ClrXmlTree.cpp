#include "StdAfx.h"
#include "FrameWork_Communication3_CLR.h"

namespace FrameWork { namespace Communication3 { namespace CLR {

XmlTree::XmlTree( const char xml_data[] )
: m_ownsUnmanagedXmlTree(true)
{
	m_pUnmanagedXmlTree = new unmanaged_xml::tree(xml_data);
}

XmlTree::XmlTree( const wchar_t xml_data[] )
: m_ownsUnmanagedXmlTree(true)
{
	m_pUnmanagedXmlTree = new unmanaged_xml::tree(xml_data);
}

XmlTree::XmlTree( unmanaged_xml::tree * ut )
: m_ownsUnmanagedXmlTree(false)
{
	m_pUnmanagedXmlTree = (unmanaged_xml::tree *)ut;
}

XmlTree::XmlTree( System::String ^ xml_data )
: m_ownsUnmanagedXmlTree(true)
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(xml_data));
	m_pUnmanagedXmlTree = new unmanaged_xml::tree(xstr);
}

XmlTree::~XmlTree( void )
{
	this->!XmlTree();
}

XmlTree::!XmlTree( void )
{
	if (m_ownsUnmanagedXmlTree)
		delete m_pUnmanagedXmlTree;
}

unmanaged_xml::tree * XmlTree::GetUnmanagedPointer()
{
	return m_pUnmanagedXmlTree;
}

void XmlTree::Clear()
{
	m_pUnmanagedXmlTree->clear();
}

bool XmlTree::ReadFromFile(System::String ^ filename )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(filename));
	bool Ret = m_pUnmanagedXmlTree->read_from_file(xstr);
	return Ret;
}

bool XmlTree::ReadFromString( System::String ^ text )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(text));
	bool Ret = m_pUnmanagedXmlTree->read_from_string(xstr);
	return Ret;
}

System::String ^ XmlTree::Type( void )
{
	return gcnew System::String(m_pUnmanagedXmlTree->type());
}

int XmlTree::NoParameters( void )
{
	return m_pUnmanagedXmlTree->no_parameters();
}

System::String ^ XmlTree::ParameterKey( const int idx )
{
	return gcnew System::String(m_pUnmanagedXmlTree->parameter(idx).first);
}

System::String ^ XmlTree::ParameterValue( const int idx )
{
	return gcnew System::String(m_pUnmanagedXmlTree->parameter(idx).second);
}

System::String ^ XmlTree::ParameterValue( System::String ^ key )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(key));
	System::String ^ Ret = gcnew System::String(m_pUnmanagedXmlTree->parameter(xstr));
	return Ret;
}

System::String ^ XmlTree::Text( void )
{
	return gcnew System::String(m_pUnmanagedXmlTree->text());
}

int XmlTree::NoChildren( void )
{
	return m_pUnmanagedXmlTree->no_children();
}

XmlTree ^ XmlTree::GetChild( const int idx )
{
	return gcnew XmlTree((unmanaged_xml::tree*) &m_pUnmanagedXmlTree->child(idx));
}

XmlTree ^ XmlTree::GetChild( System::String ^ name )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(name));
	XmlTree ^ xt = gcnew XmlTree((unmanaged_xml::tree*) m_pUnmanagedXmlTree->get_child(xstr));
	return xt;
}

XmlTree ^ XmlTree::GetParent( void )
{
	unmanaged_xml::tree* uxt =  m_pUnmanagedXmlTree->parent();
	if (uxt)
		return gcnew XmlTree(uxt);
	else
		return nullptr;
}

// 
// Set Data Methods
//

void XmlTree::SetType( System::String ^ name )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(name));
	m_pUnmanagedXmlTree->set_type(xstr);
}

void XmlTree::SetText( System::String ^ text )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(text));
	m_pUnmanagedXmlTree->set_text(xstr);
}
				
void XmlTree::RemoveAllParameters( void )
{
	m_pUnmanagedXmlTree->remove_all_parameters();
}

void  XmlTree::AddParameter(  System::String ^ name, System::String ^ value )
{
	pin_ptr<const wchar_t> xstr1(PtrToStringChars(name));
	pin_ptr<const wchar_t> xstr2(PtrToStringChars(value));
	m_pUnmanagedXmlTree->add_parameter(xstr1, xstr2);
}

void XmlTree::SetParameter( int idx, System::String ^ name, System::String ^ value )
{
	pin_ptr<const wchar_t> xstr1(PtrToStringChars(name));
	pin_ptr<const wchar_t> xstr2(PtrToStringChars(value));
	m_pUnmanagedXmlTree->set_parameter(idx, xstr1, xstr2);
}

void XmlTree::SetParameter( System::String ^ name, System::String ^ value )
{
	pin_ptr<const wchar_t> xstr1(PtrToStringChars(name));
	pin_ptr<const wchar_t> xstr2(PtrToStringChars(value));
	m_pUnmanagedXmlTree->set_parameter(xstr1, xstr2);
}

void XmlTree::RemoveAllChildren( void )
{
	m_pUnmanagedXmlTree->remove_all_children();
}

void XmlTree::AddChild( XmlTree ^ newChild )
{
	newChild->m_ownsUnmanagedXmlTree = false;
	m_pUnmanagedXmlTree->add_child(newChild->m_pUnmanagedXmlTree);
}

void XmlTree::SetChild( int idx, XmlTree ^ newChild )
{
	newChild->m_ownsUnmanagedXmlTree = false;
	m_pUnmanagedXmlTree->set_child(idx, newChild->m_pUnmanagedXmlTree);
}

System::String ^ XmlTree::GetOutput(void)
{
	int len = m_pUnmanagedXmlTree->output_length() + 1;
	wchar_t * outputWString = new wchar_t [len];
	m_pUnmanagedXmlTree->output(outputWString);
	System::String ^ Ret = gcnew System::String(outputWString);
	delete[] outputWString;
	return Ret;
}

}}}