// BoardWhiteWay.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::WhiteWay;



/*******************************************************************************************************/
/*											BoardWhiteWay													*/
/*******************************************************************************************************/

BoardWhiteWay::BoardWhiteWay(void)
{
}

BoardWhiteWay::~BoardWhiteWay(void)
{
}

void BoardWhiteWay::SetInfo(const SerialBase2_SBinfo &info)
{
	SetSport(info.Sport.c_str());
	__super::SetInfo(info);
}



size_t BoardWhiteWay::GetFieldWidth(size_t index)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetFieldWidth(index);
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardWhiteWay::GetFieldOffset(size_t index)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetFieldOffset(index);
		break;
	default:
		assert(false);
	}
	return ret;
}


size_t BoardWhiteWay::GetNumFields()
{
	unsigned ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetNumFields();	
		break;
	default:
		assert(false);
	}
	return ret;
}

bool BoardWhiteWay::ParsePacket(void)
{
	bool ret = true;

	if ((m_CurrentPacket) || (m_CurrentPacket[0]))
	{
		size_t offset = GetFieldOffset(0);

		const size_t shot = (GetNumFields() - 4);
		const size_t mins = (GetNumFields() - 3);
		const size_t secs = (GetNumFields() - 2);
		const size_t tens = (GetNumFields() - 1);

		for (size_t i = 0; i < mins; i++)
		{
			if (i == 0)	// Game period
			{
				CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], GetFieldWidth(i));
			}
			else if (i == 1)	// Game clock. WhiteWay is odd in that it always sends tenths of second. Basketball, and most other sports
			{					//  with count-down game timers, do not care about tenths precision until under 1 minute is remaining.
				char lTempClock[8] = "\0";
				// Build clock from numbers.
				strncpy(lTempClock, &m_CurrentPacket[offset], 2);
				strncat(lTempClock, ":", 1);
				strncat(lTempClock, &m_CurrentPacket[offset + 2], 2);
				strncat(lTempClock, ".", 1);
				strncat(lTempClock, &m_CurrentPacket[offset + 4], 1);

				// Correct leading zeros
				if (lTempClock[0] == '0')
				{
					lTempClock[0] = ' ';
					if (lTempClock[1] == '0')
					{
						lTempClock[1] = ' ';
						lTempClock[2] = ' ';
						if (lTempClock[3] == '0')
							lTempClock[3] = ' ';
					}
				}

				if (lTempClock[0] > '0' || lTempClock[1] > '0')	// Time greater than 1 minute - no tenths needed.
				{
					CopyStringIfChanged2(GetParsedElement(i), lTempClock, GetFieldWidth(i));
					CopyStringIfChanged2(GetParsedElement(tens), " ", 1);	// Set tens to ' ' just in case a manual clock change.
				}
				else	// Time is at 59.9 or less and we need to show tenths.
				{
					CopyStringIfChanged2(GetParsedElement(i), lTempClock, (GetFieldWidth(i) + 2));
					CopyStringIfChanged2(GetParsedElement(tens), &lTempClock[(6)], 1);	// Get tens
				}

				// Always update the mins and secs and set tens to ' '.
				CopyStringIfChanged2(GetParsedElement(mins), &lTempClock[(0)], 2);	// Get mins
				CopyStringIfChanged2(GetParsedElement(secs), &lTempClock[(3)], 2);	// Get secs
			}
			else if (i == 2 || i == 3)	// Scores
			{
				char lTempScore[4];

				if ((m_CurrentPacket[offset] >= 'A') && (m_CurrentPacket[offset] <= 'Z'))	// Decipher and set 3-digit score
					{lTempScore[0]='1';lTempScore[1]=(m_CurrentPacket[offset]-0x11);lTempScore[2]=m_CurrentPacket[offset+1];lTempScore[3]=0;}

				else	// Set literal 2-digit score
					{lTempScore[0]=m_CurrentPacket[offset];lTempScore[1]=m_CurrentPacket[offset+1];lTempScore[2]=0;}

				// Fix zeros and update
				while(lTempScore[0]==' ' || lTempScore[0]=='0') {lTempScore[0]=lTempScore[1];lTempScore[1]=lTempScore[2];lTempScore[2]=0;}
				CopyStringIfChanged2(GetParsedElement(i), lTempScore, strlen(lTempScore));
			}
			else if (i < shot)	// The rest - if any
			{
				CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], GetFieldWidth(i));
			}
			else if (i == shot)	// Shot clock
			{
				char lTempShot[3];
				strncpy(lTempShot, &m_CurrentPacket[offset], GetFieldWidth(i));
				lTempShot[GetFieldWidth(i)] = 0;

				if (lTempShot[0] == '0')	// Fix leading zero
					lTempShot[0] = ' ';

				CopyStringIfChanged2(GetParsedElement(i), lTempShot, GetFieldWidth(i));
			}
			else
				ret = false;
			
			assert(ret);
			offset += (GetFieldWidth(i) + GetFieldOffset(i+1));
		}
	}
 	return ret;
}

