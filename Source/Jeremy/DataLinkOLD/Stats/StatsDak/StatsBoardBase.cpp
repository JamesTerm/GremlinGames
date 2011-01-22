#include "stdafx.h"

#ifndef SCOREBOARD_USE_OLD_IMPLEMENTATION

extern HMODULE g_hModule;

//
// Statics for getting child class
//

static int DaktronicsEnabled = -1;
static int WhitewayEnabled = -1;
static int DsiEnabled = -1;
static int OesEnabled = -1;
static int TLMP70Enabled = -1;

StatsBoardBase * getStatsBoard(unsigned esb)
{
	// First time we need to read the ini to see what kind of boards we have
	if (DaktronicsEnabled == -1)
	{
		// Find the ini directory
		char DirectoryString[MAX_PATH], iniDirectoryString[MAX_PATH];

		char DaktronicsEnabledString[MAX_PATH], WhitewayEnabledString[MAX_PATH];
		char DsiEnabledString[MAX_PATH], OesEnabledString[MAX_PATH];
		char TLMP70EnabledString[MAX_PATH];

		char DaktronicsPortString[MAX_PATH], WhitewayPortString[MAX_PATH];
		char DsiPortString[MAX_PATH], OesPortString[MAX_PATH];
		char TLMP70PortString[MAX_PATH];

		if (!::GetModuleFileNameA( g_hModule, DirectoryString, sizeof(DirectoryString)))
		{
			DaktronicsEnabled = 0;
			WhitewayEnabled = 0;
			DsiEnabled = 0;
			OesEnabled = 0;
			TLMP70Enabled = 0;
			return  getStatsBoardNone();
		}
		char *lastSlash = strrchr(DirectoryString, '\\');
		if (lastSlash++)
			*lastSlash = 0;
		strcpy(iniDirectoryString, DirectoryString);

		// Load the ini
		//strcat(iniDirectoryString, "\\Stats Plugins\\ScoreBoard.ini");
		strcat(iniDirectoryString, "ScoreBoard.ini");
		GetPrivateProfileString(StatsDaktronics::GetStaticINIHeader(), "Enabled", "", DaktronicsEnabledString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsWhiteway::GetStaticINIHeader(), "Enabled", "", WhitewayEnabledString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsDsi::GetStaticINIHeader(), "Enabled", "", DsiEnabledString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsOes::GetStaticINIHeader(), "Enabled", "", OesEnabledString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsTransLuxMP70::GetStaticINIHeader(), "Enabled", "", TLMP70EnabledString, MAX_PATH, iniDirectoryString);


		GetPrivateProfileString(StatsDaktronics::GetStaticINIHeader(), "Port", "", DaktronicsPortString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsWhiteway::GetStaticINIHeader(), "Port", "", WhitewayPortString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsDsi::GetStaticINIHeader(), "Port", "", DsiPortString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsOes::GetStaticINIHeader(), "Port", "", OesPortString, MAX_PATH, iniDirectoryString);
		GetPrivateProfileString(StatsTransLuxMP70::GetStaticINIHeader(), "Port", "", TLMP70PortString, MAX_PATH, iniDirectoryString);

		if ((DaktronicsPortString && DaktronicsPortString[0]) && 
			(DaktronicsEnabledString && DaktronicsEnabledString[0]) &&
			(atoi(DaktronicsEnabledString) > 0))
			DaktronicsEnabled = 1;
		else
			DaktronicsEnabled = 0;

		if ((WhitewayPortString && WhitewayPortString[0]) && 
			(WhitewayEnabledString && WhitewayEnabledString[0]) &&
			(atoi(WhitewayEnabledString) > 0))
			WhitewayEnabled = 1;
		else
			WhitewayEnabled = 0;

		if ((DsiPortString && DsiPortString[0]) && 
			(DsiEnabledString && DsiEnabledString[0]) &&
			(atoi(DsiEnabledString) > 0))
			DsiEnabled = 1;
		else
			DsiEnabled = 0;

		if ((OesPortString && OesPortString[0]) && 
			(OesEnabledString && OesEnabledString[0]) &&
			(atoi(OesEnabledString) > 0))
			OesEnabled = 1;
		else
			OesEnabled = 0;

		if ((TLMP70PortString && TLMP70PortString[0]) && 
			(TLMP70EnabledString && TLMP70EnabledString[0]) &&
			(atoi(TLMP70EnabledString) > 0))
			TLMP70Enabled = 1;
		else
			TLMP70Enabled = 0;

	}
	switch(esb)
	{	
		case SBDaktronics:
			if (DaktronicsEnabled)
				return getStatsDaktronics();
			else
				return getStatsBoardNone();
			break;
		case SBWhiteway:
			if (WhitewayEnabled)
				return getStatsWhiteway();
			else
				return getStatsBoardNone();
			break;
		case SBDsi:
			if (DsiEnabled)
				return getStatsDsi();
			else
				return getStatsBoardNone();
			break;
		case SBOes:
			if (OesEnabled)
				return getStatsOes();
			else
				return getStatsBoardNone();
			break;
		case SBTransLuxMP70:
			if (TLMP70Enabled)
				return getStatsTLMP70();
			else
				return getStatsBoardNone();
			break;
		default:
			return getStatsBoardNone();
			break;
	}
}

