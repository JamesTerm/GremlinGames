#include "stdafx.h"
#include "Main.h"


  /*******************************************************************************************************/
 /*												StatsBoardBase											*/
/*******************************************************************************************************/

StatsBoardBase::StatsBoardBase(void)
:	m_Handle(NULL), m_hPortCheckEvent(NULL), m_pSerialPort(NULL), m_CurrentPacket(NULL),  m_PartialPacket(NULL), m_pThread(NULL),
	m_PlaceInFile(0), m_NoBytesRead(0), m_FileInMemory(NULL)	// Text input
{
}

StatsBoardBase::~StatsBoardBase(void)
{
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
	if (m_CurrentPacket)
	{
		free(m_CurrentPacket);
		m_CurrentPacket=NULL;
	}
	if (m_PartialPacket)
	{
		free(m_PartialPacket);
		m_PartialPacket=NULL;
	}
}

void StatsBoardBase::ConnectToScoreBoard(void)
{
	// If we already have one, we don't need to do this
	if (m_pSerialPort)
		return;

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

	unsigned PartialLength=0;
	int NumBytesRemaining=0;
	unsigned lNumBytesRead = 0;
	lNumBytesRead = m_pSerialPort->ReadFromPort( m_SerialInBuffer, 1);

	if(lNumBytesRead == 0)
	{	// Then nothing was read so just bail out and come back later to read
		m_SerialInBuffer[0] = NULL;
		Sleep(1);
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
	if(NumBytesRemaining <= (int)lNumBytesRead)
	{
		// Fully fill the partial packet
		strncat(m_PartialPacket, (char*)m_SerialInBuffer, NumBytesRemaining);
		
		if((unsigned char)m_PartialPacket[m_PacketSize - 1] == eom)
		{
			// Copy over the filled "partial" packet to the Current Packet for further processing
			strncpy(m_CurrentPacket, (char*)m_PartialPacket, strlen(m_PartialPacket));
			m_CurrentPacket[m_PacketSize] = 0;	// Zero terminate.
			
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

size_t StatsBoardBase::ManualAdvance(void)
{
	size_t ret = 0;

	for (size_t i=1;i<GetNumBytesToRead();i++)
	{
		unsigned char parsechar = (char)(m_FileInMemory[m_PlaceInFile+i]);
		unsigned char eomchar = GetEndofMessageChar();

		if ((parsechar == eomchar) && (strlen(m_CurrentPacket) == GetNumBytesToRead()))
		{
			ret = i+1;
			break;
		}
	}
	return ret;
}

// Text file for serial testing.
void StatsBoardBase::AdvanceFile(void)
{
	if (m_NoBytesRead < 1)
		return;
	assert(m_CurrentPacket);
	if (!m_CurrentPacket)
		return;
	
	if ((m_PlaceInFile+GetNumBytesToRead()*2) > m_NoBytesRead)
		m_PlaceInFile = 0;
	
	GetEndofMessageChar();

	size_t lManualAdvance = ManualAdvance();
	if (lManualAdvance > 0)
	{
		memcpy(m_CurrentPacket,m_FileInMemory+m_PlaceInFile,lManualAdvance );
		m_CurrentPacket[lManualAdvance ] = 0;
		m_PlaceInFile += lManualAdvance ;
	}
	else
	{
		memcpy(m_CurrentPacket,m_FileInMemory+m_PlaceInFile,GetNumBytesToRead());
		m_CurrentPacket[GetNumBytesToRead()] = 0;
		m_PlaceInFile += GetNumBytesToRead();
	}

	if (CheckPacket())
		if (ParsePacket())
		{
			SetEvent(m_Handle);
		}

#ifdef _DEBUG
	if (m_UseTextFile)
		Sleep(20);	// Add a delay for testing under debug when using text input files.
#endif
}

void StatsBoardBase::OpenFile(void)
{
	assert(m_CurrentPacket);
	if (!m_CurrentPacket)
		return;

	FileName = GetBoardTestFileName(FileName);
	if (FileName == NULL)
		return;

	wcscpy (m_Executable, FileName);

	m_FileInMemory=NULL;
	// Open the file with unbuffered IO
	HANDLE FileIO_Handle=CreateFile(m_Executable,
									GENERIC_READ,
									FILE_SHARE_READ,		// We try to be as helpful as possible !
									NULL,
									OPEN_EXISTING,			// Opens the file,if it exists. If the file does not exist,the function creates the file
									FILE_ATTRIBUTE_NORMAL|FILE_FLAG_NO_BUFFERING,
									NULL);
	//Debugging...
	int Error = ::GetLastError();
	
	// Did the file actually open ?
	if (FileIO_Handle==INVALID_HANDLE_VALUE)
	{	FileIO_Handle=NULL;
		return;
	}
	bool Ret=true;
	LARGE_INTEGER Size;
	if (GetFileSizeEx(FileIO_Handle,&Size))
	{	// Allocate enough memory for the entire file !
		//This assumes the file read is less than 2gb for Win32 builds, in which case should work properly
		size_t iSize=(((size_t)Size.QuadPart+511)/512)*512;
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

	m_UseTextFile = RegReadUseText();
	if (!m_UseTextFile)
	{
		ConnectToScoreBoard();
		if (m_pSerialPort)
		{
			assert(!m_pThread);
			m_pThread=new Thread<StatsBoardBase>(this);
		}
	}
	else
	{
		OpenFile();
 		if (m_NoBytesRead)
			//StartThread();
			m_pThread=new Thread<StatsBoardBase>(this);
		else
			printf("File Not Found.\nCannot find *.txt file in \"..\\LiveText\\Stats Plugins\\Database Input\" folder.\n");
// 			assert(false);	//File not found
	}
}

void StatsBoardBase::StopUpdates(void)
{
	if (!m_UseTextFile)
		DisconnectFromScoreBoard();

	//StopThread();
	delete m_pThread;
	m_pThread=NULL;
}

const wchar_t * StatsBoardBase::GetNextChangedKey(void)
{
	const char * nck = GetBoardNextChangedKey();
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

void StatsBoardBase::operator()(const void *dummy)
{
	if (m_UseTextFile)
		AdvanceFile();
	else
		ReadFromScoreBoard();
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

const bool StatsBoardBase::RegReadUseText(void)
{
	bool ret = false;

	HKEY hKey;
	DWORD retCode = 0;
	int valValue = 0;
	DWORD BufferSize = 4;

	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							L"Software\\NewTek\\DataLink\\Active",
							NULL,
							KEY_READ,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{
		retCode = (::RegQueryValueEx(	hKey,
										L"UseTextFile",
										NULL,
										NULL,
										(LPBYTE)&valValue,
										&BufferSize));
	}
	else
// 		assert(false);

	RegCloseKey(hKey);
	if ((retCode == 0) && (valValue == 1))
		ret = true;
 	
	return ret;
}

  /*******************************************************************************************************/
 /*												StatsBoardNone											*/
/*******************************************************************************************************/


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

  /*******************************************************************************************************/
 /*												SerialBase2												*/
/*******************************************************************************************************/

void SerialBase2::ConnectToScoreBoard(void)
{

	SerialBase2_SBinfo &_=m_IniInfo;
	m_pSerialPort = new SerialPort(_.Port);
	m_pSerialPort->SetPortSettings(_.BaudRate,_.DataBits ,_.Parity, (_.StopBits -1), 0);

	// Open the port supplied in the registry
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
		m_pSerialPort = new SerialPort(_.Port);
		m_pSerialPort->SetPortSettings(_.BaudRate,_.DataBits ,_.Parity, (_.StopBits -1), 0);
	}
	if (m_pSerialPort)
	{
		printf("Found %s board\n", GetBoardINIHeader());
	}
}

//returns start of the Key split from port
wchar_t *SerialBase2::ParseKey(const wchar_t source[], wchar_t *buffer,size_t &Port)
{
	if (wcslen(source) > 5)
	{
		//Splits out the key and port
		buffer[3]=0; //zero-terminate the port #
		Port = _wtoi(buffer);
		return &buffer[4];
	}
	else
		return NULL;
}

void SerialBase2::CopyStringIfChanged2(ParsedElement &destination, char * source, size_t num)
{
	//TODO: Change to accept an offset value contained in the parsed element?
	// if strings differ, perform the copy and set changed
	if (strncmp(destination.ElementName.c_str(), source, num) || (strlen(destination.ElementName.c_str()) > num))
	{
		char * Buffer = (char *)_alloca(num+1);
		strncpy(Buffer, source, num);

		//Null Terminate it
		Buffer[num] = 0;

		destination.ElementName = Buffer;
		destination.ElementUpdated = true;
	}
	else
		destination.ElementUpdated = false;
}

bool SerialBase2::ParsePacket(void) 
{
	bool ret = true;

	size_t offset = GetFieldOffset(0);

	const size_t mins = (GetNumFields() - 3);
	const size_t secs = (GetNumFields() - 2);
	const size_t tens = (GetNumFields() - 1);

	if (m_CurrentPacket)
	{
		for (size_t i = 0; i < mins; i++)
		{
			// Fix up the leading zeros, set clock and individual values.
			if (i == 0)
			{
				char lClockTemp[16];
				strncpy(lClockTemp, &m_CurrentPacket[1], 7);
				lClockTemp[7] = 0;	// Zero term

				if ((lClockTemp[0] == ' ') || (lClockTemp[0] == '0'))	// Tens minutes
				{
					lClockTemp[0] = ' ';
					if (lClockTemp[1] == '0')	// Minutes and colon
					{
						lClockTemp[1] = ' ';
						lClockTemp[2] = ' ';
						if (lClockTemp[3] == '0')	// Tens seconds
							lClockTemp[3] = ' ';
					}
				}
				CopyStringIfChanged2(GetParsedElement(i), &lClockTemp[0], GetFieldWidth(i));	// Set game clock

				CopyStringIfChanged2(GetParsedElement(mins), &lClockTemp[0], 2); // Set minutes
				CopyStringIfChanged2(GetParsedElement(secs), &lClockTemp[3], 2); // Set seconds
				CopyStringIfChanged2(GetParsedElement(tens), &lClockTemp[6], 1); // Set tenths
			}
			else
			{
				// Copy other values if changed. Assumes no need to correct leading zeros for other values like score etc.
				CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], GetFieldWidth(i));
			}

			assert(ret);
			offset += (GetFieldWidth(i) + GetFieldOffset(i+1));
		}
	}
	return ret;
}

bool SerialBase2::CheckPacket(void) 
{
	CheckCheckSum();
	return true;
}

bool SerialBase2::CheckCheckSum(void)
{
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

  /*******************************************************************************************************/
 /*												ParseReg												*/
/*******************************************************************************************************/

#define GETREGVALUE(path, name,field)		\
{											\
	val = (GetRegValue(path, name, val));	\
	wchar2char(val);						\
	Info.##field=wchar2char_pchar;			\
}


void ParseReg::operator()(void)
{
	const wchar_t * m_RegActivePath = L"Software\\NewTek\\DataLink\\Active";
	const wchar_t * val = 0;
	wchar_t tempVal[32];

	SerialBase2_SBinfo Info;

	GETREGVALUE(m_RegActivePath, L"Name",Name);
	GETREGVALUE(m_RegActivePath, L"Type",Type);
	GETREGVALUE(m_RegActivePath, L"Board",Board);

	val = (GetRegValue(m_RegActivePath, L"Sport", val));
	wcscpy(tempVal, val);
	val = (GetRegValue(m_RegActivePath, L"SportCode", val));
	Info.SportCode=(ULONG)_wtoi(val);
	wcscat(tempVal, L"_");
	wcscat(tempVal, val);
	wchar2char(tempVal);
 	Info.Sport=wchar2char_pchar;

	val = (GetRegValue(m_RegActivePath, L"Baud", val));
 	Info.BaudRate=(ULONG)_wtoi(val);
	val = (GetRegValue(m_RegActivePath, L"DataBits", val));
	Info.DataBits=(ULONG)_wtoi(val);
	val = (GetRegValue(m_RegActivePath, L"Parity", val));
	Info.Parity=(ULONG)_wtoi(val);
	val = (GetRegValue(m_RegActivePath, L"Port", val));
	Info.Port=(ULONG)_wtoi(val);
	val = (GetRegValue(m_RegActivePath, L"StopBits", val));
	Info.StopBits=(ULONG)_wtoi(val);

	AddSerialInfo(Info);
}

const wchar_t * const ParseReg::GetRegValue(const wchar_t * regPath,
											const wchar_t * regValue,
											const wchar_t * retVal)
{
	HKEY hKey;
	DWORD retCode = 0;
	wchar_t valValue[16383] = L"\0";
	DWORD BufferSize = 16383;

	retCode=(::RegOpenKeyEx(HKEY_CURRENT_USER,
							regPath,
							NULL,
							KEY_READ,
							&hKey));
	if (retCode == ERROR_SUCCESS)
	{
		retCode = (RegQueryValueEx(	hKey,
									regValue,
									NULL,
									NULL,
									(LPBYTE)valValue,
									&BufferSize));
	}
	else
// 		assert(false);

	RegCloseKey(hKey);
 	retVal = valValue;
	return retVal;
}