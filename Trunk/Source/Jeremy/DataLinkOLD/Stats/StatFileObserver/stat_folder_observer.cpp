#include "stdafx.h"
#include "stat_folder_observer.h"

using namespace std;

// Constants
const wchar_t *stat_folder_observer::INPUT_FOLDER_NAME = L"Text Input\\";
const wchar_t *stat_folder_observer::TEXT_FILE_EXTENSION = L".txt";
const wchar_t *stat_folder_observer::XML_FILE_EXTENSION = L".xml";

// Constructor
stat_folder_observer::stat_folder_observer( const wchar_t *library_file_name, short additional_locks, short additional_threads ):
	stat_container(library_file_name, additional_locks, additional_threads),
	folder_observer(false),
	node()
{
	m_extensions.add_file_extension(TEXT_FILE_EXTENSION);
	m_extensions.add_file_extension(XML_FILE_EXTENSION);
}

// Destructor
stat_folder_observer::~stat_folder_observer( void )
{
}

// get_singleton
// Returns the singleton object
stat_folder_observer &stat_folder_observer::get_singleton()
{	static stat_folder_observer ret;
	return ret;
}

// start_updates
// Called by the API to begin stats updates.
HANDLE stat_folder_observer::start_updates( void )
{
	STAT_AUTO_SYNC_START();

	std::wstring folder_path(L"");

	if (!start_observing(folder_path.append(m_module_folder_path).append(INPUT_FOLDER_NAME).c_str(), true,
		FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, true))
		close_event_handle(m_event_handle);

	return m_event_handle;
}

// stop_updates
// Called by the API to end stats updates.
void stat_folder_observer::stop_updates( HANDLE handle )
{
	STAT_AUTO_SYNC_STOP();

	stop_observing();
	clear_containers();
}

// traverse
// Overrides traverse in folder_traverser. Traverses folder for all key-value pairs.
void stat_folder_observer::traverse(const std::wstring &folder_path, bool traverse_subtree)
{
	reset_num_changed_pairs();
	folder_observer::traverse(folder_path, traverse_subtree);
	changes_complete(m_num_changed_pairs);
}

// file_operation
// Overrides file_operation in folder_traverser. Determines whether the file is an XML file or a text file and
// calls the appropriate function. If the file can't be opened, keeps trying periodically until it times out.
void stat_folder_observer::file_operation(const wstring &file_path, const wstring &file_extension)
{
	// Open the file
	HANDLE file_handle;
	DWORD end_wait_time;
	ULONGLONG last_write_time = 0;

	end_wait_time = ::GetTickCount() + MAX_TOTAL_FILE_WAIT_TIME_MS;

	while ((file_handle = ::CreateFileW(file_path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		(file_extension == XML_FILE_EXTENSION)? FILE_FLAG_SEQUENTIAL_SCAN: 0, NULL)) == INVALID_HANDLE_VALUE)
	{
		if (::GetTickCount() >= end_wait_time || ::WaitForSingleObject(m_thread_handle, FILE_WAIT_TIME_MS) == WAIT_OBJECT_0)
			return;
	}

	if (::GetFileTime(file_handle, NULL, NULL, (LPFILETIME)&last_write_time) == TRUE && is_latest_file_time(file_path.c_str(), last_write_time))
	{
		if (file_extension == TEXT_FILE_EXTENSION)
			read_text_file(file_handle);
		else
		{
			m_xml_file_state = NO_KEY_VALUE_TAGS;
			read_xml_file(file_handle);
		}

		set_file_time_if_latest(file_path.c_str(), last_write_time);
	}

	// Close the file
	::CloseHandle(file_handle);
}

// p_create_child
// Overrides p_create_child in FrameWork::xml::node. Called when a child node is created.
node *stat_folder_observer::p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters )
{
	switch (m_xml_file_state)
	{
	case NO_KEY_VALUE_TAGS:
		m_xml_root_element.assign(type);
		m_xml_file_state = ROOT_TAG;
		return this;
	case ROOT_TAG:
		m_key_from_xml.assign(type);
		m_value_from_xml.clear();
		m_xml_file_state = KEY_VALUE_TAG;
		return this;
	case KEY_VALUE_TAG:
		break;
	default:
		break;
	}

	return NULL;
}

