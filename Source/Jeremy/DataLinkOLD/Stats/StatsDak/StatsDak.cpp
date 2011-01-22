#include "stdafx.h"

#ifdef SCOREBOARD_USE_OLD_IMPLEMENTATION

StatsDak * getStatsDak()
{
   static StatsDak * st = new StatsDak();
   return st;
}

StatsDak::StatsDak(void)
: m_Handle(NULL), m_ststate(eNotStarted), m_PlaceInFile(0), m_NoBytesRead(0), m_ValuesBlocked(0), m_ParsingBlocks(0), m_hPortCheckEvent(NULL), m_pSerialPort(NULL)
{
	for (unsigned i=0;i<eDakNumFields;i++)
	{
		m_ValueStrings[i]=(char*)malloc(g_StringLengths[i]+1);
		m_ValueStrings[i][0] = 0;
		m_ValueChanged[i] = false;
	}
	m_Handle=CreateEvent	(NULL,false,false,NULL);
	StartThread();
}

StatsDak::~StatsDak(void)
{
	StopThread();
	CloseHandle(m_Handle);
	for (unsigned i=0;i<eDakNumFields;i++)
		free(m_ValueStrings[i]);
	free(m_FileInMemory);

	DisconnectFromDaktronics();
}

void StatsDak::ThreadProcessor(void)
{
#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
	AdvanceFile();
#else  //!SCOREBOARD_READ_FROM_SAMPLE_TEXT
	ReadFromDaktronics();
#endif
	Sleep(10);
}

void StatsDak::ReadFromDaktronics(void)
{
	if (!m_pSerialPort)
		return;
	byte MessageReceive[31] = { 0, 0, 0, 0, 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0,
								0, 0, 0, 0, 0, 0
							};
	if( m_pSerialPort->ReadFromPort( MessageReceive, 31 ) )
	{

		memcpy(m_CurrentPacket,MessageReceive,DAK_PACKET_LENGTH);
		m_CurrentPacket[DAK_PACKET_LENGTH] = 0;

		if (CheckPacket())
			ParsePacket();
	}

}

void StatsDak::DisconnectFromDaktronics(void)
{
	if(m_hPortCheckEvent)
	{
		CloseHandle(m_hPortCheckEvent);
		m_hPortCheckEvent = NULL;
	}
	if(m_pSerialPort)
	{
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}
}

void StatsDak::ConnectToDaktronics(void)
{
	// If we already have one, we don't need to do this
	if (m_pSerialPort)
		return;

	// Find the ini directory
	char DirectoryString[MAX_PATH], iniDirectoryString[MAX_PATH];
	char PortString[MAX_PATH];
	if (!::GetModuleFileNameA(0, DirectoryString, sizeof(DirectoryString)))
		return;
	char *lastSlash = strrchr(DirectoryString, '\\');
	if (lastSlash++)
		*lastSlash = 0;
	strcpy(iniDirectoryString, DirectoryString);

	// Load the ini
	strcat(iniDirectoryString, "\\Stats Plugins\\Daktronics.ini");
	GetPrivateProfileString("Daktronics", "Port", "", PortString, MAX_PATH, iniDirectoryString);
	// If the ini is not found, get out
	if (!PortString[0])
		return;
	unsigned lPortNumber = atoi(PortString);



	// Reserve the port from other Com Port devices
	char szEventName[MAX_PATH];
	wsprintfA( szEventName, "NewTek_PortCheck.%d", lPortNumber );
	m_hPortCheckEvent = CreateEvent( NULL, false, true, szEventName);

	// Open the port supplied in the ini
	m_pSerialPort = new SerialPort( lPortNumber );
	if( m_pSerialPort->Error() )
	{
		delete m_pSerialPort;
		m_pSerialPort = NULL;
	}
	else
	{
		m_pSerialPort->SetPortSettings();
	}
}


