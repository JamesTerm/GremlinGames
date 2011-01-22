#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
#define TLMP70_PACKET_LENGTH 60
#else SCOREBOARD_READ_FROM_SAMPLE_TEXT
#define TLMP70_PACKET_LENGTH 60
#endif

#define TLMP70_INI_HEADER "TransLuxMP70"

enum TLMP70FBFields
{
	eTLMP70FBClock,
	eTLMP70FBQuarter,
	eTLMP70FBHomeScore,
	eTLMP70FBVisitorScore,
	eTLMP70FBDown,
	eTLMP70FBToGo,
	eTLMP70FBBallOn,
	eTLMP70FBFieldTimer,
	eTLMP70FBNumFields
};

const unsigned g_TLMP70FBStringLengths[eTLMP70FBNumFields] = {
	7, 1, 2, 2, 1, 2, 2, 2
};

static const char *g_TLMP70FBKeyStrings[] = {
	"TLMP70FBClock",
	"TLMP70FBQuarter",
	"TLMP70FBHomeScore",
	"TLMP70FBVisitorScore",
	"TLMP70FBDown",
	"TLMP70FBToGo",
	"TLMP70FBBallOn",
	"TLMP70FBFieldTimer"	
};

class StatsTransLuxMP70 : public StatsBoardBase
{
public:
	StatsTransLuxMP70(void);
	~StatsTransLuxMP70(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void) {return TLMP70_PACKET_LENGTH;}
	virtual const char * GetBoardINIHeader() {return TLMP70_INI_HEADER;}
	virtual unsigned GetNumExtraEOMThreshhold(void) {return 30;}
	virtual const char * GetBoardTestFileName() {return "D:\\docs\\transluxmp70\\ids\\testorlando.TXT";}
	static const char * GetStaticINIHeader() {return TLMP70_INI_HEADER;}
	virtual const char GetEndofMessageChar() {return 0x81 /*EOT*/;} // Actually there is no end of msg. Just start of new!
	virtual void Initialize(void);
	virtual void Destroy(void);

protected:

	// StatsBoardBase Methods
	virtual bool ParsePacket(void); // returns true if changed
	virtual bool CheckPacket(void); // returns true if packet is OK
	virtual char * GetBoardValue(const char key[]);
	virtual char * GetBoardNextChangedKey(void);

	// StatsTransLuxMP70 Data
	char * m_ValueStrings[eTLMP70FBNumFields];
	bool m_ValueChanged[eTLMP70FBNumFields];
};

StatsTransLuxMP70 * getStatsTLMP70();