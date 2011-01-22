#pragma once

#define	RTME_TOASTER_SWIT_BUS_FMT_STRING \
	"media.devs.llswit"

#define	RTME_TOASTER_SWIT_INPUT_FMT_STRING \
	RTME_TOASTER_SWIT_BUS_FMT_STRING ".swit.in"

#define	RTME_TOASTER_MAIN_OUT_FMT_STRING \
	RTME_TOASTER_SWIT_BUS_FMT_STRING ".main.out"

#define	RTME_TOASTER_PREVIEW_OUT_FMT_STRING \
	RTME_TOASTER_SWIT_BUS_FMT_STRING ".preview.out"

#define	RTME_TOASTER_KEY_OUT_FMT_STRING \
	RTME_TOASTER_SWIT_BUS_FMT_STRING ".key.out"

#define	RTME_TOASTER_DSKEY_OUT_FMT_STRING \
	RTME_TOASTER_SWIT_BUS_FMT_STRING ".dskey.out"

#define	RTME_TOASTER_PROGRAM_OUT_FMT_STRING \
	RTME_TOASTER_SWIT_BUS_FMT_STRING ".program.out"

#define RTME_CARD_ROOT_FMT_STRING \
	"media.devs.vt%ld"

#define	RMTE_VIN_MEDIAPRODUCER_NAME_FMT_STRING \
	RTME_CARD_ROOT_FMT_STRING ".producers.vid%ld"

#define	RMTE_AUD_MEDIAPRODUCER_NAME_FMT_STRING \
	RTME_CARD_ROOT_FMT_STRING ".producers.aud%ld"

/*	String, contains long application name
*/
#define	SWIT_INPUT_PROPS_UI_APPNAME \
	"props.ui.appname"

/*	String, contains short aplication name, no greater than six characters
*/
#define	SWIT_INPUT_PROPS_UI_SHORTNAME \
	"props.ui.shortname"

/*	VideoToasterNT Live Audio Input Pin: (MediaProducer)
*
*	sprintf( name, RTME_TOASTER_AUD_IN_PRODUCER_FMT_STRING,
*			(long) input_num );
*
*	handle_id = sh_LockPublicHandle( sh, name );
*/
#define	RTME_TOASTER_AUD_IN_PIN_FMT_STRING \
	"media.pins.toaster.audin%ld"
#define	RTME_TOASTER_AUD_IN_PRODUCER_FMT_STRING \
	RTME_TOASTER_AUD_IN_PIN_FMT_STRING ".out"

/* If you want Quadraphonic output of these
* audio MediaProducers you append this string
* onto the MediaProducer name when you connect.
*/
#define RTME_QUAD_AUD_PRODUCER_APPEND_STRING	".qf48"

enum	/* for rtms_type */
{
	RTMS_TYPE_ERR,
	RTMS_TYPE_VIDEO,	/* 2d stream (video,alpha,zbuffer) */
	RTMS_TYPE_AUDIO,	/* 1d stream (audio) */
	RTMS_TYPE_INFO		/* information stream (positions, etc)*/
};

enum	/* for rtms_subtype if ( rtms_type == RTMS_TYPE_AUDIO ) */
{
	RTMS_SUBTYPE_AUDIO_ERR,
	RTMS_SUBTYPE_AUDIO_MONO_48k,	/* (unsupported) single channel (1,1,...) 32-bit left aligned signed */
	RTMS_SUBTYPE_AUDIO_STEREO_48k,	/* two channel (1,2,1,2,...) 32-bit left aligned signed */
	RTMS_SUBTYPE_AUDIO_QUAD_FP_48k,	/* four channel (1,2,3,4,1,2,3,4,...) floating point audio (-1.0 to +1.0) */
};

