// BoardTransLux.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::TransLux;

  /*******************************************************************************************************/
 /*											BoardTransLux												*/
/*******************************************************************************************************/

BoardTransLux::BoardTransLux(void)
{
}

BoardTransLux::~BoardTransLux(void)
{
}


void BoardTransLux::SetInfo(const SerialBase2_SBinfo &info)
{
	SetSport(info.Sport.c_str());
	__super::SetInfo(info);
}

size_t BoardTransLux::GetFieldWidth(size_t index)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eFootball:
		ret=m_Football.GetFieldWidth(index);
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardTransLux::GetFieldOffset(size_t index)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eFootball:
		ret=m_Football.GetFieldOffset(index);
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardTransLux::GetNumFields()
{
	unsigned ret;
	switch (m_Sport)
	{
	case eFootball:
		ret=m_Football.GetNumFields();	
		break;
	default:
		assert(false);
	}
	return ret;
}

bool BoardTransLux::ParsePacket(void)
{
	bool ret = true;

	size_t sub_msg_offset;
	size_t offset = GetFieldOffset(0);

	const size_t mins = (GetNumFields() - 3);
	const size_t secs = (GetNumFields() - 2);
	const size_t tens = (GetNumFields() - 1);

	const size_t clockLen = (GetFieldWidth(0));


	if ((m_CurrentPacket) && (m_CurrentPacket[0]))
	{
		// There are two EOM characters per message - 1 at end and 1 at the center. Sometimes we get the second half 
		//  as the first part of the message and so the packet halves are reversed. This section takes care of the
		//  cases where the message halves are reversed by using an offset.
		if ((m_CurrentPacket[0] == 0x01) && (m_CurrentPacket[1] == 0x01))
		{
			sub_msg_offset = 0; // msg properly framed
		}
		else if ((m_CurrentPacket[0] == 0x01) && (m_CurrentPacket[1] == 0x02))
		{
			sub_msg_offset = 30; // msg mis-framed
		}
		else
			sub_msg_offset = 0;

		for (size_t i = 0; i < GetNumFields(); i++)
		{
			if (i == 0)	// Game clock has changed.
			{
				// Fix the clock.
				char lClockTemp[14];
				strncpy(lClockTemp, &m_CurrentPacket[2 + sub_msg_offset], clockLen);
				lClockTemp[clockLen] = 0;	// Zero term just for fun.

				if ((lClockTemp[0] == ' ') || (lClockTemp[0] == '0'))	// Tens minutes
				{
					lClockTemp[0] = ' ';
					if (lClockTemp[1] == '0')	// Minutes and colon
					{
						lClockTemp[1] = ' ';
						lClockTemp[2] = ' ';
						if (lClockTemp[3] == '0')	// Tenths of seconds
							lClockTemp[3] = ' ';
					}
				}
				strncpy(&m_CurrentPacket[2 + sub_msg_offset], lClockTemp, clockLen);
				CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[(offset + sub_msg_offset)], GetFieldWidth(i));
			}
			// Handle the values in the first half of packet using "+ sub_msg_offset".
			else if (i < 4)
			{
				CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset + sub_msg_offset], GetFieldWidth(i));
			}
			else if (i == mins) // minutes
			{
				CopyStringIfChanged2(GetParsedElement(mins), &m_CurrentPacket[GetFieldOffset(0) + sub_msg_offset], 2);
			}
			else if (i == secs) // seconds
			{
				CopyStringIfChanged2(GetParsedElement(secs), &m_CurrentPacket[GetFieldOffset(0) + 3 + sub_msg_offset], 2);
			}
			else if (i == tens) // tenth of seconds
			{
				CopyStringIfChanged2(GetParsedElement(tens), &m_CurrentPacket[GetFieldOffset(0) + 6 + sub_msg_offset], 1);
			}
			// Handle the values in the second half of packet using "- sub_msg_offset".
			else if ((i >= 4) && (i < mins))
			{
				CopyStringIfChanged2(GetParsedElement(i), &m_CurrentPacket[offset - sub_msg_offset], GetFieldWidth(i));
			}
			else				// something is amiss...
				ret = false;	// This should never happen because of the error checking done previously in ReadFromScoreBoard.

			assert(ret);
			offset+=(GetFieldWidth(i) + GetFieldOffset(i+1));
		}
	}

	return ret;
}

