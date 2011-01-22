#include "stdafx.h"
#include "Main.h"
namespace PlugIn
{
	namespace DataLink
	{
		namespace Daktronics
		{
#include "Soccer.h"
		}
	}
}

using namespace PlugIn::DataLink::Daktronics;
using namespace PlugIn::DataLink::Daktronics::Soccer;


  /*******************************************************************************************************/
 /*										BoardDaktronics::Soccer											*/
/*******************************************************************************************************/

size_t BoardDaktronics::Soccer::GetFieldWidth(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
		case eSoccer_7701:
			ret=c_Soccer_7701[index].width;
			break;
		case eSoccer_7711:
			ret=c_Soccer_7711[index].width;
			break;
		case eSoccer_7611:
			ret=c_Soccer_7611[index].width;
			break;
		case eSoccer_7601:
			ret=c_Soccer_7601[index].width;
			break;
		case eSoccer_7501:
			ret=c_Soccer_7501[index].width;
			break;
		case eSoccer_7604:
			ret=c_Soccer_7604[index].width;
			break;
		default:
			assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Soccer::GetFieldOffset(size_t index) 
{
	size_t ret;
	switch (m_SportType)
	{
	case eSoccer_7701:
		ret=c_Soccer_7701[index].offset;
		break;
	case eSoccer_7711:
		ret=c_Soccer_7711[index].offset;
		break;
	case eSoccer_7611:
		ret=c_Soccer_7611[index].offset;
		break;
	case eSoccer_7601:
		ret=c_Soccer_7601[index].offset;
		break;
	case eSoccer_7501:
		ret=c_Soccer_7501[index].offset;
		break;
	case eSoccer_7604:
		ret=c_Soccer_7604[index].offset;
		break;
	default:
		assert(false);
	}
	return ret;
}

size_t BoardDaktronics::Soccer::GetNumFields()
{
	size_t ret;
	switch (m_SportType)
	{
		case eSoccer_7701:
			ret=COUNTOF(c_Soccer_7701);
			break;
		case eSoccer_7711:
			ret=COUNTOF(c_Soccer_7711);
			break;
		case eSoccer_7611:
			ret=COUNTOF(c_Soccer_7611);
			break;
		case eSoccer_7601:
			ret=COUNTOF(c_Soccer_7601);
			break;
		case eSoccer_7501:
			ret=COUNTOF(c_Soccer_7501);
			break;
		case eSoccer_7604:
			ret=COUNTOF(c_Soccer_7604);
			break;
		default:
			assert(false);
	}
	return ret;
}

BoardDaktronics::Soccer::Soccer()
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
	m_SportType=eSoccer_7701;
}

void BoardDaktronics::Soccer::CloseResources(void)
{
	if (m_pElement)
	{
		delete [] m_pElement;
		m_pElement=NULL;
	}
}

BoardDaktronics::Soccer::~Soccer()
{
	CloseResources();
}

void BoardDaktronics::Soccer::SetSportType(size_t sportType) 
{
	m_SportType=sportType;
	CloseResources();
}

void BoardDaktronics::Soccer::SetSport(const char *Sport)
{
	if (stricmp(Sport,"Soccer_7701")==0)
		SetSportType(Daktronics::Soccer::eSoccer_7701);
	else if (stricmp(Sport,"Soccer_7711")==0)
		SetSportType(Daktronics::Soccer::eSoccer_7711);
	else if (stricmp(Sport,"Soccer_7611")==0)
		SetSportType(Daktronics::Soccer::eSoccer_7611);
	else if (stricmp(Sport,"Soccer_7601")==0)
		SetSportType(Daktronics::Soccer::eSoccer_7601);
	else if (stricmp(Sport,"Soccer_7501")==0)
		SetSportType(Daktronics::Soccer::eSoccer_7501);
	else if (stricmp(Sport,"Soccer_7604")==0)
		SetSportType(Daktronics::Soccer::eSoccer_7604);
	else
		SetSportType(Daktronics::Soccer::eSoccer_7701);
//		assert (false);
}

unsigned BoardDaktronics::Soccer::GetNumBytesToRead(void)
{
	return m_PacketLength_cache[m_SportType];
}

BoardDaktronics::ParsedElement &BoardDaktronics::Soccer::GetParsedElement(size_t index)
{
	if (!m_pElement)
		m_pElement = new ParsedElement[GetNumFields()];
	return m_pElement[index];
}

const char *BoardDaktronics::Soccer::GetFieldName(size_t index)
{
	const char *ret=NULL;
	switch (m_SportType)
	{
		case eSoccer_7701:
			ret=c_Soccer_7701[index].name;
			break;
		case eSoccer_7711:
			ret=c_Soccer_7711[index].name;
			break;
		case eSoccer_7611:
			ret=c_Soccer_7611[index].name;
			break;
		case eSoccer_7601:
			ret=c_Soccer_7601[index].name;
			break;
		case eSoccer_7501:
			ret=c_Soccer_7501[index].name;
			break;
		case eSoccer_7604:
			ret=c_Soccer_7604[index].name;
			break;
		default:
			assert(false);
	}
	return ret;
}

const char *BoardDaktronics::Soccer::GetBoardNextChangedKey(void)
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

const char * BoardDaktronics::Soccer::GetBoardValue(const char key[])
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

