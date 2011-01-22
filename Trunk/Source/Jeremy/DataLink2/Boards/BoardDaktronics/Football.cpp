#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace Daktronics
		{
#include "Football.h"
		}
	}
}

using namespace PlugIn::DataLink::Daktronics;
using namespace PlugIn::DataLink::Daktronics::Football;

  /*******************************************************************************************************/
 /*										BoardDaktronics::Football										*/
/*******************************************************************************************************/

size_t BoardDaktronics::Football::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
		case eFootball_6601:
			ret=c_Football_6601[index].width;
			break;
		case eFootball_6611:
			ret=c_Football_6611[index].width;
			break;
		case eFootball_6103:
			ret=c_Football_6103[index].width;
			break;
		case eFootball_6104:
			ret=c_Football_6104[index].width;
			break;
		case eFootball_6105:
			ret=c_Football_6105[index].width;
			break;
		case eFootball_6501:
			ret=c_Football_6501[index].width;
			break;
		case eFootball_6604:
			ret=c_Football_6604[index].width;
			break;
		case eFootball_6612:
			ret=c_Football_6612[index].width;
			break;
		default:
			assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Football::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eFootball_6601:
		ret=c_Football_6601[index].offset;
		break;
	case eFootball_6611:
		ret=c_Football_6611[index].offset;
		break;
	case eFootball_6103:
		ret=c_Football_6103[index].offset;
		break;
	case eFootball_6104:
		ret=c_Football_6104[index].offset;
		break;
	case eFootball_6105:
		ret=c_Football_6105[index].offset;
		break;
	case eFootball_6501:
		ret=c_Football_6501[index].offset;
		break;
	case eFootball_6604:
		ret=c_Football_6604[index].offset;
		break;
	case eFootball_6612:
		ret=c_Football_6612[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Football::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
		case eFootball_6601:
			ret=COUNTOF(c_Football_6601);
			break;
		case eFootball_6611:
			ret=COUNTOF(c_Football_6611);
			break;
		case eFootball_6103:
			ret=COUNTOF(c_Football_6103);
			break;
		case eFootball_6104:
			ret=COUNTOF(c_Football_6104);
			break;
		case eFootball_6105:
			ret=COUNTOF(c_Football_6105);
			break;
		case eFootball_6501:
			ret=COUNTOF(c_Football_6501);
			break;
		case eFootball_6604:
			ret=COUNTOF(c_Football_6604);
			break;
		case eFootball_6612:
			ret=COUNTOF(c_Football_6612);
			break;
		default:
			assert(false);
	}
	return ret;
}

BoardDaktronics::Football::Football()
{
	m_pElement=NULL;

	for (m_SportType=0;m_SportType<eNumGameTypes;m_SportType++)
	{
		unsigned msgLength = 0;
		for (unsigned j = 0; j < GetNumFields(); j++)
			msgLength += (GetFieldWidth(j) + GetFieldOffset(j));
		
		// 4 bytes added for bom(1), eom(1), and checksum(2) characters.
		m_PacketLength_cache.push_back(msgLength + c_NoFormatBytes);
	}
	m_SportType=eFootball_6601;
}

void BoardDaktronics::Football::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardDaktronics::Football::~Football()
{
	CloseResources();
}

void BoardDaktronics::Football::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardDaktronics::Football::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Football_6601")==0)
		SetSportType(Daktronics::Football::eFootball_6601);
	else if (stricmp(Sport,"Football_6611")==0)
		SetSportType(Daktronics::Football::eFootball_6611);
	else if (stricmp(Sport,"Football_6103")==0)
		SetSportType(Daktronics::Football::eFootball_6103);
	else if (stricmp(Sport,"Football_6104")==0)
		SetSportType(Daktronics::Football::eFootball_6104);
	else if (stricmp(Sport,"Football_6105")==0)
		SetSportType(Daktronics::Football::eFootball_6105);
	else if (stricmp(Sport,"Football_6501")==0)
		SetSportType(Daktronics::Football::eFootball_6501);
	else if (stricmp(Sport,"Football_6604")==0)
		SetSportType(Daktronics::Football::eFootball_6604);
	else if (stricmp(Sport,"Football_6612")==0)
		SetSportType(Daktronics::Football::eFootball_6612);
	else
		SetSportType(Daktronics::Football::eFootball_6601);
}

unsigned BoardDaktronics::Football::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardDaktronics::ParsedElement &BoardDaktronics::Football::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardDaktronics::Football::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
		case eFootball_6601:
			ret=c_Football_6601[index].name;
			break;
		case eFootball_6611:
			ret=c_Football_6611[index].name;
			break;
		case eFootball_6103:
			ret=c_Football_6103[index].name;
			break;
		case eFootball_6104:
			ret=c_Football_6104[index].name;
			break;
		case eFootball_6105:
			ret=c_Football_6105[index].name;
			break;
		case eFootball_6501:
			ret=c_Football_6501[index].name;
			break;
		case eFootball_6604:
			ret=c_Football_6604[index].name;
			break;
		case eFootball_6612:
			ret=c_Football_6612[index].name;
			break;
		default:
			assert(false);
	}
	return ret;
}

const char *BoardDaktronics::Football::GetBoardNextChangedKey(void)
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

const char * BoardDaktronics::Football::GetBoardValue(const char key[])
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

