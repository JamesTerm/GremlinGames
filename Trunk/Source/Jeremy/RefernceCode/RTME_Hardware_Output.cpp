#include "stdafx.h"
#include "Modules.Hardware.Output.h"
#include "../RTME_Common.h"

#define DLLNAME				L"NewTekRTME.dll"

#define _MULTITHREADED_AV_

//Use this for verbose
//#define write_log DebugOutput
#define write_log

HMODULE RTMEDriverDLL=NULL;

SETUPFUNCTION(RTMEConnection *,rtc_InitializeRTME);
SETUPFUNCTION(void,rtc_ShutdownRTME,RTMEConnection *rtc);
SETUPFUNCTION(SharedHub *,rtc_GetHub,RTMEConnection *rtc);
SETUPFUNCTION(RTMEOutput *,rtme_output_Create,const char *my_appname,long device_num,long output_num,long create_flags);
SETUPFUNCTION(RTMEOutput *,rtme_output_CreateEx,const char *my_appname,long create_flags,const char *prop_long_name,const char *prop_short_name,unsigned long prop_permkey);
SETUPFUNCTION(void,rtme_output_ChangeFlags,RTMEOutput *ro,long new_create_flags);
SETUPFUNCTION(void,rtme_output_Delete,RTMEOutput *ro);
SETUPFUNCTION(long,rtme_output_AllocFrame_Flags,RTMEOutput *ro,long create_flags );
SETUPFUNCTION(VTAudChunk *,rtme_output_GetFrameAudioBuffer,RTMEOutput *ro,long frame_buf_id,long *psize);
SETUPFUNCTION(void,rtme_output_FreeFrame,RTMEOutput *ro,long frame_buf_id,long frame_buf_size);
SETUPFUNCTION(long,rtme_output_SendFrame,RTMEOutput *ro,long frame_buf_id,long frame_buf_size);
#define rtme_output_SendFrame_callback_parms 	void (*frame_displaying_callback)(void *userdata0,void *userdata1,unsigned long	 src_time)
SETUPFUNCTION(long, rtme_output_SendFrameWithCallback,RTMEOutput *ro,long frame_buf_id,rtme_output_SendFrame_callback_parms,void *userdata0,void *userdata1);
#define rtme_output_SendFrame_callbackEx_parms 	void (*frame_displaying_callback_ex)(void *userdata0,void *userdata1,unsigned long src_time,unsigned long display_flags)
SETUPFUNCTION(long, rtme_output_SendFrameWithCallbackEx,RTMEOutput *ro,long frame_buf_id,rtme_output_SendFrame_callbackEx_parms,void *userdata0,void *userdata1);
SETUPFUNCTION(long ,rtme_output_OutputToMain,RTMEOutput *ro,long exclusive);
SETUPFUNCTION(long,rtme_output_OutputToPreview,RTMEOutput *ro,long exclusive);
SETUPFUNCTION(long,rtme_output_OutputToKey,RTMEOutput *ro,long exclusive);
SETUPFUNCTION(long,rtme_output_OutputToDownKey,RTMEOutput *ro,long exclusive);
SETUPFUNCTION(VarBag *,rtme_output_GetVidVarBag,RTMEOutput *ro);
SETUPFUNCTION(VarBag *,rtme_output_GetAudVarBag,RTMEOutput *ro);
SETUPFUNCTION(void *,rtme_output_GetFrameFieldBuffer,RTMEOutput *ro,long frame_buf_id,long *psize,long field_zero_or_one);
SETUPFUNCTION(void *,rtme_output_GetFrameZBuffer,RTMEOutput *ro,long frame_buf_id,long *psize,long field_zero_or_one);
SETUPFUNCTION(void *,rtme_output_GetFrameAlphaBuffer,RTMEOutput *ro,long frame_buf_id,long *psize,long field_zero_or_one);

//TODO see if I really need this since it is in Beethoven
//SETUPFUNCTION(long,varbag_SetString,struct VarBag *vb,const char *entry_name,const char *value);
//		GETFUNCTIONPOINTER(varbag_SetString,17);