#define RTME_OUT_CREATE_VID_IS_PAL			0x0001
#define RTME_OUT_CREATE_VID_IS_NTSC_486		0x0002
#define RTME_OUT_CREATE_HAS_ALPHA			0x0004
#define RTME_OUT_CREATE_HAS_AUDIO			0x0008
#define RTME_OUT_CREATE_DISABLE_VIDEO		0x0010
#define RTME_OUT_CREATE_NO_AUTO_SELECT		0x0020	/* rtme_output_Create() ONLY */
#define RTME_OUT_CREATE_USE_FAST_VID		0x0040
#define RTME_OUT_CREATE_USE_FAST_VID_ONLY	0x0080
#define RTME_OUT_CREATE_SWIT_SELECT_NOTIFY	0x0100
#define RTME_OUT_CREATE_HAS_Z8				0x0200
#define RTME_OUT_CREATE_HAS_Z8AA			0x0400
#define RTME_OUT_CREATE_HAS_Z16				0x0800
#define RTME_OUT_CREATE_HAS_QUAD_FP_AUDIO	0x1000
#define RTME_OUT_CREATE_DEFIELD				0x2000

const unsigned RTME_AUDIO_FLAGS	= (RTME_OUT_CREATE_HAS_AUDIO|RTME_OUT_CREATE_HAS_QUAD_FP_AUDIO);

struct RTMEOutput;
struct SharedHub;
struct MediaProbe;
struct RTMEConnection;
struct VarBag;

/* An RTMF file has the file header, then an array of stream headers.
*/
struct RTMFStreamHeader
{
	short		rtms_type;				/* Stream Type */


	short		rtms_subtype;			/* Subtype of stream */

	/* 4 bytes */

	long		rtms_chunk_size;		/* Size (per chunk) of stream */

	/* 8 bytes */

	union
	{
		union
		{
			struct
			{
				long		pixel_width;			/* How many pixels wide is this data */
				long		pixel_height;			/* How many pixels high is the data */

				long		bytes_per_row;			/* actual byte-width of a scanline */
				long		reserved;				/* zero */

			} rgb;

			struct
			{
				/* 422 assumes that we've captured data by extracting a 2D rectangle
				* out of a 601 Frame.  The information below is enough to define
				* the rectangle that is written to this file.  The offset gives
				* you the upper-left corner of the video, the modulo gives you a way
				* to traverse to the next scanline, bytewidth and num_scanlines
				* give you the width and height of the recorded rectangle.
				*/
				long		captured_frame_offset;	/* Where (in the raw data frame) from 601 origin */

				long		captured_modulo;		/* line skip within raw data frame */

				long		captured_bytewidth;		/* byte width of captured data */

				long		captured_num_scanlines;	/* how many scanlines of bytewidth */

			} yuv_422;

			struct
			{
				long		captured_offset;		/* Offset (in the raw data frame) from 601 origin */

				long		captured_modulo;		/* line skip within raw data frame */

			} raw_601;

			struct
			{
				long		origin_scanline;		/* what did we start capturing in */
				long		num_scanlines;

			} composite_ntsc;

			struct
			{
				long		reserved;

			} composite_pal;

		} vid;

		union
		{
			struct
			{
				long		msb_position;			/* position of MSB in sample */

			} mono_48k;

			struct
			{
				long		msb_position;			/* position of MSB in sample */

			} stereo_48k;

		} aud;

		union
		{
			struct
			{
				long		reserved;

			} zinfo;

			struct
			{
				long		reserved;

			} ccinfo;

		} info;

	} type;

	/* 32 bytes */
};

union MediaGram
{
	/* CMD_HUBMEDIA_GET_PARAM */
	/* CMD_HUBMEDIA_SET_PARAM */
	struct
	{
		/* parameter name can be longer than this
		* union, just allocate the message to
		* accomodate the additional string length.
		*/
		char					param_name[16];

		/* The Value for the parameter needs to
		* be communicated in the buffer:
		*
		*  HubMessage.hm_reply_buf		and
		*	HubMessage.hm_reply_buf_len
		*
		*/
	} param;