void StatsDak::AdvanceFile(void)
{
	if (m_NoBytesRead < 1)
		return;
	if ((m_PlaceInFile+DAK_PACKET_LENGTH*2) > m_NoBytesRead)
		m_PlaceInFile = 0;
	else
		m_PlaceInFile += DAK_PACKET_LENGTH;

	memcpy(m_CurrentPacket,m_FileInMemory+m_PlaceInFile,DAK_PACKET_LENGTH);
	m_CurrentPacket[DAK_PACKET_LENGTH] = 0;

	if (CheckPacket())
		ParsePacket();
}

bool StatsDak::CheckPacket(void)
{
	char * tempstr = m_CurrentPacket;
	// Test Clock
	// Need at least one colon in there somewhere...
	if (!strchr(m_CurrentPacket, ':'))
		return false;
	for (unsigned i=1;i<8;i++)
		if (((m_CurrentPacket[i]>'9')||(m_CurrentPacket[i]<'0')) 
			&& m_CurrentPacket[i] != ' ' 
			&& m_CurrentPacket[i] != ':'
			&& m_CurrentPacket[i] != '.')
			return false;
	// Test Clock Status
	if ((m_CurrentPacket[8] != 's') && (m_CurrentPacket[8] != ' '))
		return false;
	// Test Shot Clock through Time Outs (period can be unexpected char according to doc)
	for (unsigned i=eDakShotClock;i<eDakPeriod;i++)
	{
		unsigned ind = i+7;
		if (((m_CurrentPacket[ind]>'9')||(m_CurrentPacket[ind]<'0')) && m_CurrentPacket[ind] != ' ')
			return false;
	}

	//Daktronics supplies a high/low 2 byte checksum
	if (CheckCheckSum())
		return true;
	else
		return false;


}

bool StatsDak::CheckCheckSum()
{
	//Daktronics supplies a high/low 2 byte checksum
	//This checks the checksum against the data (excluding start & end chars & the checksum itself)
	//  and returns false if it fails
	char * buf = m_CurrentPacket;
	int start = 1;
	int stop = 27;
	char low = NULL;
	char high = NULL;
	int	iIndex;
	unsigned int	uiChecksum;
	const unsigned char	ucaHexChar[17] = { "0123456789ABCDEF" };

	// calculate checksum
	for (iIndex=start,uiChecksum=0; iIndex<=stop; iIndex++)
		uiChecksum += (unsigned int)buf[iIndex];

	// AND total with FF to isolate low byte
	uiChecksum &= 0xff;

	// convert low nibble to hex
	low = ucaHexChar[uiChecksum & 0x0f];

	// convert high nibble to hex
	high = ucaHexChar[uiChecksum >> 4];

	if (low != m_CurrentPacket[29])
		return false;
	if (high != m_CurrentPacket[28])
		return false;

	return true;
}

void StatsDak::CopyStringIfChanged(char * destination, const char * source, size_t num, unsigned index)
{
	// if strings differ, perform the copy and set changed
	if (strncmp(destination, source, num) || (strlen(destination) > num))
	{
		strncpy(destination, source, num);

		//Null Terminate it
		destination[num] = 0;

		m_ValueChanged[index] = 1;
	}
}

void StatsDak::ParsePacket(void)
{
/*
	unsigned secsWaited = 0;
	while (m_ValuesBlocked || secsWaited > 1000)
		Sleep(40);
	if (m_ValuesBlocked)
	{
		//shouldn't take this long
		int ParsePacketWorking = 0;
		assert(ParsePacketWorking);
		return;
	}
*/
	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]))
		return;
	m_ParsingBlocks++;

	// First Parse the clock
	CopyStringIfChanged(m_ValueStrings[eDakClock], m_CurrentPacket+1, 7, eDakClock);

	CopyStringIfChanged(m_ValueStrings[eDakClockStatus], m_CurrentPacket+8, 1, eDakClockStatus);
	CopyStringIfChanged(m_ValueStrings[eDakShotClock], m_CurrentPacket+9, 2, eDakShotClock);
	CopyStringIfChanged(m_ValueStrings[eDakHomeScore], m_CurrentPacket+11, 3, eDakHomeScore);
	CopyStringIfChanged(m_ValueStrings[eDakAwayScore], m_CurrentPacket+14, 3, eDakAwayScore);
	CopyStringIfChanged(m_ValueStrings[eDakHomeFouls], m_CurrentPacket+17, 2, eDakHomeFouls);
	CopyStringIfChanged(m_ValueStrings[eDakAwayFouls], m_CurrentPacket+19, 2, eDakAwayFouls);
	
	// The rest are 1 char long, so I'll loop 'em
	for (unsigned fld=eDakHomeTOFull;fld<eDakHours;fld++)
		CopyStringIfChanged(m_ValueStrings[fld], m_CurrentPacket+fld+14, 1, fld);

	// Null terminate them
