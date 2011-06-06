#pragma once


class ASIOHost_DLL ASIOMgr  
{
public:
	ASIOMgr( void );
	virtual ~ASIOMgr( void );

	ASIODriver * GetDriver(int index);
	int GetDriverCount( void );

private:
	bool GetDriverPath(int index);
	bool GetCLSIDFromString(char *clsid,CLSID &result);
	bool FillDriverFromRegistryKey(HKEY hkey,int index);
	void ReleaseAsioDriverArray( void );
	bool InitDriverList( void );

	int m_MaxnbDrivers;
	int m_nbDrivers;
	ASIODriver ** m_AsioDriverArray;
};

class ASIOHost_DLL ASIO_OutputDevice :	public Devices::Audio::Base::DeviceWrapper,
										public Devices::Audio::Base::AudioHostDeviceWrapper
{
	public:
		
		
		
		//This will throw exceptions if no ASIO drivers are available
		ASIO_OutputDevice(
			size_t DriverIndex=0,				//The default driver index will use the first ASIO device selected
			const wchar_t *pDeviceName = NULL,	//NULL will give you the ASIO driver name as the default
			HWND hwnd=NULL); 					//unlike in DirectSound, the hwnd is optional

		virtual ~ASIO_OutputDevice( void );


	private:
		ASIOMgr m_AudioMgr;
		ASIODriver *m_AudioHost;
		Devices::Audio::Base::Host::AudioHostDevice *m_AudioHostDevice;
		wchar_t *m_DriverName; //used if no device name is present
};
