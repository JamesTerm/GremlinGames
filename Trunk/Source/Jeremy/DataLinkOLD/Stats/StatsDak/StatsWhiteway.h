#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

#define WHITEWAY_PACKET_LENGTH 22
#define WHITEWAY_INI_HEADER "Whiteway"

enum WWFields
{
	eWWPeriod,
	eWWMinutes,
	eWWSeconds,
	eWWMilliseconds,
	eWWAwayScore,
	eWWHomeScore,
	eWWShotClock,
	eWWClock,
	eWWNumFields
};

const unsigned g_WWStringLengths[eWWNumFields] = {
	4, 4, 4, 4, 4, 4, 4, 12
};

static const char *g_WWKeyStrings[] = {
	"WWPeriod",
	"WWMin",
	"WWSec",
	"WWTen",
	"WWAwayScore",
	"WWHomeScore",
	"WWShotClock",
	"WWClock"
};

class StatsWhiteway : public StatsBoardBase
{
public:
	StatsWhiteway(void);
	~StatsWhiteway(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void)		{ return WHITEWAY_PACKET_LENGTH; }
	virtual const char * GetBoardINIHeader()		{ return WHITEWAY_INI_HEADER; }
	virtual const char * GetBoardTestFileName()		{ return SCOREBOARD_FILENAME_WHITEWAY; }
	static const char * GetStaticINIHeader()		{ return WHITEWAY_INI_HEADER; }
	virtual const char GetEndofMessageChar()		{return 3 /*ETX*/;}
	virtual void Initialize(void);
	virtual void Destroy(void);


protected:

	// StatsBoardBase Methods
	virtual bool ParsePacket(void); // returns true if changed
	virtual bool CheckPacket(void); // returns true if packet is OK
	virtual char * GetBoardValue(const char key[]);
	virtual char * GetBoardNextChangedKey(void);

	// StatsWhiteway Data
	char * m_ValueStrings[eWWNumFields];
	bool m_ValueChanged[eWWNumFields];

	// StatsWhiteway Utility Methods
	bool IsAlphaNumeric(char c);
	bool CheckCheckSum(void);
};

StatsWhiteway * getStatsWhiteway();