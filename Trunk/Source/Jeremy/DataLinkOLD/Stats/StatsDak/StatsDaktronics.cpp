#include "stdafx.h"

StatsDaktronics * getStatsDaktronics()
{
	static StatsDaktronics * st = new StatsDaktronics();
	return st;
}

StatsDaktronics::StatsDaktronics(void)
{
}

StatsDaktronics::~StatsDaktronics(void)
{
}

void StatsDaktronics::Initialize(void)
{
	for (unsigned i=0;i<eDakNumFields;i++)
	{
		m_ValueStrings[i]=(char*)malloc(g_StringLengths[i]+1);
		m_ValueStrings[i][0] = 0;
		m_ValueChanged[i] = true;
	}
}

void StatsDaktronics::Destroy(void)
{
	for (unsigned i=0;i<eDakNumFields;i++)
		free(m_ValueStrings[i]);
}

bool StatsDaktronics::ParsePacket(void)
{
	if ((!m_CurrentPacket) || (!m_CurrentPacket[0]))
		return false;

	// First Parse the clock
	CopyStringIfChanged(m_ValueStrings[eDakClock], m_CurrentPacket+1, 7, eDakClock, m_ValueChanged[eDakClock]);

	// Second, parse the individual clock values.
	CopyStringIfChanged(m_ValueStrings[eDakMinutes], m_CurrentPacket+1, 2, eDakMinutes, m_ValueChanged[eDakMinutes]);
	CopyStringIfChanged(m_ValueStrings[eDakSeconds], m_CurrentPacket+4, 2, eDakSeconds, m_ValueChanged[eDakSeconds]);
	CopyStringIfChanged(m_ValueStrings[eDakMilli], m_CurrentPacket+7, 1, eDakMilli, m_ValueChanged[eDakMilli]);

	// Get rid of zeros and possible unnecessary colon.
	char lClockTemp[16];
	strcpy(lClockTemp, m_ValueStrings[eDakClock]);
	if ((lClockTemp[0] == ' ') || (lClockTemp[0] == '0'))	//Tens minutes
	{
		lClockTemp[0] = ' ';
		if (lClockTemp[1] == '0')	//Minutes and colon
		{
			lClockTemp[1] = ' ';
			lClockTemp[2] = ' ';
			if (lClockTemp[3] == '0')	//Tens seconds
			{
				lClockTemp[3] = ' ';
			}
		}
	}
	char* blank = " ";
	// Copy all clock values to individual values.
	strncpy(m_ValueStrings[eDakMinutes], &lClockTemp[0], 2);
	if (m_ValueStrings[eDakMinutes] == 0)
		strcpy (m_ValueStrings[eDakMinutes], blank);
	strncpy(m_ValueStrings[eDakSeconds], &lClockTemp[3], 2);
	strncpy(m_ValueStrings[eDakMilli], &lClockTemp[6], 1);

	// Put stripped clock values back.
	strcpy(m_ValueStrings[eDakClock], lClockTemp);


	CopyStringIfChanged(m_ValueStrings[eDakClockStatus], m_CurrentPacket+8, 1, eDakClockStatus, m_ValueChanged[eDakClockStatus]);
	CopyStringIfChanged(m_ValueStrings[eDakShotClock], m_CurrentPacket+9, 2, eDakShotClock, m_ValueChanged[eDakShotClock]);
	char lTempScore[2][4];

	strncpy(lTempScore[0], m_CurrentPacket+11, 3);
	while( ( lTempScore[0][0]==' ' ) || ( lTempScore[0][0]=='0' ) ) { lTempScore[0][0] = lTempScore[0][1]; lTempScore[0][1] = lTempScore[0][2]; lTempScore[0][2]=0; }
	if (lTempScore[0][0] == 0)
		lTempScore[0][0] = '0';

	strncpy(lTempScore[1], m_CurrentPacket+14, 3);
	while( ( lTempScore[1][0]==' ' ) || ( lTempScore[1][0]=='0' ) ) { lTempScore[1][0] = lTempScore[1][1]; lTempScore[1][1] = lTempScore[1][2]; lTempScore[1][2]=0; }
	if (lTempScore[1][0] == 0)
		lTempScore[1][0] = '0';

	CopyStringIfChanged(m_ValueStrings[eDakHomeScore], lTempScore[0], 3, eDakHomeScore, m_ValueChanged[eDakHomeScore]);
	CopyStringIfChanged(m_ValueStrings[eDakAwayScore], lTempScore[1], 3, eDakAwayScore, m_ValueChanged[eDakAwayScore]);
	CopyStringIfChanged(m_ValueStrings[eDakHomeFouls], m_CurrentPacket+17, 2, eDakHomeFouls, m_ValueChanged[eDakHomeFouls]);
	CopyStringIfChanged(m_ValueStrings[eDakAwayFouls], m_CurrentPacket+19, 2, eDakAwayFouls, m_ValueChanged[eDakAwayFouls]);
	
	// The rest are 1 char long, so I'll loop 'em
	for (unsigned fld=eDakHomeTOFull;fld<eDakHours;fld++)
		CopyStringIfChanged(m_ValueStrings[fld], m_CurrentPacket+fld+14, 1, fld, m_ValueChanged[fld]);

	// Null terminate them
//	for (unsigned i=0;i<eDakNumFields;i++)
//	{
//		m_ValueStrings[i][g_StringLengths[i]] = 0;
		// TODO: only set changed if they really changed
//		m_ValueChanged[i] = 1;
//	}


//	m_ValueStrings[eDakHours][hourstrlen] = 0;
//	m_ValueStrings[eDakMinutes][minstrlen] = 0;
//	m_ValueStrings[eDakSeconds][secstrlen] = 0;
//	m_ValueStrings[eDakMilli][milstrlen] = 0;

	// Remove Trailing Spaces from seconds and milliseconds
//	char *lSpace = strchr(m_ValueStrings[eDakSeconds], ' ');
//	if (lSpace && lSpace[0])
//		lSpace[0] = 0;
//	lSpace = strchr(m_ValueStrings[eDakMilli], ' ');
//	if (lSpace && lSpace[0])
//		lSpace[0] = 0;

	// TODO: Send changed message if this is diff than the old packet

	for (unsigned i=0;i<eDakNumFields;i++)
		if (m_ValueChanged[i])
			return true;

	// Nothing Changed
	return false;

}

