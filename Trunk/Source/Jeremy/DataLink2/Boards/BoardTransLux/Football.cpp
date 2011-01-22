#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace TransLux
		{
#include "Football.h"
		}
	}
}

using namespace PlugIn::DataLink::TransLux;
using namespace PlugIn::DataLink::TransLux::Football;


  /*******************************************************************************************************/
 /*										TransLux::Football												*/
/*******************************************************************************************************/

size_t BoardTransLux::Football::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eFootball_0:
		ret=c_Football_0[index].width;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardTransLux::Football::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eFootball_0:
		ret=c_Football_0[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardTransLux::Football::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
	case eFootball_0:
		ret=COUNTOF(c_Football_0);
		break;
	default:
		assert(false);
	}
	return ret;
}

BoardTransLux::Football::Football()
{
	m_pElement=NULL;

	for (m_SportType=0;m_SportType<eNumGameTypes;m_SportType++)
	{
		unsigned msgLength = 0;
		for (unsigned j = 0; j < GetNumFields(); j++)
			msgLength += (GetFieldWidth(j) + GetFieldOffset(j));

		// 40 bytes added for all of the useless field labels and headers, etc.
		m_PacketLength_cache.push_back(msgLength + c_NoFormatBytes);
	}
	m_SportType=eFootball_0;
}

void BoardTransLux::Football::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardTransLux::Football::~Football()
{
	CloseResources();
}

void BoardTransLux::Football::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardTransLux::Football::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Football_0")==0)
		SetSportType(::Football::eFootball_0);
	else
		assert (false);
}

unsigned BoardTransLux::Football::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardTransLux::ParsedElement &BoardTransLux::Football::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardTransLux::Football::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
	case eFootball_0:
		ret=c_Football_0[index].name;
		break;
	default:
		assert(false);
	}
	return ret;
}

const char *BoardTransLux::Football::GetBoardNextChangedKey(void)
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

const char * BoardTransLux::Football::GetBoardValue(const char key[])
{
	const char *ret=NULL;
	for (unsigned i=0;i<GetNumFields();i++)
	{
		if (_stricmp(GetFieldName(i),&key[0])==0)
			ret=m_pElement[i].ElementName.c_str();
	}
	assert(ret);
	return ret;
}