#include "stdafx.h"

StatsDsi * getStatsDsi()
{
	static StatsDsi * st = new StatsDsi();
	return st;
}

StatsDsi::StatsDsi(void)
: m_FieldNumber(0)
{
}

StatsDsi::~StatsDsi(void)
{
}

void StatsDsi::Initialize(void)
{
	m_FieldNumber = 0;
	for (unsigned i=0;i<eDSINumFields;i++)
	{
		m_ValueStrings[i]=(char*)malloc(g_DSIStringLengths[i]+1);
		m_ValueStrings[i][0] = 0;
		m_ValueChanged[i] = false;
	}
}

void StatsDsi::Destroy(void)
{
	for (unsigned i=0;i<eDSINumFields;i++)
		free(m_ValueStrings[i]);
}

bool StatsDsi::ParsePacket(void)
{
	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]))
		return false;

	if (!m_FieldNumber)
		return false;

	switch (m_FieldNumber)
	{
		case 26:
			CopyStringIfChanged(m_ValueStrings[eDSIClock], m_CurrentPacket+4, 7, eDSIClock, m_ValueChanged[eDSIClock]);
			break;
		case 27:
			CopyStringIfChanged(m_ValueStrings[eDSIShotClock], m_CurrentPacket+4, 2, eDSIShotClock, m_ValueChanged[eDSIShotClock]);
			break;
		case 28:
			return false; // not using "horn" field
			break;
		default:
			return false;
			break;
	}

	for (unsigned i=0;i<eDSINumFields;i++)
		if (m_ValueChanged[i])
			return true;

	// Nothing Changed
	return false;

}

bool StatsDsi::CheckPacket(void)
{
	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]) || (!m_CurrentPacket[1]))
		return false;

	char fieldNumber[4];
	strncpy(fieldNumber, m_CurrentPacket+1, 3);
	fieldNumber[3] = 0;
	int ifn = atoi(fieldNumber);
	if ((ifn >= 26) && (ifn <= 28))
	{
		m_FieldNumber = ifn;
		return true;
	}
	else
	{
		m_FieldNumber = 0;
		return false;
	}
}

char * StatsDsi::GetBoardNextChangedKey(void)
{
	for (unsigned i=0;i<eDSINumFields;i++)
		if (m_ValueChanged[i])
		{
			m_ValueChanged[i] = false;
			return ((char *)g_DSIKeyStrings[i]);
		}
	return NULL;
}

char * StatsDsi::GetBoardValue(const char key[])
{
	const char * ckey = key;
	for (unsigned i=0;i<eDSINumFields;i++)
		if (!_stricmp(g_DSIKeyStrings[i],ckey))
		{
			return m_ValueStrings[i];
		}

	return NULL;
}