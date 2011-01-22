#include "stdafx.h"

StatsOes * getStatsOes()
{
	static StatsOes * st = new StatsOes();
	return st;
}

StatsOes::StatsOes(void)
{
}

StatsOes::~StatsOes(void)
{
}

void StatsOes::Initialize(void)
{
	for (unsigned i=0;i<eOESNumFields;i++)
	{
		m_ValueStrings[i]=(char*)malloc(g_OESStringLengths[i]+1);
		m_ValueStrings[i][0] = 0;
		m_ValueChanged[i] = false;
	}
}

void StatsOes::Destroy(void)
{
	for (unsigned i=0;i<eOESNumFields;i++)
		free(m_ValueStrings[i]);
}

bool StatsOes::ParsePacket(void)
{
	// For informative asserts
	bool OESFormatAssert = false;

	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]))
		return false;

	char lTempDebug[30];
	strcpy(lTempDebug, m_CurrentPacket);

	// Messy stuff in here. Basically, have to check for multiple values and adjust for possible hex values
	//  and ignore colons which indicate a null such as in the case of '0' in the time '05.1'.
	char lTempClock[8];
	if ((m_CurrentPacket[3] < 0) && (m_CurrentPacket[4] < 0))
	{
		if (m_CurrentPacket[2] == 0x3a || m_CurrentPacket[2] == (char) 0xba)
			lTempClock[0] = ' ';
		else if (m_CurrentPacket[2] > '0' && m_CurrentPacket[2] < '9')
			lTempClock[0] = m_CurrentPacket[2];
		else
			lTempClock[0] = m_CurrentPacket[2] - (char) - 0x80;

		lTempClock[1] = m_CurrentPacket[3] - (char) 0x80;
		lTempClock[2] = ':';
		lTempClock[3] = m_CurrentPacket[4] - (char) 0x80;
		lTempClock[4] = m_CurrentPacket[5];
		lTempClock[5] = 0;
	}
	else if ((m_CurrentPacket[4] < 0) && (m_CurrentPacket[5] == ':'))
	{
		if (m_CurrentPacket[2] == 0x3a || m_CurrentPacket[2] == (char) 0xba)
			lTempClock[0] = ' ';
		else if (m_CurrentPacket[2] < '0' || m_CurrentPacket[2] > '9')
			lTempClock[0] = m_CurrentPacket[2] - (char) 0x80;
		else
			lTempClock[0] = m_CurrentPacket[2];

		lTempClock[1] = m_CurrentPacket[3];
		lTempClock[2] = '.';
		lTempClock[3] = m_CurrentPacket[4] - (char) 0x80;
		lTempClock[4] = 0;
	}
	else
	{
		// unknown format
		assert(OESFormatAssert);
		return false;
	}

	CopyStringIfChanged(m_ValueStrings[eOESClock], lTempClock, strlen(lTempClock), eOESClock, m_ValueChanged[eOESClock]);

	char lTempScore[2][4];

	for (unsigned i=0;i<2;i++)
	{
		lTempScore[i][3] = 0;

		lTempScore[i][0] = ' ';
		if (((m_CurrentPacket[7+i*2] < '0') || (m_CurrentPacket[7+i*2] > '9')) && (m_CurrentPacket[7+i*2] != ':'))
		{
			lTempScore[i][0] = '1';
			lTempScore[i][1] = m_CurrentPacket[7+i*2] - (char) 0x80;
			lTempScore[i][2] = m_CurrentPacket[8+i*2] - (char) 0x80;
		}
		else
		{
			if (m_CurrentPacket[7+i*2] == ' ')
				lTempScore[i][1] = '0';
			else
				lTempScore[i][1] = m_CurrentPacket[7+i*2];
			if (m_CurrentPacket[8+i*2] == ' ')
				lTempScore[i][2] = '0';
			else
				lTempScore[i][2] = m_CurrentPacket[8+i*2];
		}

		// If they were colons, they should be nothing
		if (m_CurrentPacket[7+i*2] == ':')
			lTempScore[i][1] = ' ';
		if (m_CurrentPacket[8+i*2] == ':')
			lTempScore[i][2] = ' ';
	}
	CopyStringIfChanged(m_ValueStrings[eOESHomeScore], lTempScore[0], 3, eOESHomeScore, m_ValueChanged[eOESHomeScore]);
	CopyStringIfChanged(m_ValueStrings[eOESAwayScore], lTempScore[1], 3, eOESAwayScore, m_ValueChanged[eOESAwayScore]);

	CopyStringIfChanged(m_ValueStrings[eOESPeriod], m_CurrentPacket+6, 1, eOESPeriod, m_ValueChanged[eOESPeriod]);

	CopyStringIfChanged(m_ValueStrings[eOESHomeFouls], m_CurrentPacket+13, 1, eOESHomeFouls, m_ValueChanged[eOESHomeFouls]);
	CopyStringIfChanged(m_ValueStrings[eOESAwayFouls], m_CurrentPacket+14, 1, eOESAwayFouls, m_ValueChanged[eOESAwayFouls]);

	//The 20 second part time out fields share use with possession indicator and bonus indicators.
	//Possession is +1 and bonus is +2 making each part time out value have a range of 4 possible values.
	// If part time outs are 1, 2, or 3, then full time outs is + 0x80 and then
	//		part time out 1 = 0x35 (5)
	//		part time out 2 = 0x39 (9) etc.
	//// Home Time Outs ////
	char homePartTO = '0';
	char homeFullTO = (m_CurrentPacket[11] - 0x80);

	if  ((m_CurrentPacket[11] < '0') || (m_CurrentPacket[11] > '9'))
	{
		CopyStringIfChanged(m_ValueStrings[eOESHomeTOFull], &homeFullTO, 1, eOESHomeTOFull, m_ValueChanged[eOESHomeTOFull]);

		if (m_CurrentPacket[16] == 0x34 || m_CurrentPacket[16] == 0x35 || m_CurrentPacket[16] == 0x36 || m_CurrentPacket[16] == 0x37)
			homePartTO = '1';
		else if (m_CurrentPacket[16] == 0x38 || m_CurrentPacket[16] == 0x39 || m_CurrentPacket[16] == 0x3a || m_CurrentPacket[16] == 0x3b)
			homePartTO = '2';
		else if (m_CurrentPacket[16] == 0x3c || m_CurrentPacket[16] == 0x3d || m_CurrentPacket[16] == 0x3e || m_CurrentPacket[16] == 0x3f)
			homePartTO = '3';
		else
			assert (false);

		CopyStringIfChanged(m_ValueStrings[eOESHomeTOPart], &homePartTO, 1, eOESHomeTOPart, m_ValueChanged[eOESHomeTOPart]);
	}
	// ...else Home Part Time outs must be 0 and we can grab the literal full time out.
	else
	{
		CopyStringIfChanged(m_ValueStrings[eOESHomeTOFull], m_CurrentPacket+11, 1, eOESHomeTOFull, m_ValueChanged[eOESHomeTOFull]);
		CopyStringIfChanged(m_ValueStrings[eOESHomeTOPart], &homePartTO, 1, eOESHomeTOPart, m_ValueChanged[eOESHomeTOPart]);
	}

	//// Away Time Outs ////
	char awayPartTO = '0';
	char awayFullTO = (m_CurrentPacket[12] - 0x80);

	if  ((m_CurrentPacket[12] < '0') || (m_CurrentPacket[12] > '9'))
	{
		CopyStringIfChanged(m_ValueStrings[eOESAwayTOFull], &awayFullTO, 1, eOESAwayTOFull, m_ValueChanged[eOESAwayTOFull]);

		if (m_CurrentPacket[17] == 0x34 || m_CurrentPacket[17] == 0x35 || m_CurrentPacket[17] == 0x36 || m_CurrentPacket[17] == 0x37)
			awayPartTO = '1';
		else if (m_CurrentPacket[17] == 0x38 || m_CurrentPacket[17] == 0x39 || m_CurrentPacket[17] == 0x3a || m_CurrentPacket[17] == 0x3b)
			awayPartTO = '2';
		else if (m_CurrentPacket[17] == 0x3c || m_CurrentPacket[17] == 0x3d || m_CurrentPacket[17] == 0x3e || m_CurrentPacket[17] == 0x3f)
			awayPartTO = '3';
		else
			assert (false);

		CopyStringIfChanged(m_ValueStrings[eOESAwayTOPart], &awayPartTO, 1, eOESAwayTOPart, m_ValueChanged[eOESAwayTOPart]);
	}
	// ...else Home Part Time outs must be 0 and we can grab the literal full time out.
	else
	{
		CopyStringIfChanged(m_ValueStrings[eOESAwayTOFull], m_CurrentPacket+12, 1, eOESAwayTOFull, m_ValueChanged[eOESAwayTOFull]);
		CopyStringIfChanged(m_ValueStrings[eOESAwayTOPart], &awayPartTO, 1, eOESAwayTOPart, m_ValueChanged[eOESAwayTOPart]);
	}


	char lTempShotClock[3];
	lTempShotClock[0] = m_CurrentPacket[18];
	lTempShotClock[1] = m_CurrentPacket[19];
	lTempShotClock[2] = 0;
	for (unsigned i=0;i<2;i++)
		if (lTempShotClock[i] == ':')
			lTempShotClock[i] = ' ';
	CopyStringIfChanged(m_ValueStrings[eOESShotClock], lTempShotClock, 2, eOESShotClock, m_ValueChanged[eOESShotClock]);

	for (unsigned i=0;i<eOESNumFields;i++)
		if (m_ValueChanged[i])
			return true;

	// Nothing Changed
	return false;

}

bool StatsOes::CheckPacket(void)
{
	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]) || (!m_CurrentPacket[1]))
		return false;

	if (m_CurrentPacket[1] != 't')
		return false;

// 	if (m_CurrentPacket[2]>'9' || m_CurrentPacket[2]<'0' && m_CurrentPacket[2]!=':')
// 		return false;
// 
	if (((m_CurrentPacket[5]>'9')||(m_CurrentPacket[5]<'0'))
		&& (m_CurrentPacket[5]!=':')) 
		return false;

	return true;
}

char * StatsOes::GetBoardNextChangedKey(void)
{
	for (unsigned i=0;i<eOESNumFields;i++)
		if (m_ValueChanged[i])
		{
			m_ValueChanged[i] = false;
			return ((char *)g_OESKeyStrings[i]);
		}
	return NULL;
}

char * StatsOes::GetBoardValue(const char key[])
{
	const char * ckey = key;
	for (unsigned i=0;i<eOESNumFields;i++)
		if (!_stricmp(g_OESKeyStrings[i],ckey))
		{
			return m_ValueStrings[i];
		}

	return NULL;
}