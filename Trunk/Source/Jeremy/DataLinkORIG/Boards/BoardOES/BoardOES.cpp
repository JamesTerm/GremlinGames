// BoardOES.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::OES;



  /*******************************************************************************************************/
 /*											BoardOES													*/
/*******************************************************************************************************/

BoardOES::BoardOES(void)
{
}

BoardOES::~BoardOES(void)
{
}

void BoardOES::SetInfo(const SerialBase2_SBinfo &info)
{
	SetSport(info.Sport.c_str());
	__super::SetInfo(info);
}



size_t BoardOES::GetFieldWidth(size_t index)
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

size_t BoardOES::GetFieldOffset(size_t index)
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


size_t BoardOES::GetNumFields()
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

bool BoardOES::ParsePacket(void)
{
	//TODO: This ParsePacket may not work quite right with sports other than OES Basketball.
	bool ret = false;

	if ((m_CurrentPacket) && (m_CurrentPacket[0]==0x02))
	{
		size_t offset = GetFieldOffset(0);

		const size_t shot = (GetNumFields() - 4);
		const size_t mins = (GetNumFields() - 3);
		const size_t secs = (GetNumFields() - 2);
		const size_t tens = (GetNumFields() - 1);

		for (size_t i = 0; i < GetNumFields(); i++)
		{
			// Messy stuff in here. Basically, have to check for multiple values and adjust for possible hex values
			//  and ignore colons which indicate a null such as in the case of '0' in the time '05.1'.
			if (i == 0)
			{
				char lTempClock[8];

				if ((m_CurrentPacket[3] < 0) && (m_CurrentPacket[4] < 0))
				{
					if (m_CurrentPacket[2] == 0x3a || m_CurrentPacket[2] == (char) 0xba)
						lTempClock[0] = ' ';
					else if (m_CurrentPacket[2] > '0' && m_CurrentPacket[2] < '9')
						lTempClock[0] = m_CurrentPacket[2];
					else
						lTempClock[0] = m_CurrentPacket[2] - (char) - 0x80;

					lTempClock[1] = m_CurrentPacket[3] - (char) 0x80;
					lTempClock[2] = ':';
					lTempClock[3] = m_CurrentPacket[4] - (char) 0x80;
					lTempClock[4] = m_CurrentPacket[5];
					lTempClock[5] = lTempClock[6] = ' ';
					lTempClock[7] = 0;
				}
				else if ((m_CurrentPacket[4] < 0) && (m_CurrentPacket[5] == ':'))
				{
					lTempClock[0] = lTempClock[1] = lTempClock[2] = ' ';
					if (m_CurrentPacket[2] == 0x3a || m_CurrentPacket[2] == (char) 0xba)
						lTempClock[3] = ' ';
					else if (m_CurrentPacket[2] < '0' || m_CurrentPacket[2] > '9')
						lTempClock[3] = m_CurrentPacket[2] - (char) 0x80;
					else
						lTempClock[3] = m_CurrentPacket[2];

					lTempClock[4] = m_CurrentPacket[3];
					lTempClock[5] = '.';
					lTempClock[6] = m_CurrentPacket[4] - (char) 0x80;
					lTempClock[7] = 0;
				}
				else
				{	
					ret = false;
					assert(ret);
				}

				ret = true;
				CopyStringIfChanged2(GetParsedElement(mins), &lTempClock[0], 2); // Copy minutes if it changed.
				CopyStringIfChanged2(GetParsedElement(secs), &lTempClock[3], 2); // Copy seconds if it changed.
				CopyStringIfChanged2(GetParsedElement(tens), &lTempClock[6], 1); // Copy tenths if it changed.

				CopyStringIfChanged2(GetParsedElement(i), lTempClock, strlen(lTempClock));
			}
			else if (i == shot)	// Shot clock - replace colons with spaces.
			{
				char lTempShot[3];
				strncpy(lTempShot, &m_CurrentPacket[offset], GetFieldWidth(i));

				if (lTempShot[0] == ':')
				{
					lTempShot[0] = ' ';
					if (lTempShot[1] == ':')
						lTempShot[1] = ' ';
				}
				lTempShot[2] = 0;
				CopyStringIfChanged2(GetParsedElement(i), lTempShot, GetFieldWidth(i));
			}
			else if (i == 2 || i == 3)	// Scores
			{
				size_t z = i - 2;

				char lTempScore[2][4];

				for (unsigned z=0;z<2;z++)
				{
					lTempScore[z][3] = 0;

					lTempScore[z][0] = ' ';
					if (((m_CurrentPacket[7+z*2] < '0') || (m_CurrentPacket[7+z*2] > '9')) && (m_CurrentPacket[7+z*2] != ':'))
					{
						lTempScore[z][0] = '1';
						lTempScore[z][1] = m_CurrentPacket[7+z*2] - (char) 0x80;
						lTempScore[z][2] = m_CurrentPacket[8+z*2] - (char) 0x80;
					}
					else
					{
						if (m_CurrentPacket[7+z*2] == ' ')
							lTempScore[z][1] = '0';
						else
							lTempScore[z][1] = m_CurrentPacket[7+z*2];
						if (m_CurrentPacket[8+z*2] == ' ')
							lTempScore[z][2] = '0';
						else
							lTempScore[z][2] = m_CurrentPacket[8+z*2];
					}

					// If they were colons, they should be nothing
					if (m_CurrentPacket[7+z*2] == ':')
						lTempScore[z][1] = ' ';
					if (m_CurrentPacket[8+z*2] == ':')
						lTempScore[z][2] = ' ';
				}
				CopyStringIfChanged2(GetParsedElement(i), lTempScore[z], strlen(lTempScore[z]));
			}
			else if ((i >= 4 ) && (i <= 7))	// Time outs.
			{
				//The 20 second part time out fields share use with possession indicator and bonus indicators.
				//Possession is +1 and bonus is +2 making each part time out value have a range of 4 possible values.
				// If part time outs are 1, 2, or 3, then full time outs is + 0x80 and then
				//		part time out 1 = 0x35 (5)
				//		part time out 2 = 0x39 (9) etc.
				char valTO = (m_CurrentPacket[offset] - 0x80);
				switch (i)
				{
					case 4:	//HomeTOFull
						if  ((m_CurrentPacket[offset] < '0') || (m_CurrentPacket[offset] > '9'))
						{
							CopyStringIfChanged2(GetParsedElement(i), &valTO, GetFieldWidth(i));
						}
						else
						{
							CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], GetFieldWidth(i));
						}
						break;

					case 5:	//HomeTOPart
						if  ((m_CurrentPacket[(offset - 1)] < '0') || (m_CurrentPacket[(offset - 1)] > '9'))
						{
							char homePartTO = '0';
							if (m_CurrentPacket[16] == 0x34 || m_CurrentPacket[16] == 0x35 || m_CurrentPacket[16] == 0x36 || m_CurrentPacket[16] == 0x37)
								homePartTO = '1';
							else if (m_CurrentPacket[16] == 0x38 || m_CurrentPacket[16] == 0x39 || m_CurrentPacket[16] == 0x3a || m_CurrentPacket[16] == 0x3b)
								homePartTO = '2';
							else if (m_CurrentPacket[16] == 0x3c || m_CurrentPacket[16] == 0x3d || m_CurrentPacket[16] == 0x3e || m_CurrentPacket[16] == 0x3f)
								homePartTO = '3';
							//else
								//assert (false);
							CopyStringIfChanged2(GetParsedElement(i), &homePartTO, (GetFieldWidth(i) + 1));
						}
						else
						{
							CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], (GetFieldWidth(i) + 1));
						}
						break;

					case 6:	//AwayTOFull
						if  ((m_CurrentPacket[offset] < '0') || (m_CurrentPacket[offset] > '9'))
						{
							CopyStringIfChanged2(GetParsedElement(i), &valTO, GetFieldWidth(i));
						}
						else
						{
							CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], GetFieldWidth(i));
						}
						break;

					case 7:	//AwayTOPart
						if  ((m_CurrentPacket[(offset - 1)] < '0') || (m_CurrentPacket[(offset - 1)] > '9'))
						{
							char awayPartTO = '0';
							if (m_CurrentPacket[16] == 0x34 || m_CurrentPacket[16] == 0x35 || m_CurrentPacket[16] == 0x36 || m_CurrentPacket[16] == 0x37)
								awayPartTO = '1';
							else if (m_CurrentPacket[16] == 0x38 || m_CurrentPacket[16] == 0x39 || m_CurrentPacket[16] == 0x3a || m_CurrentPacket[16] == 0x3b)
								awayPartTO = '2';
							else if (m_CurrentPacket[16] == 0x3c || m_CurrentPacket[16] == 0x3d || m_CurrentPacket[16] == 0x3e || m_CurrentPacket[16] == 0x3f)
								awayPartTO = '3';
							//else
								//assert (false);
							CopyStringIfChanged2(GetParsedElement(i), &awayPartTO, (GetFieldWidth(i) + 1));
						}
						else
						{
							CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], (GetFieldWidth(i) + 1));
						}
						break;

					default:
						assert(false);
						break;
				}
			}
			else if ((i >= 1) && (i < shot))	// All others.
			{
				CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset], GetFieldWidth(i));
			}
			offset+=(GetFieldWidth(i) + GetFieldOffset(i+1));
		}
	}
	else
	{
		ret = false;
		assert(ret);	//Something unexpected happened.
	}
	return ret;
}