bool GetFunctionPointers()
{
	bool Success=false;
	try
	{
		GETFUNCTIONPOINTER(rtc_InitializeRTME,0);
		GETFUNCTIONPOINTER(rtc_ShutdownRTME,1);
		GETFUNCTIONPOINTER(rtc_GetHub,2);
		GETFUNCTIONPOINTER(rtme_output_Create,3);
		GETFUNCTIONPOINTER(rtme_output_CreateEx,4);
		GETFUNCTIONPOINTER(rtme_output_ChangeFlags,5);
		GETFUNCTIONPOINTER(rtme_output_Delete,6);
		GETFUNCTIONPOINTER(rtme_output_AllocFrame_Flags,7);
		GETFUNCTIONPOINTER(rtme_output_GetFrameAudioBuffer,8);
		GETFUNCTIONPOINTER(rtme_output_FreeFrame,9);
		GETFUNCTIONPOINTER(rtme_output_SendFrame,10);
		GETFUNCTIONPOINTER(rtme_output_SendFrameWithCallback,11);
		GETFUNCTIONPOINTER(rtme_output_SendFrameWithCallbackEx,12);
		GETFUNCTIONPOINTER(rtme_output_OutputToMain,13);
		GETFUNCTIONPOINTER(rtme_output_OutputToPreview,14);
		GETFUNCTIONPOINTER(rtme_output_OutputToKey,15);
		GETFUNCTIONPOINTER(rtme_output_OutputToDownKey,16);
		GETFUNCTIONPOINTER(rtme_output_GetVidVarBag,17);
		GETFUNCTIONPOINTER(rtme_output_GetAudVarBag,18);
		GETFUNCTIONPOINTER(rtme_output_GetFrameFieldBuffer,19);
		GETFUNCTIONPOINTER(rtme_output_GetFrameZBuffer,20);
		GETFUNCTIONPOINTER(rtme_output_GetFrameAlphaBuffer,21);
		Success=true;
	}
	catch (int ErrorCode)
	{
		const size_t BufferSize=MAX_PATH*2;
		wchar_t TempBuf[BufferSize];
		swprintf_s(TempBuf,BufferSize,L"%s : GetFunctionPointers failed %d \n",DLLNAME,ErrorCode);
		OutputDebugString(TempBuf);
	}
	return Success;
}

//Ensure we have our path when we load the project
wchar_t g_ApplicationPath[MAX_PATH*2];

//This may throw exception
class DLLInstantiation
{
	public:
		DLLInstantiation() : m_RTC(NULL) {OpenResources(m_RTC,DLLNAME,g_ApplicationPath,RTMEDriverDLL);}
		~DLLInstantiation()		{CloseResources(m_RTC,RTMEDriverDLL);}
		RTMEConnection *GetRTMEConnection()
		{
			return m_RTC;
		}
	private:
		RTMEConnection *m_RTC;
};

static DLLInstantiation &ConnectToRTME()
{
	static DLLInstantiation s_AutoInstantiate;  //I think this is neater than DLLProcessAttach
	return s_AutoInstantiate;
}



using namespace Modules::Hardware::Output;
using namespace FrameWork::Communication;
using namespace FrameWork::Base;
using namespace FrameWork::Bitmaps;

