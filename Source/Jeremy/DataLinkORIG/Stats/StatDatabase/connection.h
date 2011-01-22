#pragma once

// class connection
// A class for managing a database connection.
class connection
{
public:
	// Constructor / destructor
	connection(const wchar_t *connection_string = L"", const wchar_t *username = L"", const wchar_t *password = L"");
	virtual ~connection();

	const ADODB::_ConnectionPtr *open();
	void close();
	const ADODB::_ConnectionPtr *get_connection() const;

protected:
	std::wstring m_connection_string;
	std::wstring m_username;
	std::wstring m_password;
	ADODB::_ConnectionPtr m_connection;
};
