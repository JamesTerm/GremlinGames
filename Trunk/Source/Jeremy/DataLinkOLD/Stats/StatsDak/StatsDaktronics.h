#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

#ifdef SCOREBOARD_READ_FROM_SAMPLE_TEXT
#define DAK_PACKET_LENGTH 31
#else SCOREBOARD_READ_FROM_SAMPLE_TEXT
#define DAK_PACKET_LENGTH 31
#endif

#define DAK_INI_HEADER "Daktronics"

enum DakFields
{
	eDakClock,//[7+1];
	eDakClockStatus,//[7+1];
	eDakShotClock,//[2+1];
	eDakHomeScore,//[3+1];
	eDakAwayScore,//[3+1];
	eDakHomeFouls,//[2+1];
	eDakAwayFouls,//[2+1];
	eDakHomeTOFull,//[1+1];
	eDakHomeTOPart,//[1+1];
	eDakHomeTOTotal,//[1+1];
	eDakAwayTOFull,//[1+1];
	eDakAwayTOPart,//[1+1];
	eDakAwayTOTotal,//[1+1];
	eDakPeriod,//[1+1];
	eDakHours,
	eDakMinutes,
	eDakSeconds,
	eDakMilli,
	eDakNumFields
};

const unsigned g_StringLengths[eDakNumFields] = {
	7, 1, 2, 3, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 8, 8, 8, 8
};

static const char *g_DakKeyStrings[] = {
	"DakClock",//[7+1];
	"DakClockStatus",//[1+1];
	"DakShotClock",//[2+1];
	"DakHomeScore",//[3+1];
	"DakAwayScore",//[3+1];
	"DakHomeFouls",//[2+1];
	"DakAwayFouls",//[2+1];
	"DakHomeTOFull",//[1+1];
	"DakHomeTOPart",//[1+1];
	"DakHomeTOTotal",//[1+1];
	"DakAwayTOFull",//[1+1];
	"DakAwayTOPart",//[1+1];
	"DakAwayTOTotal",//[1+1];
	"DakPeriod",//[1+1];	
	"DakHhr",
	"DakMin",
	"DakSec",
	"DakTen"
};

class StatsDaktronics : public StatsBoardBase
{
public:
	StatsDaktronics(void);
	~StatsDaktronics(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void) {return DAK_PACKET_LENGTH;}
	virtual const char * GetBoardINIHeader() {return DAK_INI_HEADER;}
	//virtual const char * GetBoardTestFileName() {return "D:\\docs\\daktronics\\ids\\testorlando.TXT";}
	virtual const char * GetBoardTestFileName()  { return SCOREBOARD_FILENAME_DAKTRONICS; }
	static const char * GetStaticINIHeader() {return DAK_INI_HEADER;}
	virtual const char GetEndofMessageChar() {return 4 /*EOT*/;}
	virtual void Initialize(void);
	virtual void Destroy(void);

protected:

	// StatsBoardBase Methods
	virtual bool ParsePacket(void); // returns true if changed
	virtual bool CheckPacket(void); // returns true if packet is OK
	virtual char * GetBoardValue(const char key[]);
	virtual char * GetBoardNextChangedKey(void);


	// StatsDaktronics Utility Methods
	bool CheckCheckSum(void);

	// StatsDaktronics Data
	char * m_ValueStrings[eDakNumFields];
	bool m_ValueChanged[eDakNumFields];
};

StatsDaktronics * getStatsDaktronics();