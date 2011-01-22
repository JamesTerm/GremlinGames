#include "StdAfx.h"

inline int GetFrameNumber(double seconds,double Num,double Den) { if (seconds<0) return (-0.5+(seconds/(Den/Num))); else return (0.5+(seconds/(Den/Num))); }
inline double GetSeconds(int framenumber,double Num,double Den) { return (framenumber*Den/Num); }

  /******************************************************************************/
 /*									Timecode Converters						   */
/******************************************************************************/


void TimecodeConverters::ConvertSecondsToString_NDF(char *Timecode,double Seconds,double FrameRate) {
	if (TimecodeCompare(FrameRate,0.0)) {
		strcpy(Timecode,"ErrNFrmRt");
		return;
		}

	unsigned hrs=0,mins=0,sec;

	if (Seconds<0) {
		Timecode[0]='-';
		Timecode++;
		Seconds=-Seconds;
		}

	//round the seconds to the nearest frame
	unsigned totalframes=GetFrameNumber(Seconds,FrameRate,1);
	unsigned FramesPerSecond=unsigned(FrameRate+0.5);
	sec=(unsigned int)Seconds;
	unsigned frames=totalframes-GetFrameNumber(sec,FrameRate,1);
	if (frames>=FramesPerSecond) {
		sec++;
		frames-=FramesPerSecond;
		}

	if (sec>=60) {
		mins=sec/60;
		sec-=mins*60;
		if (mins>=60) {
			hrs=mins/60;
			mins-=hrs*60;
			}
		}


	//Note: Non-Drop FrameRate format (all colons) nn:nn:nn:nn
	sprintf(Timecode,"%.2d:%.2d:%.2d:%.2d",hrs,mins,sec,frames);
	}


void TimecodeConverters::ConvertStringToSeconds_NDF(const char *Timecode,double &Seconds,double FrameRate) {
	if (TimecodeCompare(FrameRate,0.0)) {
		Seconds=-1.0;
		return;
		}
	unsigned TimeSlot[4][2];
	unsigned field=0;
	int i,j=1;

	memset(TimeSlot,0,8*sizeof(int));
	//check for negative
	bool negative=false;
	if (Timecode[0]=='-') {
		negative=true;
		Timecode++;
		}

	for (i=strlen(Timecode)-1;i>=0;i--) {
		if (field>3) break;
		if ((Timecode[i]>'9')||(Timecode[i]<'0')) { //Delimiter detected
			if (j==0) j--;
			}
		else {
			TimeSlot[field][j]=Timecode[i]-'0';
			j--;
			}
		if (j<0) {
			field++;
			j=1;
			}
		}

	Seconds=(TimeSlot[3][0]*10+TimeSlot[3][1])*3600;
	Seconds+=(TimeSlot[2][0]*10+TimeSlot[2][1])*60;
	Seconds+=TimeSlot[1][0]*10+TimeSlot[1][1];
	unsigned frames=(TimeSlot[0][0]*10+TimeSlot[0][1]);

	//for out point mode advance this value by 1
	//if (outpointmode) frames++;

	Seconds+=(frames*(1/FrameRate));
	if (negative) Seconds=-Seconds;
	}

bool TimecodeConverters::TimecodeCompare(double a,double b) {
	return (fabs(a-b)<0.001);
	}

//******************************************************************************************************************************************
static long m_lNDFTable[]	= {1080000L,108000L,18000L,1800L,300L,30L,10L,1L};
static long m_lNDF15Table[]	= {m_lNDFTable[0]/2,m_lNDFTable[1]/2,m_lNDFTable[2]/2,m_lNDFTable[3]/2,m_lNDFTable[4]/2,m_lNDFTable[5]/2,10L,1L};
static long m_lDFTable[]	= {1078920L,107892L,17982L,1798L,300L,30L,10L,1L};
static long m_lDFTable15[]  = {m_lDFTable[0]/2,m_lDFTable[1]/2,m_lDFTable[2]/2,m_lDFTable[3]/2,m_lDFTable[4]/2,m_lDFTable[5]/2,10L,1L};
static long m_l25Table[]	= {900000L, 90000L, 15000L,1500L,250L,25L,10L,1L};
static long m_l125Table[]	= {m_l25Table[0]/2,m_l25Table[1]/2,m_l25Table[2]/2,m_l25Table[3]/2,m_l25Table[4]/2,m_l25Table[5]/2,10L,1L};
static long m_l24Table[]	= {864000L, 86400L, 14400L,1440L,240L,24L,10L,1L};

