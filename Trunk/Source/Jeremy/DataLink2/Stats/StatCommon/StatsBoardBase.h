#pragma once

#define MAX_VALUE_NAME = 16383;
#define MAX_KEY_LENGTH = 255;

#define wchar2char(wchar2char_pwchar_source) \
	const size_t wchar2char_Length=wcstombs(NULL,wchar2char_pwchar_source,0)+1; \
	char *wchar2char_pchar = (char *)_alloca(wchar2char_Length);; /* ";;" is needed to fix a compiler bug */ \
	wcstombs(wchar2char_pchar,wchar2char_pwchar_source,wchar2char_Length);

#define char2wchar(char2wchar_pchar_source) \
	const size_t char2wchar_Length=((strlen(char2wchar_pchar_source)+1)*sizeof(wchar_t)); \
	wchar_t *char2wchar_pwchar = (wchar_t *)_alloca(char2wchar_Length);; /* ";;" is needed to fix a compiler bug */ \
	mbstowcs(char2wchar_pwchar,char2wchar_pchar_source,char2wchar_Length);

class SerialPort;

class StatsBoardBase 
{
public:
	StatsBoardBase(void);
	virtual ~StatsBoardBase(void);

	virtual unsigned GetNumBytesToRead(void)=0;
	virtual unsigned GetNumExtraEOMThreshhold(void) {return 0;}
	virtual const char * GetBoardINIHeader()=0;
	virtual const wchar_t * GetBoardTestFileName(const wchar_t * FileName)=0;
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
	const bool StatsBoardBase::RegReadUseText(void);

	// Used by ParseReg.
	virtual const char * GetPlugInName()=0;

protected:
	//void ThreadProcessor(void);
	friend Thread<StatsBoardBase>;
	void operator()(const void *dummy);

	HANDLE m_Handle;

	SerialPort * m_pSerialPort;
	HANDLE m_hPortCheckEvent;

	//
	// StatsBoardBase
	//

	// When using text file for serial input (testing).
	BYTE *m_FileInMemory;
	unsigned m_NoBytesRead;
	unsigned m_PlaceInFile;
	void OpenFile(void);
	void AdvanceFile(void);
	bool m_UseTextFile;
	virtual size_t ManualAdvance(void);

	// Implemented in Base
	virtual void DisconnectFromScoreBoard(void);
	void ReadFromScoreBoard(void);
	void CopyStringIfChanged(char * destination, const char * source, size_t num, unsigned index, bool &val_changed);

	char * m_CurrentPacket;
	char * m_PartialPacket;
	BYTE * m_SerialInBuffer;
	int	   m_PacketSize;

	// Implemented in derived class.
	virtual void ConnectToScoreBoard(void)=0;
	virtual bool ParsePacket(void)=0; // returns true if changed
	virtual bool CheckPacket(void)=0; // returns true if packet is OK
	virtual bool CheckCheckSum(void)=0; // returns true if checksum is ok
	virtual const char * GetBoardValue(const char key[])=0;
	virtual const char * GetBoardNextChangedKey(void)=0;

	wchar_t m_CurrentKey[256];
	Thread<StatsBoardBase> *m_pThread;

	const wchar_t *FileName;
	wchar_t m_Executable[MAX_PATH];

private:

};


class StatsBoardNone : public StatsBoardBase
{
public:
	StatsBoardNone(void);
	virtual ~StatsBoardNone(void);

	// StatsBoardBase Methods
	virtual unsigned GetNumBytesToRead(void) {return 0;}
	virtual const char * GetBoardINIHeader() {return "";}
	virtual const wchar_t * GetBoardTestFileName(const wchar_t * FileName) {return L"";}
	virtual const char GetEndofMessageChar() {return 0;}

protected:
	// StatsBoardBase Methods
	virtual bool ParsePacket(void) {return false;}
	virtual bool CheckPacket(void) {return false;}
	virtual bool CheckCheckSum(void) {return false;}
	virtual const char * GetBoardValue(const char key[]) {return NULL;}
	virtual const char * GetBoardNextChangedKey(void) {return NULL;}

 	virtual void Initialize(void);
	virtual void Destroy(void);
};


struct SerialBase2_SBinfo
{
	std::string Name, Type, Board, Sport;
	ULONG SportCode, Port, BaudRate, DataBits, Parity, StopBits;
};

class SerialBase2 : public StatsBoardBase
{
	public:
		virtual void SetInfo(const SerialBase2_SBinfo &info) {m_IniInfo=info;}
		const SerialBase2_SBinfo &GetInfo() const { return m_IniInfo; }
 		virtual void Initialize(void) {}
		virtual void Destroy(void) {}
		static wchar_t *ParseKey(const wchar_t source[], wchar_t *buffer,size_t &Port);

		virtual const char * GetPlugInName() = 0;


	protected:
		virtual bool ParsePacket(void);
		virtual bool CheckPacket(void);
		virtual bool CheckCheckSum(void);

	//TODO: change the connect to not rely on the ini file... instead use a cached member variable set with SetInfo
	protected:
		virtual void ConnectToScoreBoard(void);
		//virtual void DisconnectFromScoreBoard(void);

	protected:
		virtual const wchar_t * GetBoardTestFileName(const wchar_t * FileName) {return L"";}
		virtual const char GetEndofMessageChar() {return 4;} //EOT (should be common to all decks)

	protected:
		// Used by ParsePacket.
		virtual size_t GetFieldWidth(size_t index)=0;
		virtual size_t GetFieldOffset(size_t index)=0;
		virtual size_t GetNumFields()=0;

		struct ParsedElement 
		{
			//This is initially empty and written to by the board in ParsePacket.
			std::string ElementName;
			bool ElementUpdated;
		};
		virtual ParsedElement &GetParsedElement(size_t index)=0;

		virtual void CopyStringIfChanged2(ParsedElement &destination, char * source, size_t num);

	private:
		SerialBase2_SBinfo m_IniInfo;
};


class ParseReg
{
	public:
		ParseReg(const char * BoardName) : m_BoardName(BoardName) {}
		void operator()(void);

		const wchar_t * const ParseReg::GetRegValue(const wchar_t * regPath,
													const wchar_t * regValue,
													const wchar_t * retVal);

	protected:
		virtual void AddSerialInfo(const SerialBase2_SBinfo &info)=0;

	private:
		const char * const m_BoardName;
		const wchar_t * m_RegActivePath;

};