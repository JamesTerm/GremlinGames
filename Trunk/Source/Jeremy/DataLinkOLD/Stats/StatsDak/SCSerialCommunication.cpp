#include "StdAfx.h"

bool SerialPort::SetPortSettings(	unsigned BaudRate,
									unsigned DataBits,
									unsigned Parity,		//! NOPARITY=0.ODDPARITY=1,EVENPARITY=2,MARKPARITY=3,SPACEPARITY=4
									unsigned StopBits,		//! 1STOPBIT=0,1.5STOPBITS=1,2STOPBITS=2
									unsigned FlowControl	//! None=0, XOnXOff=1, HW=2
								)
{	// Be threadsafe
	// FunctionBlock a1(m_PortLock);
	// Errir ?
	if (Error()) return false;

	memset(&m_PortDCB,0,sizeof(m_PortDCB));
	m_PortDCB.DCBlength=sizeof(m_PortDCB);
	// Get the current port settings
	if (!GetCommState(m_ComPort,&m_PortDCB))
		// Error
		return false;
	// Now set up the values that have been sent to us
	m_PortDCB.BaudRate=BaudRate;
	m_PortDCB.fBinary=TRUE;
	m_PortDCB.fParity=FALSE;
	m_PortDCB.fDtrControl=DTR_CONTROL_DISABLE;
	m_PortDCB.fDsrSensitivity=FALSE;
	m_PortDCB.fTXContinueOnXoff=TRUE;
	// No flow control
	if (FlowControl==0)
	{	m_PortDCB.fOutxCtsFlow=FALSE;
		m_PortDCB.fOutxDsrFlow=FALSE;
		m_PortDCB.fOutX=FALSE;
		m_PortDCB.fInX=FALSE;
		m_PortDCB.fRtsControl=0;
	}
	// XOnXOff
	else if (FlowControl==1)
	{	m_PortDCB.fOutxCtsFlow=FALSE;
		m_PortDCB.fOutxDsrFlow=FALSE;
		m_PortDCB.fOutX=TRUE;
		m_PortDCB.fInX=TRUE;
		m_PortDCB.fRtsControl=0;
	}
	// Hardware
	else if (FlowControl==2)
	{	m_PortDCB.fOutxCtsFlow=TRUE;
		m_PortDCB.fOutxDsrFlow=FALSE;
		m_PortDCB.fOutX=FALSE;
		m_PortDCB.fInX=FALSE;
		m_PortDCB.fRtsControl=2;
	}

	m_PortDCB.fAbortOnError=FALSE;
	m_PortDCB.fErrorChar=FALSE;
	m_PortDCB.fNull=FALSE; 
	//m_PortDCB.fDummy2=0;
	//m_PortDCB.wReserved;
	//m_PortDCB.XonLim=2048;
	//m_PortDCB.XoffLim=512;
	m_PortDCB.ByteSize=DataBits;
	m_PortDCB.Parity=Parity;
	m_PortDCB.StopBits=StopBits;
	//m_PortDCB.XonChar=17;
	//m_PortDCB.XoffChar=19;
	//m_PortDCB.ErrorChar=0;
	//m_PortDCB.EofChar=0;
	//m_PortDCB.EvtChar=0;
	//m_PortDCB.wReserved1=0;
	// Set the current port settings
	if (!SetCommState(m_ComPort,&m_PortDCB))
		// Error
		return false;

	// Force a reset of the settings next time through
	m_TimeOuts.ReadIntervalTimeout = -1;

	// Success
	return true;
}

bool SerialPort::UpdateDCB(void)
{	// FunctionBlock a1(m_PortLock);
	// Set the state
	if (!SetCommState(m_ComPort,&m_PortDCB))
			return false;	
	// Success
	return true;
}

void SerialPort::SetTimeOuts(	DWORD ReadIntervalTimeout ,
								DWORD ReadTotalTimeoutMultiplier ,
								DWORD ReadTotalTimeoutConstant )
{	if (Error()) return;
	// Check for a change
	if ( (m_TimeOuts.ReadIntervalTimeout!=ReadIntervalTimeout) ||
		 (m_TimeOuts.ReadTotalTimeoutMultiplier!=ReadTotalTimeoutMultiplier) ||
		 (m_TimeOuts.ReadTotalTimeoutConstant!=ReadTotalTimeoutConstant) )
	{	m_TimeOuts.ReadIntervalTimeout=ReadIntervalTimeout;
		m_TimeOuts.ReadTotalTimeoutMultiplier=ReadTotalTimeoutMultiplier;
		m_TimeOuts.ReadTotalTimeoutConstant=ReadTotalTimeoutConstant;
		SetCommTimeouts(m_ComPort,&m_TimeOuts);
	}
}

