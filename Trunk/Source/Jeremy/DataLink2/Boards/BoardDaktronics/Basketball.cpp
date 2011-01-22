#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace Daktronics
		{
#include "Basketball.h"
		}
	}
}

using namespace PlugIn::DataLink::Daktronics;
using namespace PlugIn::DataLink::Daktronics::Basketball;


  /*******************************************************************************************************/
 /*										BoardDaktronics::Basketball										*/
/*******************************************************************************************************/

size_t BoardDaktronics::Basketball::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
		case eBasketball_1101:
			ret=c_Basketball_1101[index].width;
			break;
		case eBasketball_1102:
			ret=c_Basketball_1102[index].width;
			break;
		case eBasketball_1103:
			ret=c_Basketball_1103[index].width;
			break;
		case eBasketball_1104:
			ret=c_Basketball_1104[index].width;
			break;
		case eBasketball_1105:
			ret=c_Basketball_1105[index].width;
			break;
		case eBasketball_1301:
			ret=c_Basketball_1301[index].width;
			break;
		case eBasketball_1401:
			ret=c_Basketball_1401[index].width;
			break;
		case eBasketball_1402:
			ret=c_Basketball_1402[index].width;
			break;
		default:
			assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Basketball::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eBasketball_1101:
		ret=c_Basketball_1101[index].offset;
		break;
	case eBasketball_1102:
		ret=c_Basketball_1102[index].offset;
		break;
	case eBasketball_1103:
		ret=c_Basketball_1103[index].offset;
		break;
	case eBasketball_1104:
		ret=c_Basketball_1104[index].offset;
		break;
	case eBasketball_1105:
		ret=c_Basketball_1105[index].offset;
		break;
	case eBasketball_1301:
		ret=c_Basketball_1301[index].offset;
		break;
	case eBasketball_1401:
		ret=c_Basketball_1401[index].offset;
		break;
	case eBasketball_1402:
		ret=c_Basketball_1402[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Basketball::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
		case eBasketball_1101:
			ret=COUNTOF(c_Basketball_1101);
			break;
		case eBasketball_1102:
			ret=COUNTOF(c_Basketball_1102);
			break;
		case eBasketball_1103:
			ret=COUNTOF(c_Basketball_1103);
			break;
		case eBasketball_1104:
			ret=COUNTOF(c_Basketball_1104);
			break;
		case eBasketball_1105:
			ret=COUNTOF(c_Basketball_1105);
			break;
		case eBasketball_1301:
			ret=COUNTOF(c_Basketball_1301);
			break;
		case eBasketball_1401:
			ret=COUNTOF(c_Basketball_1401);
			break;
		case eBasketball_1402:
			ret=COUNTOF(c_Basketball_1402);
			break;
		default:
			assert(false);
	}
	return ret;
}

BoardDaktronics::Basketball::Basketball()
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
	m_SportType=eBasketball_1101;
}

void BoardDaktronics::Basketball::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardDaktronics::Basketball::~Basketball()
{
	CloseResources();
}

void BoardDaktronics::Basketball::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardDaktronics::Basketball::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Basketball_1101")==0)
		SetSportType(::Basketball::eBasketball_1101);
	else if (stricmp(Sport,"Basketball_1102")==0)
		SetSportType(::Basketball::eBasketball_1102);
	else if (stricmp(Sport,"Basketball_1103")==0)
		SetSportType(::Basketball::eBasketball_1103);
	else if (stricmp(Sport,"Basketball_1104")==0)
		SetSportType(::Basketball::eBasketball_1104);
	else if (stricmp(Sport,"Basketball_1105")==0)
		SetSportType(::Basketball::eBasketball_1105);
	else if (stricmp(Sport,"Basketball_1301")==0)
		SetSportType(::Basketball::eBasketball_1301);
	else if (stricmp(Sport,"Basketball_1401")==0)
		SetSportType(::Basketball::eBasketball_1401);
	else if (stricmp(Sport,"Basketball_1402")==0)
		SetSportType(::Basketball::eBasketball_1402);
	else
		SetSportType(::Basketball::eBasketball_1101);
}

unsigned BoardDaktronics::Basketball::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardDaktronics::ParsedElement &BoardDaktronics::Basketball::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardDaktronics::Basketball::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
		case eBasketball_1101:
			ret=c_Basketball_1101[index].name;
			break;
		case eBasketball_1102:
			ret=c_Basketball_1102[index].name;
			break;
		case eBasketball_1103:
			ret=c_Basketball_1103[index].name;
			break;
		case eBasketball_1104:
			ret=c_Basketball_1104[index].name;
			break;
		case eBasketball_1105:
			ret=c_Basketball_1105[index].name;
			break;
		case eBasketball_1301:
			ret=c_Basketball_1301[index].name;
			break;
		case eBasketball_1401:
			ret=c_Basketball_1401[index].name;
			break;
		case eBasketball_1402:
			ret=c_Basketball_1402[index].name;
			break;
		default:
			assert(false);
	}
	return ret;
}

const char *BoardDaktronics::Basketball::GetBoardNextChangedKey(void)
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

const char * BoardDaktronics::Basketball::GetBoardValue(const char key[])
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

	//assert(ret);
	return ret;
}
