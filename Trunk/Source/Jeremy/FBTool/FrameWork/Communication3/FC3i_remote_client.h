#pragma once

struct FRAMEWORKCOMMUNICATION3_API client
{			// Constructor
			client( const wchar_t* p_server, const int port_no = FrameWork::Communication3::config::remote_port_number );

			// Destructor
			~client( void );

			// Was there an error
			const bool error( void ) const;

			// Send a message of a given type
			const bool send( const wchar_t* p_dst_name, const void* p_data, const DWORD data_size );

			// Get the client name
			const wchar_t* server_name( void ) const;

private:	// The socket
			socket m_socket;

			// The server name
			wchar_t* m_p_server_name;

			// We need critical sections around sending
			critical_section m_lock;
};