	/* CMD_HUBMEDIA_ADD_CONSUMER */
	/* CMD_HUBMEDIA_REMOVE_CONSUMER */
	/* CMD_HUBMEDIA_ADD_NON_LOCKING_CONSUMER */
	/* CMD_HUBMEDIA_REMOVE_NON_LOCKING_CONSUMER */
	struct
	{
		long					consumer_handle_id;
	} to_producer;

	/* CMD_HUBMEDIA_OWN_PRODUCER */
	/* CMD_HUBMEDIA_FREE_PRODUCER */
	/* CMD_HUBMEDIA_OWN_CONSUMER */
	/* CMD_HUBMEDIA_FREE_CONSUMER */
	struct
	{
		long					 owner_handle_id;

		/* If ownership request fails, hh_Send() this message */
		long					 owner_fail_msg_id;

	} owner;

	/* CMD_HUBMEDIA_RELEASE_BUFFER */
	struct
	{
		long					 buffer_id;

		/* These Pointers only valid on the MediaProducer's process */
		void					*producer_userdata;
		void					*producer_userdata1;
		void					*producer_userdata2;
	} buffer;

	/* CMD_HUBMEDIA_INTRODUCTION */
	struct
	{
		/* Sent by producer to consumer before production.
		* of the first buffer.
		*/
		struct RTMFStreamHeader	intro_rtms;
	} intro;

	/* CMD_HUBMEDIA_GOODBYE */
	/* CMD_HUBMEDIA_FLUSH_PRODUCER */
	/* CMD_HUBMEDIA_PAUSE_PRODUCER */
	/* CMD_HUBMEDIA_UNPAUSE_PRODUCER */
	struct
	{
		/* Sent by producer to consumer as a "goodbye" message,
		* meaning it is no longer going to be providing data.
		*/
		long					producer_handle_id;
	} goodbye;

	/* CMD_HUBMEDIA_MEDIA_PRODUCED */
	/* CMD_HUBMEDIA_MEDIA_PRODUCED_PASSIVE */
	/* CMD_HUBMEDIA_MEDIA_BUNDLE */
	struct
	{
		/* Buffer ID of where media is */
		long					 buf_id;

		/* hh_Send() this when finished with buffer */
		long					 done_msg_id;

		/* When this mediagram is "due" */
		unsigned long			 due_time;

		/* originating mediaproducer time */
		unsigned long			 source_time;

		/* originating mediaproducer handle_id */
		long					 source_handle_id;

		/* Stream Header describing the contents of the MediaChunk */
		struct RTMFStreamHeader	 rtms;
	} media;

	/* CMD_HUBMEDIA_DISPLAYING */
	struct
	{
		/* Source time of the media being displayed */
		unsigned long			 source_time;

		/* Display flags, normally zero */
		unsigned long			 display_flags;

		unsigned long			 reserved[2];

		/* Stream Header describing the contents of the MediaChunk */
		struct RTMFStreamHeader	 rtms;
	} display;

	/* CMD_HUBMEDIA_BCAST_DISPLAYING: Broadcast-only Message */
	struct
	{
		long					 displayer_id;
		long					 displayer_time;

		unsigned long			 source_time;
		long					 source_handle_id;
	} bcast_display;
};

struct VTAudChunk	/* structure is 16 bytes */
{
	/* Get the array of sample data */
	#define vtaudchunk_GetSampleArray(ac) \
		((void *)((char *)(ac) + 16))

	/* NOTE: About AudChunkID
	 *
	 */
	char	ac_id[4];

	/* == 'A' 'S' '4' '8' Stereo 64 bit per sample (two 32 bit longwords), 48KHz */
	/*
	 * 	One audio sample is an 8 byte (2 integers) stereo pair:
	 *
	 *	|--- 32 bits ---|--- 32 bits ---|
	 *	+---+---+---+---+---+---+---+---+
	 *	|     right     |      left     |
	 *	+---+---+---+---+---+---+---+---+
	 *
	 *	Audio is signed, left-aligned, meaning maximum positive voltage
	 *	is 0x7fffffff and maximum negative voltage is 0x80000000
	 *
	 */
	#define vtaudchunk_IsStereoInt48(ac) \
		( ('A'==(ac)->ac_id[0]) && ('S'==(ac)->ac_id[1]) && \
		  ('4'==(ac)->ac_id[2]) && ('8'==(ac)->ac_id[3]) )