void BoardOES::SetSport(const char *Sport)
{
	if (_strnicmp(Sport, "Bask", 4)==0)
	{
		m_Sport=eBasketball;
		m_Basketball.SetSport(Sport);
	}
	else 
		assert (false);
}

unsigned BoardOES::GetNumBytesToRead(void)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eBasketball:
#ifndef _DEBUG
		ret=m_Basketball.GetNumBytesToRead();
#else
		if (m_UseTextFile)	// The provided text dumps have cr+lf (0x0d and 0x0a) instead of just cr (0x0d) like the docs mention.
			ret=24;			//  Will assume that the actual scoreboard outputs only 0x0d until proven otherwise. This would shorten
		//  the message length by one of course.
		else ret=m_Basketball.GetNumBytesToRead();
#endif
		break;
	default:
		assert(false);
	}
	return ret;
}


BoardOES::ParsedElement &BoardOES::GetParsedElement(size_t index)
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


const char * BoardOES::GetBoardNextChangedKey(void)
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


const char * BoardOES::GetBoardValue(const char key[])
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

const wchar_t * BoardOES::GetBoardTestFileName(const wchar_t * FileName)
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

bool BoardOES::CheckPacket(void)
{
	// May add future data checks here, but for now, just check the packet checksum and return.
	bool ret=CheckCheckSum();
	return ret;
}

