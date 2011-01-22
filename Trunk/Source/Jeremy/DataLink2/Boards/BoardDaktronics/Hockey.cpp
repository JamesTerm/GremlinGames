#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace Daktronics
		{
#include "Hockey.h"
		}
	}
}

using namespace PlugIn::DataLink::Daktronics;
using namespace PlugIn::DataLink::Daktronics::Hockey;


  /*******************************************************************************************************/
 /*										BoardDaktronics::Hockey											*/
/*******************************************************************************************************/

size_t BoardDaktronics::Hockey::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
		case eHockey_4401:
			ret=c_Hockey_4401[index].width;
			break;
		case eHockey_4402:
			ret=c_Hockey_4402[index].width;
			break;
		case eHockey_4000:
			ret=c_Hockey_4000[index].width;
			break;
		case eHockey_4102:
			ret=c_Hockey_4102[index].width;
			break;
		case eHockey_4103:
			ret=c_Hockey_4103[index].width;
			break;
		case eHockey_4104:
			ret=c_Hockey_4104[index].width;
			break;
		case eHockey_4105:
			ret=c_Hockey_4105[index].width;
			break;
		case eHockey_4601:
			ret=c_Hockey_4601[index].width;
			break;
		default:
			assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Hockey::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eHockey_4401:
		ret=c_Hockey_4401[index].offset;
		break;
	case eHockey_4402:
		ret=c_Hockey_4402[index].offset;
		break;
	case eHockey_4000:
		ret=c_Hockey_4000[index].offset;
		break;
	case eHockey_4102:
		ret=c_Hockey_4102[index].offset;
		break;
	case eHockey_4103:
		ret=c_Hockey_4103[index].offset;
		break;
	case eHockey_4104:
		ret=c_Hockey_4104[index].offset;
		break;
	case eHockey_4105:
		ret=c_Hockey_4105[index].offset;
		break;
	case eHockey_4601:
		ret=c_Hockey_4601[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Hockey::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
		case eHockey_4401:
			ret=COUNTOF(c_Hockey_4401);
			break;
		case eHockey_4402:
			ret=COUNTOF(c_Hockey_4402);
			break;
		case eHockey_4000:
			ret=COUNTOF(c_Hockey_4000);
			break;
		case eHockey_4102:
			ret=COUNTOF(c_Hockey_4102);
			break;
		case eHockey_4103:
			ret=COUNTOF(c_Hockey_4103);
			break;
		case eHockey_4104:
			ret=COUNTOF(c_Hockey_4104);
			break;
		case eHockey_4105:
			ret=COUNTOF(c_Hockey_4105);
			break;
		case eHockey_4601:
			ret=COUNTOF(c_Hockey_4601);
			break;
		default:
			assert(false);
	}
	return ret;
}

BoardDaktronics::Hockey::Hockey()
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
	m_SportType=eHockey_4402;
}

void BoardDaktronics::Hockey::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardDaktronics::Hockey::~Hockey()
{
	CloseResources();
}

void BoardDaktronics::Hockey::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardDaktronics::Hockey::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Hockey_4401")==0)
		SetSportType(Daktronics::Hockey::eHockey_4401);
	else if (stricmp(Sport,"Hockey_4000")==0)
		SetSportType(Daktronics::Hockey::eHockey_4000);
	else if (stricmp(Sport,"Hockey_4402")==0)
		SetSportType(Daktronics::Hockey::eHockey_4402);
	else if (stricmp(Sport,"Hockey_4102")==0)
		SetSportType(Daktronics::Hockey::eHockey_4102);
	else if (stricmp(Sport,"Hockey_4103")==0)
		SetSportType(Daktronics::Hockey::eHockey_4103);
	else if (stricmp(Sport,"Hockey_4104")==0)
		SetSportType(Daktronics::Hockey::eHockey_4104);
	else if (stricmp(Sport,"Hockey_4105")==0)
		SetSportType(Daktronics::Hockey::eHockey_4105);
	else if (stricmp(Sport,"Hockey_4601")==0)
		SetSportType(Daktronics::Hockey::eHockey_4601);
	else
		SetSportType(Daktronics::Hockey::eHockey_4401);
}

unsigned BoardDaktronics::Hockey::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardDaktronics::ParsedElement &BoardDaktronics::Hockey::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardDaktronics::Hockey::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
		case eHockey_4401:
			ret=c_Hockey_4401[index].name;
			break;
		case eHockey_4402:
			ret=c_Hockey_4402[index].name;
			break;
		case eHockey_4000:
			ret=c_Hockey_4000[index].name;
			break;
		case eHockey_4102:
			ret=c_Hockey_4102[index].name;
			break;
		case eHockey_4103:
			ret=c_Hockey_4103[index].name;
			break;
		case eHockey_4104:
			ret=c_Hockey_4104[index].name;
			break;
		case eHockey_4105:
			ret=c_Hockey_4105[index].name;
			break;
		case eHockey_4601:
			ret=c_Hockey_4601[index].name;
			break;
		default:
			assert(false);
	}
	return ret;
}

const char *BoardDaktronics::Hockey::GetBoardNextChangedKey(void)
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

const char * BoardDaktronics::Hockey::GetBoardValue(const char key[])
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

