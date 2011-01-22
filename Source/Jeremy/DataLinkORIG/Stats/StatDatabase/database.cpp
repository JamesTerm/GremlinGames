#include "stdafx.h"
#include "database.h"

using namespace Frameworks;

// Constants
const wchar_t *database::INPUT_FOLDER_NAME = L"Database Input\\";
const wchar_t *database::INPUT_FILE_EXTENSION = L".xml";
const wchar_t * database::cwsz_Path = L"Software\\NewTek\\DataLink\\Active";
const wchar_t * database::cwsz_QueryPath = L"Software\\NewTek\\DataLink\\Active\\Query";

// Constructor
database::database( const wchar_t *library_file_name, short additional_locks, short additional_threads ):
	stat_container(library_file_name, additional_locks, 1 + additional_threads),
	folder_traverser(false),
	node(),
	m_thread_handle(NULL)
{	std::wstring ext( INPUT_FILE_EXTENSION );
	m_extensions.add_file_extension( ext );
}

// Destructor
database::~database()
{
	std::list<connection*>::iterator iterConn;
	for (iterConn = m_connections.begin(); iterConn != m_connections.end(); iterConn++)
		delete *iterConn;
	m_connections.clear();
}

// TESTHR
// Calls FAILED, and if the return value is true, issues a com error.
void database::TESTHR(HRESULT hr)
{
	if (FAILED(hr))
		_com_issue_error(hr);
}

// get_singleton
// Returns the singleton object
database &database::get_singleton()
{	static database ret;
	return ret;
}

// start_updates
// Called by the API to begin stats updates.
HANDLE database::start_updates()
{
	STAT_AUTO_SYNC_START();

	if (connect_to_databases())
		m_stat_base_sync.THREAD_START(0, &m_thread_handle, this);
	else
		close_event_handle(m_event_handle);

	return m_event_handle;
}

// stop_updates
// Called by the API to end stats updates.
void database::stop_updates( HANDLE handle )
{
	STAT_AUTO_SYNC_STOP();

	m_stat_base_sync.THREAD_STOP(0);
	disconnect_from_databases();
	clear_containers();
}

// connect_to_databases
// Initializes COM and calls traverse.
bool database::connect_to_databases()
{
	std::wstring folder_path(L"");
	
	const wchar_t * val = L"";
	wchar_t * user;
	wchar_t * password;
	wchar_t * constring = L"";
	wchar_t * path = L"";
		
	wchar_t * tempNum = L"";
	size_t numEntries = 0;

	//constring=(driver.append(REG_getRegValue(L"Driver",)))

	if (FAILED(CoInitialize(NULL)))
		return false;
	
	//NEW: Call REG_connect_to_databases with reg values.
	// Get database connect info and number of queries.
	
	// REG_connect_to_databases();
	//TESTING:
	constring = (wchar_t*)(_alloca(sizeof(wchar_t) * (1024)));
	path = (wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(cwsz_Path) + wcslen(L"\\Query") + 1)));
	tempNum = (wchar_t*)(_alloca(sizeof(wchar_t) * 7));
	wcscpy(path, cwsz_Path);
	
	wcscpy(constring, L"DRIVER=");
	val = REG_getRegValue(path, L"Driver", val);	wcscat(constring, val); wcscat(constring, L"; SERVER=");
	val = REG_getRegValue(path, L"Server", val);	wcscat(constring, val); wcscat(constring, L"; DATABASE=");
	val = REG_getRegValue(path, L"Database", val);	wcscat(constring, val); wcscat(constring, L"; Uid=");
	val = REG_getRegValue(path, L"UserID", val);	wcscat(constring, val); wcscat(constring, L"; Pwd=");
	user=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(val) + 1)));			wcscpy(user, val);
	val = REG_getRegValue(path, L"Password", val);	wcscat(constring, val);	wcscat(constring, L"; OPTION=3");
	password=(wchar_t*)(_alloca(sizeof(wchar_t) * (wcslen(val) + 1)));		wcscpy(password, val);
	REG_connect_to_databases(constring, user, password);

	wcscat(path, L"\\Query");

//TODO: change the query gathering to match new registry storage implementation.

	REG_assembleQueries();

// 	val = REG_getRegValue(path, L"NumEntries", val);
// 	numEntries=_wtoi(val);
// 	
// 	for (size_t i = 1; i<=numEntries; i++)
// 	{
// 		tempNum = _itow(i, tempNum, 10);
// 		val = REG_getRegValue(path, tempNum, val);	
// 	}
	
	traverse(folder_path.append(m_module_folder_path).append(INPUT_FOLDER_NAME), true);

	return true;
}

	   ////////////////////////////////////////////
	  //		New registry-based database		//
///////////////////////////////////////////////////////////////////////////////////////////
bool database::REG_connect_to_databases(const wchar_t * conString, const wchar_t * user, const wchar_t * password)
{
	bool ret = true;

	if (FAILED(CoInitialize(NULL)))
		ret =false;
	else
	{
		connection * c = new connection(conString, user, password);
		m_connections.push_back(c);
		ret = true;
	}

	return ret;
}