namespace Modules
{
	namespace Hardware
	{
		namespace Output
		{


class SD_Output_Internal :	public video_input_stream_ycbcr_callback_interface,
							public audio_input_stream_callback_interface

{
	public:
		SD_Output_Internal(	SD_Output *Parent,
			const SD_Output::ChannelOutputEnum OutputTo=SD_Output::ePassive,
			const wchar_t ThisAppName[]=L"SD_Output"			///< Unique name of this app; Throws exceptions if unable to connect successfully
			);
		~SD_Output_Internal();

		void SendFrameCallback(const bitmap_ycbcr_u8 &image, const bitmap_ycbcr_u8 *upper,unsigned long display_frags);

	public: //from video input stream YCbCr callback interface
		virtual void receive_image_from_input(const FrameWork::Bitmaps::bitmap_ycbcr_u8 &image, const video_input_stream_ycbcr::image_info &frame_info);
		virtual void flush_all_images( void );

	public: //from audio input stream callback interface
		virtual void audio_input_stream_receive_buffer_from_input(const void *buffer, const audio_input_stream_buffer_info &buffer_info);
		virtual void audio_input_stream_flush_all_images( void );

	private:
		bool SetOutputFormat(size_t YRes,bool Alpha,bool Video,bool Audio,const char *AppName="SD_Output");

		SD_Output * const m_pParent;
		RTMEConnection *m_RTC;
		//! The output engine
		RTMEOutput		*m_RTMEOutput;
		size_t m_OutputFormatFlags; //cache to ensure we have frame integrity
		long m_Field0Size;
		//cache to handle field callbacks.  If this is zero, this means that no frames are currently allocated
		long m_BufferID;
		const bitmap_ycbcr_u8 *m_UpperField; ///< cache to handle field callbacks to return the correct offset on the send frame callback

		FrameWork::Audio::fqueue<FrameWork::Audio::sample_type_quad_f32> audio_queue;
		FrameWork::Threads::critical_section m_queue_lock;
		
		std::map<size_t, FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_quad_f32> *> m_hq_resamplers;

		size_t get_supported_resampler_rates(size_t idx);
};

		}
	}
}

bool SD_Output_Internal::SetOutputFormat(size_t YRes,bool Alpha,bool Video,bool Audio,const char *AppName)
{
	size_t ExtraAdd=0;
	// Get the new flags
	size_t l_NewFlags=0;
	if (Video)
		l_NewFlags=	( (YRes==288)?RTME_OUT_CREATE_VID_IS_PAL:0 )|
		( (YRes==243)?RTME_OUT_CREATE_VID_IS_NTSC_486:0 )|
		( (Audio)    ?RTME_AUDIO_FLAGS:0 )|
		//( (FPAudio)  ?RTME_OUT_CREATE_HAS_QUAD_FP_AUDIO:0 )|
		( (Alpha)    ?RTME_OUT_CREATE_HAS_ALPHA:0 )|ExtraAdd;
	else	l_NewFlags=	( (Audio)    ?RTME_AUDIO_FLAGS:0 )|
		RTME_OUT_CREATE_DISABLE_VIDEO|ExtraAdd;

	// Now open it !
	if (!m_RTMEOutput)	
	{	// Create the output
		write_log("rtme_output_Create Start\n");
		// Get the output, even if it takes some time
		while(!m_RTMEOutput)
		{	
			m_RTMEOutput=rtme_output_CreateEx(NULL,(long)l_NewFlags,AppName,AppName,0);
			Sleep(30);
		}
		write_log("rtme_output_Create Finish\n");

		// If we have an alpha-channel, I would like to go on DSK
		write_log("rtme_output_OutputToDownKey Start\n");
		if (Alpha) rtme_output_OutputToDownKey(m_RTMEOutput,true);
		write_log("rtme_output_OutputToDownKey Finish\n");
	}
	else
	{	// Change the output names
		write_log("rtme_output_ChangeFlags Start\n");
		rtme_output_ChangeFlags(m_RTMEOutput,(long)l_NewFlags);
		write_log("rtme_output_ChangeFlags Finish\n");
	}
	m_OutputFormatFlags=l_NewFlags;
	return m_RTMEOutput!=NULL;
}

