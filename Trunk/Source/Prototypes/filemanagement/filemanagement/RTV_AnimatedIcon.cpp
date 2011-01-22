#include "stdafx.h"

/*
class FileManagementDLL RTV_AnimatedIcon : public AnimatedIcon_Interface
{
protected:
	unsigned		m_PlaybackFrameNumber;
	unsigned		m_FrameIncrement;
	unsigned		m_XRes,m_YRes;
	WindowBitmap	*m_AnimationBitmap;
	byte			*m_AnimationMemory;
	char*			m_fileName;

public:
*/
RTV_AnimatedIcon::RTV_AnimatedIcon()
{
	m_PlaybackFrameNumber = 0;
	m_XRes = m_YRes = 0;
	m_AnimationBitmap = NULL;
	m_AnimationMemory = NULL;
	m_fileName = NULL;
	m_minAcceptableFrames = 2;

	m_startFrame = 0;
	m_endFrame = 0xfffffff;
	m_frameRate = 29.97;
	m_startTickCount;
	m_maxFrame = 0;
	m_maxTime = 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
RTV_AnimatedIcon::~RTV_AnimatedIcon()
{
	if (m_AnimationBitmap) 
	{	delete m_AnimationBitmap;
		m_AnimationBitmap=NULL;
	}

	if (m_AnimationMemory) 
	{	CustomMemory_aligned_free(m_AnimationMemory);
		m_AnimationMemory=NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool RTV_AnimatedIcon::ScreenObject_RAW_DrawItem(RECT *rect,HDC hdc,RECT *ClippingRgn)
{
	if (m_AnimationBitmap)
	{	
		// Draw the bitmap
		m_AnimationBitmap->WindowBitmap_AlphaBlend(hdc,*rect,BitmapTile_StretchX|BitmapTile_StretchY);
		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool RTV_AnimatedIcon::AnimatedIcon_CreateAnimatedIcon(char* filename)
{
	if ((IsRTV(filename)!=RTVLIB_UNKNOWN)&&
		(ReadRTVNumberOfFrames(filename) > m_minAcceptableFrames))
	{	
		bool		VideoChannel,IsFielded,AlphaChannel,AudioChannel;
		unsigned	SampleRate;
									
		// Get information about the file
		if (GetRTVProperties(filename,VideoChannel,m_XRes,m_YRes,m_frameRate,IsFielded,AlphaChannel,AudioChannel,SampleRate))
		{
			m_frameRate /= 2.0; // fields versus frames
			if (VideoChannel)
			{
				// Create the image at the right size here
				if (!m_AnimationBitmap) m_AnimationBitmap=new WindowBitmap(m_XRes,m_YRes);
				else m_AnimationBitmap->Resize(m_XRes, m_YRes);

				// Create the memory buffer
				if (m_AnimationMemory) 
				{	CustomMemory_aligned_free(m_AnimationMemory);
					m_AnimationMemory=NULL;
				}
				m_AnimationMemory=(byte*)CustomMemory_aligned_malloc(m_XRes*m_YRes*2);
				m_fileName = filename;

				// Set up all of the looping variables
				m_maxFrame = ReadRTVNumberOfFrames(m_fileName);
				if (m_frameRate <= 0.0) m_frameRate = 15.0;
				m_maxTime = m_maxFrame / m_frameRate;

				return true;
			}
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned RTV_AnimatedIcon::FindFrame(double time)
{
	if (time < 0.0) time = m_maxTime;
	unsigned r = (unsigned)(m_frameRate * time);
	if (r >= m_maxFrame) r = m_maxFrame-1;
	return r;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool RTV_AnimatedIcon::AnimatedIcon_StartAnimation(double startTime, double endTime)
{
	if (m_AnimationBitmap && m_AnimationMemory && m_fileName)
	{
		m_startFrame = FindFrame(startTime);
		m_endFrame = FindFrame(endTime);

		// The current frame number
		if (m_startFrame < m_endFrame)
			m_PlaybackFrameNumber=m_startFrame;
		else m_PlaybackFrameNumber=m_endFrame;

		// Find the current Time
		m_startTickCount = GetTickCount();
		RenderImage(m_PlaybackFrameNumber);

		// Notify of the Change
		Changed();

		return true;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool RTV_AnimatedIcon::AnimatedIcon_StopAnimation()
{
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool RTV_AnimatedIcon::AnimatedIcon_OnTimer()
{
	long elapsedTime = GetTickCount() - m_startTickCount;
	int frameIncrement = 2;// (m_frameRate / 1000.0) * elapsedTime;

	if (frameIncrement)
	{
		if (m_startFrame > m_endFrame)
		{
			if (frameIncrement > m_PlaybackFrameNumber)
				m_PlaybackFrameNumber = 0;
			else
			{
				frameIncrement *= -1;
				m_PlaybackFrameNumber += frameIncrement;
			}
			if (m_PlaybackFrameNumber < m_endFrame) m_PlaybackFrameNumber = m_startFrame;
		}
		else
		{
			m_PlaybackFrameNumber += frameIncrement;
			if (m_PlaybackFrameNumber > m_endFrame) m_PlaybackFrameNumber = m_startFrame;
		}
		bool ret = RenderImage(m_PlaybackFrameNumber);
		Changed();
		return ret;
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool RTV_AnimatedIcon::RenderImage(unsigned frame)
{
	if (frame >= m_maxFrame) frame = m_maxFrame-1;
	if (m_fileName && NewTek_DriveBlock(m_fileName,100) && m_AnimationBitmap && m_AnimationMemory)
	{	
		// Read the RTV frame
		if (ReadRTVFile(m_fileName,frame,m_AnimationMemory,NULL,NULL,NULL,NULL,false))
		{	// Unblock the drive
			NewTek_DriveUnBlock(m_fileName);
			
			byte *tField0=m_AnimationMemory;
			for(unsigned y=0;y<m_YRes;y++)
			{	decode_ycbcr8_bgra((byte*)m_AnimationBitmap->GetPixel_32BPP(0,y),tField0,m_XRes); 
				tField0+=m_XRes*2;	
			}
			return true;
		}
		else
		{	// Unblock the drive
			NewTek_DriveUnBlock(m_fileName);
		}
	}
	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////