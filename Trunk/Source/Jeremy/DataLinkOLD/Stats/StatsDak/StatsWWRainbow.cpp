#include "stdafx.h"

StatsWhiteway * getStatsWhiteway()
{
	static StatsWhiteway * st = new StatsWhiteway();
	return st;
}

StatsWhiteway::StatsWhiteway(void)
{
}

StatsWhiteway::~StatsWhiteway(void)
{
}

void StatsWhiteway::Initialize(void)
{
	for (unsigned i=0;i<eWWNumFields;i++)
	{
		m_ValueStrings[i]=(char*)malloc(g_WWStringLengths[i]+1);
		m_ValueStrings[i][0] = 0;
		m_ValueChanged[i] = false;
	}
}

void StatsWhiteway::Destroy(void)
{
	for (unsigned i=0;i<eWWNumFields;i++)
		free(m_ValueStrings[i]);
}

bool StatsWhiteway::ParsePacket(void)
{
	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]))
		return false;

	// First Parse the clock here
	CopyStringIfChanged(m_ValueStrings[eWWPeriod], m_CurrentPacket+5, 1, eWWPeriod, m_ValueChanged[eWWPeriod]);
	CopyStringIfChanged(m_ValueStrings[eWWMinutes], m_CurrentPacket+6, 2, eWWMinutes, m_ValueChanged[eWWMinutes]);
	CopyStringIfChanged(m_ValueStrings[eWWSeconds], m_CurrentPacket+8, 2, eWWSeconds, m_ValueChanged[eWWSeconds]);
	CopyStringIfChanged(m_ValueStrings[eWWMilliseconds], m_CurrentPacket+10, 1, eWWMilliseconds, m_ValueChanged[eWWMilliseconds]);
	char lTempScore[2][4];
	for (unsigned i=0;i<2;i++)
	{
		if (('A' <= m_CurrentPacket[11+i*2]) && (m_CurrentPacket[11+i*2] <= 'Z'))
		{
			lTempScore[i][0] = '1';
			lTempScore[i][1] = (char)((unsigned)(m_CurrentPacket[11+i*2]) - 65/*ASCII A*/ + 48/*ASCII 0*/);
			lTempScore[i][2] = m_CurrentPacket[12+i*2];
			lTempScore[i][3] = 0;
		}
		else
		{
			lTempScore[i][0] = m_CurrentPacket[11+i*2];
			lTempScore[i][1] = m_CurrentPacket[12+i*2];
			lTempScore[i][2] = 0;
		}
	}

	CopyStringIfChanged(m_ValueStrings[eWWAwayScore], lTempScore[0], lTempScore[0][2]?3:2, eWWAwayScore, m_ValueChanged[eWWAwayScore]);
	CopyStringIfChanged(m_ValueStrings[eWWHomeScore], lTempScore[1], lTempScore[1][2]?3:2, eWWHomeScore, m_ValueChanged[eWWHomeScore]);
	CopyStringIfChanged(m_ValueStrings[eWWShotClock], m_CurrentPacket+15, 2, eWWShotClock, m_ValueChanged[eWWShotClock]);

	char lTempClock[12];
	strcpy(lTempClock, m_ValueStrings[eWWMinutes]);
	strcat(lTempClock, ":");
	strcat(lTempClock, m_ValueStrings[eWWSeconds]);
	strcat(lTempClock, ".");
	strcat(lTempClock, m_ValueStrings[eWWMilliseconds]);

	CopyStringIfChanged(m_ValueStrings[eWWClock], lTempClock, strlen(lTempClock), eWWClock, m_ValueChanged[eWWClock]);

	// Parse the rest here

	for (unsigned i=0;i<eWWNumFields;i++)
		if (m_ValueChanged[i])
			return true;

	// Nothing Changed
	return false;

}

bool StatsWhiteway::IsAlphaNumeric(char c)
{
	if ((c>='0')&&(c<='9'))
		return true;
	if ((c>='A')&&(c<='Z'))
		return true;
	if ((c>='a')&&(c<='z'))
		return true;

	return false;
}

bool StatsWhiteway::CheckPacket(void)
{
	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]))
		return false;

	// first and last aren't data
	if (IsAlphaNumeric(m_CurrentPacket[0]))
		return false;
	if (IsAlphaNumeric(m_CurrentPacket[WHITEWAY_PACKET_LENGTH-1]))
		return false;

	// but the rest is
	for (unsigned i=1;i<WHITEWAY_PACKET_LENGTH-2;i++)
		if (!IsAlphaNumeric(m_CurrentPacket[i]))
			return false;

	// Check CheckSum here

	if (!CheckCheckSum())
		return false;
	else
		return true;

}

bool StatsWhiteway::CheckCheckSum()
{
	//Daktronics supplies a high/low 2 byte checksum
	//This checks the checksum against the data (excluding start & end chars & the checksum itself)
	//  and returns false if it fails
	char * buf = m_CurrentPacket;
	int start = 5;						// Skip
	int stop = WHITEWAY_PACKET_LENGTH-4; // subtract 1 to get the last byte, then
										 // exclude the 2 checksum bytes and the 1 end byte = -4
	char low = NULL;
	char high = NULL;
	int	iIndex;
	unsigned int	uiChecksum;
	const unsigned char	ucaHexChar[17] = { "0123456789ABCDEF" };

	// calculate checksum
	for (iIndex=start,uiChecksum=0; iIndex<=stop; iIndex++)
		uiChecksum += (unsigned int)buf[iIndex];

	// TODO what the heck is this about???
	// White Way checksum seems to be off by 1,
	// or off by 2 if the 4th byte is set to 0...
	uiChecksum -= 1;
	if (buf[4] == '0')
		uiChecksum -= 1;

	// AND total with FF to isolate low byte
	uiChecksum &= 0xff;

	// convert low nibble to hex
	low = ucaHexChar[uiChecksum & 0x0f];

	// convert high nibble to hex
	high = ucaHexChar[uiChecksum >> 4];

	if (low != m_CurrentPacket[WHITEWAY_PACKET_LENGTH-2])
		return false;
	if (high != m_CurrentPacket[WHITEWAY_PACKET_LENGTH-3])
		return false;

	return true;
}

char * StatsWhiteway::GetBoardNextChangedKey(void)
{
	for (unsigned i=0;i<eWWNumFields;i++)
		if (m_ValueChanged[i])
		{
			m_ValueChanged[i] = false;
			return ((char *)g_WWKeyStrings[i]);
		}
	return NULL;
}

char * StatsWhiteway::GetBoardValue(const char key[])
{
	const char * ckey = key;
	for (unsigned i=0;i<eWWNumFields;i++)
		if (!_stricmp(g_WWKeyStrings[i],ckey))
		{
			return m_ValueStrings[i];
		}

	return NULL;
}