void BoardWhiteWay::SetSport(const char *Sport)
{
	if (_strnicmp(Sport, "Bask", 4)==0)
	{
		m_Sport=eBasketball;
		m_Basketball.SetSport(Sport);
	}
	else 
		assert (false);
}

unsigned BoardWhiteWay::GetNumBytesToRead(void)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetNumBytesToRead();
		break;
	default:
		assert(false);
	}
	return ret;
}


BoardWhiteWay::ParsedElement &BoardWhiteWay::GetParsedElement(size_t index)
{
	ParsedElement *ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=&m_Basketball.GetParsedElement(index);
		break;
	default:
		assert(false);
	}
	return *ret;
}


const char * BoardWhiteWay::GetBoardNextChangedKey(void)
{
	const char *ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetBoardNextChangedKey();
		break;
	default:
		assert(false);
	}
	return ret;
}


const char * BoardWhiteWay::GetBoardValue(const char key[])
{
	const char *ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetBoardValue(key);
		break;
	default:
		assert(false);
	}
	return ret;
}

const wchar_t * BoardWhiteWay::GetBoardTestFileName(const wchar_t * FileName)
{	
	HKEY hKey;
	DWORD retCode = 0;
	DWORD BufferSize = MAX_PATH;
	wchar_t val[300];

	retCode=(::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Wow6432Node\\Newtek\\LiveText",
		NULL,
		KEY_READ,
		&hKey));
	if (retCode == ERROR_SUCCESS)
	{
		retCode=(::RegGetValue(	hKey,
			NULL,
			L"Directory",
			RRF_RT_ANY,
			NULL,
			LPBYTE(&val),
			&BufferSize));
		wcscat(val, cwsz_Remainder);
		FileName = val;
	}
	else
		FileName = NULL;
	// 		assert(false);

	RegCloseKey(hKey);

	return FileName;
}

bool BoardWhiteWay::CheckPacket(void)
{
	// May add future data checks here, but for now, just check the packet checksum and return.
	bool ret=CheckCheckSum();
	return ret;
}

bool BoardWhiteWay::CheckCheckSum(void)
{
	bool ret = false;	//TODO: implement
	// 	//WhiteWay supplies a 1 byte cumulative checksum that is: AND with 0x7f, if result is less than 0x20, add 0x20.

	// 	//This checks the checksum against the data (excluding start & end chars & the checksum itself)
	// 	//  and returns false if it fails
	// 	char * buf = m_CurrentPacket;
	// 	int start = 0;
	// 	int stop = 20;
	// 	char low = NULL;
	// 	char high = NULL;
	// 	int	iIndex;
	// 	unsigned int uiChecksum;
	// 	const unsigned char	ucaHexChar[17] = { "0123456789ABCDEF" };
	// 
	// 	// calculate checksum
	// 	for (iIndex=start,uiChecksum=0; iIndex<=stop; iIndex++)
	// 		uiChecksum += (unsigned int)buf[iIndex];
	// 
	// 	// AND total with FF to isolate low byte
	// 	uiChecksum &= 0x7f;
	// 	if (uiChecksum < 0x20)
	// 		uiChecksum += 0x20;
	// 
	// 	// convert low nibble to hex
	// 	low = ucaHexChar[uiChecksum & 0x0f];
	// 
	// 	// convert high nibble to hex
	// 	high = ucaHexChar[uiChecksum >> 4];
	// 
	// 	if (low == m_CurrentPacket[29] && high == m_CurrentPacket[28])
	// 		ret = true;
	// 	else
	// 		ret = false;
	ret = true;
	return ret;
}

/*******************************************************************************************************/
/*											ParseReg_WhiteWay												*/
/*******************************************************************************************************/

ParseReg_WhiteWay::ParseReg_WhiteWay(const char * BoardName) : ParseReg(BoardName)
{
}