//	for (unsigned i=0;i<eDakNumFields;i++)
//	{
//		m_ValueStrings[i][g_StringLengths[i]] = 0;
		// TODO: only set changed if they really changed
//		m_ValueChanged[i] = 1;
//	}

	// Now Parse the Clock into separate values
	char lClockCopy[16];
	strcpy(lClockCopy, m_ValueStrings[eDakClock]);
	char * currString = lClockCopy;
	char* lDot = strrchr(currString, '.');
	char* lLastColon = strrchr(currString, ':');
	lLastColon[0] = 0;
	char* lHoursColon = strrchr(currString, ':');
	lLastColon[0] = ':';
	unsigned hourstrlen = 0;
	if (lHoursColon)
	{
		hourstrlen = lHoursColon - currString;
		CopyStringIfChanged(m_ValueStrings[eDakHours], currString, hourstrlen, eDakHours);
		currString = lHoursColon+1;
	}
	else
	{
		CopyStringIfChanged(m_ValueStrings[eDakHours], "0", 1, eDakHours);
	}
	unsigned minstrlen = lLastColon-currString;
	unsigned secstrlen = lDot?lDot-lLastColon-1:7-(lLastColon-lClockCopy)-1;
	unsigned milstrlen = lDot?7-(lDot-currString)-1:0;
	CopyStringIfChanged(m_ValueStrings[eDakMinutes], currString, minstrlen, eDakMinutes);
	CopyStringIfChanged(m_ValueStrings[eDakSeconds], lLastColon+1, secstrlen, eDakSeconds);
	if (milstrlen)
		CopyStringIfChanged(m_ValueStrings[eDakMilli], lDot+1, milstrlen, eDakMilli);
	else
		CopyStringIfChanged(m_ValueStrings[eDakMilli], "00", 1, eDakMilli);

//	m_ValueStrings[eDakHours][hourstrlen] = 0;
//	m_ValueStrings[eDakMinutes][minstrlen] = 0;
//	m_ValueStrings[eDakSeconds][secstrlen] = 0;
//	m_ValueStrings[eDakMilli][milstrlen] = 0;

	// Remove Trailing Spaces from seconds and milliseconds
//	char *lSpace = strchr(m_ValueStrings[eDakSeconds], ' ');
//	if (lSpace && lSpace[0])
//		lSpace[0] = 0;
//	lSpace = strchr(m_ValueStrings[eDakMilli], ' ');
//	if (lSpace && lSpace[0])
//		lSpace[0] = 0;

	// TODO: Send changed message if this is diff than the old packet
	m_ParsingBlocks--;

	for (unsigned i=0;i<eDakNumFields;i++)
		if (m_ValueChanged[i])
		{
			SetEvent(m_Handle);
			break;
		}

}

void StatsDak::OpenFile(void)
//Unused in final version, of course
{
	const char *FileName = "D:\\docs\\daktronics\\ids\\testorlando.TXT";
	m_FileInMemory=NULL;
	// Open the file with unbuffered IO
	HANDLE FileIO_Handle=CreateFile(	FileName,
										GENERIC_READ,
										FILE_SHARE_READ,		// We try to be as helpful as possible !
										NULL,
										OPEN_EXISTING,			// Opens the file,if it exists. If the file does not exist,the function creates the file
										FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING,
										NULL);
	// Did the file actually open ?
	if (FileIO_Handle==INVALID_HANDLE_VALUE)
	{	FileIO_Handle=NULL;
		return;
	}
	bool Ret=true;
	LARGE_INTEGER Size;
	if (GetFileSizeEx(FileIO_Handle,&Size))
	{	// Allocate enough memory for the entire file !
		DWORD iSize=((Size.QuadPart+511)/512)*512;
		m_FileInMemory=(byte*)malloc(iSize);
		// Read the entire file
		DWORD iSize2=iSize;
		ReadFile(FileIO_Handle,m_FileInMemory,iSize,&iSize2,NULL);
		m_NoBytesRead=unsigned(Size.QuadPart);
		//m_CurrentPosition=0;
		Ret=(iSize2==Size.QuadPart);
	}
	// Close the file handle
	CloseHandle(FileIO_Handle);

	memcpy(m_CurrentPacket,m_FileInMemory,DAK_PACKET_LENGTH);
	m_CurrentPacket[DAK_PACKET_LENGTH] = 0;
}

