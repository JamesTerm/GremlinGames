#include "stdafx.h"
#include "database.h"

// Constants
const wchar_t *database::INPUT_FOLDER_NAME = L"Database Input\\";
const wchar_t *database::INPUT_FILE_EXTENSION = L".xml";

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

	if (FAILED(CoInitialize(NULL)))
		return false;

	traverse(folder_path.append(m_module_folder_path).append(INPUT_FOLDER_NAME), true);

	return true;
}

// disconnect_from_databases
// Clears query and connection containers and unitializes COM.
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
					(timeout_index >= 0)? _wtoi64(p_parameters[timeout_index].m_p_value): DEFAULT_COMMAND_TIMEOUT_MS,
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
