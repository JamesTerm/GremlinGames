// GG_Framework.Base AVI_Writer.h
#pragma once

namespace GG_Framework
{
	namespace Base
	{
		class FRAMEWORK_BASE_API I_AVI_Image
		{
		public:
			virtual ~I_AVI_Image() {}
			static I_AVI_Image* CaptureFromOSG(int X, int Y, unsigned W, unsigned H);
			static void DestroyImage(I_AVI_Image* image);
			virtual bool CaptureScreen(int xsrc, int ysrc, int w, int h) = 0;
		};


		class FRAMEWORK_BASE_API I_AVI_Writer
		{
		public:
			virtual bool WriteImage(I_AVI_Image* Iimage) = 0; //!< Make sure the image size is the same as that passed in to the c'tor

			static I_AVI_Writer* CreateWriter(const char* filename, unsigned width, unsigned height);
			static void DestroyWriter(I_AVI_Writer*& writer);
		};
	}
}