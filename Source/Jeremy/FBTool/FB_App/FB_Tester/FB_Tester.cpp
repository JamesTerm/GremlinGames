// DataLinkTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FB_Tester.h"

using namespace std;

void DebugOutput(const char *format, ... )
{
	va_list marker;
	va_start(marker,format);
	static char Temp[2048];
	vsprintf_s(Temp,2048,format,marker);
	OutputDebugStringA(Temp);
	va_end(marker); 
}


void DisplayHelp()
{
	printf(
		"*All output directed to VS debug Output Window*\n"
		"1 \t\t\tDoes thing 1.\n"
		"2 \t\t\tDoes thing 2.\n"
		"(R)eset \t\tStarts over.\n"
		"(H)elp \t\t\t(displays this)\n"
		"\n(Q)uit at anytime to exit this application.\n"
		);
}

void CommandLineInterface()
{
	char input_line[128];
	while (cout << ">",cin.getline(input_line,128))
	{
		char		command[32];
		char		str_1[MAX_PATH];
		char		str_2[MAX_PATH];
		char		str_3[MAX_PATH];
		char		str_4[MAX_PATH];

		command[0]=0;
		str_1[0]=0;
		str_2[0]=0;
		str_3[0]=0;
		str_4[0]=0;

		if (sscanf( input_line,"%s %s %s %s %s",command,str_1,str_2,str_3,str_4)>=1)
		{
			if (!_strnicmp( input_line, "test", 1))
			{
				DebugOutput("Test %d\n",8675309);
				DebugOutput("Test2 %x\n",0xabacab);
			}
			else if (!_strnicmp( input_line, "Reset", 1))
			{

			}
			else if (!_strnicmp( input_line, "1", 1))
			{

			}
			else if (!_strnicmp( input_line, "2", 1))
			{

			}
			else if (!_strnicmp( input_line, "Help", 1))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Quit", 1))
				break;
			else
				cout << "huh? - try \"help\"" << endl;
		}
	}
}


void main(void)
{
	// Use menu for testing code.
	DisplayHelp();
	CommandLineInterface();

}