void TimecodeConverters::ConvertSecondsToString__DF(char *TimeCode,double Seconds,double FrameRate) {
	if (TimecodeCompare(FrameRate,0.0)) {
		strcpy(TimeCode,"ErrNFrmRt");
		return;
		}

	int i,j;
	long *table;
	int flag=0;
	bool DropFrame=false;
	int Frames=GetFrameNumber(Seconds,FrameRate,1);

	// Select Conversion Table	
		 if (TimecodeCompare(FrameRate,30000.0/1001.0))	{ table = m_lDFTable; DropFrame=true; }
	else if (TimecodeCompare(FrameRate,15000.0/1001.0))	{ table = m_lDFTable15; DropFrame=true; }
	else if (TimecodeCompare(FrameRate,15.0))			table = m_lNDF15Table;
	else if (TimecodeCompare(FrameRate,30.0))			table = m_lNDFTable;
	else if (TimecodeCompare(FrameRate,25.0))			table = m_l25Table;
	else if (TimecodeCompare(FrameRate,12.5))			table = m_l125Table;	
	else if (TimecodeCompare(FrameRate,24.0))			table = m_l24Table;
	else												table = m_lNDFTable;

	// Negative timecode !
	if (Frames<0) 
	{	TimeCode[0]='-';
		TimeCode++;
		Frames=-Frames;
	}
	
	for (i=0,j=0;i<8;i++,j++)
	{	int temp=(int)(Frames/table[i]);
		TimeCode[j]=temp+'0';	// ASCII conversion
		Frames-=temp*table[i];

		if (DropFrame)
		{	// if minutesx10, do the drop frame test
			if (i==2)
			{	if (Frames<2) flag=1;
				else Frames-=2L;
			}
			if (i==3) 
			{	// this is where the 2 frames get added in
				if (!flag) Frames+=2L;
			}
		}
		
		// add punctuation
		if (1==j||4==j||7==j)
		{
			j++;
			if ((8==j)&&(table==m_lDFTable))
				 TimeCode[j] = '.';
			else TimeCode[j] = ':';
		}
	}
	if (strlen(TimeCode)<11) TimeCode[j++]=' ';
	TimeCode[j]=0;	// null terminate
}

void TimecodeConverters::ConvertStringToSeconds__DF(const char *TimeCode,double &Seconds,double FrameRate) {	
	if (TimecodeCompare(FrameRate,0.0)) {
		Seconds=-1.0;
		return;
		}

	// Select Conversion Table
	int Frames;
	long *table;
	bool DropFrame=false;
	     if (TimecodeCompare(FrameRate,30000.0/1001.0))	{ table = m_lDFTable; DropFrame=true; }
	else if (TimecodeCompare(FrameRate,15000.0/1001.0))	{ table = m_lDFTable15;  DropFrame=true; }
	else if (TimecodeCompare(FrameRate,15.0))			table = m_lNDF15Table;
	else if (TimecodeCompare(FrameRate,30.0))			table = m_lNDFTable;
	else if (TimecodeCompare(FrameRate,25.0))			table = m_l25Table;
	else if (TimecodeCompare(FrameRate,12.5))			table = m_l125Table;	
	else if (TimecodeCompare(FrameRate,24.0))			table = m_l24Table;
	else												table = m_lNDFTable;

	// Negative timecode !
	bool Negate=false;
	if (TimeCode[0]=='-')
	{	TimeCode++;
		Negate=true;
	}

	// Do the rest
	Frames=0;
	int j=7;
	int k=-1;
	for (int i=strlen(TimeCode)-1;i>=0;i--)
	{	if ((TimeCode[i]>'9')||(TimeCode[i]<'0')) 
		{	if (k==-1) j-=2;
			if (k== 0) j-=1;
			k=-1;
			continue;
		}
		else
		{	k++;
			if (k==2) k=0;
		}

		// Finished ?
		char temp=TimeCode[i]-'0';
		Frames+=(DWORD)temp*table[j];
		j--; 			
	}

	// Negate the timecode
	if (Negate) Frames=-Frames;
	Seconds=GetSeconds(Frames,FrameRate,1);
}

