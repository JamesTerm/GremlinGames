#pragma once

bool STATCOMMON_API load_from_string( const std::string	&xml_data, node base_node[] );
bool STATCOMMON_API load_from_string( const std::wstring	&xml_data, node base_node[] );

bool STATCOMMON_API load_from_string( const char	  xml_data[], node base_node[] );
bool STATCOMMON_API load_from_string( const wchar_t xml_data[], node base_node[] );