#pragma once

const char * const cwsz_PlugInName= "TransLux";
const unsigned c_NoFormatBytes = 1;
const wchar_t cwsz_Remainder[28] = L"\\Stats Plugins\\TransLux.txt";

#ifndef COUNTOF
#define COUNTOF(x) sizeof(x)/sizeof(*x)
#endif

enum eMajorSportType
{
	eFootball
};

class BoardTransLux : public SerialBase2
{
public:
	BoardTransLux(void);
	virtual ~BoardTransLux(void);

	// For testing serial input from a serial dump in a text file.
	virtual const wchar_t * GetBoardTestFileName(const wchar_t * FileName);

	virtual void SetInfo(const SerialBase2_SBinfo &info);

	virtual const char * GetPlugInName() {return cwsz_PlugInName;}

protected: //from StatBoardBase
	virtual unsigned GetNumBytesToRead(void);
	virtual const char * GetBoardINIHeader() {return cwsz_PlugInName;}
	virtual unsigned GetNumExtraEOMThreshhold(void) {return 30;}
	virtual const char GetEndofMessageChar() {return 0x81;}

protected: //from SerialBase2
	virtual const char * GetBoardValue(const char key[]);
	virtual const char * GetBoardNextChangedKey(void);
	bool ParsePacket(void);

	virtual bool CheckPacket(void);
	virtual bool CheckCheckSum(void);

	virtual size_t GetFieldWidth(size_t index);
	virtual size_t GetFieldOffset(size_t index);
	virtual size_t GetNumFields();
	virtual ParsedElement &GetParsedElement(size_t index);

	virtual void SetSport(const char *Sport);

	unsigned m_FieldNumber;

private:
	eMajorSportType m_Sport;

	////////////////////////
	////// Football //////
	class Football
	{
	public:
		Football();
		~Football();
		void SetSportType(size_t sportType);
		size_t GetFieldWidth(size_t index);
		size_t GetFieldOffset(size_t index);
		unsigned GetNumBytesToRead(void);
		ParsedElement &GetParsedElement(size_t index);
		size_t GetNumFields();
		virtual const char * GetBoardNextChangedKey(void);
		const char *GetBoardValue(const char key[]);
		void SetSport(const char *Sport);

	private:
		const char *GetFieldName(size_t index);
		size_t m_SportType;

		//caching the width sizes in constructor to save work in GetNumBytesToRead
		std::vector<unsigned> m_PacketLength_cache;
		ParsedElement * m_pElement;

		void CloseResources(void);

	} m_Football;
};

class ParseReg_TransLux : public ParseReg
{
public:
	ParseReg_TransLux(const char * BoardName);
	~ParseReg_TransLux();
	HANDLE live_updates_start( void );
	const wchar_t *live_updates_get_changed_key( void );
	void live_updates_release_changed_key( const wchar_t key[] );
	const wchar_t *live_updates_get_value( const wchar_t key[] );
	void live_updates_release_value( const wchar_t value[] );

protected: // From ParseReg
	virtual void AddSerialInfo(const SerialBase2_SBinfo &info);

private:
	struct Plugin_Element
	{
		wchar_t *keyName;
		BoardTransLux board;
	};

	Plugin_Element *GetPlugInElement(const wchar_t key[]);

	std::vector<Plugin_Element> m_PlugIn;
};