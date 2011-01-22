// DataLinkTester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DataLinkTester.h"

using namespace std;

//Note: Global pointers are fine
HMODULE g_hStats=NULL;
HANDLE	g_LiveUpdates_UpdateEvent=NULL;

size_t testCounter = 0;

const wchar_t *cwsz_DLLFilename=NULL;  //this gets set dynamically

#define SETUPFUNCTION(ret, name, ...) \
	typedef ret (__cdecl *PS_##name )(__VA_ARGS__); \
	PS_##name name = NULL;

#define GETFUNCTIONPOINTER(ftn, index) \
	if ((ftn =(PS_##ftn) GetProcAddress(g_hStats,#ftn))==NULL) throw index;


void DebugOutput(const char *format, ... )
{
// 	int len = 0;
// 	char * buf = "\0";
// 	va_list marker;
// 	va_start(marker,format);
// 
// 	len = _vscprintf(format, marker) + 1;
// 	buf = (char *)(_alloca(sizeof(char) * len));
// 	vsprintf_s(buf,len,format,marker);
// 	OutputDebugStringA(buf);
// 	va_end(marker);

// WIDE VERSION
// 	va_list marker;
// 	va_start(marker,format);
// 	static wchar_t Temp[4096];
// 	vswprintf_s(Temp,4096,format,marker);
// 	OutputDebugString(Temp);
// 	va_end(marker); 

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
		"test \t\t\tShow scoreboard info.\n"
		"start <plugin> [text]\tTest with particular plugin. The text switch re-directs\n"
		" (i.e. \"start 6 text\")\t  serial input from serial port to text file in the\n"
		"\t\t\t  LiveText\\Stats Plugins folder as PLUGINNAME.txt\n"
		"\t\t\t(0=Aggregator 1=Clock 2=Database 3=FileObserver\n"
		"\t\t\t 4=Network\n"
		"\t\t\t 5=Daktronics 6=DSI 7=OES 8=TransLux 9=WhiteWay)\n"
		"stop \t\t\tStops all.\n"
		"reset \t\t\tUnloads all.\n"
		"Help \t\t\t(displays this)\n"
		"\nType \"Quit\" at anytime to exit this application.\n"
		);
}

// The entry point for different DLL values
SETUPFUNCTION( HANDLE,			live_updates_start);
SETUPFUNCTION( const wchar_t *, live_updates_get_value,				const wchar_t [] );
SETUPFUNCTION( void,			live_updates_release_value,			const wchar_t [] );
SETUPFUNCTION( const wchar_t *, live_updates_get_changed_key );
SETUPFUNCTION( void,			live_updates_release_changed_key,	const wchar_t [] );
SETUPFUNCTION( void,			live_updates_end,					HANDLE );


bool GetFunctionPointers()
{
	bool Success=false;
	try
	{
		GETFUNCTIONPOINTER(live_updates_start,				0);
		GETFUNCTIONPOINTER(live_updates_get_value,			1);
		GETFUNCTIONPOINTER(live_updates_release_value,		2);
		GETFUNCTIONPOINTER(live_updates_get_changed_key,	3);
		GETFUNCTIONPOINTER(live_updates_release_changed_key,4);
		GETFUNCTIONPOINTER(live_updates_end,				5);
		Success=true;
	}
	catch (int ErrorCode)
	{
		DebugOutput("%s : GetFunctionPointers failed %d \n","Aggregator.stat",ErrorCode);
	}
	return Success;
}

class Threaded_Clock_Test
{

	public:
		Threaded_Clock_Test() : m_Thread(NULL)
		{
			if (g_hStats)
			{
				m_Thread = new Thread<Threaded_Clock_Test>(this);
			}
		}
		~Threaded_Clock_Test()
		{
			delete m_Thread;
			m_Thread=NULL;
		}
	protected:
		void operator()(const void *)
		{
			// Get the next changed key
			const wchar_t *p_next_key = live_updates_get_changed_key();

			// If there is no new key, just wait again
			if ( p_next_key )
			{
				// Get the value
				const wchar_t *p_next_val = live_updates_get_value( p_next_key );
				if ( p_next_val )
				{	
					if (wcslen(p_next_key) == 0 || wcslen(p_next_val) == 0)
						Sleep(1);
					// Display it on output
					DebugOutput( "[%ls] = %ls\n", p_next_key, p_next_val );
					testCounter++; printf("Counter: %d\n", testCounter);

					// Release the value
					live_updates_release_value( p_next_val );	
				}

				// Release the key
				live_updates_release_changed_key( p_next_key );
			}
			else
				Sleep(1);
		}

	private:
		friend Thread<Threaded_Clock_Test>;
		Thread<Threaded_Clock_Test> *m_Thread;
};


bool OpenResources()
{
	bool ret=true;
	//Note: everything in here is a one-time operation since we have a global pointer (and since it only needs to load these resources once)
	if (!g_hStats)
	{
		// get proc addresses
		g_hStats = ::LoadLibraryW( cwsz_DLLFilename );

		// Check for failures
		if ( !g_hStats ) 
		{
			::fwprintf( stderr, L"Failed to load the stats DLLs." );
			assert (false);
			ret=false;
		}

		if (ret)
			ret = GetFunctionPointers();
		
		assert(ret);
		// Get the live updates instance
		g_LiveUpdates_UpdateEvent = live_updates_start();
		if (g_LiveUpdates_UpdateEvent)
		{
			// Wait for a status update
			::WaitForSingleObject( g_LiveUpdates_UpdateEvent, 5000 );
		}
		else
			ret=false;

		if (!ret) 
		{
			FreeLibrary(g_hStats);
			g_hStats=NULL;
			printf("Plugin Failed!\n");
		}
	}
	return ret;
}

void CloseResources()
{
	if (g_hStats)
	{
		assert(g_LiveUpdates_UpdateEvent);
		live_updates_end(g_LiveUpdates_UpdateEvent);
		g_LiveUpdates_UpdateEvent=NULL;
		FreeLibrary(g_hStats);
		g_hStats=NULL;
		cwsz_DLLFilename=NULL;
	}
}


void SetUseTextFile(int useText)
{
	HKEY hKey;
	DWORD retCode = 0;
	DWORD exists = 0;
	DWORD size = 4;

	retCode = (::RegCreateKeyEx(HKEY_CURRENT_USER,
								L"Software\\NewTek\\DataLink\\Active",
								NULL, NULL,
								REG_OPTION_NON_VOLATILE,
								KEY_ALL_ACCESS,
								NULL,
								&hKey,
								&exists));
	if ((retCode == ERROR_SUCCESS) && (exists == REG_CREATED_NEW_KEY) || (exists == REG_OPENED_EXISTING_KEY))
	{
		//Entry was either successfully created or already exists. Now will try to create/update key value and data.
		retCode = (::RegSetValueEx(	hKey,
									L"UseTextFile",
									NULL,
									REG_DWORD,
									(LPBYTE)&useText,
									size));
	}
	else
	{
		assert(false);	//Could neither open nor create!
	}
	RegCloseKey(hKey);
	//TODO: Better error handling.
}

void CommandLineInterface()
{
	SetUseTextFile(0);

	Threaded_Clock_Test *tct=NULL;
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
				delete tct;
				tct=NULL;
				CloseResources();
			}

			else if (!_strnicmp( input_line, "Start", 3))
			{
				if (!tct)
				{
					wchar_t *PluginToUse=NULL;
					switch (atoi(str_1))
					{
						case 0:	
							PluginToUse=L"Aggregator.stat";
							break;
						case 1:
							PluginToUse=L"Clock.stat";
							break;
						case 2:
							PluginToUse=L"Database.stat";
							break;
						case 3:
							PluginToUse=L"FileObserver.stat";
							break;
						case 4:
							PluginToUse=L"Network.stat";
							break;
						case 5:
							PluginToUse=L"Daktronics.stat";
							break;
						case 6:
							PluginToUse=L"DSI.stat";
							break;
						case 7:
							PluginToUse=L"OES.stat";
							break;
						case 8:
							PluginToUse=L"TransLux.stat";
							break;
						case 9:
							PluginToUse=L"WhiteWay.stat";
							break;
						default:
							assert(false);
					}
					if (!_strnicmp( str_2, "text", 4) && (atoi(str_1) > 4))
						SetUseTextFile(1);

					bool result;
					if  ((!cwsz_DLLFilename) || (wcscmp(PluginToUse,cwsz_DLLFilename)))
					{
						CloseResources();
						cwsz_DLLFilename=PluginToUse;
						result=OpenResources();
					}
					assert(!tct); //santity check
					if (result)
					{
						tct=new Threaded_Clock_Test();
						SetUseTextFile(0);	// Reset the use text file registry flag to false.
					}
				}
				else
					printf("Doh! already started\n");
			}

			else if (!_strnicmp( input_line, "Stop", 3))
			{
				delete tct;
				tct=NULL;
			}

			else if (!_strnicmp( input_line, "Help", 1))
				DisplayHelp();
			else if (!_strnicmp( input_line, "Quit", 1))
				break;
			else
				cout << "huh? - try \"help\"" << endl;
		}
	}
	delete tct;
}


void main(void)
{
	// Use menu for testing code.
	DisplayHelp();
	CommandLineInterface();

	CloseResources();
}