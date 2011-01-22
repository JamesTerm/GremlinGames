#pragma once

#include "..\StatCommon\Main.h"
#include "connection.h"
#include "query.h"

// class database
class database : public stat_container, public folder_traverser, public Frameworks::node
{
public:
	// Constants
	const static DWORD DEFAULT_COMMAND_TIMEOUT_MS = 1000;
	const static DWORD DEFAULT_THREAD_WAIT_TIME_MS = 60000;
	const static size_t MAX_ENTRY_NAME = 255;
	const static size_t MAX_ENTRY_VALUE = 16383;
	
	// Static member functions
	static void TESTHR(HRESULT hr);
	static database &get_singleton();

	// Virtual functions called by the API
	virtual HANDLE start_updates( void );
	virtual void stop_updates( HANDLE handle );

protected:
	// Typedefs for a list of connection objects and a multiset of query objects
	typedef std::list<connection*> connection_list;
	typedef std::multiset<query, query::query_cmp> query_multiset;

	// Enumeration for the state of the XML input file
	enum XML_FILE_STATE
	{
		NO_DATABASE_TAGS = 0,
		SQL_DATABASES_TAG = 1,
		DATABASE_TAG = 2
	};

	// Constants
	static const wchar_t *INPUT_FOLDER_NAME;
	static const wchar_t *INPUT_FILE_EXTENSION;
	static const wchar_t * cwsz_Path;
	static const wchar_t * cwsz_QueryPath;

	// Synchronization objects
	HANDLE m_thread_handle;

	XML_FILE_STATE m_xml_file_state;
	connection_list m_connections;
	query_multiset m_queries;

	// Constructor / destructor
	database( const wchar_t *library_file_name = LIBRARY_FILE_NAME, short additional_locks = 0, short additional_threads = 0 );
	virtual ~database();

	bool connect_to_databases();
	void disconnect_from_databases();
///////////////////
//	
	// New methods for connecting to database based upon registry values.
	bool REG_connect_to_databases(const wchar_t * conString = L"", const wchar_t * user = L"", const wchar_t * password = L"");
	void REG_disconnect_from_databases();
	bool REG_getQueries(const wchar_t * key_name, const wchar_t * command, DWORD timeout_ms, bool is_multiple_keys);
	bool REG_assembleQueries();
	const wchar_t * REG_getRegValue(const wchar_t * path,
									const wchar_t * regValue,
									const wchar_t * retVal);	// Returns retVal as result.
	DWORD REG_getRegValueN(const wchar_t * path,
							const wchar_t * regValue);
//
///////////////////
	void resort_query(query_multiset::iterator &i);

	// folder_traverser overrides
	virtual void file_operation(const std::wstring &file_path, const std::wstring &file_extension);

	// FrameWork::xml::node overrides
	virtual node *p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters );
	virtual void create_child_end( const wchar_t type[], node *p_child );

private:
	// Called by Thread
	void operator()( const void* );

	// Friend classes
	friend query;
	friend Thread<database>;
};
