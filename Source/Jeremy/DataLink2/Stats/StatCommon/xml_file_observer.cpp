#include "StdAfx.h"
#include "FrameWork.XML.h"

using namespace Frameworks;

// Constructor
xml_file_observer::xml_file_observer( const wchar_t filename[], node observer[] )
	:	file_observer( filename ),
		m_p_node( observer )
{	// Always start with a change
	file_changed( 0 );
}

// Destructor
xml_file_observer::~xml_file_observer( void )
{

}

// Overload this if you want to know when the file is changed
void xml_file_observer::file_changed( const __int64 time_changed )
{
	// Process the file
	load_from_file( filename(), m_p_node );
}