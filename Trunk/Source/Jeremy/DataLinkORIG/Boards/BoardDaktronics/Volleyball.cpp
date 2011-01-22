#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace Daktronics
		{
#include "Volleyball.h"
		}
	}
}

using namespace PlugIn::DataLink::Daktronics;
using namespace PlugIn::DataLink::Daktronics::Volleyball;


  /*******************************************************************************************************/
 /*										BoardDaktronics::Volleyball										*/
/*******************************************************************************************************/

size_t BoardDaktronics::Volleyball::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
		case eVolleyball_2101:
			ret=c_Volleyball_2101[index].width;
			break;
		case eVolleyball_2111:
			ret=c_Volleyball_2111[index].width;
			break;
		case eVolleyball_2103:
			ret=c_Volleyball_2103[index].width;
			break;
		case eVolleyball_2104:
			ret=c_Volleyball_2104[index].width;
			break;
		case eVolleyball_2105:
			ret=c_Volleyball_2105[index].width;
			break;
		default:
			assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Volleyball::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eVolleyball_2101:
		ret=c_Volleyball_2101[index].offset;
		break;
	case eVolleyball_2111:
		ret=c_Volleyball_2111[index].offset;
		break;
	case eVolleyball_2103:
		ret=c_Volleyball_2103[index].offset;
		break;
	case eVolleyball_2104:
		ret=c_Volleyball_2104[index].offset;
		break;
	case eVolleyball_2105:
		ret=c_Volleyball_2105[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Volleyball::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
		case eVolleyball_2101:
			ret=COUNTOF(c_Volleyball_2101);
			break;
		case eVolleyball_2111:
			ret=COUNTOF(c_Volleyball_2111);
			break;
		case eVolleyball_2103:
			ret=COUNTOF(c_Volleyball_2103);
			break;
		case eVolleyball_2104:
			ret=COUNTOF(c_Volleyball_2104);
			break;
		case eVolleyball_2105:
			ret=COUNTOF(c_Volleyball_2105);
			break;
		default:
			assert(false);
	}
	return ret;
}

BoardDaktronics::Volleyball::Volleyball()
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
	m_SportType=eVolleyball_2101;
}

void BoardDaktronics::Volleyball::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardDaktronics::Volleyball::~Volleyball()
{
	CloseResources();
}

void BoardDaktronics::Volleyball::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardDaktronics::Volleyball::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Volleyball_2101")==0)
		SetSportType(Daktronics::Volleyball::eVolleyball_2101);
	else if (stricmp(Sport,"Volleyball_2111")==0)
		SetSportType(Daktronics::Volleyball::eVolleyball_2111);
	else if (stricmp(Sport,"Volleyball_2103")==0)
		SetSportType(Daktronics::Volleyball::eVolleyball_2103);
	else if (stricmp(Sport,"Volleyball_2104")==0)
		SetSportType(Daktronics::Volleyball::eVolleyball_2104);
	else if (stricmp(Sport,"Volleyball_2105")==0)
		SetSportType(Daktronics::Volleyball::eVolleyball_2105);
	else
		SetSportType(Daktronics::Volleyball::eVolleyball_2101);
//		assert (false);
}

unsigned BoardDaktronics::Volleyball::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardDaktronics::ParsedElement &BoardDaktronics::Volleyball::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardDaktronics::Volleyball::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
		case eVolleyball_2101:
			ret=c_Volleyball_2101[index].name;
			break;
		case eVolleyball_2111:
			ret=c_Volleyball_2111[index].name;
			break;
		case eVolleyball_2103:
			ret=c_Volleyball_2103[index].name;
			break;
		case eVolleyball_2104:
			ret=c_Volleyball_2104[index].name;
			break;
		case eVolleyball_2105:
			ret=c_Volleyball_2105[index].name;
			break;
		default:
			assert(false);
	}
	return ret;
}

const char *BoardDaktronics::Volleyball::GetBoardNextChangedKey(void)
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

const char * BoardDaktronics::Volleyball::GetBoardValue(const char key[])
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

