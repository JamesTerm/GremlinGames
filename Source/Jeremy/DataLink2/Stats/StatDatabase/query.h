#pragma once

class connection;
class database;

// class query
// A class for managing a database query.
class query
{
public:
	// Constructor / destructor
	query(connection *c = NULL, const wchar_t *key_name = L"", const wchar_t *command = L"", DWORD timeout_ms = 0, bool is_multiple_keys = false);
	virtual ~query();

	DWORD get_next_change_time() const;
	unsigned long execute(database *db, DWORD current_time, bool suspend_if_no_connection);

	// A functor for comparing two query objects.
	typedef struct query_cmp
	{
		bool operator() ( const query& dbq1, const query& dbq2 ) const
		{
			return dbq1.m_next_change_time_ms < dbq2.m_next_change_time_ms;
		}
	};

protected:
	connection *m_connection;
	std::wstring m_key_name;
	std::wstring m_command;
	DWORD m_timeout_ms;
	DWORD m_next_change_time_ms;
	bool m_is_multiple_keys;
};