	#define vtaudchunk_SetStereoInt48Type(ac) \
		{ (ac)->ac_id[0]='A'; (ac)->ac_id[1]='S'; (ac)->ac_id[2]='4'; (ac)->ac_id[3]='8'; }

	/* == 'F' 'Q' '4' '8' Quad (4) Floating Point numbers per sample, 48KHz */
	/*
	 * 	One audio sample is a 16 byte quadraphonic:
	 *
	 *	|-- 32 bit.fp --|-- 32 bit.fp --|-- 32 bit.fp --|-- 32 bit.fp --|
	 *	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	 *	|  fwd-right.fp |  fwd-left.fp  | rear-right.fp |  rear-left.fp |
	 *	+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
	 *
	 *	Floating Point audio samples are "normalized" meaning a
	 *  FP value of +-1.0 will be at "clipping level" of the output
	 *	hardware.
	 *
	 */
	#define vtaudchunk_IsQuadFP48(ac) \
		( ('F'==(ac)->ac_id[0]) && ('Q'==(ac)->ac_id[1]) && \
		  ('4'==(ac)->ac_id[2]) && ('8'==(ac)->ac_id[3]) )

	#define vtaudchunk_SetQuadFP48Type(ac) \
		{ (ac)->ac_id[0]='F'; (ac)->ac_id[1]='Q'; (ac)->ac_id[2]='4'; (ac)->ac_id[3]='8'; }

	/* -------------------------------------------------------------------- */
	/*  Number of bytes of audio samples following this structure           */
	/*  (byte count does not include the size of this struct)               */
	/*
	 *	int		num_samples;
	 *
	 *	if ( vtaudchunk_IsStereoInt48(ac) )
	 *	{
	 *		long			*s32sample;
	 *
	 *		s32sample = (long *) vtaudchunk_GetSampleArray(ac);
	 *
	 *		num_samples = vtaudchunk_GetStereoInt48NumSamples( ac );
	 *
	 *		etc...
	 *	}
	 *	else if ( vtaudchunk_IsQuadFP48(ac) )
	 *	{
	 *		float			*fp32sample;
	 *
	 *		fp32sample = (float *) vtaudchunk_GetSampleArray(ac);
	 *
	 *		num_samples = vtaudchunk_GetQuadFP48NumSamples( ac );
	 *
	 *		etc...
	 *	}
	 *	else
	 *	{
	 *		printf("Unrecognized Audio Packet\n");
	 *	}
	 *
	 */
	/* -------------------------------------------------------------------- */
	long	ac_num_bytes;

	#define vtaudchunk_GetStereoInt48NumSamples(ac) \
		((ac)->ac_num_bytes >> 3)

	#define vtaudchunk_GetQuadFP48NumSamples(ac) \
		((ac)->ac_num_bytes >> 4)

	#define vtaudchunk_SetStereoInt48NumSamples(ac,num_samples) \
		{(ac)->ac_num_bytes = ((num_samples) << 3)}

	#define vtaudchunk_SetQuadFP48NumSamples(ac,num_samples) \
		{(ac)->ac_num_bytes = ((num_samples) << 4)}

	long	ac_reserved0;
	long	ac_reserved1;
};



