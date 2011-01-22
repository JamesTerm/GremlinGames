#pragma once

//********************************************************************************************************************
class SerialPort
{	protected:		//! The actual handle on the serial port
					HANDLE			m_ComPort;

					//! The current port number
					unsigned		m_PortNumber;

					//! The port settings
					DCB				m_PortDCB;

					//! Update the DCB Settings
					bool UpdateDCB(void);

					//! The timeouts
					COMMTIMEOUTS	m_TimeOuts;

					//! Set the timeouts
					void SetTimeOuts(	DWORD ReadIntervalTimeout ,
										DWORD ReadTotalTimeoutMultiplier ,
										DWORD ReadTotalTimeoutConstant );

	public:			/*! Set the actual port to open
						Once this is open it will assign the default port
						settings to the current structure. To set them to some
						other values, you need to set them up manually. 
						Setting the port to NUmber=-1 will close the handles */
					bool SetPort(unsigned Number=-1);

					/*! Get the current port number, -1 means error */
					unsigned GetPort(void) { return m_PortNumber; }

					//! Error ?
					bool Error(void) { return (GetPort()==-1); }

					/*! This will send a block of memory to the com port
						the return value is the number of bytes actually sent to the port */
					unsigned WriteToPort(BYTE *Buffer,unsigned NoBytes);

					/*! This sends a string */
					bool WriteStringToPort(char *String);

					/*! This will read a given number of bytes from the serial port.
						the return value is the actual number of bytes that have been read.
						If no information is available to read, then the return value is 0.*/
					unsigned ReadFromPort(	BYTE *Buffer,unsigned NoBytesMax=1);

					//! Simulate a timeout the hard way :0
					unsigned ReadFromPort(	BYTE *Buffer,unsigned NoBytesMax,
											unsigned TotalTimeOut/*=1000*/,
											unsigned IntervalBetweenChars=10);

					//! Update the current settings and make them active on the port.
					bool SetPortSettings
						(	unsigned BaudRate=9600,	//! 110,300,600,1200,2400,4800,9600,14400,19200,38400,56000,57600,115200,128000,256000
							unsigned DataBits=8,	//! 5,6,7,8
							unsigned Parity=0,		//! NOPARITY=0.ODDPARITY=1,EVENPARITY=2,MARKPARITY=3,SPACEPARITY=4
							unsigned StopBits=0,	//! 1STOPBIT=0,1.5STOPBITS=1,2STOPBITS=2
							unsigned FlowControl=0	//! None=0, XOnXOff=1, HW=2
						);

					//! Constructor
					SerialPort(unsigned Number=-1);

					//! Destructor
					~SerialPort(void);
};

//********************************************************************************************************************