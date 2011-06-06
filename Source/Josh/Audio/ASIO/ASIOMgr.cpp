// ASIOMgr.cpp: implementation of the ASIOMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Devices_Audio_ASIO.h"

using namespace FrameWork::Base;
using namespace Devices::Audio::ASIO;

ASIOMgr::ASIOMgr()
{
	ASIODriver::ClearCallbackTables();
	CoInitialize(NULL);
	m_AsioDriverArray = NULL;
	InitDriverList();
}

ASIOMgr::~ASIOMgr()
{
	ReleaseAsioDriverArray();
	CoUninitialize();
}

bool ASIOMgr::InitDriverList()
{
	LONG 			cr;
	HKEY			hkEnum;
	char			keyname[MAXDRVNAMELEN];
	
	m_nbDrivers = 0;
	int index = 0;
	cr = RegOpenKey(HKEY_LOCAL_MACHINE,"software\\asio",&hkEnum);
	while (cr == ERROR_SUCCESS) 
	{
		if ((cr = RegEnumKey(hkEnum,index,(LPTSTR)keyname,MAXDRVNAMELEN))== ERROR_SUCCESS) 
		{
			DebugOutput(L"%s\n",keyname);
		}		
		index++;
	}
	//
	// on alloue le tableau de drivers.
	// m_MaxnbDrivers n'est pas forcément le bon nombre de drivers
	// car la registry est mal foutue. on peut avoir des entrées 
	// qui ne sont pas des drivers.
	// 
	m_MaxnbDrivers = index;
	m_AsioDriverArray = new ASIODriver*[m_MaxnbDrivers];
	for (int i=0;i<m_MaxnbDrivers;i++)
	{
		m_AsioDriverArray[i] = NULL;
	}

	index = 0;
	cr = RegOpenKey(HKEY_LOCAL_MACHINE,"software\\asio",&hkEnum);
	int driverid = 0;
	while (cr == ERROR_SUCCESS) 
	{
		m_AsioDriverArray[driverid] = new ASIODriver();
		if (m_AsioDriverArray[driverid]->SetId(driverid))
		{
			if ((cr = RegEnumKey(hkEnum,index,(LPTSTR)m_AsioDriverArray[driverid]->getName(),MAXDRVNAMELEN))== ERROR_SUCCESS) 
			{
				if (FillDriverFromRegistryKey(hkEnum,driverid))
				{
					driverid++;
				}
				else
				{
					// hkEnum est une clef bidon.
					delete m_AsioDriverArray[driverid];
					m_AsioDriverArray[driverid]=NULL;
				}
			}		
			else
			{
				// hkEnum est une clef bidon.
				delete m_AsioDriverArray[driverid];
				m_AsioDriverArray[driverid]=NULL;
			}
		}
		else
		{
			// dépassement du nombre max de driver  utilisable par cette API
			// (MAX_DRIVERS_CALLBACK)
			delete m_AsioDriverArray[driverid];
			m_AsioDriverArray[driverid]=NULL;
		}
		index++;
	}
	m_nbDrivers = driverid;

	if (hkEnum) RegCloseKey(hkEnum);
	if (m_nbDrivers==0) 
		return false;
	else
		return true;
}

void ASIOMgr::ReleaseAsioDriverArray()
{
	if (m_AsioDriverArray!=NULL)
	{
		for (int d=0;d<m_MaxnbDrivers;d++)
		{
			if (m_AsioDriverArray[d]!=NULL)
			{
				delete m_AsioDriverArray[d];
				m_AsioDriverArray[d] = NULL;
			}
		}
		delete m_AsioDriverArray;
		m_AsioDriverArray = NULL;
	}
}
bool ASIOMgr::FillDriverFromRegistryKey(HKEY hkey,int index)
{
	LONG 			cr;
	HKEY			hksub;
	DWORD			datatype,datasize;
	bool			r = false;
	const char * keyname = m_AsioDriverArray[index]->getName();
	if ((cr = RegOpenKeyEx(hkey,(LPCTSTR)keyname,0,KEY_READ,&hksub)) == ERROR_SUCCESS) {

		datatype = REG_SZ; 
		datasize = CLSID_STRING_LEN;
		char clsid[CLSID_STRING_LEN];
		cr = RegQueryValueEx(hksub,"clsid",0,&datatype,(LPBYTE)clsid,&datasize);
		CLSID theCLSID;
		if (cr == ERROR_SUCCESS && GetCLSIDFromString(clsid,theCLSID)) 
		{
			m_AsioDriverArray[index]->setCLSID(theCLSID);
			m_AsioDriverArray[index]->setCLSID(clsid);
			if (GetDriverPath(index))
			{
				r = true;
			}
		}
		RegCloseKey(hksub);
	}
	return r;
}

