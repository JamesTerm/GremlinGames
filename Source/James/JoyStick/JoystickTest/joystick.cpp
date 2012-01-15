#include "stdafx.h"

class IJoystick
{
	public:
		struct JoyState 
		{
			size_t JoystickNumber; //Which joystick does this state apply to
			//All floats range from -1.0 to 1.0
			float    lX;                    ///< x-axis position
			float    lY;                    ///< y-axis position
			float    lZ;                    ///< z-axis position
			float    lRx;                   ///< x-axis rotation
			float    lRy;                   ///< y-axis rotation
			float    lRz;                   ///< z-axis rotation
			float    rgSlider[2];          ///< extra axes positions
			float    rgPOV[4];             ///< POV directions in degrees- should be positive integer, if POV is not used it will be -1.0
			///128 buttons, each bank is 32 bits where each bit is the flagged value
			///Typically you will only need the first bank for 32 buttons
			DWORD    ButtonBank[4];          
		};

		///This provides some generic capabilities of the Joystick.  You can test flags to determine what is available
		struct JoystickInfo
		{
			//These could be handy when displaying which joystick to set for a control
			std::string ProductName;
			std::string InstanceName;
			enum JoyCapFlag_enum
			{
				fX_Axis	=0x00000001,
				fY_Axis	=0x00000002,
				fZ_Axis	=0x00000004,
				fX_Rot	=0x00000008,
				fY_Rot	=0x00000010,
				fZ_Rot	=0x00000020,
				fSlider0=0x00000040,
				fSlider1=0x00000080,
				fPOV_0	=0x00000100,
				fPOV_1	=0x00000200,
				fPOV_2	=0x00000400,
				fPOV_3	=0x00000800
			};
			DWORD JoyCapFlags;
			int nSliderCount;  // Number of returned slider controls
			int nPOVCount;     // Number of returned POV controls
			int nButtonCount;  // Number of returned buttons
			//Check this to ensure the JoyStick initialized properly (we may just do an assert on it)
			BOOL bPresent;
		};

		/// Use this in conjunction with GetJoyInfo to gather all preliminary Joystick information
		virtual size_t GetNoJoysticksFound()=0;
		/// Take care to submit index entry < the number of joysticks available.  This will let you know what axis will be used
		/// As well as the number of sliders, Pov's and buttons
		virtual const JoystickInfo &GetJoyInfo(size_t nr) const=0;
		///Here is the main polling function which typically gets called during a time change.  Simply call this with the desired enumerated joystick
		///index and retrieve its current state
		/// \return true if successful, so do not read values if it returns false (it may typically return false on initial setup)
		virtual bool read_joystick (size_t nr, JoyState &Info)=0;
};

#pragma comment(lib, "dinput8")
#pragma comment(lib, "dxGUID")

class DirectInput_Joystick : public IJoystick
{
	public:
		/// \param hwnd If you can easily provide the hwnd then it can handle the case like not listen if the window is minimized. (it is optional)
		DirectInput_Joystick(HWND hwnd=NULL);
		~DirectInput_Joystick();

	public:	//These are only public for the OS callbacks
		BOOL DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi);
		BOOL EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi);
	protected:	//from IJoystick
		virtual bool read_joystick (size_t nr, JoyState &Info);
		virtual size_t GetNoJoysticksFound() {return m_JoyInfo.size();}
		virtual const JoystickInfo &GetJoyInfo(size_t nr) const {return m_JoyInfo[nr].Common;}
	private:
		LPDIRECTINPUT8 m_lpDInput;
		struct DIJoyInfo
		{
			JoystickInfo Common;
			LPDIRECTINPUTDEVICE8 lpDInputDevice;
		};
		std::vector<DIJoyInfo> m_JoyInfo;
		HWND const m_HWnd;
};

#define write_log printf