SD_Output_Internal::SD_Output_Internal(SD_Output *Parent,const SD_Output::ChannelOutputEnum OutputTo,const wchar_t ThisAppName[]) : m_pParent(Parent)
{
	m_RTC=ConnectToRTME().GetRTMEConnection();  //this may throw exception

	m_RTMEOutput=NULL;
	wchar2char(ThisAppName);
	if (SetOutputFormat(240,false,true,true,wchar2char_pchar))
	{
		switch(OutputTo)
		{
			case SD_Output::ePreview:
				rtme_output_OutputToPreview(m_RTMEOutput,0);
				break;
			case SD_Output::eMain:
				rtme_output_OutputToMain(m_RTMEOutput,0);
				break;
			case SD_Output::eKey:
				rtme_output_OutputToKey(m_RTMEOutput,0);
				break;
			case SD_Output::eDSK:
				rtme_output_OutputToDownKey(m_RTMEOutput,0);
				break;
		}
	}
	else assert(false);
	m_BufferID=0;

		
	
	size_t i=0;
	size_t rate=get_supported_resampler_rates(i);

	for (;rate=get_supported_resampler_rates(i); i++)
		m_hq_resamplers[rate] = new FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_quad_f32>(48000, rate);
}

SD_Output_Internal::~SD_Output_Internal()
{
	if (m_RTMEOutput) 
	{	
		// Die !
		DebugOutput(L"rtme_output_Delete Start\n");		
		rtme_output_Delete(m_RTMEOutput);
		DebugOutput(L"rtme_output_Delete Finish\n");
		// pedantic
		m_RTMEOutput=NULL;
	}

	size_t i=0; 
	size_t rate=get_supported_resampler_rates(i);

	for (;rate=get_supported_resampler_rates(i); i++)
		delete m_hq_resamplers[rate];
}

size_t SD_Output_Internal::get_supported_resampler_rates(size_t idx){
	
	size_t src_rates[] = 	{
								8000,
								11025,
								16000,
								22050,
								32000,
								44100,
								96000,
								0				//0 terminated
							};
#ifdef _DEBUG
	assert(idx < sizeof(src_rates)/sizeof(size_t));
#endif

	return src_rates[idx];
}

struct FramePacket
{
	const bitmap_ycbcr_u8 *image;
	const bitmap_ycbcr_u8 *upper; //save the upper field info
};


void SD_Output_Internal::SendFrameCallback(const bitmap_ycbcr_u8 &image, const bitmap_ycbcr_u8 *upper,unsigned long display_frags)
{
	//sorry this was suppose to be a per field callback but it isn't
	if (upper)
	{
		//m_pParent->m_VideoFrameReceive->return_frame_displayed(*upper);	// Forget about it...this takes up too much damn time.
		m_pParent->m_VideoFrameReceive->return_image(*upper,false);
	}

	m_pParent->m_VideoFrameReceive->return_frame_displayed(image);
	m_pParent->m_VideoFrameReceive->return_image(image,false);
}

void frame_displaying_callback_ex (void *userdata0,void *userdata1,unsigned long src_time,unsigned long display_flags)
{
#ifdef _DEBUG
	char szDbgMsg[256] = { 0 };
	::sprintf(szDbgMsg, "\ndisplay callback w/ src_time=%d and packet ptr=%p\n", (int)src_time, userdata1);
	::OutputDebugStringA(szDbgMsg);
#endif

	SD_Output_Internal *instance=(SD_Output_Internal *)userdata0;
	FramePacket *packet=(FramePacket *)userdata1;
	instance->SendFrameCallback(*packet->image,packet->upper,display_flags);
	delete packet;
}

