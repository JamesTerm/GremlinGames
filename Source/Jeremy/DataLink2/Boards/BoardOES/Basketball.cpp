#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace OES
		{
#include "Basketball.h"
		}
	}
}

using namespace PlugIn::DataLink::OES;
using namespace PlugIn::DataLink::OES::Basketball;


/*******************************************************************************************************/
/*										BoardOES::Basketball										*/
/*******************************************************************************************************/

size_t BoardOES::Basketball::GetFieldWidth(size_t index) 
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

size_t BoardOES::Basketball::GetFieldOffset(size_t index) 
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

size_t BoardOES::Basketball::GetNumFields()
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

BoardOES::Basketball::Basketball()
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
	m_SportType=eBasketball_0;
}

void BoardOES::Basketball::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardOES::Basketball::~Basketball()
{
	CloseResources();
}

void BoardOES::Basketball::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardOES::Basketball::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Basketball_0")==0)
		SetSportType(::Basketball::eBasketball_0);
	else
		SetSportType(::Basketball::eBasketball_0);
}

unsigned BoardOES::Basketball::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardOES::ParsedElement &BoardOES::Basketball::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardOES::Basketball::GetFieldName(size_t index)
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

const char *BoardOES::Basketball::GetBoardNextChangedKey(void)
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

const char * BoardOES::Basketball::GetBoardValue(const char key[])
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