ParseReg_WhiteWay::~ParseReg_WhiteWay()
{
	for (size_t i=0;i<m_PlugIn.size();i++)
	{
		assert(m_PlugIn[i].keyName==NULL);
		m_PlugIn[i].board.StopUpdates();
		//This currently has no effect... may want to remove
		m_PlugIn[i].board.Destroy();
		HANDLE h = m_PlugIn[i].board.GetHandle();
		ResetEvent(h);
	}
}

HANDLE ParseReg_WhiteWay::live_updates_start( void )
{
	HANDLE h = CreateEvent (NULL,false,false,NULL);
	for (size_t i=0;i<m_PlugIn.size();i++)
	{
		//m_PlugIn[i].Initialize();  //most likely will not need this
		m_PlugIn[i].board.SetHandle(h);
		m_PlugIn[i].board.StartUpdates();
	}
	return h;
}

//Unfortunately the current convention shows the live_updates_get_changed_key in the dropdown menu so we cannot currently embed the port into the
//key.  An assert has been put in place to assume that there are no more than one instance of any SerialBase2 plugin.  If we find users that indeed
//run multiple decks of the same brand then we can enable this and resolve a new mechanism that allows to handle "UI" keys 
//(or some other type of solution)
//  [4/22/2010 James]
#undef __SupportMultipleDecks__

const wchar_t *ParseReg_WhiteWay::live_updates_get_changed_key( void )
{
	const wchar_t * ret;
#ifdef __SupportMultipleDecks__
	for (size_t i=0;i<m_PlugIn.size();i++)
	{
		//Borrowing the ret
		ret=m_PlugIn[i].board.GetNextChangedKey();
		if (ret)
		{
			assert(m_PlugIn[i].keyName==NULL);
			if (m_PlugIn[i].keyName)
				free(m_PlugIn[i].keyName);  
			//No need to set to NULL since it is being assigned below

			//TODO: check to see if it already exists
			m_PlugIn[i].keyName = (wchar_t *)_alloca((wcslen(ret)+ 5) * sizeof(wchar_t));
			//size_t x = wcslen(ret); /// testing
			swprintf(m_PlugIn[i].keyName, L"%3.3d_%s", m_PlugIn[i].board.GetInfo().Port, ret);
			//m_BufferPortKey[7] = 'X'; /// testing
			ret=m_PlugIn[i].keyName;
		}
	}
#else
	assert (m_PlugIn.size()<2);

	if (m_PlugIn.size())
		ret=m_PlugIn[0].board.GetNextChangedKey();
	else
	{
		ret = NULL;
		//DebugBreak();
	}
#endif
	return ret;
}


ParseReg_WhiteWay::Plugin_Element *ParseReg_WhiteWay::GetPlugInElement(const wchar_t key[])
{
#ifdef __SupportMultipleDecks__

	wchar_t *Buffer=(wchar_t *)_alloca((wcslen(key)+1)*sizeof(wchar_t));
	wcscpy(Buffer,key);
	size_t Port;
	wchar_t *Name=SerialBase2::ParseKey(key,Buffer,Port);
	Plugin_Element *ret=NULL;
	for (size_t i=0;i<m_PlugIn.size();i++)
	{
		if ((m_PlugIn[i].board.GetInfo().Port==Port) && (wcscmp(m_PlugIn[i].keyName,key)==0))
		{
			ret=&m_PlugIn[i];
			break;
		}
	}
	return ret;

#else

	Plugin_Element *ret=NULL;
	if (wcsnicmp(key,L"WW",2)==0)
	{
		if (m_PlugIn.size())
			ret=&m_PlugIn[0];
	}
	return ret;
#endif
}

//TODO: ensure free is called (trace from exported function) to avoid memory leaks. 
void ParseReg_WhiteWay::live_updates_release_changed_key( const wchar_t key[] )
{
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin && plugin->keyName)
	{
		free(plugin->keyName);
		plugin->keyName=NULL;
	}
}

const wchar_t *ParseReg_WhiteWay::live_updates_get_value( const wchar_t key[] )
{
	const wchar_t * ret=NULL;
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin)
		ret=plugin->board.GetValue(key);

	return ret;
}

void ParseReg_WhiteWay::live_updates_release_value( const wchar_t value[] )
{
}

void ParseReg_WhiteWay::AddSerialInfo(const SerialBase2_SBinfo &info)
{
	Plugin_Element NewInstance;
	NewInstance.keyName=NULL;
	if (stricmp(info.Board.c_str(),cwsz_PlugInName)==0)
	{
		NewInstance.board.SetInfo(info);
		m_PlugIn.push_back(NewInstance);
	}
}