void SD_Output_Internal::receive_image_from_input(const bitmap_ycbcr_u8 &image, const video_input_stream_ycbcr::image_info &frame_info)
{
	//TODO check video format integrity here:
	if (!m_RTMEOutput)
		return;

	long Field0Size=0;
	long Field1Size=0;
	video_output_stream_ycbcr::image_info::e_field_type Field=frame_info.m_field_type;
	switch (Field)
	{
		case video_output_stream_ycbcr::image_info::e_field_type_frame:
			{
				long m_BufferID=0;//rtme_output_AllocFrame_Flags(m_RTMEOutput,(long)m_OutputFormatFlags);
				size_t TimeOut=0;
				
				while(!(m_BufferID=rtme_output_AllocFrame_Flags(m_RTMEOutput,(long)m_OutputFormatFlags)) && (TimeOut++<30))
					Sleep(5);

				if (!m_BufferID)
				{
					assert(false);
					return ;
				}

				pixel_ycbcr_u8 *DestField0=(pixel_ycbcr_u8*)rtme_output_GetFrameFieldBuffer(m_RTMEOutput,m_BufferID,&Field0Size,0);
				pixel_ycbcr_u8 *DestField1=(pixel_ycbcr_u8*)rtme_output_GetFrameFieldBuffer(m_RTMEOutput,m_BufferID,&Field1Size,1);

				//Progressive frames are even field first.
				// Note: const &image will use--> const pixel_type*	operator() ( const int y ) const;
				//because it can only call methods that are const, so the pointer returned is const pixel_type*, which is not compatable with:
				//bitmap_ycbcr_u8( pixel_type *p_data, const int xres, const int yres, const int stride=0 );
				//So we'll need to const_cast to be able to use this (we may want a new constructor for read only buffers)
				//Fortunately, we are only using these for read, so it should be fine.
				bitmap_ycbcr_u8 ImageField0(const_cast<pixel_ycbcr_u8*>(image(0)),image.xres(),image.yres()/2,image.stride()*2);
				bitmap_ycbcr_u8 ImageField1(const_cast<pixel_ycbcr_u8*>(image(1)),image.xres(),image.yres()/2,image.stride()*2);
				bitmap_ycbcr_u8 RTMEDest0(DestField0,image.xres(),image.yres()/2);
				bitmap_ycbcr_u8 RTMEDest1(DestField1,image.xres(),image.yres()/2);
				RTMEDest0=ImageField0;
				RTMEDest1=ImageField1;
				
				//for now, since this is really testing, we're doing the muxing here so pull from the queue
				//and fill in the audio section of the rtme frame
				long l_size=0;
	
				VTAudChunk *aud_chunk= rtme_output_GetFrameAudioBuffer(m_RTMEOutput, m_BufferID, &l_size);
				
				FrameWork::Audio::sample_type_quad_f32 *output_samples = new FrameWork::Audio::sample_type_quad_f32[aud_chunk->ac_num_bytes>>4];
#ifdef _MULTITHREADED_AV_
				m_queue_lock.lock();
#endif
				if(audio_queue.pop(output_samples, aud_chunk->ac_num_bytes>>4))
					memcpy(aud_chunk+1, &output_samples[0], aud_chunk->ac_num_bytes);
				else//would rather not do this, but for first test lets try it
				{
					// mwatkins - Since I don't send audio yet, this gets called to much and is annoying.
					//DebugOutput(L"audio queue did not have enough samples, filling buffer w/ silence\n");
					::memset(aud_chunk+1, 0, aud_chunk->ac_num_bytes);
				}
#ifdef _MULTITHREADED_AV_
				m_queue_lock.unlock();
#endif
			
				//rtme_output_SendFrame(m_RTMEOutput, m_BufferID, l_size+Field0Size+Field1Size);
				{
					FramePacket *packet=new FramePacket;
					packet->image=&image;
					packet->upper=NULL;
					rtme_output_SendFrameWithCallbackEx(m_RTMEOutput, m_BufferID, frame_displaying_callback_ex,this,packet);
				}
	
				delete[] output_samples;
	
				m_BufferID=0;

				break;
			}
		case video_output_stream_ycbcr::image_info::e_field_type_upper_field:
			if (m_BufferID==0)
			{

				{
					size_t TimeOut=0;
					while(!(m_BufferID=rtme_output_AllocFrame_Flags(m_RTMEOutput,(long)m_OutputFormatFlags)) && (TimeOut++<30))
						Sleep(5);

					if (!m_BufferID)
					{
						assert(false);
						return ;
					}

				}

				m_UpperField=&image;

				pixel_ycbcr_u8 *DestField0=(pixel_ycbcr_u8*)rtme_output_GetFrameFieldBuffer(m_RTMEOutput,m_BufferID,&m_Field0Size,0);
				if (image.yres()==243)
				{
					bitmap_ycbcr_u8 ImageField0(const_cast<pixel_ycbcr_u8*>(image(1)),image.xres(),image.yres()-3);
					bitmap_ycbcr_u8 RTMEDest0(DestField0,image.xres(),image.yres()-3);
					RTMEDest0=ImageField0;
				}
				else
				{
					bitmap_ycbcr_u8 RTMEDest0(DestField0,image.xres(),image.yres());
					RTMEDest0=image;
				}

			}
			break;
		case video_output_stream_ycbcr::image_info::e_field_type_lower_field:
			if (m_BufferID!=0)
			{
				pixel_ycbcr_u8 *DestField1=(pixel_ycbcr_u8*)rtme_output_GetFrameFieldBuffer(m_RTMEOutput,m_BufferID,&Field1Size,1);
				if (image.yres()==243)
				{
					bitmap_ycbcr_u8 ImageField1(const_cast<pixel_ycbcr_u8*>(image(2)),image.xres(),image.yres()-3);
					bitmap_ycbcr_u8 RTMEDest1(DestField1,image.xres(),image.yres()-3);
					RTMEDest1=ImageField1;
				}
				else
				{
					bitmap_ycbcr_u8 RTMEDest1(DestField1,image.xres(),image.yres());
					RTMEDest1=image;
				}

				//for now, since this is really testing, we're doing the muxing here so pull from the queue
				//and fill in the audio section of the rtme frame
				long l_size=0;
	
				VTAudChunk *aud_chunk= rtme_output_GetFrameAudioBuffer(m_RTMEOutput, m_BufferID, &l_size);
				
				FrameWork::Audio::sample_type_quad_f32 *output_samples = new FrameWork::Audio::sample_type_quad_f32[aud_chunk->ac_num_bytes>>4];
#ifdef _MULTITHREADED_AV_
				m_queue_lock.lock();
#endif
				if(audio_queue.pop(output_samples, aud_chunk->ac_num_bytes>>4))
					memcpy(aud_chunk+1, &output_samples[0], aud_chunk->ac_num_bytes);
				else//would rather not do this, but for first test lets try it
				{				
					// mwatkins - Since I don't send audio yet, this gets called to much and is annoying.
					//DebugOutput(L"audio queue did not have enough samples, filling buffer w/ silence\n");
					::memset(aud_chunk+1, 0, aud_chunk->ac_num_bytes);
				}
#ifdef _MULTITHREADED_AV_		
				m_queue_lock.unlock();
#endif
			
				//rtme_output_SendFrame(m_RTMEOutput, m_BufferID, l_size+Field0Size+Field1Size);
				{
					FramePacket *packet=new FramePacket;
					packet->image=&image;
					packet->upper=m_UpperField;
					rtme_output_SendFrameWithCallbackEx(m_RTMEOutput, m_BufferID, frame_displaying_callback_ex,this,packet);
				}

	
				delete[] output_samples;
	
				m_BufferID=0;
			}
			break;
	}

	
	
	
	
}
void SD_Output_Internal::flush_all_images( void )
{
	//ensure RTME doesn't have any outstanding frames
	if (m_BufferID)
	{
		rtme_output_FreeFrame(m_RTMEOutput,m_BufferID,m_Field0Size<<1);
		m_BufferID=0;
	}
}
/*
#define byte unsigned char 
#include "../../../user/james killian/audiotesting/wave/wave.h"
#pragma comment(lib, "../../../../bin/debug/wave.lib")
#undef byte
*/

