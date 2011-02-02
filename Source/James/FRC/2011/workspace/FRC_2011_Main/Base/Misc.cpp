#include <string>

namespace Base
{

void DebugOutput(const char *format, ... )
{
	va_list marker;
	va_start(marker,format);
		static char Temp[2048];
		vsprintf(Temp,format,marker);
		//OutputDebugString(Temp);
	va_end(marker); 
}

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





};
