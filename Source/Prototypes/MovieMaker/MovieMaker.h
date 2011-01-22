#ifndef _MOVIEMAKER_H
#define _MOVIEMAKER_H

//  ===============================================
//  MovieMaker class definition.
//  ===============================================

//-------------------------------------------
//Link with: gdi32.lib vfw32.lib opengl32.lib
//-------------------------------------------

#include <windows.h>
#include <vfw.h>

#define TEXT_HEIGHT	20
#define AVIIF_KEYFRAME	0x00000010L // this frame is a key frame.
#define BUFSIZE 260

class MovieMaker {
private:
    //CString FName;
    char fname[64];
    int width;
    int height;
	int ulX, ulY;

  	AVISTREAMINFO strhdr;
	PAVIFILE pfile;
	PAVISTREAM ps;
	PAVISTREAM psCompressed;
	PAVISTREAM psText;
	AVICOMPRESSOPTIONS opts;
	AVICOMPRESSOPTIONS FAR * aopts[1];
	DWORD dwTextFormat;
	char szText[BUFSIZE];
	int nFrames;
	bool bOK;


public:
    MovieMaker();
    ~MovieMaker();

    inline bool IsOK() const { return bOK; };
    void StartCapture(const char *name, int x, int y, int w, int h);
    void EndCapture();
    bool Snap();
};

#endif