//
// StatsBoardNone
//

StatsBoardNone * getStatsBoardNone()
{
	static StatsBoardNone * st = new StatsBoardNone();
	return st;
}

void StatsBoardNone::Initialize(void)
{
}

void StatsBoardNone::Destroy(void)
{
}

StatsBoardNone::StatsBoardNone(void)
{

}

StatsBoardNone::~StatsBoardNone(void)
{

}

//
// StatsBoardBase
//

StatsBoardBase::StatsBoardBase(void)
: m_Handle(NULL), m_PlaceInFile(0), m_NoBytesRead(0), m_hPortCheckEvent(NULL), m_pSerialPort(NULL), 
m_CurrentPacket(NULL), m_FileInMemory(NULL), m_PartialPacket(NULL)
{
}

StatsBoardBase::~StatsBoardBase(void)
{
//	DebugOutput("Scoreboard Stats Thread Stopped");
	CloseHandle(m_Handle);

	DisconnectFromScoreBoard();
}


void StatsBoardBase::DisconnectFromScoreBoard(void)
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

	if (m_FileInMemory)
		free(m_FileInMemory);
	if (m_CurrentPacket)
		free(m_CurrentPacket);
	if (m_PartialPacket)
		free(m_PartialPacket);
}

void StatsBoardBase::ConnectToScoreBoard(void)
{
	// If we already have one, we don't need to do this
	if (m_pSerialPort)
		return;

	// Find the ini directory
	char DirectoryString[MAX_PATH], iniDirectoryString[MAX_PATH];
	char PortString[MAX_PATH];
	if (!::GetModuleFileNameA( g_hModule, DirectoryString, sizeof(DirectoryString)))
		return;
	char *lastSlash = strrchr(DirectoryString, '\\');
	if (lastSlash++)
		*lastSlash = 0;
	strcpy(iniDirectoryString, DirectoryString);

	// Load the ini
	strcat(iniDirectoryString, "ScoreBoard.ini");
	GetPrivateProfileString(GetBoardINIHeader(), "Port", "", PortString, MAX_PATH, iniDirectoryString);
	if (!PortString[0])
	{
		printf("INI not found for %s\n", GetBoardINIHeader());
		return;
	}
	unsigned lPortNumber = atoi(PortString);
	unsigned lBaudRate = 9600;
	unsigned lDataBits = 8;
	unsigned lParity = 0;
	unsigned lStopBits = 0;
	unsigned lFlowControl = 0;
	char BaudRateString[MAX_PATH], DataBitsString[MAX_PATH], ParityString[MAX_PATH],
		StopBitsString[MAX_PATH];

	GetPrivateProfileString(GetBoardINIHeader(), "BaudRate", "", BaudRateString, MAX_PATH, iniDirectoryString);
	GetPrivateProfileString(GetBoardINIHeader(), "DataBits", "", DataBitsString, MAX_PATH, iniDirectoryString);
	GetPrivateProfileString(GetBoardINIHeader(), "Parity", "", ParityString, MAX_PATH, iniDirectoryString);
	GetPrivateProfileString(GetBoardINIHeader(), "StopBits", "", StopBitsString, MAX_PATH, iniDirectoryString);

	if (BaudRateString[0])
		lBaudRate = atoi(BaudRateString);
	if (DataBitsString[0])
		lDataBits = atoi(DataBitsString);
	if (ParityString[0])
		lParity = atoi(ParityString);
	if (StopBitsString[0])
	{
		float flt = (float)atof(StopBitsString);
		if ((flt > 0.9) && (flt < 1.1))
			lStopBits = 0;
		else if ((flt > 1.1) && (flt < 1.9))
			lStopBits = 1;
		else if ((flt > 1.9) && (flt < 2.1))
			lStopBits = 2;
	}

	// Reserve the port from other Com Port devices
	char szEventName[MAX_PATH];
	wsprintfA( szEventName, "NewTek_PortCheck.%d", lPortNumber );
	m_hPortCheckEvent = CreateEvent( NULL, false, true, szEventName);

	// Open the port supplied in the ini
	m_pSerialPort = new SerialPort( lPortNumber );
	unsigned tries = 0;
	while( m_pSerialPort->Error() )
	{
		delete m_pSerialPort;
		m_pSerialPort = NULL;
		tries++;
		printf("Cannot create serial port for %s, try %i.\n", GetBoardINIHeader(), tries);
		if (tries > 6)
		{
			printf("Giving Up.\n");
			break;
		}
		Sleep(300);
		m_pSerialPort = new SerialPort( lPortNumber );
	}
	if (m_pSerialPort)
	{
		m_pSerialPort->SetPortSettings(lBaudRate, lDataBits, lParity, lStopBits, lFlowControl);
		printf("Found %s board\n", GetBoardINIHeader());
	}
}
void StatsBoardBase::ReadFromScoreBoard(void)
{
	// This routine just frames the message. It does not try to validate the interior
	// of the message. For instance we could scan the filled message packet for internal 
	// End of Msg characters, but unfortunately the TransLux MP-70 protocol chose to embed
	// the eom into the middle of the message. Instead of making an ugly special case, let
	// the protocol routines do the message validation. Here, just try to frame it.

	char * pEndOfMsg;
	unsigned char eom=GetEndofMessageChar();

	m_PacketSize = GetNumBytesToRead();

	if (!m_pSerialPort)
		return;
	assert(m_CurrentPacket);
	if (!m_CurrentPacket)
		return;

	unsigned PartialLength;
	int NumBytesRemaining=0;
	unsigned lNumBytesRead = 0;
	lNumBytesRead = m_pSerialPort->ReadFromPort( m_SerialInBuffer, 1);
	
	if(lNumBytesRead == 0)
	{	// Then nothing was read so just bail out and come back later to read
		m_SerialInBuffer[0] = NULL;
		//Sleep(1);
		return;
	}
	m_SerialInBuffer[lNumBytesRead] = NULL; // Make it look like a string
	if (lNumBytesRead == m_PacketSize)
		m_PartialPacket[0] = NULL;	// Looks like we got a full packet, so we should get rid of the partial.
	PartialLength = strlen(m_PartialPacket);
	if(PartialLength == 0) // Then just starting to fill the partial packet
	{
		// Look for the eom (End of Msg) character
		pEndOfMsg = strchr( (char *)m_SerialInBuffer, (int)eom);
		if(pEndOfMsg != NULL)
		{
			// If we found the eom, then the pointer into the string is not NULL
			pEndOfMsg++;
			if(strlen(pEndOfMsg) == 0 && lNumBytesRead == m_PacketSize)
			{	// Then the message was perfectly framed directly from the serial read
				strncat(m_PartialPacket, (char *)m_SerialInBuffer, m_PacketSize);
			}
			else
			{	// The message was not perfectly framed by the serial read so gather 
				// the partial message.
				strncat(m_PartialPacket, pEndOfMsg, strlen(pEndOfMsg));
				return;
			}
			m_SerialInBuffer[0] = NULL;
		}

	}
	PartialLength = strlen(m_PartialPacket);
	NumBytesRemaining = m_PacketSize - PartialLength;
	if(NumBytesRemaining <= lNumBytesRead)
	{
		// Fully fill the partial packet
		strncat(m_PartialPacket, (char*)m_SerialInBuffer, NumBytesRemaining);
		
		if((unsigned char)m_PartialPacket[m_PacketSize - 1] == eom)
		{
			// Copy over the filled "partial" packet to the Current Packet for further processing
			strncpy(m_CurrentPacket, (char*)m_PartialPacket, strlen(m_PartialPacket));
			// Zero terminate.
			m_CurrentPacket[m_PacketSize] = 0;

			if (CheckPacket())
			{
				if (ParsePacket())
				{
					SetEvent(m_Handle);
				}
			}
			m_CurrentPacket[0] = NULL;
			m_PartialPacket[0] = NULL;
			if((lNumBytesRead - NumBytesRemaining) > 0)
			{
				strncat(m_PartialPacket, (char*)m_SerialInBuffer+NumBytesRemaining, lNumBytesRead - NumBytesRemaining);
				// Must Null terminate this string buffer
			}
			m_SerialInBuffer[0] = NULL;
			return;
		}
		else
		{	// We read enough characters, but the message was not properly terminated
			// Set all buffers to empty and start over.
			m_CurrentPacket[0] = NULL;
			m_PartialPacket[0] = NULL;
			m_SerialInBuffer[0] = NULL;
			return;
		}
	}
	else
	{
		// Still don't have a full message packet
		strncat(m_PartialPacket, (char*)m_SerialInBuffer, lNumBytesRead);
		m_SerialInBuffer[0] = NULL;
		// If EOM found but packet is wrong size, start over
		if(strlen(m_PartialPacket) > (GetNumExtraEOMThreshhold()) &&
			(unsigned char)m_PartialPacket[strlen(m_PartialPacket) - 1] == eom)
		{
			m_CurrentPacket[0] = NULL;
			m_PartialPacket[0] = NULL;
			m_SerialInBuffer[0] = NULL;
			return;
		}
	}
	return;
}


