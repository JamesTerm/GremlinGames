#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

#define DSI_PACKET_LENGTH 35
#define DSI_INI_HEADER "DSI"

enum DSIFields
{
	eDSIClock,
	eDSIShotClock,
	eDSINumFields
};

const unsigned g_DSIStringLengths[eDSINumFields] = {
	8, 8
};

static const char *g_DSIKeyStrings[] = {
	"DSIClock",
	"DSIShotClock"
};

class StatsDsi : public StatsBoardBase
{
public:
	StatsDsi(void);
	~StatsDsi(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void) {return DSI_PACKET_LENGTH;}
	virtual const char * GetBoardINIHeader() {return DSI_INI_HEADER;}
	virtual const char * GetBoardTestFileName() {return "D:\\docs\\dsi\\ids\\GSW.TXT";}
	virtual const char GetEndofMessageChar() {return 3 /*ETX*/;}	// optional to help test, for those without a standard
															// size for packets
	virtual void Initialize(void);
	virtual void Destroy(void);

	static const char * GetStaticINIHeader() {return DSI_INI_HEADER;}

protected:

	// StatsBoardBase Methods
	virtual bool ParsePacket(void); // returns true if changed
	virtual bool CheckPacket(void); // returns true if packet is OK
	virtual char * GetBoardValue(const char key[]);
	virtual char * GetBoardNextChangedKey(void);

	// StatsDsi Data
	char * m_ValueStrings[eDSINumFields];
	bool m_ValueChanged[eDSINumFields];
	unsigned m_FieldNumber;

};

StatsDsi * getStatsDsi();