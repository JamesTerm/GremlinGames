
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

void Dout(size_t line, const char *writeFmt, va_list argptr)
{
	switch(line)
	{
		case 1:
			GG_Framework::UI::DebugOut_PDCB::TEXT1 = BuildString2(writeFmt,argptr);
			break;
		case 2:
			GG_Framework::UI::DebugOut_PDCB::TEXT2 = BuildString2(writeFmt,argptr);
			break;
		case 3:
			GG_Framework::UI::DebugOut_PDCB::TEXT3 = BuildString2(writeFmt,argptr);
			break;
		case 4:
			GG_Framework::UI::DebugOut_PDCB::TEXT4 = BuildString2(writeFmt,argptr);
			break;
		case 5:
			GG_Framework::UI::DebugOut_PDCB::TEXT5 = BuildString2(writeFmt,argptr);
			break;
	}
}

void Dout(size_t line, const char *writeFmt, ...)
{
	// Get the arguments
	va_list args;
	va_start( args , writeFmt );
	Dout(line,writeFmt,args);
}