void StatsBoardBase::AdvanceFile(void)
{
	if (m_NoBytesRead < 1)
		return;
	assert(m_CurrentPacket);
	if (!m_CurrentPacket)
		return;
	
	unsigned lManualAdvance = 0;

	if ((m_PlaceInFile+GetNumBytesToRead()*2) > m_NoBytesRead)
		m_PlaceInFile = 0;
	if (GetEndofMessageChar())
	{
		for (unsigned i=1;i<GetNumBytesToRead();i++)
		{
			char parsechar = (char)(m_FileInMemory[m_PlaceInFile+i]);
			char eomchar = GetEndofMessageChar();
			if (parsechar == eomchar)
			{
				lManualAdvance = i+1;
				break;
			}
		}
	}

	if (lManualAdvance)
	{
		memcpy(m_CurrentPacket,m_FileInMemory+m_PlaceInFile,lManualAdvance);
		m_CurrentPacket[lManualAdvance] = 0;
		m_PlaceInFile += lManualAdvance;
	}
	else
	{
		memcpy(m_CurrentPacket,m_FileInMemory+m_PlaceInFile,GetNumBytesToRead());
		m_CurrentPacket[GetNumBytesToRead()] = 0;
		m_PlaceInFile += GetNumBytesToRead();
	}

	if (CheckPacket())
		if (ParsePacket())
			SetEvent(m_Handle);
}