bool TimecodeConverters::IsDropFrameCompatible(double FrameRate) {
	bool DropFrameCompatible=false;
	if ((TimecodeCompare(FrameRate,30000.0/1001.0))||
		(TimecodeCompare(FrameRate,15000.0/1001.0))||
		(TimecodeCompare(FrameRate,15.0))||
		(TimecodeCompare(FrameRate,30.0))||
		(TimecodeCompare(FrameRate,25.0))||
		(TimecodeCompare(FrameRate,12.5))||
		(TimecodeCompare(FrameRate,24.0)))	DropFrameCompatible=true;
	return DropFrameCompatible;
	}


void TimecodeConverters::Timecode_ConvertToString(char *Timecode,double Seconds,double FrameRate,double ScalingFactor,double Offset,unsigned AttrFlags) {
	if ((TimecodeCompare(FrameRate,0.0))||((TimecodeCompare(ScalingFactor,0.0)))) {
		strcpy(Timecode,"ErrNFrmSc");
		return;
		}

	//Pre-Decrement offset
	Seconds+=Offset;
	//Scale value
	Seconds*=ScalingFactor;
	unsigned ViewMode=AttrFlags&TC_ViewModeReservedBits;

	switch (ViewMode) {
		case eTC_Normal_View:
			//Use drop frame method if frame rate is compatible and if user didn't explitly request for ndf
			if ((IsDropFrameCompatible(FrameRate))&&(!(AttrFlags&TC_Non_DropFrame))) {
				ConvertSecondsToString__DF(Timecode,Seconds,FrameRate);
				}
			else 
				ConvertSecondsToString_NDF(Timecode,Seconds,FrameRate);
			break;
		case eTC_Decimal_View:
			sprintf(Timecode,"%.2f",Seconds);
			break;
		case eTC_Integer_View:
			sprintf(Timecode,"%d",(int)Seconds);
			break;
		case eTC_Frame_View:
			sprintf(Timecode,"%d",GetFrameNumber(Seconds,FrameRate,1));
			break;
		}
	}

void TimecodeConverters::Timecode_ConvertToSeconds(const char *Timecode,double &Seconds,double FrameRate,double ScalingFactor,double Offset,unsigned AttrFlags) {
	if ((TimecodeCompare(FrameRate,0.0))||((TimecodeCompare(ScalingFactor,0.0)))) {
		Seconds=-1.0;
		return;
		}

	unsigned ViewMode=AttrFlags&TC_ViewModeReservedBits;

	switch (ViewMode) {
		case eTC_Normal_View:
			//Use drop frame method if frame rate is compatible and if user didn't explitly request for ndf
			if ((IsDropFrameCompatible(FrameRate))&&(!(AttrFlags&TC_Non_DropFrame))) {
				ConvertStringToSeconds__DF(Timecode,Seconds,FrameRate);
				}
			else ConvertStringToSeconds_NDF(Timecode,Seconds,FrameRate);
			break;
		case eTC_Decimal_View:
		case eTC_Integer_View:
			Seconds=atof(Timecode);
			break;
		case eTC_Frame_View:
			GetSeconds(atof(Timecode),FrameRate,1);
			break;
		}

	Seconds/=ScalingFactor; //Assert ScalingFactor!=0
	Seconds-=Offset;
	}




void TimecodeConverters::Timecode_SecondsToBCD(unsigned long &BCD,double  Seconds,double FrameRate,double ScalingFactor,double Offset,unsigned AttrFlags) {
	if (TimecodeCompare(FrameRate,0.0)) {
		BCD=0xc0dedbad;
		return;
		}

	char Timecode[16];
	//Only the DropFrame flag will have any effect with this function
	Timecode_ConvertToString(Timecode,Seconds,FrameRate,ScalingFactor,Offset,AttrFlags);
	//Now to convert the string into BCD
	unsigned length=strlen(Timecode);
	unsigned digit;
	BCD=0;
	for (unsigned i=0;i<length;i++) {
		if ((Timecode[i]>'9')||(Timecode[i]<'0')) continue;
		digit=Timecode[i]-'0';
		BCD<<=4;
		BCD|=digit;
		}
	}

