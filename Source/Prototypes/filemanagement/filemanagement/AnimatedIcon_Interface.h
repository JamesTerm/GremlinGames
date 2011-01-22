class AnimatedIcon_Interface;

#ifndef __AnimatedIcon_Interface_100400_
#define __AnimatedIcon_Interface_100400_

class FileManagementDLL AnimatedIcon_Interface : public ScreenObject
{
public:
	virtual bool AnimatedIcon_CreateAnimatedIcon(char* filename) = 0;
	virtual bool AnimatedIcon_StartAnimation(double startTime = 0.0, double endTime = -1.0) = 0;
	virtual bool AnimatedIcon_StopAnimation() = 0;
	virtual void AnimatedIcon_RenderAtTime(double time) = 0;
	virtual bool AnimatedIcon_OnTimer() = 0;
};



class FileManagementDLL CustomIcon_Interface : public ScreenObject {
	public:
	virtual bool CustomIcon_CreateBitmap(FileProperties* fileprops,WindowBitmap* windowbmp)=0;
	};

#endif	// #ifndef __AnimatedIcon_Interface_100400_