void StatsBoardBase::OpenFile(void)
//Unused in final version, of course
{
	assert(m_CurrentPacket);
	if (!m_CurrentPacket)
		return;
	const char *FileName = GetBoardTestFileName();
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

	memcpy(m_CurrentPacket,m_FileInMemory,GetNumBytesToRead());
	m_CurrentPacket[GetNumBytesToRead()] = 0;
}

void StatsBoardBase::StartUpdates(void)
{
	if (!GetNumBytesToRead())
		return;
	m_CurrentPacket = (char *)malloc(GetNumBytesToRead()+1);
	m_PartialPacket = (char *)malloc(GetNumBytesToRead()+1);
	m_SerialInBuffer = (byte *)malloc(GetNumBytesToRead()+1);
	m_PartialPacket[0] = NULL;
	m_CurrentPacket[0] = NULL;
	m_SerialInBuffer[0] = NULL;
#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
	OpenFile();
	if (m_NoBytesRead)
		StartThread();
#else  //!SCOREBOARD_READ_FROM_SAMPLE_TEXT
	ConnectToScoreBoard();
	if (m_pSerialPort)
		StartThread();
#endif

}

void StatsBoardBase::StopUpdates(void)
{
#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
#else //!SCOREBOARD_READ_FROM_SAMPLE_TEXT
	DisconnectFromScoreBoard();
#endif
	StopThread();
}

const wchar_t * StatsBoardBase::GetNextChangedKey(void)
{
	char * nck = GetBoardNextChangedKey();
	if (nck)
	{
		char2wchar(nck);
		wcscpy(m_CurrentKey,char2wchar_pwchar);
		return m_CurrentKey;
	}
	else return NULL;
}

const wchar_t * StatsBoardBase::GetValue(const wchar_t key[])
{
	wchar2char(key);
	const char * ckey = wchar2char_pchar;
	const char * gbv = GetBoardValue(ckey);
	if (!gbv)
		return NULL;

	char2wchar(gbv);
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

void StatsBoardBase::ReleaseValue(const wchar_t value[])
{
	free((wchar_t*)value);
}

void StatsBoardBase::ThreadProcessor(void)
{
#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
	AdvanceFile();
#else  //!SCOREBOARD_READ_FROM_SAMPLE_TEXT
	ReadFromScoreBoard();
#endif
	//Sleep(1);
}

void StatsBoardBase::CopyStringIfChanged(char * destination, const char * source, size_t num, unsigned index, bool &val_changed)
{
	// if strings differ, perform the copy and set changed
	if (strncmp(destination, source, num) || (strlen(destination) > num))
	{
		strncpy(destination, source, num);

		//Null Terminate it
		destination[num] = 0;

		val_changed = 1;
	}
}

#endif