#ifndef	__TimecodeConverters__
#define	__TimecodeConverters__

//Attribute flags:

//The first 2 bits are enumerated to select a mode type display
#define eTC_Normal_View					0
#define eTC_Decimal_View				1
#define eTC_Integer_View				2
#define eTC_Frame_View					3
//Mask out these bits to view the mode enumeration
#define TC_ViewModeReservedBits			3

//! Only the TC_Non_DropFrame flag will have any effect with BCD functions
#define TC_Non_DropFrame				4
//! These only apply to TimecodeFloat Only----------------------------------
#define TCF_Draw_Percentage_Sign		8
#define TCF_UseBaseClassConversionOnly	16

class TimecodeConverters {
	public:
		//! You must have a valid framerate!  These will throw exception if framerate is zero
		static void Timecode_ConvertToString (char *Timecode,double  Seconds,double FrameRate,double ScalingFactor=1.0,double Offset=0.0,unsigned AttrFlags=0);
		static void Timecode_ConvertToSeconds(const char *Timecode,double &Seconds,double FrameRate,double ScalingFactor=1.0,double Offset=0.0,unsigned AttrFlags=0);
		//! Only the TC_Non_DropFrame flag will have any effect with BCD functions
		static void Timecode_SecondsToBCD(unsigned long &BCD,double  Seconds,double FrameRate,double ScalingFactor=1.0,double Offset=0.0,unsigned AttrFlags=0);
		static void Timecode_BCDToSeconds(unsigned long  BCD,double &Seconds,double FrameRate,double ScalingFactor=1.0,double Offset=0.0,unsigned AttrFlags=0);
		static void Timecode_BCDToString(char *Timecode,unsigned long BCD,unsigned AttrFlags=0);
		static void Timecode_StringToBCD(const char *Timecode,unsigned long &BCD);

	private:
		//! the actual conversions (Drop Frame and Non-Drop Frame)
		static void ConvertSecondsToString__DF(char *Timecode,double  Seconds,double FrameRate);
		static void ConvertStringToSeconds__DF(const char *Timecode,double &Seconds,double FrameRate);
		static void ConvertSecondsToString_NDF(char *Timecode,double  Seconds,double FrameRate);
		static void ConvertStringToSeconds_NDF(const char *Timecode,double &Seconds,double FrameRate);
		static bool TimecodeCompare(double a,double b);
		static bool IsDropFrameCompatible(double FrameRate);
	};

#endif __TimecodeConverters__