unsigned SerialPort::ReadFromPort(byte *Buffer,unsigned NoBytesMax)
{	// FunctionBlock a(m_PortLock);
	// Error, port is not configured
	if (Error()) return 0;

	// Restore any timeout values
	//SetTimeOuts( MAXDWORD , 0 , 0 );

	// Try to read from the port
	DWORD Ret=0;
	ReadFile(m_ComPort,Buffer,NoBytesMax,&Ret,NULL);

	// return the number of bytes actually written
	return Ret;
}

unsigned SerialPort::ReadFromPort(			byte *Buffer,
											unsigned NoBytesMax,
											unsigned TotalTimeOut,
											unsigned IntervalBetweenChars)
{	// FunctionBlock a(m_PortLock);
	// Error, port is not configured
	if (Error()) return 0;

	// Setup timeouts
	//SetTimeOuts( IntervalBetweenChars , 1 , TotalTimeOut );
	
	// Try to read from the port
	DWORD Ret=0;
	ReadFile(m_ComPort,Buffer,NoBytesMax,&Ret,NULL);

	// return the number of bytes actually written
	return Ret;
}

unsigned SerialPort::WriteToPort(byte *Buffer,unsigned NoBytes)
{	// FunctionBlock a(m_PortLock);
	// Error, port is not configured
	if (Error()) return 0;

	// Try to write to the port
	DWORD Ret=NoBytes;

	//DebugOutput("%d %s\n",m_PortNumber,Buffer);
	WriteFile(m_ComPort,Buffer,NoBytes,&Ret,NULL);

	// return the number of bytes actually written
	return Ret;
}

bool SerialPort::WriteStringToPort(char *String)
{	// FunctionBlock a(m_PortLock);
	// Error, port is not configured
	if (Error()) return false;
	// Now write it to the port
	unsigned Length=strlen(String);
	return (WriteToPort((byte*)String,Length)==Length);
}

bool SerialPort::SetPort(unsigned Number)
{	// FunctionBlock a1(m_PortLock);
	// Close any existing port
	if ((m_ComPort)&&(m_ComPort!=INVALID_HANDLE_VALUE))
	{	CloseHandle(m_ComPort);
		m_ComPort=NULL;
	}
	// The port number
	m_PortNumber=Number;
	// Nuker the settings
	memset(&m_PortDCB,0,sizeof(m_PortDCB));
	memset(&m_TimeOuts,0,sizeof(m_TimeOuts));
	// No port to open
	if (Number==-1) return true;
	// Error, only consider what might ever be valid ports
	if ((m_PortNumber<1)||(m_PortNumber>256))
	{	// Close it
		SetPort(-1);
		// Error
		return false;
	}
	// This is teh port name for what is being used
	char l_PortNameToUse[32];
	sprintf(l_PortNameToUse,"\\\\.\\COM%d",Number);
	// Open the serial port.
	m_ComPort=CreateFile(	l_PortNameToUse,// Pointer to the name of the port
							GENERIC_READ|GENERIC_WRITE,	// Access (read-write) mode
							0,				// Share mode
							NULL,			// Pointer to the security attribute
							OPEN_EXISTING,	// How to open the serial port
							0,				// Port attributes
							NULL);			// Handle to port with attribute to copy
	// Error ?
	if ((!m_ComPort)||(m_ComPort==INVALID_HANDLE_VALUE))
	{	// Close it
		SetPort(-1);
		// Error
		return false;
	}

	// Initialise the port settings structure
	m_PortDCB.DCBlength=sizeof(m_PortDCB);

	// Get the current port settings
	if (!SetPortSettings())
	{	// Close it
		SetPort(-1);
		// Error
		return false;
	}

	// Get the current timeout settings
	if (!GetCommTimeouts(m_ComPort,&m_TimeOuts))
	{	// Close it
		SetPort(-1);
		// Error
		return false;
	}

	// We set some pretty reasonable settings
	memset( &m_TimeOuts , 0 , sizeof(m_TimeOuts) );
	m_TimeOuts.ReadIntervalTimeout = MAXDWORD;
	
	// Now setup the port timeouts
	if (!SetCommTimeouts(m_ComPort,&m_TimeOuts))
	{	// Close it
		SetPort(-1);

		// Error
		return false;
	}

	// Success
	return true;
}

SerialPort::SerialPort(unsigned Number)
{	// The port is not yet open
	m_ComPort=INVALID_HANDLE_VALUE;
	m_PortNumber=-1;
	// Seteup the port as specified by the user
	SetPort(Number);
}

SerialPort::~SerialPort(void)
{	// Close the port
	SetPort(-1);
}

int __declspec(dllexport) DoNothing(void) { return 0; }