void StatsDak::StartUpdates(void)
{
#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
	OpenFile();
	AdvanceFile();
#else  //!SCOREBOARD_READ_FROM_SAMPLE_TEXT
	ConnectToDaktronics();
#endif
}

void StatsDak::StopUpdates(void)
{
#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
#else //!SCOREBOARD_READ_FROM_SAMPLE_TEXT
	DisconnectFromDaktronics();
#endif
}

const wchar_t * StatsDak::GetNextChangedKey(void)
{
	for (unsigned i=0;i<eDakNumFields;i++)
		if (m_ValueChanged[i])
		{
			m_ValueChanged[i] = false;
			char2wchar(g_DakKeyStrings[i]);
			wcscpy(m_CurrentKey,char2wchar_pwchar);
			return m_CurrentKey;
		}
	return NULL;
}

const wchar_t * StatsDak::GetValue(const wchar_t key[])
{
	wchar2char(key);
	char * ckey = wchar2char_pchar;
	for (unsigned i=0;i<eDakNumFields;i++)
		if (!stricmp(g_DakKeyStrings[i],ckey))
		{
			m_ValuesBlocked++;

			char2wchar(m_ValueStrings[i]);
			unsigned len = ::wcslen(char2wchar_pwchar);
			wchar_t * wValueToPass = (wchar_t*)malloc(sizeof(wchar_t)*(len+1));

			wcscpy(wValueToPass, char2wchar_pwchar);

			// Trim the whitespace off these before they are sent
			// Do this here so we can keep the original format stored for comparison purposes
			wchar_t * wTrimmedValueToPass = (wchar_t*)malloc(sizeof(wchar_t)*(len+1));
//			wchar_t * tempWCPointer = wValueToPass+(len-1)*sizeof(wchar_t);
			unsigned j = 1;
			wchar_t * tempWCPointer = &wValueToPass[(len-j)];
			while (tempWCPointer[0] == ' ')
			{
				tempWCPointer[0] = 0;
				tempWCPointer = &wValueToPass[(len-++j)];
			}
			tempWCPointer = wValueToPass;
			while (tempWCPointer[0] == ' ')
				tempWCPointer++;

			wcscpy(wTrimmedValueToPass, tempWCPointer);
			free(wValueToPass);

			// Freed by ReleaseValue
			return wTrimmedValueToPass;
		}

	return NULL;
}

void StatsDak::ReleaseValue(const wchar_t value[])
{
	free((wchar_t*)value);
	m_ValuesBlocked--;
}

HANDLE live_updates_start( void )
{
	getStatsDak()->StartUpdates();
	HANDLE h = getStatsDak()->GetHandle();
	ResetEvent(h);
	return(h);
}

void live_updates_release_value( const wchar_t value[] )
{
	getStatsDak()->ReleaseValue(value);
}

void live_updates_release_changed_key( const wchar_t key[] )
{

}

const wchar_t *live_updates_get_changed_key( void )
{
	const wchar_t * k = getStatsDak()->GetNextChangedKey();
	return k;
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{
	const wchar_t * v = getStatsDak()->GetValue(key);
	return v;
}

void live_updates_end( HANDLE hHandle )
{
	getStatsDak()->StopUpdates();
	HANDLE h = getStatsDak()->GetHandle();
	ResetEvent(h);
}

#endif