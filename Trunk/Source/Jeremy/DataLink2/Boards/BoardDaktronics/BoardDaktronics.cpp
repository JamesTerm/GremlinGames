// BoardDaktronics.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::Daktronics;



  /*******************************************************************************************************/
 /*											BoardDaktronics												*/
/*******************************************************************************************************/

BoardDaktronics::BoardDaktronics(void)
{
}

BoardDaktronics::~BoardDaktronics(void)
{
}

void BoardDaktronics::SetInfo(const SerialBase2_SBinfo &info)
{
	SetSport(info.Sport.c_str());
	__super::SetInfo(info);
}



size_t BoardDaktronics::GetFieldWidth(size_t index)
{
	unsigned ret;
	switch (m_Sport)
	{
		case eBasketball:
			ret=m_Basketball.GetFieldWidth(index);
			break;
		case eFootball:
			ret=m_Football.GetFieldWidth(index);
			break;
		case eBaseball:
			ret=m_Baseball.GetFieldWidth(index);
			break;
		case eVolleyball:
			ret=m_Volleyball.GetFieldWidth(index);
			break;
		case eSoccer:
			ret=m_Soccer.GetFieldWidth(index);
			break;
		case eHockey:
			ret=m_Hockey.GetFieldWidth(index);
			break;
		default:
			assert(false);
	}
	return ret;
}

size_t BoardDaktronics::GetFieldOffset(size_t index)
{
	unsigned ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetFieldOffset(index);
		break;
	case eFootball:
		ret=m_Football.GetFieldOffset(index);
		break;
	case eBaseball:
		ret=m_Baseball.GetFieldOffset(index);
		break;
	case eVolleyball:
		ret=m_Volleyball.GetFieldOffset(index);
		break;
	case eSoccer:
		ret=m_Soccer.GetFieldOffset(index);
		break;
	case eHockey:
		ret=m_Hockey.GetFieldOffset(index);
		break;
	default:
		assert(false);
	}
	return ret;
}


size_t BoardDaktronics::GetNumFields()
{
	unsigned ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetNumFields();	
		break;
	case eFootball:
		ret=m_Football.GetNumFields();
		break;
	case eBaseball:
		ret=m_Baseball.GetNumFields();
		break;
	case eVolleyball:
		ret=m_Volleyball.GetNumFields();
		break;
	case eSoccer:
		ret=m_Soccer.GetNumFields();
		break;
	case eHockey:
		ret=m_Hockey.GetNumFields();
		break;
	default:
		assert(false);
	}
	return ret;
}

void BoardDaktronics::SetSport(const char *Sport)
{
	if (_strnicmp(Sport, "Bask", 4)==0)
	{
		m_Sport=eBasketball;
		m_Basketball.SetSport(Sport);
	}
	else if (_strnicmp(Sport, "Foot", 4)==0)
	{
		m_Sport=eFootball;
		m_Football.SetSport(Sport);
	}
	else if (_strnicmp(Sport, "Base", 4)==0)
	{
		m_Sport=eBaseball;
		m_Baseball.SetSport(Sport);
	}
	else if (_strnicmp(Sport, "Voll", 4)==0)
	{
		m_Sport=eVolleyball;
		m_Volleyball.SetSport(Sport);
	}
	else if (_strnicmp(Sport, "Socc", 4)==0)
	{
		m_Sport=eSoccer;
		m_Soccer.SetSport(Sport);
	}
	else if (_strnicmp(Sport, "Hock", 4)==0)
	{
		m_Sport=eHockey;
		m_Hockey.SetSport(Sport);
	}
	else 
		assert (false);
}

unsigned BoardDaktronics::GetNumBytesToRead(void)
{
	unsigned ret;
	switch (m_Sport)
	{
		case eBasketball:
			ret=m_Basketball.GetNumBytesToRead();
			break;
		case eFootball:
			ret=m_Football.GetNumBytesToRead();
			break;
		case eBaseball:
			ret=m_Baseball.GetNumBytesToRead();
			break;
		case eVolleyball:
			ret=m_Volleyball.GetNumBytesToRead();
			break;
		case eSoccer:
			ret=m_Soccer.GetNumBytesToRead();
			break;
		case eHockey:
			ret=m_Hockey.GetNumBytesToRead();
			break;
		default:
			assert(false);
	}
	return ret;
}


