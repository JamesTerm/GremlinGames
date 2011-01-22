#pragma once

const char * const cwsz_PlugInName= "OES";
const unsigned c_NoFormatBytes = 2;	// 2 is added for the bom + eom markers.
const wchar_t cwsz_Remainder[23] = L"\\Stats Plugins\\OES.txt";

#ifndef COUNTOF
#define COUNTOF(x) sizeof(x)/sizeof(*x)
#endif

enum eMajorSportType
{
	eBasketball/*,	//Will support more as soon as we can get some data on message construction.
	eFootball,
	eBaseball,
	eHockey,
	eSoccer,
	eVolleyball*/
};

class BoardOES : public SerialBase2
{
public:
	BoardOES(void);
	virtual ~BoardOES(void);

	// For testing serial input from a serial dump in a text file.
	virtual const wchar_t * GetBoardTestFileName(const wchar_t * FileName);

	virtual void SetInfo(const SerialBase2_SBinfo &info);

protected: //from StatBoardBase
	virtual unsigned GetNumBytesToRead(void);
	virtual const char * GetBoardINIHeader() {return cwsz_PlugInName;}
	virtual const char GetEndofMessageChar() {return 0x0a;}

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

	//TODO: find the base version of this in StatBoardBase
	virtual void SetSport(const char *Sport);
	//switch sport major type call the aggregator w/ minor type
private:
	eMajorSportType m_Sport;

	////////////////////////
	////// Basketball //////
	class Basketball
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
		size_t m_SportType; //may want to use your minor enum here
		//caching the width sizes in constructor to save work in GetNumBytesToRead
		std::vector<unsigned> m_PacketLength_cache;
		ParsedElement * m_pElement;

		void CloseResources(void);

	} m_Basketball;
};

//TODO: Make this templated when we add future plugins.
class ParseReg_OES : public ParseReg
{
public:
	ParseReg_OES(const char * BoardName);
	~ParseReg_OES();
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
		BoardOES board;
	};

	Plugin_Element *GetPlugInElement(const wchar_t key[]);

	std::vector<Plugin_Element> m_PlugIn;
};