// create_child_end
// Overrides create_child_end in FrameWork::xml::node. Called when a child node is finished being created.
void stat_folder_observer::create_child_end( const wchar_t type[], node *p_child )
{
	switch (m_xml_file_state)
	{
	case NO_KEY_VALUE_TAGS:
		break;
	case ROOT_TAG:
		if (type == m_xml_root_element)
			m_xml_file_state = NO_KEY_VALUE_TAGS;
		break;
	case KEY_VALUE_TAG:
		if (type == m_key_from_xml)
		{
			set_pair(m_key_from_xml.c_str(), m_value_from_xml.c_str());
			m_xml_file_state = ROOT_TAG;
		}
		break;
	default:
		break;
	}
}

// add_text
// Overrides add_text in FrameWork::xml::node. Called when text is added to an XML tag's value.
void stat_folder_observer::add_text( const wchar_t text[], const int no_chars )
{
	if (m_xml_file_state == KEY_VALUE_TAG)
		m_value_from_xml.append(text, no_chars).c_str();
}

// read_xml_file
// Reads an xml file.
void stat_folder_observer::read_xml_file(HANDLE file_handle)
{
	// Create a parser
	parser	local_parser( this );

	// The block file to read from the file
	const DWORD block_size = 4096;

	// Read from the file
	while( true )
	{	// A temporary block
		char	block[ block_size ];

		// The size to read
		DWORD	read_size = block_size;
		::ReadFile( file_handle, block, block_size, &read_size, NULL );		

		// Add it to the parser
		local_parser( block, read_size );

		// Finished ?
		if ( read_size != block_size ) break;
	}
}

// read_text_file
// Reads a text file.
void stat_folder_observer::read_text_file(HANDLE file_handle)
{
	DWORD file_size = ::GetFileSize(file_handle, NULL);
	char *char_file_buffer;
	wchar_t *file_buffer;
	DWORD bytes_read = 0;

	if (file_size > 0)
	{
		char_file_buffer = new char[file_size + 1];
		file_buffer = new wchar_t[file_size + 1];

		if (::ReadFile(file_handle, char_file_buffer, file_size, &bytes_read, NULL) == TRUE && bytes_read == file_size)
		{	char_file_buffer[ file_size ] = 0;	// Terminate the string
			MultiByteToWideChar(CP_ACP,0,char_file_buffer,-1,file_buffer,file_size+1);
//			char *char_file_buffer = (char*)file_buffer;
//			for (DWORD i = bytes_read - 1; i != (DWORD)-1; i--)
//				file_buffer[i] = (wchar_t)char_file_buffer[i];
//			file_buffer[bytes_read] = NULL_CHAR;
			parse_text_file_buffer(file_buffer);
		}

		delete[] file_buffer;
		delete[] char_file_buffer;
	}		
}

// parse_text_file_buffer
// Parses a text file buffer and adds key-value pairs to the dictionary.
void stat_folder_observer::parse_text_file_buffer(const wchar_t *file_buffer)
{
	wstring key(L""), value(L"");
	wchar_t *i, *begin_key, *end_key, *begin_value, *end_value;

	for (i = (wchar_t *)file_buffer; *i != NULL_CHAR;)
	{
		for (; iswspace(*i); i++);

		if (*i != NULL_CHAR)
		{
			end_key = begin_key = i;

			for (; *i != NULL_CHAR && *i != NEW_LINE_CHAR && *i != VALUE_CHAR;)
			{
				if (!iswspace(*(i++)))
					end_key = i;
			}

			if (*i == VALUE_CHAR)
			{
				for (i++; *i != NEW_LINE_CHAR && iswspace(*i); i++);

				end_value = begin_value = i;

				for (; *i != NULL_CHAR && *i != NEW_LINE_CHAR;)
				{
					if (!iswspace(*(i++)))
						end_value = i;
				}

				key.clear();
				for (wchar_t *j = begin_key; j < end_key; j++)
				{
					if (*j != TAB_CHAR)
						key.push_back(*j);
				}

				value.assign(begin_value, end_value - begin_value);

				set_pair(key.c_str(), value.c_str());
			}
		}
	}
}
