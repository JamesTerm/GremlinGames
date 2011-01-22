#include "stdafx.h"
#include "Main.h"

using namespace std;

// Constructor
folder_traverser::folder_traverser(bool check_all_files):
	m_extensions(check_all_files)
{
}

// Destructor
folder_traverser::~folder_traverser()
{
}

// find_first_file
// Calls FindFirstFileW and returns whether it succeeded.
bool folder_traverser::find_first_file(const wstring &folder_path, HANDLE &hFind, const LPWIN32_FIND_DATA p_find_data)
{
	return ((hFind = FindFirstFileW(folder_path.c_str(), p_find_data)) != INVALID_HANDLE_VALUE);
}

// find_next_file
// Calls FindNextFileW and returns whether it succeeded.
bool folder_traverser::find_next_file(const HANDLE hFind, const LPWIN32_FIND_DATA p_find_data)
{
	return (FindNextFileW(hFind, p_find_data) == TRUE);
}

// find_close
// Calls FindClose.
void folder_traverser::find_close(const HANDLE hFind)
{
	FindClose(hFind);
}

// is_dots
// Returns whether a folder ends in dots, either single or double.
bool folder_traverser::is_dots(const wstring &folder_name) const
{
	return (is_single_dot(folder_name) || is_double_dot(folder_name));
}

// is_single_dot
// Returns whether the folder ends in a single dot.
bool folder_traverser::is_single_dot(const wstring &folder_name) const
{
	return (folder_name == L".");
}

// is_double_dot
// Returns whether the folder ends in a double dot.
bool folder_traverser::is_double_dot(const wstring &folder_name) const
{
	return (folder_name == L"..");
}

// traverse
// Virtual function that traverses a folder.
void folder_traverser::traverse(const wstring &folder_path, bool traverse_subtree)
{
	m_traverse_subtree = traverse_subtree;
	m_directory_level = 0;
	traverse_folder(folder_path);
}

// traverse_folder
// Non-virtual function, called by traverse, that does most of the work traversing a folder.
void folder_traverser::traverse_folder(const wstring &folder_path)
{
	wstring next_path(folder_path + L"*");
	HANDLE hFind = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find_data;
	bool traverse_subtree = m_traverse_subtree;
	long directory_level = m_directory_level + 1;
	const wstring *file_extension = NULL;

	if (!(continue_traversing() && begin_folder_operation(next_path, hFind, &find_data)))
		return;

	do {
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (m_traverse_subtree && folder_check(next_path.assign(find_data.cFileName), traverse_subtree, directory_level))
			{
				swap(m_traverse_subtree, traverse_subtree);
				swap(m_directory_level, directory_level);
				traverse_folder(next_path.append(L"\\").insert(0, folder_path));
				swap(m_traverse_subtree, traverse_subtree);
				swap(m_directory_level, directory_level);
			}
		}
		else
		{
			if (file_check(next_path.assign(find_data.cFileName), file_extension))
				file_operation(next_path.insert(0, folder_path), *file_extension);
		}
	} while (continue_traversing() && next_file_check(hFind, &find_data));

	end_folder_operation(hFind);
}

// continue_traversing
// Virtual function, called by traverse_folder to determine whether to continue traversing
// the entire folder tree.
bool folder_traverser::continue_traversing()
{
	return true;
}

// begin_folder_operation
// Virtual function, called by traverse_folder to initialize before traversing a subfolder and to determine
// whether to traverse that subfolder.
bool folder_traverser::begin_folder_operation(const std::wstring &folder_path, HANDLE &hFind, const LPWIN32_FIND_DATA p_find_data)
{
	return find_first_file(folder_path, hFind, p_find_data);
}

// end_folder_operation
// Virtual function, called by traverse_folder on a subfolder after that subfolder's tree is traversed.
void folder_traverser::end_folder_operation(const HANDLE hFind)
{
	find_close(hFind);
}

// folder_check
// Virtual function, called by traverse_folder to determine whether to traverse a subfolder.
bool folder_traverser::folder_check(const wstring &folder_name, bool &traverse_subtree, long &directory_level)
{
	traverse_subtree = m_traverse_subtree;
	directory_level = m_directory_level + 1;

	return (!is_dots(folder_name));
}

// file_check
// Virtual function, called by traverse_folder to determine whether to perform a file operation
// on a file.
bool folder_traverser::file_check(const wstring &file_name, const std::wstring *&file_extension)
{
	return ((file_extension = m_extensions.get_valid_file_extension(file_name)) != NULL);
}

// file_operation
// Virtual function, called by traverse_folder, that does the work on each file.
void folder_traverser::file_operation(const std::wstring &file_path, const std::wstring &file_extension)
{
}

// next_file_check
// Virtual function, called by traverse_folder to get the next file and to determine whether to
// continue traversing the current folder.
bool folder_traverser::next_file_check(const HANDLE hFind, const LPWIN32_FIND_DATA p_find_data)
{
	return find_next_file(hFind, p_find_data);
}