bool DirectInput_Joystick::read_joystick(size_t nr,JoyState &Info) 
{ 
	LPDIRECTINPUTDEVICE8 pDev=m_JoyInfo[nr].lpDInputDevice;
	JoystickInfo &caps=m_JoyInfo[nr].Common;
    HRESULT hr; 
 
	DIJOYSTATE2 js;
	memset(&js,0,sizeof(DIJOYSTATE2));

    // poll the joystick to read the current state
    hr = pDev->Poll();
	//Note: S_FALSE is DI_NOEFFECT which is successful
	if (FAILED (hr))
	{
		// did the read fail because we lost input for some reason? 
		// if so, then attempt to reacquire. If the second acquire 
		// fails, then the error from GetDeviceData will be 
		// DIERR_NOTACQUIRED, so we won't get stuck an infinite loop. 
		hr=pDev->Acquire();
		while (hr == DIERR_INPUTLOST) 
			hr=pDev->Acquire();

		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of 
		// switching, so just try again later 
		return false;
	}

    // get data from the joystick 
    hr=pDev->GetDeviceState(sizeof(DIJOYSTATE2),&js);
	//Now to translate
	Info.JoystickNumber=nr;
	Info.lX=(float)js.lX/1000.0f;
	Info.lY=(float)js.lY/1000.0f;
	Info.lZ=(float)js.lZ/1000.0f;
	Info.lRx=(float)js.lRx/1000.0f;
	Info.lRy=(float)js.lRy/1000.0f;
	Info.lRz=(float)js.lRz/1000.0f;
	for (int i=0;i<caps.nSliderCount;i++)
		Info.rgSlider[i]=(float)js.rglSlider[i]/1000.0f;
	for (int i=0;i<caps.nPOVCount;i++)
	{
		int result=(int)js.rgdwPOV[i];
		Info.rgPOV[i]=result==-1?-1.0f:(float)result/100.0f;
	}

	//Now for the buttons (not really optimized but should be fine)
	Info.ButtonBank[0]=Info.ButtonBank[1]=Info.ButtonBank[2]=Info.ButtonBank[3]=0;
	for (int i=0;i<32;i++)
	{
		size_t NewBit=js.rgbButtons[i]>>7; //bring bit 7 to bit 0
		NewBit<<=i;
		Info.ButtonBank[0]|=NewBit;

		NewBit=js.rgbButtons[i+32]>>7; //bring bit 7 to bit 0
		NewBit<<=i;
		Info.ButtonBank[1]|=NewBit;

		NewBit=js.rgbButtons[i+64]>>7; //bring bit 7 to bit 0
		NewBit<<=i;
		Info.ButtonBank[2]|=NewBit;

		NewBit=js.rgbButtons[i+128]>>7; //bring bit 7 to bit 0
		NewBit<<=i;
		Info.ButtonBank[3]|=NewBit;
	}
	return hr==S_OK;
}

//-----------------------------------------------------------------------------
// Name: EnumObjectsCallback()
// Desc: Callback function for enumerating objects (axes, buttons, POVs) on a 
//       joystick. This function enables user interface elements for objects
//       that are found to exist, and scales axes min/max values.
//-----------------------------------------------------------------------------
BOOL DirectInput_Joystick::EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi)
{
	//TODO I'm assuming that the instance of this object corresponds to the last added joystick (it should since it will be set in the enum)
	assert(m_JoyInfo.size()>0);
	JoystickInfo &info=m_JoyInfo[m_JoyInfo.size()-1].Common;

    // For axes that are returned, set the DIPROP_RANGE property for the
    // enumerated axis in order to scale min/max values.
    if( pdidoi->dwType & DIDFT_AXIS )
    {
        DIPROPRANGE diprg; 
        diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
        diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
        diprg.diph.dwHow        = DIPH_BYID; 
        diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
        diprg.lMin              = -1000; 
        diprg.lMax              = +1000; 
    
        // Set the range for the axis
        if( FAILED( m_JoyInfo[m_JoyInfo.size()-1].lpDInputDevice->SetProperty( DIPROP_RANGE, &diprg.diph ) ) ) 
            return DIENUM_STOP;
         
    }

	if (pdidoi->dwType & DIDFT_BUTTON)
		info.nButtonCount++;

    // Set the UI to reflect what objects the joystick supports
	if (pdidoi->guidType == GUID_XAxis)
		info.JoyCapFlags|=JoystickInfo::fX_Axis;
    else if (pdidoi->guidType == GUID_YAxis)
		info.JoyCapFlags|=JoystickInfo::fY_Axis;
	else if (pdidoi->guidType == GUID_ZAxis)
		info.JoyCapFlags|=JoystickInfo::fZ_Axis;
	else if (pdidoi->guidType == GUID_RxAxis)
		info.JoyCapFlags|=JoystickInfo::fX_Rot;
	else if (pdidoi->guidType == GUID_RyAxis)
		info.JoyCapFlags|=JoystickInfo::fY_Rot;
	else if (pdidoi->guidType == GUID_RzAxis)
		info.JoyCapFlags|=JoystickInfo::fZ_Rot;
	else if (pdidoi->guidType == GUID_Slider)
    {
        switch( info.nSliderCount++ )
        {
            case 0 :
				info.JoyCapFlags|=JoystickInfo::fSlider0;
                break;

            case 1 :
				info.JoyCapFlags|=JoystickInfo::fSlider1;
                break;
			default :
				assert(false);  //I'd be surprised... we can still work fine
        }
    }
    else if (pdidoi->guidType == GUID_POV)
    {
        switch( info.nPOVCount++ )
        {
            case 0 :
				info.JoyCapFlags|=JoystickInfo::fPOV_0;
                break;

            case 1 :
				info.JoyCapFlags|=JoystickInfo::fPOV_1;
                break;

            case 2 :
				info.JoyCapFlags|=JoystickInfo::fPOV_2;
                break;

            case 3 :
				info.JoyCapFlags|=JoystickInfo::fPOV_3;
                break;
        }
    }

    return DIENUM_CONTINUE;
}


