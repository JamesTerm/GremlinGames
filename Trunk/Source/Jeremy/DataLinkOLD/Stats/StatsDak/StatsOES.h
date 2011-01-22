#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

// The OES Sample has an extra newline char at the end of a packet
#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
#define OES_PACKET_LENGTH 23
#else SCOREBOARD_READ_FROM_SAMPLE_TEXT
#define OES_PACKET_LENGTH 23
#endif
#define OES_INI_HEADER "OES"

enum OESFields
{
	eOESClock,
	eOESShotClock,
	eOESAwayScore,
	eOESHomeScore,
	eOESHomeFouls,
	eOESAwayFouls,
	eOESHomeTOPart,
	eOESHomeTOFull,
	eOESAwayTOPart,
	eOESAwayTOFull,
	eOESPeriod,
	eOESNumFields
};

const unsigned g_OESStringLengths[eOESNumFields] = {
	8, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

static const char *g_OESKeyStrings[] = {
	"OESClock",
	"OESShotClock",
	"OESAwayScore",
	"OESHomeScore",
	"OESHomeFouls",
	"OESAwayFouls",
	"OESHomeTOPart",
	"OESHomeTOFull",
	"OESAwayTOPart",
	"OESAwayTOFull",
	"OESPeriod"
};

class StatsOes : public StatsBoardBase
{
public:
	StatsOes(void);
	~StatsOes(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void) {return OES_PACKET_LENGTH;}
	virtual const char * GetBoardINIHeader() {return OES_INI_HEADER;}
	virtual const char * GetBoardTestFileName() { return SCOREBOARD_FILENAME_OES; }

	virtual const char GetEndofMessageChar() {return 0x0d /*line feed*/;}

	static const char * GetStaticINIHeader() {return OES_INI_HEADER;}
	virtual void Initialize(void);
	virtual void Destroy(void);

protected:

	// StatsBoardBase Methods
	virtual bool ParsePacket(void); // returns true if changed
	virtual bool CheckPacket(void); // returns true if packet is OK
	virtual char * GetBoardValue(const char key[]);
	virtual char * GetBoardNextChangedKey(void);

	// StatsOes Data
	char * m_ValueStrings[eOESNumFields];
	bool m_ValueChanged[eOESNumFields];

	// StatsOes Utility Methods
	bool CheckCheckSum(void);
};

StatsOes * getStatsOes();