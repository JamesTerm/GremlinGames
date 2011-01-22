#include "stdafx.h"

StatsTransLuxMP70 * getStatsTLMP70()
{
	static StatsTransLuxMP70 * st = new StatsTransLuxMP70();
	return st;
}

StatsTransLuxMP70::StatsTransLuxMP70(void)
{
}

StatsTransLuxMP70::~StatsTransLuxMP70(void)
{
}

void StatsTransLuxMP70::Initialize(void)
{
	for (unsigned i=0;i<eTLMP70FBNumFields;i++)
	{
		m_ValueStrings[i]=(char*)malloc(g_StringLengths[i]+1);
		m_ValueStrings[i][0] = 0;
		m_ValueChanged[i] = false;
	}
}

void StatsTransLuxMP70::Destroy(void)
{
	for (unsigned i=0;i<eTLMP70FBNumFields;i++)
		free(m_ValueStrings[i]);
}

bool StatsTransLuxMP70::ParsePacket(void)
{
	unsigned int sub_msg_offset0, sub_msg_offset1;

	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]))
		return false;
	if ((m_CurrentPacket[0] == 0x01) && (m_CurrentPacket[1] == 0x01))
	{
		sub_msg_offset0 = 0; // msg properly framed
		sub_msg_offset1 = 30;
	}
	else if ((m_CurrentPacket[0] == 0x01) && (m_CurrentPacket[1] == 0x02))
	{
		//printf("Msg mis-framed\n");
		sub_msg_offset0 = 30; // msg mis-framed
		sub_msg_offset1 = 0;
	}

//	// First Parse the clock
	CopyStringIfChanged(m_ValueStrings[eTLMP70FBClock], m_CurrentPacket+2+sub_msg_offset0, 7, 
		eTLMP70FBClock, m_ValueChanged[eTLMP70FBClock]);
	CopyStringIfChanged(m_ValueStrings[eTLMP70FBQuarter], m_CurrentPacket+11+sub_msg_offset0, 1, 
		eTLMP70FBQuarter, m_ValueChanged[eTLMP70FBQuarter]);
	
	char lTempScore[2][3];
	strncpy(lTempScore[0], m_CurrentPacket+20+sub_msg_offset0, 2);// Copy home score
	if (lTempScore[0][1] == ' ')
		lTempScore[0][1] = '0';
	strncpy(lTempScore[1], m_CurrentPacket+27+sub_msg_offset0, 2);// Copy visitor score
	if (lTempScore[1][1] == ' ')
		lTempScore[1][1] = '0';

	CopyStringIfChanged(m_ValueStrings[eTLMP70FBHomeScore], lTempScore[0], 2, 
		eTLMP70FBHomeScore, m_ValueChanged[eTLMP70FBHomeScore]);
	CopyStringIfChanged(m_ValueStrings[eTLMP70FBVisitorScore], lTempScore[1], 2, 
		eTLMP70FBVisitorScore, m_ValueChanged[eTLMP70FBVisitorScore]);

	CopyStringIfChanged(m_ValueStrings[eTLMP70FBDown], m_CurrentPacket+2+sub_msg_offset1, 1, // Stopped here
		eTLMP70FBDown, m_ValueChanged[eTLMP70FBDown]);
	CopyStringIfChanged(m_ValueStrings[eTLMP70FBToGo], m_CurrentPacket+9+sub_msg_offset1, 2, 
		eTLMP70FBToGo, m_ValueChanged[eTLMP70FBToGo]);
	CopyStringIfChanged(m_ValueStrings[eTLMP70FBBallOn], m_CurrentPacket+22+sub_msg_offset1, 2, 
		eTLMP70FBBallOn, m_ValueChanged[eTLMP70FBBallOn]);
	CopyStringIfChanged(m_ValueStrings[eTLMP70FBFieldTimer], m_CurrentPacket+27+sub_msg_offset1, 2, 
		eTLMP70FBFieldTimer, m_ValueChanged[eTLMP70FBFieldTimer]);

	// TODO: Send changed message if this is diff than the old packet

	for (unsigned i=0;i<eDakNumFields;i++)
		if (m_ValueChanged[i])
			return true;

	// Nothing Changed
	return false;
}

bool StatsTransLuxMP70::CheckPacket(void)
{
	// See if packet has the correct form
	if(!((m_CurrentPacket[0] == 0x01) && 
		((m_CurrentPacket[01] == 0x01) || (m_CurrentPacket[01] == 0x02))))
		return false;

	if(((unsigned char)m_CurrentPacket[29] != 0x81) && ((unsigned char)m_CurrentPacket[59] != 0x81))
		return false;

	return true;

}

char * StatsTransLuxMP70::GetBoardNextChangedKey(void)
{
	for (unsigned i=0;i<eTLMP70FBNumFields;i++)
		if (m_ValueChanged[i])
		{
			m_ValueChanged[i] = false;
			return ((char *)g_TLMP70FBKeyStrings[i]);
		}
	return NULL;
}

char * StatsTransLuxMP70::GetBoardValue(const char key[])
{
	const char * ckey = key;
	for (unsigned i=0;i<eTLMP70FBNumFields;i++)
		if (!_stricmp(g_TLMP70FBKeyStrings[i],ckey))
		{
			return m_ValueStrings[i];
		}

	return NULL;
}