BOOL CALLBACK EnumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,VOID* pContext )
{
	DirectInput_Joystick *Instance=(DirectInput_Joystick *)pContext;
	assert(pContext);
	return Instance->EnumObjectsCallback(pdidoi);
}

BOOL DirectInput_Joystick::DIEnumDevicesCallback( LPCDIDEVICEINSTANCE lpddi) 
{

	write_log("DirectInput: Found %s %s\n",lpddi->tszProductName,lpddi->tszInstanceName);
	DIJoyInfo NewEntry;
	NewEntry.Common.ProductName=lpddi->tszProductName;
	NewEntry.Common.InstanceName=lpddi->tszInstanceName;
	try
	{
		if (FAILED(m_lpDInput->CreateDevice(lpddi->guidInstance,&NewEntry.lpDInputDevice,NULL)))
			return DIENUM_STOP;

		LPDIRECTINPUTDEVICE8 device=NewEntry.lpDInputDevice;

		NewEntry.Common.bPresent=FALSE;
		if (FAILED(device->SetDataFormat(&c_dfDIJoystick2))) throw 0;

		if (m_HWnd)
		{
			//let DInput know how this device should interact with the system and with other DInput applications
			if (FAILED(NewEntry.lpDInputDevice->SetCooperativeLevel(m_HWnd,DISCL_EXCLUSIVE | DISCL_FOREGROUND))) throw 1;
		}

		NewEntry.Common.bPresent = TRUE;
		NewEntry.Common.JoyCapFlags=0;
		NewEntry.Common.nSliderCount = 0;  // Number of returned slider controls
		NewEntry.Common.nPOVCount = 0;     // Number of returned POV controls
		NewEntry.Common.nButtonCount = 0;  // Number of returned Buttons
		m_JoyInfo.push_back(NewEntry);
		// Enumerate the joystick objects. The callback function enabled user
		// interface elements for objects that are found, and sets the min/max
		// values property for discovered axes.
		if(FAILED(device->EnumObjects( ::EnumObjectsCallback,(VOID*)this, DIDFT_ALL )))	throw 2;
		return DIENUM_CONTINUE;
	}
	catch (int ErrorCode)
	{
		write_log("DirectInput_Joystick::DIEnumDevicesCallback failed %d\n",ErrorCode);
		write_log("ErrorCode %x\n",GetLastError());

		IDirectInputDevice_Release( NewEntry.lpDInputDevice );
		NewEntry.lpDInputDevice = NULL;
		//Be sure to flag as failed
		if (NewEntry.Common.bPresent==TRUE)
			m_JoyInfo[m_JoyInfo.size()-1].Common.bPresent=FALSE;
		return DIENUM_CONTINUE;  //this one failed but others may succeed
	}
}


BOOL CALLBACK DIEnumDevicesCallback( LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) 
{
	DirectInput_Joystick *Instance=(DirectInput_Joystick *)pvRef;
	assert(pvRef);
	return Instance->DIEnumDevicesCallback(lpddi);
}


// Modified to handle DirectInput joysticks (such as USB devices)
DirectInput_Joystick::DirectInput_Joystick(HWND hwnd) :
	m_HWnd(hwnd)
{
	{
		if(SUCCEEDED(DirectInput8Create(GetModuleHandle(NULL),DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&m_lpDInput,NULL))) 
		{
			HRESULT hr;
			hr=IDirectInput_EnumDevices( m_lpDInput, DI8DEVCLASS_GAMECTRL,::DIEnumDevicesCallback,(void *)this, DIEDFL_ATTACHEDONLY );
			assert(hr==S_OK);
		}
	}

	//nr_joysticks = found;
	write_log( "%d joystick(s) found.\n", m_JoyInfo.size());
}


DirectInput_Joystick::~DirectInput_Joystick (void) 
{
    size_t i;

    if(m_lpDInput) 
	{
		for(i = 0;i < m_JoyInfo.size();i++) 
		{
			if(m_JoyInfo[i].lpDInputDevice) 
			{
				m_JoyInfo[i].lpDInputDevice->Unacquire();
				m_JoyInfo[i].lpDInputDevice->Release();
				m_JoyInfo[i].lpDInputDevice = NULL;
			}
		}
		m_lpDInput->Release();
		m_lpDInput=NULL;
	}
}


