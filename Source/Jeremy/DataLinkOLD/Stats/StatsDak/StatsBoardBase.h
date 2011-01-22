#ifdef SCOREBOARD_EXPORTS
#define STATS_API __declspec(dllexport)
#else
#define STATS_API __declspec(dllimport)
#pragma comment (lib,"ScoreBoardStats")
#endif

//#define DAK_PACKET_LENGTH 31
//#define DAK_INI_HEADER "Daktronics"

class SerialPort;

enum eSBType
{
	SBNone = 0,
	SBDaktronics = 1,
	SBWhiteway = 2,
	SBDsi = 3,
	SBOes = 4,
	SBTransLuxMP70 = 5,
	SBCount = 6
	
};

class StatsBoardBase : public WorkerThread
{
public:
	StatsBoardBase(void);
	~StatsBoardBase(void);

	virtual unsigned GetNumBytesToRead(void)=0;
	virtual unsigned GetNumExtraEOMThreshhold(void) {return 0;}
	virtual const char * GetBoardINIHeader()=0;
	virtual const char * GetBoardTestFileName()=0;
	virtual const char GetEndofMessageChar()=0;	// No longer optional

	const wchar_t * GetNextChangedKey(void);
	const wchar_t * GetValue(const wchar_t key[]);
	void ReleaseValue(const wchar_t value[]);
	void StartUpdates(void);
	void StopUpdates(void);
	virtual void Initialize(void)=0;
	virtual void Destroy(void)=0;
	HANDLE GetHandle() {return m_Handle;}
	void SetHandle(HANDLE h) 
	{
		m_Handle = h;
	}
protected:
	void ThreadProcessor(void);

	HANDLE m_Handle;

	SerialPort * m_pSerialPort;
	HANDLE m_hPortCheckEvent;

	//
	// StatsBoardBase
	//

	// Temp, read from test file
	byte *m_FileInMemory;
	unsigned m_NoBytesRead;
	unsigned m_PlaceInFile;
	void OpenFile(void);
	void AdvanceFile(void);

	// Implementated in Base
	void ConnectToScoreBoard(void);
	void DisconnectFromScoreBoard(void);
	void ReadFromScoreBoard(void);
	void CopyStringIfChanged(char * destination, const char * source, size_t num, unsigned index, bool &val_changed);

	char * m_CurrentPacket;
	char * m_PartialPacket;
	byte * m_SerialInBuffer;
	int		m_PacketSize;

	virtual bool ParsePacket(void)=0; // returns true if changed
	virtual bool CheckPacket(void)=0; // returns true if packet is OK
	virtual char * GetBoardValue(const char key[])=0;
	virtual char * GetBoardNextChangedKey(void)=0;

	wchar_t m_CurrentKey[256];
};

StatsBoardBase * getStatsBoard(unsigned esb);

class StatsBoardNone : public StatsBoardBase
{
public:
	StatsBoardNone(void);
	~StatsBoardNone(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void) {return 0;}
	virtual const char * GetBoardINIHeader() {return "";}
	virtual const char * GetBoardTestFileName() {return "";}
	virtual const char GetEndofMessageChar() {return 0;}

protected:

	// StatsBoardBase Methods
	virtual bool ParsePacket(void) {return false;}
	virtual bool CheckPacket(void) {return false;}
	virtual char * GetBoardValue(const char key[]) {return NULL;}
	virtual char * GetBoardNextChangedKey(void) {return NULL;}

	virtual void Initialize(void);
	virtual void Destroy(void);
};

StatsBoardNone * getStatsBoardNone();


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