void SD_Output_Internal::audio_input_stream_receive_buffer_from_input(const void *buffer, const FrameWork::Communication::audio_input_stream_buffer_info &buffer_info)///audio only does not work currently because the rtme frames are actually being filled in the video recieve method
{
	if (!m_RTMEOutput)
		return;

	void *l_pData=0;
	size_t l_MemorySize=0;

	FrameWork::Audio::aud_buf_quad_f32 output;
	
	//while I'm not sure that the best place for resampling is at the end of the processing chain, for now, it's here

	const wchar_t *wsz_SampleType=buffer_info.m_SampleType.c_str();
	//Here is where we switch between various buffer types
	if (wcscmp(wsz_SampleType,L"aud_buf_stereo_s16")==0)
	{
		FrameWork::Audio::aud_buf_stereo_s16 *buf=(FrameWork::Audio::aud_buf_stereo_s16 *)buffer;
		FrameWork::Audio::aud_buf_quad_f32 preoutput((FrameWork::Audio::sample_type_quad_f32 *)0, 0, buf->sample_rate());

		if(buf->sample_rate()!=output.sample_rate())
		{
			FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_quad_f32> *resampl = m_hq_resamplers[buf->sample_rate()];
			preoutput=(*buf);
			output = (*resampl)(preoutput);
		}
		else
			output=(*buf);
		
		//l_pData = output();
		//l_MemorySize=output.size_in_bytes();
	}
	else if (wcscmp(wsz_SampleType,L"aud_buf_oct_s16")==0)
	{
		FrameWork::Audio::aud_buf_oct_s16 *buf=(FrameWork::Audio::aud_buf_oct_s16 *)buffer;
		FrameWork::Audio::aud_buf_quad_f32 preoutput((FrameWork::Audio::sample_type_quad_f32 *)0, 0, buf->sample_rate());
		if(buf->sample_rate()!=output.sample_rate())
		{
			FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_quad_f32> *resampl = m_hq_resamplers[buf->sample_rate()];
			preoutput=(*buf);
			output = (*resampl)(preoutput);
		}
		else
			output=(*buf);

		//l_pData=output();
		//l_MemorySize=output.size_in_bytes();
	}
	else if (wcscmp(wsz_SampleType,L"aud_buf_stereo_f32")==0)
	{	
		FrameWork::Audio::aud_buf_stereo_f32 *buf=(FrameWork::Audio::aud_buf_stereo_f32 *)buffer;
		FrameWork::Audio::aud_buf_quad_f32 preoutput((FrameWork::Audio::sample_type_quad_f32 *)0, 0, buf->sample_rate());
		if(buf->sample_rate()!=output.sample_rate())
		{
			FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_quad_f32> *resampl = m_hq_resamplers[buf->sample_rate()];
			preoutput=(*buf);
			output = (*resampl)(preoutput);
		}
		else
			output=(*buf);

		//l_pData=output();
		//l_MemorySize=output.size_in_bytes();
	}
	else if (wcscmp(wsz_SampleType,L"aud_buf_oct_f32")==0)
	{		
		FrameWork::Audio::aud_buf_oct_f32 *buf=(FrameWork::Audio::aud_buf_oct_f32 *)buffer;
		FrameWork::Audio::aud_buf_quad_f32 preoutput((FrameWork::Audio::sample_type_quad_f32 *)0, 0, buf->sample_rate());
		if(buf->sample_rate()!=output.sample_rate())
		{
			FrameWork::Audio::resampler<FrameWork::Audio::aud_buf_quad_f32> *resampl = m_hq_resamplers[buf->sample_rate()];
			preoutput=(*buf);
			output = (*resampl)(preoutput);
		}
		else
			output=(*buf);

		//l_pData=output();
		//l_MemorySize=output.size_in_bytes();
	}
	else
		assert(false);

	if(output.size())
	{	
		//scale to sqrt(2) 1.4142135623730950488016887242097
		output*=1.4142135623730950488016887242097f;

#ifdef _MULTITHREADED_AV_		
		m_queue_lock.lock();
#endif
		//per James suggestion, we'll check that the queue isnt way too large already (which might cause obvious audio drift)
		//and if so clear it and start off fresh
		//DebugOutput(L"pre audio queue size %i\n", audio_queue.size());
		if(audio_queue.size()>9800)	//~<2 frames, since even at 48000 passthrough we still have ~>half this amount
		{		
//leaving this alternative multithreaded path commented out till I can actually test it and see the results
/*#ifdef _MULTITHREADED_AV_		
			m_queue_lock.unlock();
#endif
#ifndef _MULTITHREADED_AV_*/
			DebugOutput(L"audio queue size exceeded threshold, flushing queue\n");
			audio_queue.clear();
/*#else
		//this option just doesnt help when we're running on the same thread as the video input callback, we could be
		//way ahead of output, but the processing for future samples (which is already happenning with this single
		//thread codepath) blocks output of currently available ones, 
		//something else i'm considering is if we might want to move this up ahead of the resample
			
			//give output some time to catch up without us blocking the queue pop w/ the queue lock :P
			Sleep(100);

			m_queue_lock.lock();
#endif*/
		}

		audio_queue.push(output(), output.size());

		//DebugOutput(L"post audio queue size %i\n", audio_queue.size());
#ifdef _MULTITHREADED_AV_		
		m_queue_lock.unlock();
#endif
		/*
		static FileWriter::Wave::WaveFile wave;
		static bool Started=false;
		if (!Started)
		{
			wave.OpenForWrite("c:\\Testpbdata.wav",48000,-32,2);
			//wave.OpenForWrite("c:\\Test.wav",(DWORD)GetSampleRate(),(short)GetFormatBitDepth(GetSampleFormat()),(WORD)GetNoChannels());
			Started=true;
		}
		for (size_t i=0;i<output.size();i++)
			wave.WriteStereoFloatSample( output[i][0], output[i][1] );

		if(false)
			wave.Close();
		*/
/*
		VTAudChunk *aud_chunk;
		long l_buffer_id = 0;
		long l_size = l_MemorySize;
		size_t total_out=0;

		while(total_out < l_MemorySize)
		{

			while(!(l_buffer_id = rtme_output_AllocFrame_Flags(m_RTMEOutput, m_OutputFormatFlags)))
			{
				Sleep(5);
			}

			aud_chunk= rtme_output_GetFrameAudioBuffer(m_RTMEOutput, l_buffer_id, &l_size);
		
			memcpy(aud_chunk+1, &((unsigned char *)l_pData)[total_out], std::min((size_t)aud_chunk->ac_num_bytes, (size_t)l_MemorySize-total_out));
	
			total_out +=aud_chunk->ac_num_bytes;
				
			rtme_output_SendFrame(m_RTMEOutput, l_buffer_id, l_size);
		}*/

	}
	
}

