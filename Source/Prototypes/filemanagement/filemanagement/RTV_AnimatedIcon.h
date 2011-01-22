class RTV_AnimatedIcon;

#ifndef __RTV_AnimatedIcon_100400_
#define __RTV_AnimatedIcon_100400_

class FileManagementDLL RTV_AnimatedIcon : public AnimatedIcon_Interface
{
protected:
	unsigned		m_PlaybackFrameNumber;
	unsigned		m_XRes,m_YRes;
	WindowBitmap	*m_AnimationBitmap;
	byte			*m_AnimationMemory;
	char*			m_fileName;

	unsigned		m_startFrame;
	unsigned		m_endFrame;
	unsigned		m_maxFrame;
	unsigned		m_minAcceptableFrames;
	double			m_frameRate;
	double			m_maxTime;
	long			m_startTickCount;

	double			m_loopStart, m_loopEnd;

	unsigned FindFrame(double time);
	bool RenderImage(unsigned frame);

public:
	RTV_AnimatedIcon();
	~RTV_AnimatedIcon();

	bool ScreenObject_RAW_DrawItem(RECT *rect,HDC hdc,RECT *ClippingRgn=NULL);

	virtual long ScreenObject_GetPreferedXSize(void){return -1;}
	virtual long ScreenObject_GetPreferedYSize(void){return -1;}

	bool AnimatedIcon_CreateAnimatedIcon(char* filename);
	bool AnimatedIcon_StartAnimation(double startTime = 0.0, double endTime = -1.0);
	bool AnimatedIcon_StopAnimation();
	void AnimatedIcon_RenderAtTime(double time){RenderImage(FindFrame(time));}
	bool AnimatedIcon_OnTimer();
};

#endif	// #ifndef __RTV_AnimatedIcon_100400_