void TimecodeConverters::Timecode_BCDToSeconds(unsigned long BCD,double &Seconds,double FrameRate,double ScalingFactor,double Offset,unsigned AttrFlags) {
	if (TimecodeCompare(FrameRate,0.0)) {
		Seconds=-1;
		return;
		}

	char Timecode[16];
	Timecode[ 0]=((BCD&0x30000000)>>(4*7))+'0';
	Timecode[ 1]=((BCD&0x0f000000)>>(4*6))+'0';
	Timecode[ 2]=':';  //Does not matter for drop frame since this gets passed through to the attributes
	Timecode[ 3]=((BCD&0x00700000)>>(4*5))+'0';
	Timecode[ 4]=((BCD&0x000f0000)>>(4*4))+'0';
	Timecode[ 5]=':';
	Timecode[ 6]=((BCD&0x00007000)>>(4*3))+'0';
	Timecode[ 7]=((BCD&0x00000f00)>>(4*2))+'0';
	Timecode[ 8]=':';
	Timecode[ 9]=((BCD&0x00000030)>>(4*1))+'0';
	Timecode[10]=(BCD&0x0000000f)+'0';
	Timecode[11]=0;
	//convert BCD into string
	Timecode_ConvertToSeconds(Timecode,Seconds,FrameRate,ScalingFactor,Offset,AttrFlags);
	}


void TimecodeConverters::Timecode_BCDToString(char *Timecode,unsigned long BCD,unsigned AttrFlags) {

	/*
	Its amazing how much more work you could do to accomplish the same result... Keeping this here for sg's

	//break this up into hours mins and seconds
	unsigned hours=  ((BCD&0xF0000000)>>28)*10;
			 hours+=  (BCD&0x0F000000)>>24;
	unsigned mins=   ((BCD&0x00F00000)>>20)*10;
			 mins+=   (BCD&0x000F0000)>>16;
	unsigned secs=   ((BCD&0x0000F000)>>12)*10;
			 secs+=   (BCD&0x00000F00)>> 8;
	unsigned frames= ((BCD&0x000000F0)>> 4)*10; 
			 frames+= (BCD&0x0000000F);

	if (!(AttrFlags&TC_Non_DropFrame)) {
		sprintf(Timecode,"%2d:%2d:%2d.%2d",hours,mins,secs,frames);
		}
	else {
		sprintf(Timecode,"%2d:%2d:%2d:%2d",hours,mins,secs,frames);
		}
	if (Timecode[0]==' ') Timecode[0]='0';
	if (Timecode[3]==' ') Timecode[3]='0';
	if (Timecode[6]==' ') Timecode[6]='0';
	if (Timecode[9]==' ') Timecode[9]='0';
	*/

	unsigned HH,MM,SS,FF;
	HH=MM=SS=FF=BCD;
	HH>>=24;
	MM=(MM>>16)&0xFF;
	SS=(SS>>8)&0xFF;
	FF&=0xFF;
	if (!(AttrFlags&TC_Non_DropFrame)) {
		sprintf(Timecode,"%.2x:%.2x:%.2x.%.2x",HH,MM,SS,FF);
		}
	else {
		sprintf(Timecode,"%.2x:%.2x:%.2x:%.2x",HH,MM,SS,FF);
		}
	}

void TimecodeConverters::Timecode_StringToBCD(const char *Timecode,unsigned long &BCD) {
	char *TimeSlot=(char *)&BCD;
	unsigned field=0;
	int i,j=1;

	BCD=0;
	//check for negative
	bool negative=false;
	if (Timecode[0]=='-') {
		negative=true;
		Timecode++;
		}

	for (i=strlen(Timecode)-1;i>=0;i--) {
		if (field>3) break;
		if ((Timecode[i]>'9')||(Timecode[i]<'0')) { //Delimiter detected
			if (j==0) j--;
			}
		else {
			TimeSlot[field]|=(Timecode[i]-'0') << ((j^1)<<2);
			j--;
			}
		if (j<0) {
			field++;
			j=1;
			}
		}
	//TODO see how to work with negative (if possible)
	//if (negative) Seconds=-Seconds;
	}
