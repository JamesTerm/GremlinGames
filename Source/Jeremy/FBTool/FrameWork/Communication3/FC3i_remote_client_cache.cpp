#include "StdAfx.h"
#include "FrameWork.Communication3.h"

namespace FC3i  = FrameWork::Communication3::implementation;
namespace FC3ir = FrameWork::Communication3::implementation::remote;

using namespace FC3ir;

struct global_client_cache : public client_cache {};
static global_client_cache g_global_client_cache;

client_cache& client_cache::get_cache( void )
{	return g_global_client_cache;
}

// Constructor
client_cache::client_cache( void )
{
}

// Destructor
client_cache::~client_cache( void )
{	// Destroy all clients
	while( !m_p_clients.empty() )
	{	delete m_p_clients.back();
		m_p_clients.pop_back();
	}
}

// Send a message to a client
const bool client_cache::send( const wchar_t *p_client_name, const wchar_t* p_dst_name, const void* p_data, const DWORD data_size )
{	// We lock for reading
	{	read_auto_lock	rd( m_clients_lock );

		// Cycle over all items
		for( std::vector< client* >::iterator i = m_p_clients.begin(); i != m_p_clients.end(); i++ )
		if ( !::wcsicmp( p_client_name, (*i)->server_name() ) )
		{	// If this item has an error, we need to skip
			if ( (*i)->error() ) break;
			
			// Send using this item
			return (*i)->send( p_dst_name, p_data, data_size );
		}
	}

	// We lock for writing
	{	write_auto_lock	wt( m_clients_lock );

		// Cycle over all items
		for( std::vector< client* >::iterator i = m_p_clients.begin(); i != m_p_clients.end(); i++ )
		if ( !::wcsicmp( p_client_name, (*i)->server_name() ) )
		{	// If this item has an error, then we need to remove it from the list
			if ( (*i)->error() )
			{	// We delete this item and remove it from the list	
				m_p_clients.erase( i );
				break;
			}
			
			// Send using this item
			return (*i)->send( p_dst_name, p_data, data_size );
		}

		// We create a new item
		client* p_new_client = new client( p_client_name );
		if ( p_new_client->error() ) 
		{	delete p_new_client;
			return false;
		}
		
		// Add this item to the list and send
		m_p_clients.push_back( p_new_client );
		return p_new_client->send( p_dst_name, p_data, data_size );
	}
}