#pragma once

const char * const cwsz_PlugInName= "Generic";
const unsigned c_NoFormatBytes = 0;	// Not used
const wchar_t cwsz_Remainder[28] = L"\\Stats Plugins\\Generic.txt";

#ifndef COUNTOF
#define COUNTOF(x) sizeof(x)/sizeof(*x)
#endif

enum eMajorSportType
{
	eGeneric
};

class BoardGeneric : public SerialBase2
{
public:
	BoardGeneric(void);
	virtual ~BoardGeneric(void);

	// For testing serial input from a serial dump in a text file.
	virtual const wchar_t * GetBoardTestFileName(const wchar_t * FileName);

	virtual void SetInfo(const SerialBase2_SBinfo &info);

protected: //from StatBoardBase
	virtual unsigned GetNumBytesToRead(void);
	virtual const char * GetBoardINIHeader() {return cwsz_PlugInName;}
	virtual const char GetEndofMessageChar() {return 0x02;}

protected: //from SerialBase2
	virtual const char * GetBoardValue(const char key[]);
	virtual const char * GetBoardNextChangedKey(void);
	virtual bool ParsePacket(void);

	virtual bool CheckPacket(void);
	virtual bool CheckCheckSum(void);

	virtual size_t GetFieldWidth(size_t index);
	virtual size_t GetFieldOffset(size_t index);

	virtual size_t GetNumFields();
	virtual ParsedElement &GetParsedElement(size_t index);

	virtual const char * GetPlugInName() {return cwsz_PlugInName;}

	virtual void SetSport(const char *Sport);
private:
	eMajorSportType m_Sport;

	////////////////////////
	////// Basketball //////
	class GenericSport
	{
	public:
		Basketball();
		~Basketball();
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

	} m_GenericSport;
};

class ParseReg_Generic : public ParseReg
{
public:
	ParseReg_Generic(const char * BoardName);
	~ParseReg_Generic();
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
		BoardGeneric board;
	};

	Plugin_Element *GetPlugInElement(const wchar_t key[]);

	std::vector<Plugin_Element> m_PlugIn;
};