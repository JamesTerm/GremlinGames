#pragma once

#include "..\StatCommon\Main.h"
#include "..\StatCommon\FrameWork.XML.h"

// class stat_folder_observer
// A class that inherits from stat_container and observes a folder for changes, which are added to the changed queue.
class stat_folder_observer : public stat_container, public folder_observer, public node
{
public:
	// Static member functions
	static stat_folder_observer &get_singleton();

	// Virtual functions called by the API
	virtual HANDLE start_updates( void );
	virtual void stop_updates( HANDLE handle );

protected:
	// Enumeration for the state of the XML file
	enum XML_FILE_STATE
	{
		NO_KEY_VALUE_TAGS = 0,
		ROOT_TAG = 1,
		KEY_VALUE_TAG = 2
	};

	// Constants
	static const wchar_t *INPUT_FOLDER_NAME;
	static const wchar_t *TEXT_FILE_EXTENSION;
	static const wchar_t *XML_FILE_EXTENSION;
	static const DWORD FILE_WAIT_TIME_MS = 10;
	static const DWORD MAX_TOTAL_FILE_WAIT_TIME_MS = 1000;
	static const wchar_t NULL_CHAR = L'\0';
	static const wchar_t NEW_LINE_CHAR = L'\n';
	static const wchar_t VALUE_CHAR = L'=';
	static const wchar_t TAB_CHAR = L'\t';

	// XML objects
	XML_FILE_STATE m_xml_file_state;
	std::wstring m_xml_root_element;
	std::wstring m_key_from_xml;
	std::wstring m_value_from_xml;

	// Constructor / destructor
	stat_folder_observer( const wchar_t *library_file_name = LIBRARY_FILE_NAME, short additional_locks = 0, short additional_threads = 0 );
	virtual ~stat_folder_observer( void );

	// folder_traverser overrides
	virtual void traverse(const std::wstring &folder_path, bool traverse_subtree);
	virtual void file_operation(const std::wstring &file_path, const std::wstring &file_extension);

	// FrameWork::xml::node overrides
	virtual node *p_create_child( const wchar_t type[], const int no_parameters, const parameter *p_parameters );
	virtual void create_child_end( const wchar_t type[], node *p_child );
	virtual void add_text( const wchar_t text[], const int no_chars );

	// File reading functions
	void read_text_file(HANDLE file_handle);
	void read_xml_file(HANDLE file_handle);
	void parse_text_file_buffer(const wchar_t *file_buffer);
};
