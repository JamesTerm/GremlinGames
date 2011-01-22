#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace DSI
		{
#include "Basketball.h"
		}
	}
}

using namespace PlugIn::DataLink::DSI;
using namespace PlugIn::DataLink::DSI::Basketball;


  /*******************************************************************************************************/
 /*										DSI::Basketball													*/
/*******************************************************************************************************/

size_t BoardDSI::Basketball::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eBasketball_0:
		ret=c_Basketball_0[index].width;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDSI::Basketball::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eBasketball_0:
		ret=c_Basketball_0[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDSI::Basketball::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
	case eBasketball_0:
		ret=COUNTOF(c_Basketball_0);
		break;
	default:
		assert(false);
	}
	return ret;
}

BoardDSI::Basketball::Basketball()
{
	m_pElement=NULL;

	for (m_SportType=0;m_SportType<eNumGameTypes;m_SportType++)
	{
		unsigned msgLength = 0;
		for (unsigned j = 0; j < GetNumFields(); j++)
			msgLength += GetFieldWidth(j);

		// 11 bytes added for element headers (3, 3, and 3 +2 for bom/eom) for  for clock and shot clock.
		m_PacketLength_cache.push_back(msgLength + c_NoFormatBytes);
	}
	m_SportType=eBasketball_0;
}

void BoardDSI::Basketball::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardDSI::Basketball::~Basketball()
{
	CloseResources();
}

void BoardDSI::Basketball::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardDSI::Basketball::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Basketball")==0)
		SetSportType(::Basketball::eBasketball_0);
	else
		assert (false);
}

unsigned BoardDSI::Basketball::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardDSI::ParsedElement &BoardDSI::Basketball::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardDSI::Basketball::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
	case eBasketball_0:
		ret=c_Basketball_0[index].name;
		break;
	default:
		assert(false);
	}
	return ret;
}

const char *BoardDSI::Basketball::GetBoardNextChangedKey(void)
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

const char * BoardDSI::Basketball::GetBoardValue(const char key[])
{
	const char *ret=NULL;
	for (unsigned i=0;i<GetNumFields();i++)
	{
		if (_stricmp(GetFieldName(i),&key[0])==0)
		{
			ret=m_pElement[i].ElementName.c_str();
		}
	}
	assert(ret);
	return ret;
}