bool BoardOES::CheckCheckSum(void)
{
 	bool ret = false;	//TODO: implement
// 	//OES supplies a 1 byte cumulative checksum that is: AND with 0x7f, if result is less than 0x20, add 0x20.

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
 /*											ParseReg_OES												*/
/*******************************************************************************************************/

ParseReg_OES::ParseReg_OES(const char * BoardName) : ParseReg(BoardName)
{
}

ParseReg_OES::~ParseReg_OES()
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

HANDLE ParseReg_OES::live_updates_start( void )
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

const wchar_t *ParseReg_OES::live_updates_get_changed_key( void )
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


ParseReg_OES::Plugin_Element *ParseReg_OES::GetPlugInElement(const wchar_t key[])
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
	if (wcsnicmp(key,L"oes",3)==0)
	{
		if (m_PlugIn.size())
			ret=&m_PlugIn[0];
	}
	return ret;
#endif
}

//TODO: ensure free is called (trace from exported function) to avoid memory leaks. 
void ParseReg_OES::live_updates_release_changed_key( const wchar_t key[] )
{
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin && plugin->keyName)
	{
		free(plugin->keyName);
		plugin->keyName=NULL;
	}
}

const wchar_t *ParseReg_OES::live_updates_get_value( const wchar_t key[] )
{
	const wchar_t * ret=NULL;
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin)
		ret=plugin->board.GetValue(key);

	return ret;
}

void ParseReg_OES::live_updates_release_value( const wchar_t value[] )
{
}

void ParseReg_OES::AddSerialInfo(const SerialBase2_SBinfo &info)
{
	Plugin_Element NewInstance;
	NewInstance.keyName=NULL;
	if (stricmp(info.Board.c_str(),cwsz_PlugInName)==0)
	{
		NewInstance.board.SetInfo(info);
		m_PlugIn.push_back(NewInstance);
	}
}