BoardDaktronics::ParsedElement &BoardDaktronics::GetParsedElement(size_t index)
{
	ParsedElement *ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=&m_Basketball.GetParsedElement(index);
		break;
	case eFootball:
		ret=&m_Football.GetParsedElement(index);
		break;
	case eBaseball:
		ret=&m_Baseball.GetParsedElement(index);
		break;
	case eVolleyball:
		ret=&m_Volleyball.GetParsedElement(index);
		break;
	case eSoccer:
		ret=&m_Soccer.GetParsedElement(index);
		break;
	case eHockey:
		ret=&m_Hockey.GetParsedElement(index);
		break;
	default:
		assert(false);
	}
	return *ret;
}


const char * BoardDaktronics::GetBoardNextChangedKey(void)
{
	const char *ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetBoardNextChangedKey();
		break;
	case eFootball:
		ret=m_Football.GetBoardNextChangedKey();
		break;
	case eBaseball:
		ret=m_Baseball.GetBoardNextChangedKey();
		break;
	case eVolleyball:
		ret=m_Volleyball.GetBoardNextChangedKey();
		break;
	case eSoccer:
		ret=m_Soccer.GetBoardNextChangedKey();
		break;
	case eHockey:
		ret=m_Hockey.GetBoardNextChangedKey();
		break;
	default:
		assert(false);
	}
	return ret;
}


const char * BoardDaktronics::GetBoardValue(const char key[])
{
	const char *ret;
	switch (m_Sport)
	{
	case eBasketball:
		ret=m_Basketball.GetBoardValue(key);
		break;
	case eFootball:
		ret=m_Football.GetBoardValue(key);
		break;
	case eBaseball:
		ret=m_Baseball.GetBoardValue(key);
		break;
	case eVolleyball:
		ret=m_Volleyball.GetBoardValue(key);
		break;
	case eSoccer:
		ret=m_Soccer.GetBoardValue(key);
		break;
	case eHockey:
		ret=m_Hockey.GetBoardValue(key);
		break;
	default:
		assert(false);
	}
	return ret;
}

const wchar_t * BoardDaktronics::GetBoardTestFileName(const wchar_t * FileName)
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
	// 		assert(false);

	RegCloseKey(hKey);

	return FileName;
}

bool BoardDaktronics::CheckPacket(void)
{
	// May add future data checks here, but for now, just check the packet checksum and return.
	bool ret=CheckCheckSum();
	return ret;
}

bool BoardDaktronics::CheckCheckSum(void)
{
	bool ret = false;
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

	if (low == m_CurrentPacket[29] && high == m_CurrentPacket[28])
		ret = true;
	else
		ret = false;

	return ret;
}

  /*******************************************************************************************************/
 /*											ParseReg_Daktronics											*/
/*******************************************************************************************************/

ParseReg_Daktronics::ParseReg_Daktronics(const char * BoardName) : ParseReg(BoardName)
{
}

ParseReg_Daktronics::~ParseReg_Daktronics()
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

HANDLE ParseReg_Daktronics::live_updates_start( void )
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

const wchar_t *ParseReg_Daktronics::live_updates_get_changed_key( void )
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


ParseReg_Daktronics::Plugin_Element *ParseReg_Daktronics::GetPlugInElement(const wchar_t key[])
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

		if (wcsnicmp(key,L"dak",3)==0)
		{
			if (m_PlugIn.size())
				ret=&m_PlugIn[0];
		}

	return ret;
	#endif
}

//TODO: ensure free is called (trace from exported function) to avoid memory leaks. 
void ParseReg_Daktronics::live_updates_release_changed_key( const wchar_t key[] )
{
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin && plugin->keyName)
	{
		free(plugin->keyName);
		plugin->keyName=NULL;
	}
}

const wchar_t *ParseReg_Daktronics::live_updates_get_value( const wchar_t key[] )
{
	const wchar_t * ret=NULL;
	Plugin_Element *plugin=GetPlugInElement(key);
	if (plugin)
		ret=plugin->board.GetValue(key);

	return ret;
}

void ParseReg_Daktronics::live_updates_release_value( const wchar_t value[] )
{
}

void ParseReg_Daktronics::AddSerialInfo(const SerialBase2_SBinfo &info)
{
	Plugin_Element NewInstance;
	NewInstance.keyName=NULL;
	if (stricmp(info.Board.c_str(),cwsz_PlugInName)==0)
	{
		NewInstance.board.SetInfo(info);
		m_PlugIn.push_back(NewInstance);
	}
}
