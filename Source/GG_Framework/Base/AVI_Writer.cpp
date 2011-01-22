// GG_Framework.Base AVI_Writer.cpp
#include "stdafx.h"
#include "GG_Framework.Base.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <windowsx.h>

using namespace GG_Framework::Base;
#define AVIIF_KEYFRAME	0x00000010L // this frame is a key frame.

class AVI_Writer : public I_AVI_Writer
{
public:
	AVI_Writer();
	~AVI_Writer();
	
	bool Initialize(const char* filename, unsigned width, unsigned height);	//!< Asks for the AVI compression format, returns false for a problem
	bool WriteImage(I_AVI_Image* Iimage); //!< Make sure the image size is the same as that passed in to the c'tor

private:
	PAVIFILE pfile;
	PAVISTREAM ps;
	PAVISTREAM psCompressed;
	AVICOMPRESSOPTIONS opts;
	bool ready;
	int nFrames;
};

static HANDLE  MakeDib( HBITMAP hbitmap, UINT bits )
{
	HANDLE              hdib ;
	HDC                 hdc ;
	BITMAP              bitmap ;
	UINT                wLineLen ;
	DWORD               dwSize ;
	DWORD               wColSize ;
	LPBITMAPINFOHEADER  lpbi ;
	LPBYTE              lpBits ;
	
	GetObject(hbitmap,sizeof(BITMAP),&bitmap);

	//
	// DWORD align the width of the DIB
	// Figure out the size of the colour table
	// Calculate the size of the DIB
	//
	wLineLen = (bitmap.bmWidth*bits+31)/32 * 4;
	wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);
	dwSize = sizeof(BITMAPINFOHEADER) + wColSize +
		(DWORD)(UINT)wLineLen*(DWORD)(UINT)bitmap.bmHeight;

	//
	// Allocate room for a DIB and set the LPBI fields
	//
	hdib = GlobalAlloc(GHND,dwSize);
	if (!hdib)
		return hdib ;

	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hdib) ;

	lpbi->biSize = sizeof(BITMAPINFOHEADER) ;
	lpbi->biWidth = bitmap.bmWidth ;
	lpbi->biHeight = bitmap.bmHeight ;
	lpbi->biPlanes = 1 ;
	lpbi->biBitCount = (WORD) bits ;
	lpbi->biCompression = BI_RGB ;
	lpbi->biSizeImage = dwSize - sizeof(BITMAPINFOHEADER) - wColSize ;
	lpbi->biXPelsPerMeter = 0 ;
	lpbi->biYPelsPerMeter = 0 ;
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;
	lpbi->biClrImportant = 0 ;

	//
	// Get the bits from the bitmap and stuff them after the LPBI
	//
	lpBits = (LPBYTE)(lpbi+1)+wColSize ;

	hdc = CreateCompatibleDC(NULL) ;

	GetDIBits(hdc,hbitmap,0,bitmap.bmHeight,lpBits,(LPBITMAPINFO)lpbi, DIB_RGB_COLORS);

	// Fix this if GetDIBits messed it up....
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;

	DeleteDC(hdc) ;
	GlobalUnlock(hdib);

	return hdib ;
}

class AVI_Image : public I_AVI_Image
{
public:
	LPBITMAPINFOHEADER alpbi;
	virtual ~AVI_Image()
	{
		if (alpbi)
			GlobalFreePtr(alpbi);
	}

	virtual bool CaptureScreen( int xsrc, int ysrc, int w, int h )
	{
		// Create a normal DC and a memory DC for the entire screen. The 
		// normal DC provides a "snapshot" of the screen contents. The 
		// memory DC keeps a copy of this "snapshot" in the associated 
		// bitmap. 
	 
		HDC hdcScreen = wglGetCurrentDC();
		HDC hdcCompatible = CreateCompatibleDC(hdcScreen); 
	 
		// Create a compatible bitmap for hdcScreen. 

		HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, 
							   //  GetDeviceCaps(hdcScreen, HORZRES), 
							   //  GetDeviceCaps(hdcScreen, VERTRES)); 
								 w, 
								 h ); 

		if (hbmScreen == 0)
			{
			printf( "hbmScreen == NULL\n" );
			return false;
			}
	 
		// Select the bitmaps into the compatible DC. 
	 
		if (!SelectObject(hdcCompatible, hbmScreen)) 
			{
			printf( "Couldn't SelectObject()\n" );
			return false;
			}
	 
		if (!BitBlt(hdcCompatible, 
					 0,0, 
					 w, h, 
					 hdcScreen, 
					// 512,512,
					 xsrc, ysrc,
					 SRCCOPY)) 
			{
				printf("Screen to Compat Blt Failed\n" );
				return false;
			}

		DeleteDC( hdcCompatible );

		alpbi = (LPBITMAPINFOHEADER)GlobalLock(MakeDib(hbmScreen, 32));
		DeleteObject( hbmScreen );

