#include "StdAfx.h"
#include "FrameWork.XML.h"

namespace Frameworks
{

bool load_from_string( const std::string	&xml_data, node base_node[] )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data.c_str(), xml_data.size() );

	// Return
	return !local_parser.error();
}

bool load_from_string( const std::wstring	&xml_data, node base_node[] )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data.c_str(), xml_data.size() );

	// Return
	return !local_parser.error();
}


bool load_from_string( const char xml_data[], node base_node[] )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data, strlen( xml_data ) );

	// Return
	return !local_parser.error();
}

bool load_from_string( const wchar_t xml_data[], node base_node[] )
{	// Create a parser
	parser	local_parser( base_node );
	
	// Add it to the parser
	const bool xml_error = local_parser( xml_data, wcslen( xml_data ) );

	// Return
	return !local_parser.error();
}

}