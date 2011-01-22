#include "stdafx.h"
#include "connection.h"
#include "database.h"

// Constructor
connection::connection(const wchar_t *connection_string, const wchar_t *username, const wchar_t *password):
m_connection_string(connection_string), m_username(username), m_password(password), m_connection(NULL)
{
}

// Destructor
connection::~connection()
{
	close();
}

// open
// Opens a database connection.
const ADODB::_ConnectionPtr *connection::open()
{
	if (m_connection == NULL)
	{
		try
		{
			database::TESTHR(m_connection.CreateInstance(__uuidof(ADODB::Connection)));
			m_connection->CursorLocation = ADODB::adUseClient;
			database::TESTHR(m_connection->Open(m_connection_string.c_str(), m_username.c_str(), m_password.c_str(), ADODB::adConnectUnspecified));
		}
		catch (_com_error e)
		{
			close();
		}
	}

	return get_connection();
}

// close
// Closes a database connection.
void connection::close()
{
}

// get_connection
// Returns the connection object.
const ADODB::_ConnectionPtr *connection::get_connection() const
{
	return &m_connection;
}
