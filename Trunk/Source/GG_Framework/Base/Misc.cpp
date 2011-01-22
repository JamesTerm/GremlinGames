#include "stdafx.h"
#include "GG_Framework.Base.h"

namespace GG_Framework
{
	namespace Base
	{

void DebugOutput(const char *format, ... )
{
	static OpenThreads::Mutex mutex;
	GG_Framework::Base::RefMutexWrapper rmw(mutex);
	va_list marker;
	va_start(marker,format);
		static char Temp[2048];
		vsprintf(Temp,format,marker);
		OutputDebugString(Temp);
		//APIDebugOutput(Temp);    We may want this later
	va_end(marker); 
}
//////////////////////////////////////////////////////////////////////////

int CompareFileLastWriteTimes(const char* f1, const char* f2)
{
	ASSERT(f1 && f2);

	WIN32_FIND_DATA FindFileData1;
	HANDLE hFind1 = FindFirstFile(f1, &FindFileData1);

	WIN32_FIND_DATA FindFileData2;
	HANDLE hFind2 = FindFirstFile(f2, &FindFileData2);

	if ((hFind1==INVALID_HANDLE_VALUE) && (hFind2==INVALID_HANDLE_VALUE))
		return -4;

	if (hFind1==INVALID_HANDLE_VALUE)
		return -3;

	if (hFind2==INVALID_HANDLE_VALUE)
		return -2;

	// They both exist, compare the times
	return CompareFileTime(&(FindFileData1.ftLastWriteTime), &(FindFileData2.ftLastWriteTime));
}
//////////////////////////////////////////////////////////////////////////

std::string BuildString(const char *format, ... )
{
	char Temp[2048];
	va_list marker;
	va_start(marker,format);
	vsprintf(Temp,format,marker);
	va_end(marker); 
	std::string ret(Temp);
	return ret;
}
//////////////////////////////////////////////////////////////////////////

char* GetLastSlash(char* fn, char* before)
{
	if (!fn) return NULL;
	char* lastSlash = before ? before-1 : fn+strlen(fn);

	while (lastSlash > fn)
	{
		if ((*lastSlash == '/') || (*lastSlash == '\\'))
			return lastSlash;
		--lastSlash;
	}

	return NULL;
}
//////////////////////////////////////////////////////////////////////////

std::string GetContentDir_FromFile(const char* fn)
{
	if (fn == NULL)
		return NULL;

	// Make sure we are working with a full path
	static OpenThreads::Mutex mutex;
	GG_Framework::Base::RefMutexWrapper rmw(mutex);
	static char buff[_MAX_PATH];
	if( _fullpath( buff, fn, _MAX_PATH ) == NULL )
		strcpy(buff, fn);

	// Strip the filename off to just get the parent folder
	char* lastSlash = GetLastSlash(buff, NULL);
	if (lastSlash)
	{
		*lastSlash = 0;

		// Now work up the path until we find Objects or Scenes
		while (lastSlash = GetLastSlash(buff, lastSlash))
		{
			// The content Directory SHOULD be right above the objects or scenes or images or scripts directory
			if (	!strnicmp(lastSlash+1, "objects/", 8)
				||	!strnicmp(lastSlash+1, "objects\\", 8)
				||	!stricmp(lastSlash+1, "objects")
				||	!strnicmp(lastSlash+1, "scenes\\", 7)
				||	!strnicmp(lastSlash+1, "scenes/", 7)
				||	!stricmp(lastSlash+1, "scenes")
				||	!strnicmp(lastSlash+1, "images\\", 7)
				||	!strnicmp(lastSlash+1, "images/", 7)
				||	!stricmp(lastSlash+1, "images")
				||	!strnicmp(lastSlash+1, "scripts\\", 8)
				||	!strnicmp(lastSlash+1, "scripts/", 8)
				||	!stricmp(lastSlash+1, "scripts"))
			{
				*lastSlash = 0;
				break;
			}
		}
	}
	std::string ret(buff);

	return ret;
}
//////////////////////////////////////////////////////////////////////////

//! Returns false iff c == [ 'f', 'F', 'n', 'N', '0', 0 ]
bool ParseBooleanFromChar(char c)
{
	c = toupper(c);
	if ((c == 'F') || (c == 'N') || (c == '0') || (c == 0))
		return false;
	else
		return true;
}
//////////////////////////////////////////////////////////////////////////

void StripCommentsAndTrailingWhiteSpace(char* line)
{
	for (char* eol = line; ; ++eol)
	{
		if ((eol[0] == '\n') || (eol[0] == '\r') || (eol[0] == '#') || (eol[0] == '\0'))
		{
			eol[0] = '\0';
			--eol;
			while ((eol >= line) && ((eol[0]==' ') || (eol[0]=='\t')))
			{
				eol[0] = '\0';
				--eol;
			}
			return;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

std::string TrimString( const std::string& StrToTrim )
{
	// Find first non whitespace char in StrToTrim
	std::string::size_type First = StrToTrim.find_first_not_of(std::string(" \n\t\r"));
	// Check whether something went wrong?
	if( First == std::string::npos )
	{
		First = StrToTrim.size()-1;
	}

	// Find last non whitespace char from StrToTrim
	std::string::size_type Last = StrToTrim.find_last_not_of(std::string(" \n\t\r"));
	// If something didn't go wrong, Last will be recomputed to get real length of substring
	if( Last == std::string::npos )
	{
		Last = 0;
	}

	std::string::size_type count = (Last>First) ? (( Last + 1 ) - First) : 0;
	if (count > 0)
		return (StrToTrim.substr( First, count ));
	else return std::string("");
}

bool ReadStringMapFromIniFile(std::string filename, StringMap& resMap)
{
	FILE* file = fopen(filename.c_str(), "r");
	if (!file) return false;

	// Start reading line by line until no more lines
	char buff[1024];
	char* line = fgets(buff, 1024, file);
	while (line)
	{
		StripCommentsAndTrailingWhiteSpace(line);
		char* eq = strchr(line, '=');
		if (eq)
		{
			*eq = '\0';
			std::string name = TrimString(line);
			std::string val = TrimString(eq+1);
			if (!name.empty() && !val.empty())
				resMap[name] = val;
		}
		line = fgets(buff, 1024, file);
	}


	return true;
}
//////////////////////////////////////////////////////////////////////////







// Run this startup before anything else can possibly run.
// Because this relies on code changes, this is not required
//#pragma init_seg( compiler )
struct global_list_of_variables
{
	typedef std::pair<void*,const char*>	variable_desc;
	struct cmp 
	{	__forceinline bool operator() ( const variable_desc &a, const variable_desc &b ) const 
	{	if ( a.first == b.first ) 
	return ( ::strcmp( a.second, b.second ) > 0 );
	else return ( a.first < b.first );
	}
	};

	// Constructor
	global_list_of_variables( void )
	{	// Start the critical section
		::InitializeCriticalSection( &m_cs );
	}

	// Destructor
	~global_list_of_variables( void )
	{	// Free the critical section
		::DeleteCriticalSection( &m_cs );

		// Check for memory leaks
		if ( m_all_variables.size() )
		{	::OutputDebugStringA( "Warning : Memory leaks detected.\n" );
		for( std::set< variable_desc, cmp >::iterator i = m_all_variables.begin(); i != m_all_variables.end(); i++ ) log( *i );
		::OutputDebugStringA( "Instantiating debugger.\n" );
		halt();
		}
	}

	// Finished
	void halt( void )
	{	
		// Stop cold
		::DebugBreak();
	}

	// Display an error log of this value
	void log( const variable_desc& val )
	{	// Dump the value
		char addr[ 19 ];
		::sprintf( addr, "0x%p", val.first );
		::OutputDebugStringA( addr );
		::OutputDebugStringA( " " );
		::OutputDebugStringA( val.second );
		::OutputDebugStringA( "\n" );
	}

	// Add to list
	void add( void* p_data, const char* p_name )
	{	// Create a new item
		variable_desc	new_item( p_data, p_name );		

		// Lock the list
		::EnterCriticalSection( &m_cs );

		// Add this entry
		m_all_variables.insert( new_item );

		// Leave the list
		::LeaveCriticalSection( &m_cs );
	}

	// Remove from list
	void remove( void* p_data, const char* p_name )
	{	// Create a new item
		variable_desc	old_item( p_data, p_name );

		// Enter the list
		::EnterCriticalSection( &m_cs );

		// Get the item on the list		
		std::set< variable_desc, cmp >::iterator i = m_all_variables.find( old_item );
		if ( i == m_all_variables.end() )
		{	// Error, this value was not allocated !
			::OutputDebugStringA( "Warning : Memory deleted that was not allocated.\n" );
			log( old_item );
			::OutputDebugStringA( "Instantiating debugger.\n" );
			halt();
		}
		else
		{	// Remove the values
			m_all_variables.erase( i );
		}

		// Leave the critical section
		::LeaveCriticalSection( &m_cs );
	}

	// The list of all variables
	std::set< variable_desc, cmp >	m_all_variables;

	// We need a critical section
	CRITICAL_SECTION	m_cs;
};

static global_list_of_variables	_list_;

// Internal methods
void GG_Framework::Base::track_memory_impl::add( void* p_data, const char* p_name )
{
	_list_.add( p_data, p_name );
}

void GG_Framework::Base::track_memory_impl::remove( void* p_data, const char* p_name )
{
	_list_.remove( p_data, p_name );
}




	};
};