		if (alpbi == NULL)
		{
			printf("Convert To Bitmap Header Failed\n" );
			return false;
		}

		// All is well 
		return true;
	}
};

I_AVI_Writer* I_AVI_Writer::CreateWriter(const char* filename, unsigned width, unsigned height)
{
	AVI_Writer* ret = new AVI_Writer;
	if (!ret->Initialize(filename, width, height))
	{
		delete ret;
		ret = NULL;
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////

void I_AVI_Image::DestroyImage(I_AVI_Image* image){delete image;}
I_AVI_Image* I_AVI_Image::CaptureFromOSG(int X, int Y, unsigned W, unsigned H)
{
	AVI_Image* ret = new AVI_Image;
	if (!ret->CaptureScreen(X,Y,W,H))
	{
		delete ret;
		ret = NULL;
	}
	return ret;
}
///////////////////////////////////////////////////////////////////////////////////////////////

void I_AVI_Writer::DestroyWriter(I_AVI_Writer*& writer){delete writer; writer = NULL;}

AVI_Writer::AVI_Writer() : pfile(NULL), ps(NULL), psCompressed(NULL), ready(false), nFrames(0)
{
	AVIFileInit();
}
///////////////////////////////////////////////////////////////////////////////////////////////

AVI_Writer::~AVI_Writer()
{
	if (ps)
        {
		AVIStreamClose(ps);
        ps = NULL;
        }

	if (psCompressed)
        {
		AVIStreamClose(psCompressed);
        psCompressed = NULL;
        }

	if (pfile)
        {
		AVIFileClose(pfile);
        pfile = NULL;
        }

	WORD wVer = HIWORD(VideoForWindowsVersion());
	if (wVer >= 0x010A)
	    {
		AVIFileExit();
	    }
}
///////////////////////////////////////////////////////////////////////////////////////////////

bool AVI_Writer::Initialize(const char* filename, unsigned width, unsigned height)	//!< Asks for the AVI compression format, returns false for a problem
{
	try
	{
		HRESULT hr;
		ASSERT(filename);

		// First open the file
		hr = AVIFileOpen(&pfile,		    // returned file pointer
					   filename,							// file name
					   OF_WRITE | OF_CREATE,		    // mode to open file with
					   NULL);							// use handler determined
														// from file extension....
		if (hr != AVIERR_OK)
			return false;



		// Create the standard header
		AVISTREAMINFO strhdr;
		_fmemset(&strhdr, 0, sizeof(strhdr));
			strhdr.fccType                = streamtypeVIDEO;// stream type
			strhdr.fccHandler             = 0;
			strhdr.dwScale                = 1;
			strhdr.dwRate                 = 33; //300;		    // 15 fps
			strhdr.dwSuggestedBufferSize  = width*height*4;
			SetRect(&strhdr.rcFrame, 0, 0,		    // rectangle for stream
				(int) width,
				(int) height);



		// And create the stream;
		hr = AVIFileCreateStream(pfile,		    // file pointer
								 &ps,		    // returned stream pointer
								 &strhdr);	    // stream header
		if (hr != AVIERR_OK)
			return false;



		// The AVI compressor options
		AVICOMPRESSOPTIONS FAR * aopts[1];
		aopts[0] = &opts;
		if (!AVISaveOptions(NULL, 0, 1, &ps, (LPAVICOMPRESSOPTIONS FAR *) &aopts))
			return false;


		// ready to go
		ready = true;
		return true;
	}
	catch (...)
	{
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////

bool AVI_Writer::WriteImage(I_AVI_Image* Iimage) //!< Make sure the image size is the same as that passed in to the c'tor
{
	AVI_Image* image = (AVI_Image*)Iimage;
	HRESULT hr;

	// Make sure all went well so far
	if (!ready) return false;

	try
	{

		// If we have not created the compressed stream, do so now since we have a good idea of the format
		if (!psCompressed)
		{
			// Make the compressed stream
			hr = AVIMakeCompressedStream(&psCompressed, ps, &opts, NULL);
			if (hr != AVIERR_OK)
			{
				ready = false;
				return false;
			}

			// Set the stream's format
			hr = AVIStreamSetFormat(psCompressed, 0,
					   image->alpbi,	    // stream format
				       image->alpbi->biSize +   // format size
				       image->alpbi->biClrUsed * sizeof(RGBQUAD));
			if (hr != AVIERR_OK)
			{
				ready = false;
				return false;
			}
		}

		// Write the data directly
		hr = AVIStreamWrite(psCompressed,	// stream pointer
			nFrames * 1, // 10,				// time of this frame
			1,				// number to write
			(LPBYTE) image->alpbi +		// pointer to data
				image->alpbi->biSize +
				image->alpbi->biClrUsed * sizeof(RGBQUAD),
				image->alpbi->biSizeImage,	// size of this frame
			AVIIF_KEYFRAME,			 // flags....
			NULL,
			NULL);
		++nFrames;

		return (hr == AVIERR_OK);
	}
	catch (...)
	{
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////
