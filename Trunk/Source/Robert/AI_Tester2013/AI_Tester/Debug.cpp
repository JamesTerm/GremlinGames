
#include "stdafx.h"
#include <stdarg.h>
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;

std::string BuildString2(const char *format, va_list argptr)
{
	char Temp[2048];
	vsprintf(Temp,format,argptr);
	std::string ret(Temp);
	return ret;
}

std::string *GetString(size_t line)
{
	std::string *ret=NULL;
	switch(line)
	{
	case 1:		ret=&GG_Framework::UI::DebugOut_PDCB::TEXT1;	break;
	case 2:		ret=&GG_Framework::UI::DebugOut_PDCB::TEXT2;	break;
	case 3:		ret=&GG_Framework::UI::DebugOut_PDCB::TEXT3;	break;
	case 4:		ret=&GG_Framework::UI::DebugOut_PDCB::TEXT4;	break;
	case 5:		ret=&GG_Framework::UI::DebugOut_PDCB::TEXT5;	break;
	}
	return ret;
}

void Dout(size_t line, const char *writeFmt, va_list argptr)
{
	std::string *text=GetString(line);
	if (text)
		*text=BuildString2(writeFmt,argptr);
}

void Dout(size_t line, size_t startingColumn, const char *writeFmt, va_list argptr)
{
	std::string *text=GetString(line);
	if (!text) return;

	const size_t kLineLength = 21;
	const size_t kNumLines = 6;

	size_t start = startingColumn - 1;
	size_t maxLength = kLineLength - start;
	char lineBuffer[kLineLength + 1];

	assert (startingColumn >= 1 && startingColumn < kLineLength);
	assert ((line>0) && (line<=5));

	// snprintf appends NULL to its output.  Therefore we can't write directly to the buffer.
	size_t length = vsnprintf(lineBuffer, kLineLength + 1, writeFmt, argptr);
	if (length < 0) length = kLineLength;

	char textBuffer[kLineLength + 1];
	strncpy(textBuffer,text->c_str(),kLineLength + 1);
	if (text->length()<kLineLength)
		textBuffer[text->length()]=' ';  //convert the zero into a space
	memcpy(textBuffer + start , lineBuffer, std::min(maxLength,length));
	*text=textBuffer;
}

void Dout(size_t line, const char *writeFmt, ...)
{
	// Get the arguments
	va_list args;
	va_start( args , writeFmt );
	Dout(line,writeFmt,args);
}

void Dout(size_t line, size_t startingColumn, const char *writeFmt, ...)
{
	// Get the arguments
	va_list args;
	va_start( args , writeFmt );
	Dout(line,startingColumn,writeFmt,args);
}
