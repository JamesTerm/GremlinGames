#include "stdafx.h"
#include "Main.h"

using namespace std;

// Constructor
file_extension_list::file_extension_list(bool check_all_files):
	m_check_all_files(check_all_files)
{
}

// Destructor
file_extension_list::~file_extension_list()
{
}

// extract_file_extension
// Extracts the file extension from one string and copies it to another string.
const wstring *file_extension_list::extract_file_extension(wstring &file_extension, const wstring &file_name_or_extension)
{
	wstring::size_type first_index;

	if ((first_index = file_name_or_extension.find_last_of(L"\\")) != wstring::npos)
		first_index++;
	else
		first_index = 0;
	file_extension.assign(file_name_or_extension.substr(first_index, file_name_or_extension.size()));

	if ((first_index = file_extension.find_last_of(L".")) != wstring::npos)
		file_extension.erase(0, first_index);
	else
		file_extension.insert(0, L".");

	return &file_extension;
}

// add_file_extension
// Adds a file extension to the set.
const wstring *file_extension_list::add_file_extension(const wstring &file_name_or_extension)
{
	static wstring file_extension(L"");

	return &*(m_file_extension_set.insert(*extract_file_extension(file_extension, file_name_or_extension)).first);
}

// get_valid_file_extension
// Checks to see whether a file extension exists in the set. If it does, it returns it,
// otherwise it returns NULL.
const wstring *file_extension_list::get_valid_file_extension(const wstring &file_name_or_extension)
{
	wstring file_extension(L"");
	wstring_set::const_iterator in_set;

	if (m_check_all_files)
		return add_file_extension(file_name_or_extension);

	if ((in_set = m_file_extension_set.find(*extract_file_extension(file_extension, file_name_or_extension))) != m_file_extension_set.end())
		return &*in_set;

	return NULL;
}