#define SETUPFUNCTION(ret, name, ...) \
	typedef ret (__cdecl *DriverProc_##name )(__VA_ARGS__); \
	DriverProc_##name name = NULL;

#define GETFUNCTIONPOINTER(ftn,index) \
	if ((ftn =(DriverProc_##ftn) GetProcAddress(RTMEDriverDLL,#ftn))==NULL) throw index;

//Returns size of the bytes copied... 
//return 0... tries no additional path (e.g. if it can be loaded since its in the paths)
//returns -2 for a case index which fails
//returns -1 when no more entries are available
inline size_t GetApplicationPath(size_t index,wchar_t *BufferToFill,size_t size)
{
	size_t BytesCopied=-1;
	//TODO expand this once we have some registry keys
	if (index==0)
		return 0;

	return BytesCopied;
}

inline bool FindAndLoadDLL(HMODULE &RealDriverDLL,const wchar_t *RealDllName,bool (*GetFunctionPointers)(void) )
{
	bool Success=false;
	//for now we'll just go down the list in order, but we may wish to sort according to file date

	wchar_t ApplicationPath[MAX_PATH*2];
	size_t index=0,AppPathSizeReturn;
	while ((AppPathSizeReturn=GetApplicationPath(index++,ApplicationPath,MAX_PATH*2)) != -1)
	{
		if (AppPathSizeReturn==-2) continue;  //some paths may not exist
		if (AppPathSizeReturn==0)
		{
			ApplicationPath[0]=0;
		}
		else
		{
			//Set current working directory
			SetCurrentDirectory(ApplicationPath);
		}
		wcscat_s(ApplicationPath,RealDllName);
		RealDriverDLL=LoadLibrary(ApplicationPath);
		if (RealDriverDLL)
		{
			Success=GetFunctionPointers();
			if (!Success) 
			{
				FreeLibrary(RealDriverDLL);
				RealDriverDLL=NULL;
			}
			else
			{
				const size_t BufferSize=MAX_PATH*2;
				wchar_t TempBuf[BufferSize];
				swprintf_s(TempBuf,BufferSize,L"* * * Using %s \n",ApplicationPath);
				OutputDebugString(TempBuf);
				break; //success, we'll use this one
			}
		}
	}

	//Dump a courtesy to output to show we couldn't find the dll
	if (!Success)
	{
		const size_t BufferSize=MAX_PATH*2;
		wchar_t TempBuf[BufferSize];
		swprintf_s(TempBuf,BufferSize,L"* * * Unable to find %s \n",RealDllName);
		OutputDebugString(TempBuf);
	}

	return Success;
}

//TO Use these RTME calls you'll need:
//SETUPFUNCTION0(RTMEConnection *,rtc_InitializeRTME);
//SETUPFUNCTION1(void,rtc_ShutdownRTME,RTMEConnection *rtc);

//and

//GETFUNCTIONPOINTER(rtc_InitializeRTME,0);
//GETFUNCTIONPOINTER(rtc_ShutdownRTME,1);


typedef RTMEConnection *(__cdecl *DriverProc_rtc_InitializeRTME )();
extern DriverProc_rtc_InitializeRTME rtc_InitializeRTME;
typedef void (__cdecl *DriverProc_rtc_ShutdownRTME )(RTMEConnection *rtc);
extern DriverProc_rtc_ShutdownRTME rtc_ShutdownRTME;

bool GetFunctionPointers();

inline void CloseResources(RTMEConnection *RTC,HMODULE &RTMEDriverDLL)
{
	if (RTMEDriverDLL)
	{
		if (RTC)
			rtc_ShutdownRTME(RTC);
		FreeLibrary(RTMEDriverDLL);
		RTMEDriverDLL = NULL;
	}
}

inline void OpenResources(RTMEConnection *&RTC,const wchar_t *DllName,wchar_t *ApplicationPath,HMODULE &RTMEDriverDLL)
{
	assert(!RTMEDriverDLL); //just checking
	if (!RTMEDriverDLL)
	{
		if (!FindAndLoadDLL(RTMEDriverDLL,DllName,GetFunctionPointers))
			CloseResources(RTC,RTMEDriverDLL),throw"NewTekRTME : failed to load."; __FUNCSIG__;

		GetCurrentDirectory(MAX_PATH*2,ApplicationPath);
		RTC=rtc_InitializeRTME();
		if (!RTC)
			CloseResources(RTC,RTMEDriverDLL),throw"NewTekRTME : Cannot initialise the realtime engine."; __FUNCSIG__;
	}
}
