#pragma once

#include "file_extension_list.h"

// class folder_traverser
// A generic class for traversing a folder.
class STATCOMMON_API folder_traverser
{
public:
	// Constructor / destructor
	folder_traverser(bool check_all_files = false);
	virtual ~folder_traverser();

	bool find_first_file(const std::wstring &folder_path, HANDLE &hFind, const LPWIN32_FIND_DATA p_find_data);
	bool find_next_file(const HANDLE hFind, const LPWIN32_FIND_DATA p_find_data);
	void find_close(const HANDLE hFind);
	bool is_dots(const std::wstring &folder_name) const;
	bool is_single_dot(const std::wstring &folder_name) const;
	bool is_double_dot(const std::wstring &folder_name) const;

	// Virtual traversal functions
	virtual void traverse(const std::wstring &folder_path, bool traverse_subtree);
	virtual bool continue_traversing();
	virtual bool begin_folder_operation(const std::wstring &folder_path, HANDLE &hFind, const LPWIN32_FIND_DATA p_find_data);
	virtual void end_folder_operation(const HANDLE hFind);
	virtual bool folder_check(const std::wstring &folder_name, bool &traverse_subtree, long &directory_level);
	virtual bool file_check(const std::wstring &file_name, const std::wstring *&file_extension);
	virtual void file_operation(const std::wstring &file_path, const std::wstring &file_extension);
	virtual bool next_file_check(const HANDLE hFind, const LPWIN32_FIND_DATA p_find_data);

protected:
	file_extension_list m_extensions;
	bool m_traverse_subtree;
	long m_directory_level;

private:
	// Main traversal function
	void traverse_folder(const std::wstring &folder_path);
};
