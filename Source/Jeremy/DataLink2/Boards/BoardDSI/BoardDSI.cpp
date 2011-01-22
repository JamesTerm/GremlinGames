// BoardDSI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::DSI;

  /*******************************************************************************************************/
 /*											BoardDSI													*/
/*******************************************************************************************************/

BoardDSI::BoardDSI(void)
{
}

BoardDSI::~BoardDSI(void)
{
}


void BoardDSI::SetInfo(const SerialBase2_SBinfo &info)
{
	SetSport(info.Sport.c_str());
	__super::SetInfo(info);
}

size_t BoardDSI::GetFieldWidth(size_t index)
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

size_t BoardDSI::GetFieldOffset(size_t index)
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

size_t BoardDSI::GetNumFields()
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

size_t BoardDSI::ManualAdvance(void)
{
	size_t ret = 0;

	for (size_t i=1;i<GetNumBytesToRead();i++)
	{
		unsigned char parsechar = (char)(m_FileInMemory[m_PlaceInFile+i]);
		unsigned char eomchar = GetEndofMessageChar();

		if (parsechar == eomchar)
		{
			ret = i+1;
			break;
		}
	}
	return ret;
}

bool BoardDSI::ParsePacket(void)
{
	// Customized ParsePacket because DSI messages are weird.
	// DSI has a header of ### (026 - clock, 027 - shot, 028 - horn) before each value followed by its own eom marker.

	bool ret = false;

	if ((m_CurrentPacket) || (m_CurrentPacket[0]))
	{
		char headerBuf[4];
		strncpy(headerBuf, m_CurrentPacket + 1, 3);
		headerBuf[3] = 0;
		size_t checkHeader = atoi(headerBuf);

		const size_t mins = (GetNumFields() - 3);
		const size_t secs = (GetNumFields() - 2);
		const size_t tens = (GetNumFields() - 1);

		char lTempClock[8] = "\0";
		char lTempShot[3] = "\0";

		switch((int)checkHeader)
		{
		case 26:
			// Game Clock
			// Fix leading zeros:
			strncpy(lTempClock, &m_CurrentPacket[4], 7);
			if ((lTempClock[0] != '0') || (lTempClock[1] != '0'))
			{
				lTempClock[5] = lTempClock[6] = ' ';	// Over 1 minute and no tenth needed.
			}
			lTempClock[7] = 0;	// Zero term

			// Fix leading zeros.
			if ((lTempClock[0] == ' ') || (lTempClock[0] == '0'))	//Mm:ss.t
			{
				lTempClock[0] = ' ';
				if (lTempClock[1] == '0')	//mM:ss.t
				{
					lTempClock[1] = ' ';
					lTempClock[2] = ' ';	// colon
					if (lTempClock[3] == '0')	//mm:Ss.t
					{
						lTempClock[3] = ' ';
					}
				}
			}
			// Set clock and individual values.
			CopyStringIfChanged2(GetParsedElement(0), lTempClock, strlen(lTempClock));

			CopyStringIfChanged2(GetParsedElement(mins), &lTempClock[0], 2);
			CopyStringIfChanged2(GetParsedElement(secs), &lTempClock[3], 2);
			CopyStringIfChanged2(GetParsedElement(tens), &lTempClock[6], 1);

			break;
		case 27:
			// Shot Clock
			// Fix leading zeros:
			strncpy(lTempShot, &m_CurrentPacket[4], 2);
			lTempShot[2] = 0;

			if (lTempShot[0] == '0')
			{
				lTempShot[0] = ' ';
				strncpy (&m_CurrentPacket[4], lTempShot, 2);
			}

			CopyStringIfChanged2(GetParsedElement(1), &m_CurrentPacket[4], GetFieldWidth(1));
			break;
		case 28:
			// Horn indicator - we don't care about - do nothing for now.
			break;
		default:
			break;
		}
	}
	return true;
}

void BoardDSI::SetSport(const char *Sport)
{
	if (_strnicmp(Sport, "Bask", 4)==0)
	{
		m_Sport=eBasketball;
		m_Basketball.SetSport(Sport);
	}
	else 
		assert (false);
}

unsigned BoardDSI::GetNumBytesToRead(void)
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


BoardDSI::ParsedElement &BoardDSI::GetParsedElement(size_t index)
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


const char * BoardDSI::GetBoardNextChangedKey(void)
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


const char * BoardDSI::GetBoardValue(const char key[])
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

const wchar_t * BoardDSI::GetBoardTestFileName(const wchar_t * FileName)
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
		Sleep(1);
	}
	else
		FileName = NULL;

	RegCloseKey(hKey);

	return FileName;
}

bool BoardDSI::CheckPacket(void)
{
	// May add future data checks here, but for now, just check the packet checksum and return.
	m_FieldNumber = 0;
	bool ret = false;

	if ((m_CurrentPacket) || (m_CurrentPacket[0]) || (m_CurrentPacket[1]))
	{
		char fieldNumber[4];
		strncpy(fieldNumber, m_CurrentPacket+1, 3);
		fieldNumber[3] = 0;
		int ifn = atoi(fieldNumber);
		if (ifn = 26)
		{
			m_FieldNumber = ifn;
			ret = true;
		}
		else
		{
			m_FieldNumber = 0;
			ret = false;
		}
	}
	return ret;
}

bool BoardDSI::CheckCheckSum(void)
{
	//DSI does not use a checksum.
	return true;
}

  /*******************************************************************************************************/
 /*											ParseReg_DSI												*/
/*******************************************************************************************************/

ParseReg_DSI::ParseReg_DSI(const char * BoardName) : ParseReg(BoardName)
{
}

ParseReg_DSI::~ParseReg_DSI()
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

HANDLE ParseReg_DSI::live_updates_start( void )
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

const wchar_t *ParseReg_DSI::live_updates_get_changed_key( void )
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


ParseReg_DSI::Plugin_Element *ParseReg_DSI::GetPlugInElement(const wchar_t key[])
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
	if (wcsnicmp(key,L"dsi",3)==0)
	{
		if (m_PlugIn.size())
			ret=&m_PlugIn[0];
	}
	return ret;
#endif
}

//TODO: ensure free is called (trace from exported function) to avoid memory leaks. 
void ParseReg_DSI::live_updates_release_changed_key( const wchar_t key[] )
{
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin && plugin->keyName)
	{
		free(plugin->keyName);
		plugin->keyName=NULL;
	}
}

const wchar_t *ParseReg_DSI::live_updates_get_value( const wchar_t key[] )
{
	const wchar_t * ret=NULL;
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin)
		ret=plugin->board.GetValue(key);

	return ret;
}

void ParseReg_DSI::live_updates_release_value( const wchar_t value[] )
{
}

void ParseReg_DSI::AddSerialInfo(const SerialBase2_SBinfo &info)
{
	Plugin_Element NewInstance;
	NewInstance.keyName=NULL;
	if (stricmp(info.Board.c_str(),cwsz_PlugInName)==0)
	{
		NewInstance.board.SetInfo(info);
		m_PlugIn.push_back(NewInstance);
	}
}