void BoardTransLux::SetSport(const char *Sport)
{
	if (_strnicmp(Sport, "Foot", 4)==0)
	{
		m_Sport=eFootball;
		m_Football.SetSport(Sport);
	}
	else 
		assert (false);
}

unsigned BoardTransLux::GetNumBytesToRead(void)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eFootball:
		ret=m_Football.GetNumBytesToRead();
		break;
	default:
		assert(false);
	}
	return ret;
}


BoardTransLux::ParsedElement &BoardTransLux::GetParsedElement(size_t index)
{
	ParsedElement *ret;
	switch (m_Sport)
	{
	case eFootball:
		ret=&m_Football.GetParsedElement(index);
		break;
	default:
		assert(false);
	}
	return *ret;
}


const char * BoardTransLux::GetBoardNextChangedKey(void)
{
	const char *ret;
	switch (m_Sport)
	{
	case eFootball:
		ret=m_Football.GetBoardNextChangedKey();
		break;
	default:
		assert(false);
	}
	return ret;
}


const char * BoardTransLux::GetBoardValue(const char key[])
{
	const char *ret;
	switch (m_Sport)
	{
	case eFootball:
		ret=m_Football.GetBoardValue(key);
		break;
	default:
		assert(false);
	}
	return ret;
}

const wchar_t * BoardTransLux::GetBoardTestFileName(const wchar_t * FileName)
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
	 		assert(false);

	RegCloseKey(hKey);

	return FileName;
}

bool BoardTransLux::CheckPacket(void)
{
	// May add future data checks here, but for now, just check the packet checksum and return.
 	bool ret=CheckCheckSum();
 	return ret;
}

bool BoardTransLux::CheckCheckSum(void)
{
	//TransLux does not use a checksum.
	return true;
}

/*******************************************************************************************************/
/*											ParseReg_TransLux												*/
/*******************************************************************************************************/

ParseReg_TransLux::ParseReg_TransLux(const char * BoardName) : ParseReg(BoardName)
{
}

ParseReg_TransLux::~ParseReg_TransLux()
{
	for (size_t i=0;i<m_PlugIn.size();i++)
	{
		assert(m_PlugIn[i].keyName==NULL);
		m_PlugIn[i].board.StopUpdates();
		// ... may want to remove
		m_PlugIn[i].board.Destroy();
		HANDLE h = m_PlugIn[i].board.GetHandle();
		ResetEvent(h);
	}
}

HANDLE ParseReg_TransLux::live_updates_start( void )
{
	HANDLE h = CreateEvent (NULL,false,false,NULL);
	for (size_t i=0;i<m_PlugIn.size();i++)
	{
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

const wchar_t *ParseReg_TransLux::live_updates_get_changed_key( void )
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
		ret = NULL;
#endif
	return ret;
}


ParseReg_TransLux::Plugin_Element *ParseReg_TransLux::GetPlugInElement(const wchar_t key[])
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
	if (wcsnicmp(key,L"TLFP",3)==0)
	{
		if (m_PlugIn.size())
			ret=&m_PlugIn[0];
	}
	return ret;
#endif
}

void ParseReg_TransLux::live_updates_release_changed_key( const wchar_t key[] )
{
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin && plugin->keyName)
	{
		free(plugin->keyName);
		plugin->keyName=NULL;
	}
}

const wchar_t *ParseReg_TransLux::live_updates_get_value( const wchar_t key[] )
{
	const wchar_t * ret=NULL;
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin)
		ret=plugin->board.GetValue(key);

	return ret;
}

void ParseReg_TransLux::live_updates_release_value( const wchar_t value[] )
{
}

void ParseReg_TransLux::AddSerialInfo(const SerialBase2_SBinfo &info)
{
	Plugin_Element NewInstance;
	NewInstance.keyName=NULL;
	if (stricmp(info.Board.c_str(),cwsz_PlugInName)==0)
	{
		NewInstance.board.SetInfo(info);
		m_PlugIn.push_back(NewInstance);
	}
}
