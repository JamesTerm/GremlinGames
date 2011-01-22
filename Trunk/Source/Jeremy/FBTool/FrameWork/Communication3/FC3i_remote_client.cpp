#include "StdAfx.h"
#include "FrameWork.Communication3.h"

namespace FC3i  = FrameWork::Communication3::implementation;
namespace FC3ir = FrameWork::Communication3::implementation::remote;

using namespace FC3ir;

// Constructor
client::client( const wchar_t* p_server, const int port_no )
	: m_p_server_name( new wchar_t [ ::wcslen( p_server ) + 1 ] )
{	// Store the name
	::wcscpy( m_p_server_name, p_server );
	
	// Get the hosr name
	const size_t no_chars = ::wcslen( p_server )*2 + 1;
	char *p_serverA = new char [ no_chars ];
	::wcstombs( p_serverA, p_server, no_chars );
	hostent* p_host = ::gethostbyname( p_serverA );	

	// Try to connect to the server
	sockaddr_in sin = { 0 };
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = p_host ? ( ( (in_addr*)(p_host->h_addr) )->s_addr ) :  ::inet_addr( p_serverA );
	sin.sin_port = ::htons( port_no );

	// Delete the string
	delete [] p_serverA;

	// Connect to the port
	if ( !m_socket.connect( (sockaddr*)&sin, sizeof(sin) ) )
	{	// Could not connect to server
		m_socket.closesocket();
	}
}

// Destructor
client::~client( void )
{	// Close the socket
	m_socket.closesocket();

	// Free the memory
	delete [] m_p_server_name;
}

// Get the client name
const wchar_t* client::server_name( void ) const
{	// Get the server name
	return m_p_server_name;
}

// Was there an error
const bool client::error( void ) const
{	// If there is no socket
	return m_socket.error();
}

// Send a message of a given type
const bool client::send( const wchar_t* p_dst_name, const void* p_data, const DWORD data_size )
{	// We cannot have two threads in the same process using the same socket at the same time
	FC3::implementation::auto_lock	lock( m_lock );
	
	// Check for errors
	if ( m_socket.error() ) return false;

	// The destination name size
	const DWORD dst_size  = (DWORD)( ::wcslen( p_dst_name ) + 1 ) * sizeof( wchar_t );

	// Build the header packet.
	const FC3ir::tcpip_message_header hdr = {	FC3ir::tcpip_message_header::current_version, 
												FC3ir::tcpip_message_header::message_type_send,
												dst_size, 
												data_size };

	// Send the header
	if ( !m_socket.send( (char*)&hdr, sizeof(hdr), 0 ) ) goto error;

	// Send the destination
	if ( !m_socket.send( (char*)p_dst_name, dst_size, 0 ) ) goto error;

	// Send the data
	if ( !m_socket.send( (char*)p_data, data_size, 0 ) ) goto error;

	// Success
	return true;

error:
	// Close the socket
	m_socket.closesocket();

	// Error 
	return false;
}