// Get insert query from registry values.
bool database::REG_getQueries(const wchar_t * key_name, const wchar_t * command, DWORD timeout_ms, bool is_multiple_keys)
{
	bool ret = false;
	query q(*(m_connections.rbegin()), key_name, command, timeout_ms, is_multiple_keys);
	m_queries.insert(q);
	ret = true;

	return ret;
}

// Assemble a query (command, key, timeout, and multiple entries flag) by looping through enumerated registry keys.
bool database::REG_assembleQueries()
{
	wchar_t * path = 0;

	//
	HKEY hKey;
	DWORD retCode;

	retCode=RegOpenKeyEx(	HKEY_CURRENT_USER,
							cwsz_QueryPath,
							0,
							KEY_READ,
							&hKey);
	if (retCode == ERROR_SUCCESS)
	{
		TCHAR		achKey[255];					// buffer for subkey name
		DWORD		cbName;							// size of name string
		TCHAR		achClass[MAX_PATH] = TEXT("");	// buffer for class name
		DWORD		cchClassName = MAX_PATH;		// size of class string
		DWORD		cSubKeys=0;						// number of subkeys
		DWORD		cbMaxSubKey;					// longest subkey size
		DWORD		cchMaxClass;					// longest class string
		DWORD		cValues;						// number of values for key
		DWORD		cchMaxValue;					// longest value name
		DWORD		cbMaxValueData;					// longest value data
		DWORD		cbSecurityDescriptor;			// size of security descriptor
		FILETIME	ftLastWriteTime;				// last write time

//		TCHAR achValue[MAX_VALUE_NAME];
		DWORD cchValue = 16383;

		// Get the class name and the value count. 
		retCode = RegQueryInfoKey(
			hKey,                    // key handle
			achClass,                // buffer for class name
			&cchClassName,           // size of class string
			NULL,                    // reserved
			&cSubKeys,               // number of subkeys
			&cbMaxSubKey,            // longest subkey size
			&cchMaxClass,            // longest class string
			&cValues,                // number of values for this key
			&cchMaxValue,            // longest value name
			&cbMaxValueData,         // longest value data
			&cbSecurityDescriptor,   // security descriptor
			&ftLastWriteTime);       // last write time

		if (cSubKeys)
		{
			for (size_t i=0; i<cSubKeys; i++)
			{ 
				cbName = 255;
				retCode = RegEnumKeyEx(	hKey, i,
										achKey,
										&cbName,
										NULL,
										NULL,
										NULL,
										&ftLastWriteTime);
				if (retCode == ERROR_SUCCESS)
				{
					//Assemble queries
					const wchar_t * command = 0; wchar_t * _command = 0;
					const wchar_t * timeout = 0; DWORD _timeout = 0;
					const wchar_t * multi = 0; bool _multi = false;
					const wchar_t * tempVal = 0;

					path=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(cwsz_Path)+wcslen(L"\\Query\\")+wcslen(achKey)+1)));
					wcscpy(path, cwsz_Path);
					wcscat(path, L"\\Query\\");
					wcscat(path, achKey);
					tempVal=(wchar_t*)(_alloca(sizeof(wchar_t)*8));

					command = REG_getRegValue(path, L"Command", command);
					_command=(wchar_t*)(_alloca(sizeof(wchar_t)*(wcslen(command)+1)));
					wcscpy(_command, command);
					tempVal = REG_getRegValue(cwsz_Path, L"Timeout", tempVal);
					_timeout = _wtoi(tempVal);
					if (REG_getRegValueN(path, L"Multi") !=0)
						_multi = true;

					// Insert the query values into the query object.
					REG_getQueries(achKey, _command, _timeout, _multi);
				}
			}
		}
	}
	return true;
}

// Close out the connection, clear containers, and un-initialize com.
void database::REG_disconnect_from_databases()
{
	m_queries.clear();
	m_connections.clear();

	CoUninitialize();
}

// Get a value from the registry (non-managed).
const wchar_t * database::REG_getRegValue(	const wchar_t * path,
											const wchar_t * regValue,
											const wchar_t * retVal)
{
	HKEY hKey;
	DWORD retCode = 0;
	const TCHAR value[MAX_ENTRY_VALUE] = L"\0";
	DWORD BufferSize = MAX_ENTRY_VALUE;

	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							path,
							NULL,
							KEY_READ,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{
		retCode = (::RegQueryValueEx(	hKey,
										regValue,
										NULL,
										NULL,
										(LPBYTE)&value,
										&BufferSize));
		
	}
// 	else
// 		assert(false);
	
	retVal = value;
	RegCloseKey(hKey);

	return retVal;
}

// Get DWORD from registry.
DWORD database::REG_getRegValueN(	const wchar_t * path,
									const wchar_t * regValue)
{
	HKEY hKey;
	DWORD retCode = 0;
	int value = 0;
	DWORD BufferSize = 256;

	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							path,
							NULL,
							KEY_READ,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{
		retCode = (::RegQueryValueEx(	hKey,
										regValue,
										NULL,
										NULL,
										(LPBYTE)&value,
										&BufferSize));

	}
	else
		//TODO: more error handling?
		assert(false);

	RegCloseKey(hKey);
	return value;
}