void SD_Output_Internal::audio_input_stream_flush_all_images( void )
{
}

//static 
SD_Output* SD_Output::ClassFactory(const ChannelOutputEnum OutputTo, const wchar_t ThisAppName[])
{
	SD_Output *pRetVal = new SD_Output(OutputTo, ThisAppName);
	pRetVal->startup_communication_channel();
	return pRetVal;
}


SD_Output::SD_Output(const ChannelOutputEnum OutputTo,const wchar_t ThisAppName[])
: Output_Base(ThisAppName), m_Internal(NULL)
{
	m_Internal.reset(new SD_Output_Internal(this, OutputTo, ThisAppName));
}

SD_Output::~SD_Output()
{
	this->shutdown_communication_channel();	// close communication channel first b/c I don't want to receive any more media
	m_Internal.reset();						// MUST MUST detroy the internal obj before closing resources
	CloseResources();						// CloseResources destroys my input streams.
}

void SD_Output::receive_image_from_input(const FrameWork::Bitmaps::bitmap_ycbcr_u8 &image, const video_input_stream_ycbcr::image_info &frame_info)
{
	if (m_Internal)
		m_Internal->receive_image_from_input(image,frame_info);
	//This is no longer called here since it will be called when the send frame callback has confirmed its been flushed
	//__super::receive_image_from_input(image,frame_info);
}
void SD_Output::flush_all_images( void )
{
	if (m_Internal)
		m_Internal->flush_all_images();
}

void SD_Output::audio_input_stream_receive_buffer_from_input(const void *buffer, const audio_input_stream_buffer_info &buffer_info)
{
	if (m_Internal)
		m_Internal->audio_input_stream_receive_buffer_from_input(buffer,buffer_info);
	__super::audio_input_stream_receive_buffer_from_input(buffer,buffer_info);
}

void SD_Output::audio_input_stream_flush_all_images( void )
{
	if (m_Internal)
		m_Internal->audio_input_stream_flush_all_images();
}