bool StatsDaktronics::CheckPacket(void)
{
//	char * tempstr = m_CurrentPacket;
	// Test Clock
 	// Need at least one colon in there somewhere...
//  	if (!strchr(m_CurrentPacket, ':'))
//  		return false;
// 	for (unsigned i=1;i<8;i++)
// 		if (((m_CurrentPacket[i]>'9')||(m_CurrentPacket[i]<'0')) 
// 			&& m_CurrentPacket[i] != ' ' 
// 			&& m_CurrentPacket[i] != ':'
// 			&& m_CurrentPacket[i] != '.')
// 			return false;
// 	// Test Clock Status
// 	if ((m_CurrentPacket[8] != 's') && (m_CurrentPacket[8] != ' '))
// 		return false;
// 	// Test Shot Clock through Time Outs (period can be unexpected char according to doc)
// 	for (unsigned i=eDakShotClock;i<eDakPeriod;i++)
// 	{
// 		unsigned ind = i+7;
// 		if (((m_CurrentPacket[ind]>'9')||(m_CurrentPacket[ind]<'0')) && m_CurrentPacket[ind] != ' ')
// 			return false;
// 	}

	//Daktronics supplies a high/low 2 byte checksum
	if (CheckCheckSum())
		return true;
	else
		return false;


}

bool StatsDaktronics::CheckCheckSum()
{
	//Daktronics supplies a high/low 2 byte checksum
	//This checks the checksum against the data (excluding start & end chars & the checksum itself)
	//  and returns false if it fails
	char * buf = m_CurrentPacket;
	int start = 1;
	int stop = 27;
	char low = NULL;
	char high = NULL;
	int	iIndex;
	unsigned int	uiChecksum;
	const unsigned char	ucaHexChar[17] = { "0123456789ABCDEF" };

	// calculate checksum
	for (iIndex=start,uiChecksum=0; iIndex<=stop; iIndex++)
		uiChecksum += (unsigned int)buf[iIndex];

	// AND total with FF to isolate low byte
	uiChecksum &= 0xff;

	// convert low nibble to hex
	low = ucaHexChar[uiChecksum & 0x0f];

	// convert high nibble to hex
	high = ucaHexChar[uiChecksum >> 4];

	if (low != m_CurrentPacket[29])
		return false;
	if (high != m_CurrentPacket[28])
		return false;

	return true;
}

char * StatsDaktronics::GetBoardNextChangedKey(void)
{
	for (unsigned i=0;i<eDakNumFields;i++)
		if (m_ValueChanged[i])
		{
			m_ValueChanged[i] = false;
			return ((char *)g_DakKeyStrings[i]);
		}
	return NULL;
}

char * StatsDaktronics::GetBoardValue(const char key[])
{
	const char * ckey = key;
	for (unsigned i=0;i<eDakNumFields;i++)
		if (!_stricmp(g_DakKeyStrings[i],ckey))
		{
			return m_ValueStrings[i];
		}

	return NULL;
}