///////////////////////////////////////////////////////////////////////////////////////////
	 //	END:	New registry-based database		//
	/////////////////////////////////////////////

// disconnect_from_databases
// Clears query and connection containers and uninitializes COM.
void database::disconnect_from_databases()
{
	m_queries.clear();
	m_connections.clear();

	CoUninitialize();
}

// operator()
// Overrides operator() in stat_base. Called repeatedly between calls to THREAD_START
// and THREAD_STOP.
void database::operator()( const void* )
{
	if (!m_stat_base_sync.THREAD_CHECK(0, 5))
		return;

	query_multiset::iterator first_query;
	DWORD current_time, next_change_time, wait_time;

	if (!m_queries.empty() && (current_time = ::GetTickCount()) >= (first_query = m_queries.begin())->get_next_change_time())
	{
		reset_num_changed_pairs();
		first_query->execute(this, current_time, true);
		resort_query(first_query);
		changes_complete(m_num_changed_pairs);
	}

	if (m_queries.empty())
		wait_time = DEFAULT_THREAD_WAIT_TIME_MS;
	else if ((current_time = ::GetTickCount()) < (next_change_time = m_queries.begin()->get_next_change_time()))
		wait_time = next_change_time - current_time;
	else
		wait_time = 0;

	if (wait_time > 0)
		::WaitForSingleObject(m_thread_handle, wait_time);
}

// resort_query
// Erases a query from the multiset and inserts it to the same multiset.
void database::resort_query(query_multiset::iterator &i)
{
	query q;

	if (m_queries.size() > 1)
	{
		q = *i;
		i = m_queries.erase(i);
		m_queries.insert(q);
	}
}

// file_operation
// Overrides file_operation in folder_traverser. Initializes the XML file state and calls load_from_file.
void database::file_operation(const std::wstring &file_path, const std::wstring &file_extension)
{
	m_xml_file_state = NO_DATABASE_TAGS;
	load_from_file(file_path.c_str(), this);
}

// p_create_child
// Overrides p_create_child in FrameWork::xml::node. Called when a child node is created.
node *database::p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters )
{
	int connection_string_index = -1, command_index = -1, key_index = -1, timeout_index = -1, multiple_keys_index = -1;

	switch (m_xml_file_state)
	{
	case NO_DATABASE_TAGS:
		if (wcscmp(type, L"sql_databases") == 0)
		{
			m_xml_file_state = SQL_DATABASES_TAG;
			return this;
		}
		break;
	case SQL_DATABASES_TAG:
		if (wcscmp(type, L"database") == 0 && no_parameters >= 1)
		{
			for (int i = 0; i < no_parameters; i++)
			{
				if (wcscmp(p_parameters[i].m_p_name, L"connection_string") == 0)
					connection_string_index = i;
			}

			if (connection_string_index >= 0)
			{
				connection * c = new connection(p_parameters[connection_string_index].m_p_value, L"", L"");
				m_connections.push_back(c);
				m_xml_file_state = DATABASE_TAG;
				return this;
			}
		}
		break;
	case DATABASE_TAG:
		if (wcscmp(type, L"query") == 0 && no_parameters >= 2)
		{
			for (int i = 0; i < no_parameters; i++)
			{
				if (wcscmp(p_parameters[i].m_p_name, L"command") == 0)
					command_index = i;
				else if (wcscmp(p_parameters[i].m_p_name, L"key") == 0)
					key_index = i;
				else if (wcscmp(p_parameters[i].m_p_name, L"timeout") == 0)
					timeout_index = i;
				else if (wcscmp(p_parameters[i].m_p_name, L"multiple_keys") == 0)
					multiple_keys_index = i;
			}

			if (command_index >= 0 && key_index >= 0)
			{
				query q(*(m_connections.rbegin()), p_parameters[key_index].m_p_value, p_parameters[command_index].m_p_value,
					(timeout_index >= 0)? _wtoi(p_parameters[timeout_index].m_p_value): DEFAULT_COMMAND_TIMEOUT_MS,
					(multiple_keys_index >= 0 && wcscmp(p_parameters[multiple_keys_index].m_p_value, L"true") == 0));
				m_queries.insert(q);
			}
		}
		break;
	default:
		break;
	}

	return NULL;
}

// create_child_end
// Overrides create_child_end in FrameWork::xml::node. Called when a child node is finished being created.
void database::create_child_end( const wchar_t type[], node *p_child )
{
	switch (m_xml_file_state)
	{
	case NO_DATABASE_TAGS:
		break;
	case SQL_DATABASES_TAG:
		if (wcscmp(type, L"sql_databases") == 0)
			m_xml_file_state = NO_DATABASE_TAGS;
		break;
	case DATABASE_TAG:
		if (wcscmp(type, L"database") == 0)
			m_xml_file_state = SQL_DATABASES_TAG;
		break;
	default:
		break;
	}
}
