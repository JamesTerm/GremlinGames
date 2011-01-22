#pragma once

#include <vcclr.h>

namespace DataLinkUI
{

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

//	Converts wchar_t to char -result contained in wchar2char_pchar.
#define wchar2char(wchar2char_pwchar_source) \
	const size_t wchar2char_Length=wcstombs(NULL,wchar2char_pwchar_source,0)+1; \
	char *wchar2char_pchar = (char *)_alloca(wchar2char_Length);; /* ";;" is needed to fix a compiler bug */ \
	wcstombs(wchar2char_pchar,wchar2char_pwchar_source,wchar2char_Length);

//	Converts char to wchar_t -result contained in char2wchar_pwchar.
#define char2wchar(char2wchar_pchar_source) \
	const size_t char2wchar_Length=((strlen(char2wchar_pchar_source)+1)*sizeof(wchar_t)); \
	wchar_t *char2wchar_pwchar = (wchar_t *)_alloca(char2wchar_Length);; /* ";;" is needed to fix a compiler bug */ \
	mbstowcs(char2wchar_pwchar,char2wchar_pchar_source,char2wchar_Length);

//	Converts string to wchar_t -result contained in wch.
#define string2wchar(string2wchar_pchar_source) \
	pin_ptr<const wchar_t> wch = PtrToStringChars(string2wchar_pchar_source); \
	size_t convertedChars = 0; \
	size_t sizeInBytes = ((string2wchar_pchar_source->Length + 1) * 2); \
	char *ch = (char *)_alloca(sizeInBytes);; /* ";;" is needed to fix a compiler bug */ \
	wcstombs_s(&convertedChars, ch, sizeInBytes, wch, sizeInBytes);

//	Converts wchar_t to System::String -result contained in ss.
#define wchar2string(wchar2string_pstring_source) \
	String ^ss = gcnew String(wchar2string_pstring_source);

// Useful macro for getting the count of something.
#ifndef COUNTOF
#define COUNTOF(x) sizeof(x)/sizeof(*x)
#endif

// Global constants defined for registry values.
const static wchar_t * cwsz_LiveTextPath = L"SOFTWARE\\Wow6432Node\\Newtek\\LiveText";
const static wchar_t * cwsz_RegPath = L"Software\\NewTek\\DataLink";
const static wchar_t * cwsz_ActivePath = L"Software\\NewTek\\DataLink\\Active";
const static wchar_t * cwsz_ActiveQueryPath = L"Software\\NewTek\\DataLink\\Active\\Query";
const static wchar_t * cwsz_ActiveSitesPath = L"Software\\NewTek\\DataLink\\Active\\Sites";
const static wchar_t * cwsz_SettingsPath = L"Software\\NewTek\\DataLink\\Settings";

const static wchar_t * cwsz_UseSerial = L"SerialEnabled";
const static wchar_t * cwsz_UseDatabase = L"DatabaseEnabled";
const static wchar_t * cwsz_UseNetwork = L"NetworkEnabled";

const static wchar_t * cwsz_UseTextFile = L"UseTextFile";

// Persistent flags.
bool g_EnableDatabaseEntries = false;
bool g_EnableNetworkEntries = false;

// Registry setting values (and serial values)
enum RegSettingsValues
{
	eSettingsName,
	eSettingsBoard,
	eSettingsSport,
	eSettingsSportCode,
	eSettingsPort,
	eSettingsBaud,
	eSettingsParity,
	eSettingsStopBits,
	eSettingsDataBits,
	eSettingsNumValues
};

// Registry active setting values (and serial values)
enum RegActiveValues
{
	eActiveName,
	eActiveBoard,
	eActiveSport,
	eActiveSportCode,
	eActivePort,
	eActiveBaud,
	eActiveParity,
	eActiveStopBits,
	eActiveDataBits,
	eActiveNumValues
};

// Settings list
struct Reg_Settings_List
{
	RegSettingsValues RegSettingsList;
	const wchar_t RegSettingsValName[64];
};

// Active list
struct Reg_Active_List
{
	RegActiveValues RegActiveList;
	const wchar_t RegActiveValName[64];
};

const Reg_Settings_List RegSettingsList[] = {
	{ eSettingsName,		L"Name"		},
	{ eSettingsBoard,		L"Board"	},
	{ eSettingsSport,		L"Sport"	},
	{ eSettingsSportCode,	L"SportCode"},
	{ eSettingsPort,		L"Port"		},
	{ eSettingsBaud,		L"Baud"		},
	{ eSettingsParity,		L"Parity"	},
	{ eSettingsStopBits,	L"StopBits"	},
	{ eSettingsDataBits,	L"DataBits"	}
};

const Reg_Active_List RegActiveList[] = {
	{ eActiveName,			L"Name"				},
	{ eActiveBoard,			L"Board"			},
	{ eActiveSport,			L"Sport"			},
	{ eActiveSportCode,		L"SportCode"		},
	{ eActivePort,			L"Port"				},
	{ eActiveBaud,			L"Baud"				},
	{ eActiveParity,		L"Parity"			},
	{ eActiveStopBits,		L"StopBits"			},
	{ eActiveDataBits,		L"DataBits"			}
};

// Registry read/write/delete classes.
class Registrar
{
	public:
		Registrar(void);
		~Registrar(void);

		const wchar_t * const RegRead(	const wchar_t regPath[MAX_KEY_LENGTH],
										const wchar_t subType[MAX_VALUE_NAME],
										const DWORD valType,
										const wchar_t valName[MAX_VALUE_NAME],
										const wchar_t * retVal);

		const wchar_t * const RegRead2(	const wchar_t regPath[MAX_KEY_LENGTH],
										const wchar_t subType[MAX_VALUE_NAME],
										const DWORD valType,
										const wchar_t valName[MAX_VALUE_NAME],
										const wchar_t * retVal);

		bool RegReadBool(	const wchar_t regPath[MAX_KEY_LENGTH],
							const wchar_t valName[MAX_VALUE_NAME]);

		DWORD RegReadNum(	const wchar_t regPath[MAX_KEY_LENGTH],
							const wchar_t valName[MAX_VALUE_NAME]);
							

		bool RegWrite(	const wchar_t regPath[MAX_KEY_LENGTH],
						const DWORD valType,
						const wchar_t valName[MAX_VALUE_NAME],
						const wchar_t valValue[MAX_VALUE_NAME]);
		
		bool RegWriteBool(	const wchar_t regPath[MAX_KEY_LENGTH],
							const wchar_t valName[MAX_VALUE_NAME],
							const int valValue);

		bool RegWriteNum(	const wchar_t* path,
							const wchar_t* name,
							const DWORD value);

		bool RegDelete(	const wchar_t regPath[MAX_KEY_LENGTH],
						const wchar_t valName[MAX_VALUE_NAME]);

		bool RegDeleteATree(const wchar_t regPath[MAX_KEY_LENGTH],
							const wchar_t valName[MAX_VALUE_NAME]);

		bool RegDeleteSubs (const wchar_t regPath[MAX_KEY_LENGTH]);

	protected:

};

}