//--------------------------------------------------------------------------------Test--------------------------------------------------------
// Converts a GUID to a string
inline void GUIDtoa(GUID id,char *string) {
	sprintf(string,"%x-%x-%x-%x%x-%x%x%x%x%x%x",id.Data1,id.Data2,id.Data3,
		id.Data4[0],id.Data4[1],id.Data4[2],id.Data4[3],id.Data4[4],id.Data4[5],id.Data4[6],id.Data4[7]);
}

HWND FindConsoleHandle()
{
	GUID ID;
	TCHAR title[512];
	TCHAR tempGUIDtitle[512];
	LPCTSTR temptitle;
	if (FAILED(CoCreateGuid(&ID)))
		return NULL;
	GUIDtoa(ID,tempGUIDtitle);
	temptitle=_T(tempGUIDtitle);
	if (GetConsoleTitle(title,sizeof(title)/sizeof(TCHAR))==0)
		return NULL; //doh
	SetConsoleTitle(temptitle);
	HWND wnd=FindWindow(NULL,temptitle);
	SetConsoleTitle(title);
	return wnd;
}


IJoystick &GetJoystick()
{
	static DirectInput_Joystick JoyStick(FindConsoleHandle());
	return JoyStick;
}

void main(int argc,char *argv[])
{
	size_t JoyNum=0;
	if (argc==2)
		JoyNum=atoi(argv[1]);
	IJoystick &JoyStick=GetJoystick();
	size_t NoJoySticks=JoyStick.GetNoJoysticksFound();
	if (NoJoySticks)
	{
		printf("Buttons: 1=next joystick, 2=exit, 3=enum axis\n");
		for(size_t i=0;i<NoJoySticks;i++)
		{
			IJoystick::JoystickInfo info=JoyStick.GetJoyInfo(i);
			assert(info.bPresent);
			printf("----------------------------------------\n");
			printf("Joystick %s Caps:\n",info.ProductName.c_str());
			printf("Sliders=%d Povs=%d Buttons=%d\n",info.nSliderCount,info.nPOVCount,info.nButtonCount);
			if (info.JoyCapFlags&IJoystick::JoystickInfo::fX_Axis)
				printf("X Axis, ");
			if (info.JoyCapFlags&IJoystick::JoystickInfo::fY_Axis)
				printf("Y Axis, ");
			if (info.JoyCapFlags&IJoystick::JoystickInfo::fZ_Axis)
				printf("Z Axis, ");
			if (info.JoyCapFlags&IJoystick::JoystickInfo::fX_Rot)
				printf("X Rot, ");
			if (info.JoyCapFlags&IJoystick::JoystickInfo::fY_Rot)
				printf("Y Rot, ");
			if (info.JoyCapFlags&IJoystick::JoystickInfo::fZ_Rot)
				printf("Z Rot, ");
			printf("\n----------------------------------------\n");
		}
		IJoystick::JoyState joyinfo;
		memset(&joyinfo,0,sizeof(IJoystick::JoyState));
		bool done=false;
		size_t Display=0;
		while(!done)
		{
			if (JoyStick.read_joystick (JoyNum,joyinfo))
			{
				switch(Display)
				{
				case 0:
					printf("\r xaxis=%f yaxis=%f zaxis=%f button=0x%x             ",joyinfo.lX,joyinfo.lY,joyinfo.lZ,joyinfo.ButtonBank[0]);
					break;
				case 1:
					printf("\r xRot=%f yRot=%f zRot=%f POV=%f               ",joyinfo.lRx,joyinfo.lRy,joyinfo.lRz,joyinfo.rgPOV[0]);
					break;
				case 2:
					printf("\r slider0=%f slider1=%f                              ",joyinfo.rgSlider[0],joyinfo.rgSlider[1]);
					break;
				case 3:
					break;
				}

				if (joyinfo.ButtonBank[0]==1)
				{
					JoyNum++;
					if (JoyNum>=NoJoySticks)
						JoyNum=0;
				}
				if (joyinfo.ButtonBank[0]==2)
					done=true;
				if (joyinfo.ButtonBank[0]==4)
				{
					Display++;
					if (Display>2)
						Display=0;
					Sleep(200); //hack a way to toggle easy
				}
			}
			Sleep(22); //about 30 times a sec
		}
	}
	else
	{
		printf("None found\n");
		Sleep(2000);
	}
}
