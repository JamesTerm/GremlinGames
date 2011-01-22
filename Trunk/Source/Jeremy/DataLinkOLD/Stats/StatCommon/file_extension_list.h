#pragma once

// class file_extension_list
class STATCOMMON_API file_extension_list
{
public:
	// Static member functions
	static const std::wstring *extract_file_extension(std::wstring &file_extension, const std::wstring &file_name_or_extension);

	// Constructor / destructor
	file_extension_list(bool check_all_files = false);
	virtual ~file_extension_list();

	const std::wstring *add_file_extension(const std::wstring &file_name_or_extension);
	const std::wstring *get_valid_file_extension(const std::wstring &file_name_or_extension);

protected:
	// Typedef for a set of strings
	typedef std::set<std::wstring> wstring_set;

	bool m_check_all_files;
	wstring_set m_file_extension_set;
};