bool ASIOMgr::GetCLSIDFromString(char *clsid,CLSID &result)
{

	char  wCLSID[CLSID_STRING_LEN*2];
	if (MultiByteToWideChar(CP_ACP,0,(LPCSTR)clsid,-1,(LPWSTR)wCLSID,CLSID_STRING_LEN*2)==0)
	{
		return false;
	}
	if ((CLSIDFromString((LPOLESTR)wCLSID,(LPCLSID)&result)) == S_OK) 
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ASIOMgr::GetDriverPath(int index)
{
	HKEY hkey;
	bool r = false;
	char keyname[CLSID_STRING_LEN + 32];
	sprintf(keyname,"clsid\\%s\\InprocServer32",m_AsioDriverArray[index]->getCLSID());
	if ((RegOpenKey(HKEY_CLASSES_ROOT,keyname,&hkey)) == ERROR_SUCCESS) 
	{
		LPBYTE path = new BYTE[MAXPATHLEN];
		DWORD datasize = MAXPATHLEN;
		DWORD datatype = REG_SZ;
		if (RegQueryValueEx(hkey,0,0,&datatype,path,&datasize)==ERROR_SUCCESS) 
		{
			m_AsioDriverArray[index]->setPath((char*)path);
			r = true;
		}
		delete path;
		path = NULL;
	}
	return r;
}

int ASIOMgr::GetDriverCount()
{
	return m_nbDrivers;
}

ASIODriver * ASIOMgr::GetDriver(int index)
{
	if (index >= m_nbDrivers || index<0)
		return NULL;
	return m_AsioDriverArray[index];
}

  /*******************************************************************************************************/
 /*												ASIO_OutputDevice										*/
/*******************************************************************************************************/
using namespace Devices::Audio::Base::Host;
ASIO_OutputDevice::ASIO_OutputDevice(size_t DriverIndex,const wchar_t *pDeviceName,HWND hwnd) :
	m_AudioHostDevice(NULL),
	DeviceWrapper(m_AudioHostDevice->GetAudioDevice()),
	AudioHostDeviceWrapper(m_AudioHostDevice)
{	
	if (m_AudioMgr.GetDriverCount()==0)
		assert(false),throw "No ASIO Drivers available" __FUNCSIG__;
	m_AudioHost=m_AudioMgr.GetDriver((int)DriverIndex);
	if (!m_AudioHost)
		assert(false),throw "Error reading ASIO driver" __FUNCSIG__;
	m_AudioHost->OpenDriver(hwnd);
	if (!m_AudioHost->Start())
		assert(false),throw "Unable to start ASIO driver" __FUNCSIG__;
	assert(m_AudioHost->getName());
	if (!pDeviceName)
	{
		const size_t WordSize=strlen(m_AudioHost->getName())+1;
		m_DriverName=new wchar_t[WordSize]; //TODO figure out how to 
		MultiByteToWideChar(CP_ACP,0,m_AudioHost->getName(),-1,m_DriverName,(int)WordSize);
		pDeviceName=m_DriverName;
	}
	else
		m_DriverName=NULL;
	m_AudioHostDevice= new AudioHostDevice(m_AudioHost->GetAudioHost_Interface(),pDeviceName);
	//Now that I have my audio host device I can link it
	SetAudioDevice(m_AudioHostDevice->GetAudioDevice());
	SetAudioHostDevice(m_AudioHostDevice);
}

ASIO_OutputDevice::~ASIO_OutputDevice()
{
	if (m_AudioHostDevice)
	{
		delete m_AudioHostDevice;
		m_AudioHostDevice=NULL;
	}
	if (m_DriverName)
		delete [] m_DriverName;
	//Note: I do not own m_AudioHost
}
