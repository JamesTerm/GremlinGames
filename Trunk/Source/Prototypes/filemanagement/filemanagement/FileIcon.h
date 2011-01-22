class FileIcon;

#ifndef __FileIconH_
#define __FileIconH_


#define UNKNOWNDRIVEICON	"File Icons\\UNKNOWNDRIVEICON.jpg"
#define REMOVABLEDRIVEICON	"File Icons\\REMOVABLEDRIVEICON.jpg"
#define FIXEDDRIVEICON		"File Icons\\FIXEDDRIVEICON.jpg"
#define NETWORKDRIVEICON	"File Icons\\NETWORKDRIVEICON.jpg"
#define CDROMDRIVEICON		"File Icons\\CDROMDRIVEICON.jpg"
#define RAMDISKDRIVEICON	"File Icons\\RAMDISKDRIVEICON.jpg"
#define MYCOMPUTERICON		"File Icons\\My Computer.jpg"
#define IMAGEFOLDERICON		"File Icons\\SubFolder.tga"
#define FILEICONSIZEX		80
#define FILEICONSIZEY		60

#define FileIconChanged_NewImageSet "FileIconChanged_NewImageSet"

/*! FileIcon
A FileIcon gets a FileProperties object and finds the appropriate Icon to use.  It is a ScreenObject, so it can draw
that icon when it needs to.  If the icon changes in any way, it can call Changed();
  */

class DirectoryLayout;
class FileButton;
class FileManagementDLL FileIcon : public ScreenObject, public DynamicListener
{
	friend DirectoryLayout;
	friend FileButton;
private:
	static tList<AnimatedIcon_Interface *> *s_AnimatedIconInterface;
	static tList<CustomIcon_Interface *> *s_CustomIconInterface;

	static void CreateAnimatedIconInterface();
	static void CreateCustomIconInterface();

	double			m_loopStart, m_loopEnd, m_defaultTime;

	// For use with NULL file properties
	WindowBitmap_Cache	m_defaultIcon;

protected:
	// From ScreenObject
	bool ScreenObject_RAW_DrawItem(RECT *rect,HDC hdc,RECT *ClippingRgn=NULL);

	WindowBitmap*			m_useBitmap;
	bool					m_doIOwn;
	bool					m_amIAnimatingNow;
	bool					m_showDefAnimFrame;

	FileProperties*					m_properties;
	WindowBitmap_Cache*				m_icon;
	AnimatedIcon_Interface*			m_animationInterface;
	CustomIcon_Interface*			m_customInterface;
	char*							m_fileName;

	char* ReadDriveIconFile(FileProperties* fileProps);
	char* ReadFolderIconFile(FileProperties* fileProps);
	char* ReadInvalidIconFile(FileProperties* fileProps);
	char* ReadSpecialIconFile(FileProperties* fileProps);
	char* ReadAutoIconFile(FileProperties* fileProps);
	char* SaveAutoImage(FileProperties* fileProps, WindowBitmap* copyMe);
	char* ReadDefaultIconFile(FileProperties* fileProps);

	// Keep track of my FileButton for deferred loading
	DirectoryLayout* m_directoryLayout;

public:			
	// Ask the bitmap item what size it really should be, 
	// These will return -1 if the size is unimportant (i.e. they are stretched or tiled)
	long ScreenObject_GetPreferedXSize(void);
	long ScreenObject_GetPreferedYSize(void);
	
	////////////////////////
	// Constructors & Destructors
	FileIcon();
	virtual ~FileIcon();

	bool FileIcon_StartAnimation();
	bool FileIcon_StopAnimation();
	bool FileIcon_OnTimer();

	void FileIcon_UseThisIcon(WindowBitmap* useThisOne, bool doIOwn = false); // Use NULL to revert to the old icon
	char* FileIcon_SetIconFile(char* imageFilename);
	char* FileIcon_SetFilename(char* filename);
	char* FileIcon_SetFileProperties(FileProperties* fileProps);

	bool FileIcon_AmIAnimated();
	bool FileIcon_ShowingAnimFrame(){return m_showDefAnimFrame;}
	bool FileIcon_ShowDefAnimFrame(bool show = true){m_showDefAnimFrame = show; Changed();return m_showDefAnimFrame;}
	bool FileIcon_AmIAnimatingNow(){return m_amIAnimatingNow;}
	char* FileIcon_GetFilename(){return m_fileName;}

	// From DynamicListener
	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	void FileIcon_SetDefaultTime(double time){m_defaultTime = time;}
	void FileIcon_SetLoopStartTime(double time){m_loopStart = time;}
	void FileIcon_SetLoopEndTime(double time){m_loopEnd = time;}

	double FileIcon_GetDefaultTime(){return m_defaultTime;}
	double FileIcon_GetLoopStartTime(){return m_loopStart;}
	double FileIcon_GetLoopEndTime(){return m_loopEnd;}

	//! Uses NewTek_New to create the proper AnimatedIcon_Interface
	static AnimatedIcon_Interface* GetAnimatedIconReader(char* fileName);
	static CustomIcon_Interface* GetCustomIconReader(FileProperties* fileprops,WindowBitmap* windowbmp);
	static void OnProcessDetach();
};

#endif  //_FileIconH_


