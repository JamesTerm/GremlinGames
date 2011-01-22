#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

#define WHITEWAY_PACKET_LENGTH 12
#define WHITEWAY_INI_HEADER "WhitewayRainbow"

enum WWRFields
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

const unsigned g_WWRStringLengths[eWWNumFields] = {
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

class StatsWWRainbow : public StatsBoardBase
{
public:
	StatsWWRainbow(void);
	~StatsWWRainbow(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void) {return WHITEWAY_PACKET_LENGTH;}
	virtual const char * GetBoardINIHeader() {return WHITEWAY_INI_HEADER;}
	//virtual const char * GetBoardTestFileName() {return "D:\\docs\\whiteway\\ids\\staples.TXT";}
	virtual const char * GetBoardTestFileName() 
		//{return "C:\\Users\\Art\\NewTek_Serial\\ArtsChanges\\Scoreboard Protocols\\WWProblem_9Sep09\\whiteway.TXT";}
		{return "C:\\Users\\Art\\NewTek_Serial\\ArtsChanges\\Scoreboard Protocols\\WWProblem_9Sep09\\whiteway.TXT";}
		//{return "C:\\Users\\Art\\NewTek_Serial\\ArtsChanges\\Scoreboard Protocols\\whiteway\\ids\\staples.TXT";}
	static const char * GetStaticINIHeader() {return WHITEWAY_INI_HEADER;}
	virtual const char GetEndofMessageChar() {return 0xAA ;}
	virtual void Initialize(void);
	virtual void Destroy(void);


protected:

	// StatsBoardBase Methods
	virtual bool ParsePacket(void); // returns true if changed
	virtual bool CheckPacket(void); // returns true if packet is OK
	virtual char * GetBoardValue(const char key[]);
	virtual char * GetBoardNextChangedKey(void);

	// StatsWhiteway Data
	char * m_ValueStrings[eWWRNumFields];
	bool m_ValueChanged[eWWRNumFields];

	// StatsWhiteway Utility Methods
	bool IsAlphaNumeric(char c);
	bool CheckCheckSum(void);
};

StatsWhiteway * getStatsWhiteway();