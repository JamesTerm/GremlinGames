//#define __UseSpeedEditBeta__

/*	A list of registry entries for locating products

	Beta installers use a different location so as not to interfere with the previous version.
	You can disregard BETA products in shipping builds.

	All assume HKLM.
*/

// Multi-codec Pack (NT22, NT25, SHQ) - InstallLocation is useless, they are codes
#define MCP_REG_KEY				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\Uninstall\\{F5E60A73-06AC-4611-8CB3-109CB012EBCE}_is1"
#define MCP_VALUE_NAME			L"InstallLocation"

// NewTekSD (VT card drivers)  (ie. RTME)
#define NTSD_REG_KEY			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{F5E60A73-06AC-4611-8CB3-109CB012EBCE}_is1"
#define NTSD_VALUE_NAME			L"InstallLocation"

// SX-SDI
#define SXSDI_REG_KEY			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{B0DE1B3B-A422-42E8-BFFA-EB1F2550D7A8}_is1"
#define SXSDI_VALUE_NAME		L"InstallLocation"

// SpeedEDIT 1.x BETA
#define SE1BETA_REG_KEY			L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{590D5FA0-D407-45D3-9AEB-519EF42B8925}_is1"
#define SE1BETA_VALUE_NAME		L"InstallLocation"
#define SE1BETA_LOC				L"\\"

// SpeedEDIT 1.x
#define SE1_REG_KEY				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{E9B27E46-D402-43A1-8840-542210C60C8F}_is1"
#define SE1_VALUE_NAME			L"InstallLocation"
#define SE1_LOC					L"\\"

// VT4
#define VT4_REG_KEY				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{5AC8F6D6-28CA-445b-B0FB-5E38E5077286}_is1"
#define VT4_VALUE_NAME			L"InstallLocation"
#define VT4_LOC					L"\\Plugins\\VT\\"

// VT5
#define VT5_REG_KEY				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{5AC8F6D6-28CA-445b-B0FB-5E38E5077286}_is1"
#define VT5_VALUE_NAME			L"InstallLocation"
#define VT5_LOC					L"\\"

// Useful when in VT debug mode
#define VTD_REG_KEY				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\VTdebug"
#define VTD_VALUE_NAME			L"Path"
#define VTD_LOC					L"\\"

// TriCaster - all as of 06-12-2007
#define TC_REG_KEY				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\TriCaster.exe"
#define TC_VALUE_NAME			L"Path"
#define TC_LOC					L"\\Plugins\\"

// SE1BETA_REG_KEY and SE1_REG_KEY are mutually exclusive, so client code can use these names for all speed edit cases
#ifdef __UseSpeedEditBeta__
#define SPEEDEDIT_REG_KEY		SE1BETA_REG_KEY
#define SPEEDEDIT_VALUE_NAME	SE1BETA_VALUE_NAME
#define SPEEDEDIT_LOC			SE1BETA_LOC
#else
#define SPEEDEDIT_REG_KEY		SE1_REG_KEY
#define SPEEDEDIT_VALUE_NAME	SE1_VALUE_NAME
#define SPEEDEDIT_LOC			SE1_LOC
#endif	__UseSpeedEditBeta__
