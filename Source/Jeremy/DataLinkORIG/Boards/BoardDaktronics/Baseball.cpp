#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace Daktronics
		{
#include "Baseball.h"
		}
	}
}

using namespace PlugIn::DataLink::Daktronics;
using namespace PlugIn::DataLink::Daktronics::Baseball;


  /*******************************************************************************************************/
 /*										BoardDaktronics::Baseball										*/
/*******************************************************************************************************/

size_t BoardDaktronics::Baseball::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
		case eBaseball_5501:
			ret=c_Baseball_5501[index].width;
			break;
		case eBaseball_5601:
			ret=c_Baseball_5601[index].width;
			break;
		case eBaseball_5602:
			ret=c_Baseball_5602[index].width;
			break;
		default:
			assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Baseball::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eBaseball_5501:
		ret=c_Baseball_5501[index].offset;
		break;
	case eBaseball_5601:
		ret=c_Baseball_5601[index].offset;
		break;
	case eBaseball_5602:
		ret=c_Baseball_5602[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Baseball::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
		case eBaseball_5501:
			ret=COUNTOF(c_Baseball_5501);
			break;
		case eBaseball_5601:
			ret=COUNTOF(c_Baseball_5601);
			break;
		case eBaseball_5602:
			ret=COUNTOF(c_Baseball_5602);
			break;
		default:
			assert(false);
	}
	return ret;
}

BoardDaktronics::Baseball::Baseball()
{
	m_pElement=NULL;

	for (m_SportType=0;m_SportType<eNumGameTypes;m_SportType++)
	{
		size_t msgLength = 0;
		for (size_t j = 0; j < GetNumFields(); j++)
			msgLength += (GetFieldWidth(j) + GetFieldOffset(j));
		
		// 4 bytes added for bom(1), eom(1), and checksum(2) characters.
		m_PacketLength_cache.push_back(msgLength + c_NoFormatBytes);
	}
	m_SportType=eBaseball_5501;
}

void BoardDaktronics::Baseball::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardDaktronics::Baseball::~Baseball()
{
	CloseResources();
}

void BoardDaktronics::Baseball::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardDaktronics::Baseball::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Baseball_5501")==0)
		SetSportType(Daktronics::Baseball::eBaseball_5501);
	else if (stricmp(Sport,"Baseball_5601")==0)
		SetSportType(Daktronics::Baseball::eBaseball_5601);
	else if (stricmp(Sport,"Baseball_5602")==0)
		SetSportType(Daktronics::Baseball::eBaseball_5602);
	else
		SetSportType(Daktronics::Baseball::eBaseball_5501);
//		assert (false);
}

unsigned BoardDaktronics::Baseball::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardDaktronics::ParsedElement &BoardDaktronics::Baseball::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardDaktronics::Baseball::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
		case eBaseball_5501:
			ret=c_Baseball_5501[index].name;
			break;
		case eBaseball_5601:
			ret=c_Baseball_5601[index].name;
			break;
		case eBaseball_5602:
			ret=c_Baseball_5602[index].name;
			break;
		default:
			assert(false);
	}
	return ret;
}

const char *BoardDaktronics::Baseball::GetBoardNextChangedKey(void)
{
	const char *ret=NULL;
	for (unsigned i=0;i<GetNumFields();i++)
	{
		if (GetParsedElement(i).ElementUpdated)
		{
			m_pElement[i].ElementUpdated=false;
			ret=GetFieldName(i);
			break;
		}
	}
	return ret;
}

const char * BoardDaktronics::Baseball::GetBoardValue(const char key[])
{
	const char *ret=NULL;
	for (unsigned i=0;i<GetNumFields();i++)
	{
		if (_stricmp(GetFieldName(i),&key[0])==0)
		{
			ret=m_pElement[i].ElementName.c_str();
			break;
		}
	}
	assert(ret);
	return ret;
}

