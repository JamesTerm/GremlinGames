#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

#define DAK_PACKET_LENGTH 31

class SerialPort;

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

class StatsDak : public WorkerThread
{
public:
	StatsDak(void);
	~StatsDak(void);

	const wchar_t * GetNextChangedKey(void);
	const wchar_t * GetValue(const wchar_t key[]);
	void StartUpdates(void);
	void StopUpdates(void);
	HANDLE GetHandle() {return m_Handle;}
	void ReleaseValue(const wchar_t value[]);
protected:
	void ThreadProcessor(void);

	HANDLE m_Handle;

	SerialPort * m_pSerialPort;
	HANDLE m_hPortCheckEvent;
	
	unsigned m_ValuesBlocked;
	unsigned m_ParsingBlocks;

	VariableBlock m_TurnProcess;

	//
	// StatsDak
	//

	// Temp, read from test file
	byte *m_FileInMemory;
	unsigned m_NoBytesRead;
	unsigned m_PlaceInFile;
	void OpenFile(void);
	void AdvanceFile(void);

	// Real Implementation
	void ConnectToDaktronics(void);
	void DisconnectFromDaktronics(void);
	void ReadFromDaktronics(void);



	char m_CurrentPacket[DAK_PACKET_LENGTH+1];

	char * m_ValueStrings[eDakNumFields];
	bool m_ValueChanged[eDakNumFields];

	void ParsePacket(void);
	bool CheckPacket(void);
	bool CheckCheckSum(void);
	void CopyStringIfChanged(char * destination, const char * source, size_t num, unsigned index);


	wchar_t m_CurrentKey[256];
};



StatsDak * getStatsDak();

// Stats Interface designed by Andrew

// Start by calling this. It will return a handle that is "triggered"
// when a change has occured to the stats. For instance, you could do a
// WaitForSingleEvent( handle, 100 ); on this event to see when stats have
// changed.
extern "C" STATS_API HANDLE             live_updates_start( void );

// Get a value from a key.
extern "C" STATS_API const wchar_t    *live_updates_get_value( const wchar_t key[] );
// Because you are returned a pointer that you do not own from
//live_updates_get_value,
// you will need to release it with this fcn. Do not call another
//live_updates_get_value
// until values returned by the previous values have been released.
extern "C" STATS_API void                 live_updates_release_value( const
wchar_t value[] );

// Get the next key that has changed
extern "C" STATS_API const wchar_t    *live_updates_get_changed_key( void );
// Same as above, you need to release the pointers before you can read
//the next changed key
extern "C" STATS_API void                 live_updates_release_changed_key( const
wchar_t key[] );

// All done with stats
extern "C" STATS_API void                 live_updates_end( HANDLE hHandle );