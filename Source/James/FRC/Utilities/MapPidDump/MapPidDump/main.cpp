//Just using basic stuff here... don't need the deprecated warnings
#define  _CRT_SECURE_NO_WARNINGS

#include <fcntl.h>			//Needed only for _O_RDWR definition
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include <xutility>

using namespace std;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;

//Got some win32 stuff here... not going to include windows.h though
//I've gotta pack it since the bfSize must precede immediately after the bfType

#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif // WIN32

typedef struct tagBITMAPFILEHEADER { 
	WORD    bfType; 
	DWORD   bfSize; 
	WORD    bfReserved1; 
	WORD    bfReserved2; 
	DWORD   bfOffBits; 
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
	DWORD  biSize; 
	LONG   biWidth; 
	LONG   biHeight; 
	WORD   biPlanes; 
	WORD   biBitCount; 
	DWORD  biCompression; 
	DWORD  biSizeImage; 
	LONG   biXPelsPerMeter; 
	LONG   biYPelsPerMeter; 
	DWORD  biClrUsed; 
	DWORD  biClrImportant; 
} BITMAPINFOHEADER, *PBITMAPINFOHEADER; 

#if (defined(_WIN32) || defined(__WIN32__))
#pragma pack(pop)
#else
#pragma pack()
#endif // WIN32


#define MakeID(a,b) ((b)<<8|(a))

struct pixel_bgr_u8		{ typedef unsigned char value_type;		value_type m_b, m_g, m_r; };
const pixel_bgr_u8 Pixel_Color[]=
{
	{0x10,0xf0,0x10}, //Green
	{0xf0,0x10,0xf0}, //Magenta
	{0xf0,0xf0,0x10}, //Cyan
	{0x10,0xf0,0xf0}, //Yellow
	{0xf0,0xf0,0xf0}, //white
};

const size_t c_X_Resolution=720;
const size_t c_Y_Resolution=540;

//Not as fancy as Andrew's but gets the job done
class Bitmap
{
	public:
		Bitmap(size_t XRes, size_t YRes) : m_XRes(XRes),m_YRes(YRes),m_pData(NULL)
		{
			m_stride_in_bytes = m_XRes * 3; //nothing fancy for this

			size_t BitmapSize=YRes*m_stride_in_bytes;
			m_pData=new pixel_bgr_u8[XRes*YRes];
			memset(m_pData,0,BitmapSize);  //fill with black
		}
		~Bitmap()
		{
			if (m_pData)
			{
				delete [] m_pData;
				m_pData=NULL;
			}
		}

		// Get the resolution
		int xres( void ) const
		{	return m_XRes;
		}

		int yres( void ) const
		{	return m_YRes;
		}

		// Get pixel locations
		pixel_bgr_u8 &operator() ( size_t x, size_t y )
		{
			assert( ( x>=0 ) && ( x<m_XRes ) && ( y>=0 ) && ( y<m_YRes ) );
			assert( m_pData );

			return ( (pixel_bgr_u8*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x ];
		}
		pixel_bgr_u8 &operator() ( size_t x, size_t y ) const
		{
			assert( ( x>=0 ) && ( x<m_XRes ) && ( y>=0 ) && ( y<m_YRes ) );
			assert( m_pData );

			return ( (pixel_bgr_u8*)( (BYTE*)m_pData + y*m_stride_in_bytes ) )[ x ];

		}
		// Get the image pointers
		pixel_bgr_u8 *operator()()
		{
			return m_pData;
		}

		const pixel_bgr_u8 *operator()() const
		{
			return m_pData;
		}

		const size_t size( void ) const
		{	return m_XRes*m_YRes*sizeof(pixel_bgr_u8);
		}
	private:
		size_t m_XRes,m_YRes, m_stride_in_bytes;
		pixel_bgr_u8 *m_pData;
};

//This obtains the value assuming the start point is a number and the end point is a space
double MapPidDump_GetValue(const char *source)
{
	double ret=0.0;
	char Buffer[8];
	//copy it all... then we'll chop off the space
	memcpy(Buffer,source,8);
	char *Terminator=strchr(Buffer,' ');
	if (!Terminator) //or linefeed terminated
		Terminator=strchr(Buffer,'\n');
	assert(Terminator);  //may need to give back error
	Terminator[0]=0; //now we have a string
	ret=atof(Buffer);
	return ret;
}

//The main class that puts the pieces together
class MapPidDump
{
	public:
		//Dest can be null for practice run
		MapPidDump(const char Source[],double Velocity_Scaler=1.0/400.0,double CS_Scaler=1.0/3.0) : 
		  m_Bitmap(c_X_Resolution,c_Y_Resolution),m_Velocity_Scaler(Velocity_Scaler),m_CS_Scaler(CS_Scaler),
		  m_ColumnIndex(0),m_SourceFileHandle(-1),m_DestFileHandle(-1)
		{
			m_SourceFileHandle=_open(Source, _O_RDONLY );
			m_Error= (m_SourceFileHandle==-1);
			if (m_Error)
				printf("source %s not found \n",Source);
			for (size_t i=0;i<eNoColumns;i++)
			{
				m_Amplitude[i]=1.0;
				m_Offset[i]=0.0;
			}
			m_Offset[eCalibratedScaler]=-CS_Scaler;
		}

		~MapPidDump()
		{
			if (m_SourceFileHandle!=-1)
			{
				_close(m_SourceFileHandle);
				m_SourceFileHandle=-1;
			}
			if (m_DestFileHandle!=-1)
			{
				_close(m_DestFileHandle);
				m_DestFileHandle=-1;
			}
		}

		bool IsError() const {return m_Error;}

		void operator()(const char *Dest=NULL)
		{
			const size_t ChunkSize=1024;
			char buffer[ChunkSize];
			int result;
			size_t count;
			size_t splice_offset=0;
			do 
			{
				result=_read(m_SourceFileHandle,buffer+splice_offset,ChunkSize-24);  //make size a bit less to add the termination
				result+=splice_offset; //absorb the splice now
				assert(result<ChunkSize);
				if (result>0)
				{
					//make the source a string by terminating it
					buffer[result]=0;  //note result is cardinal so using as ordinal puts it one past the last position
					count=Process(buffer,result);
					if (count==0)
						break;  //break if not successful
					splice_offset=result-count;
					//save this for next time
					if (splice_offset>0)
						memcpy(buffer,buffer+(count-1),splice_offset);
				}
			}	while (result>0);
			WriteBitmap(Dest); //checks null implicitly
		}
	private:
		void WriteBitmap(const char *Dest)
		{
			if (Dest)
			{
				m_DestFileHandle=_open(Dest,_O_WRONLY|_O_CREAT,_S_IWRITE);
				if (m_DestFileHandle!=-1)
				{
					size_t HeaderSize=sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

					//First the bitmap file header
					BITMAPFILEHEADER fileheader;
					memset(&fileheader,0,sizeof(BITMAPFILEHEADER));
					fileheader.bfType=MakeID('B','M');  //Specifies the file type, must be BM. 
					fileheader.bfSize=m_Bitmap.size() + HeaderSize;
					fileheader.bfOffBits=HeaderSize;
					_write(m_DestFileHandle,&fileheader,sizeof(BITMAPFILEHEADER));

					//Now the bitmap info header
					BITMAPINFOHEADER infoheader;
					memset(&infoheader,0,sizeof(BITMAPINFOHEADER));
					infoheader.biSize=sizeof(BITMAPINFOHEADER); //Specifies the number of bytes required by the structure. 
					infoheader.biWidth=m_Bitmap.xres();
					//Specifies the height of the bitmap, in pixels. If biHeight is positive, the bitmap is a bottom-up DIB and  its origin is the 
					//lower-left corner. If biHeight is negative, the bitmap is a top-down DIB and its origin is the upper-left corner. 
					infoheader.biHeight=m_Bitmap.yres();
					infoheader.biPlanes=1; //Specifies the number of planes for the target device. This value must be set to 1. 
					infoheader.biBitCount=24;  //good ole 24bit rgb no alpha
					infoheader.biCompression=0;  //0 is BI_RGB uncompressed
					//Thats enough stuff the rest can be zero's
					_write(m_DestFileHandle,&infoheader,sizeof(BITMAPINFOHEADER));
					//Now to just dump the whole bitmap
					_write(m_DestFileHandle,m_Bitmap(),m_Bitmap.size());

					//all done... close
					_close(m_DestFileHandle);
					m_DestFileHandle=-1;
				}
			}
		}
		enum ColumnItems
		{
			eVoltage, ePredictedVelocity, eEncoderVelocity, eCalibratedScaler, eNoColumns
		};

		bool ProcessColumn(const double *Elements)
		{
			const int XRes=c_X_Resolution;
			const int YRes=c_Y_Resolution;
			const int HalfYRes=YRes>>1;

			for (size_t i=0;i<eNoColumns;i++)
			{
				//simply plot each point
				double NormalizedValue=Elements[i];
				//conduct the normalizing
				switch (i)
				{
					case ePredictedVelocity:
					case eEncoderVelocity:
						NormalizedValue*=m_Velocity_Scaler;
						break;
					case eCalibratedScaler:
						NormalizedValue*=m_CS_Scaler;
						break;
				}

				double Sample=max(min((NormalizedValue*m_Amplitude[i])+m_Offset[i],1.0),-1.0);
				//With the samples now get the y positions
				int YPos=(int)((float)(HalfYRes-1)*Sample)+HalfYRes;

				//Now with x and y positions computed fill in the pixel
				pixel_bgr_u8 &pixel=m_Bitmap(m_ColumnIndex,YPos);
				pixel=Pixel_Color[i];  
			}
			m_ColumnIndex++;
			return (m_ColumnIndex<XRes);
		}
		//All processing here, returns number of bytes processed... client will manage merging remainder for next iteration
		//if returns 0... then we cannot process any more and client will need to stop calling
		size_t Process(const char * Source,size_t size)
		{
			const char *EqualPointer=Source;
			double Elements[eNoColumns];
			const char *EndEqualPtr=strrchr(Source,'=');
			bool TestColumn,Error=false;
			//Traverse to each = sign; since it is now in a buffer we can navigate around freely
			while (EqualPointer=strchr(EqualPointer+1,'='))
			{
				//Save the last find for next time as this avoid splice management of the numbers
				if (EqualPointer==EndEqualPtr)
					break;

				#if 0
				//ensure we have a terminator
				const char *Terminator=strchr(EqualPointer,' ');
				if (!Terminator) //or linefeed terminated
					Terminator=strchr(EqualPointer,'\n');
				if (!Terminator)
					break;
				#endif

				double value=MapPidDump_GetValue(EqualPointer+1);
				char Command=EqualPointer[-1];  //looks crazy but will address the letter before the equal sign
				switch (Command)
				{
				case 'v':
					Elements[eVoltage]=value;
					break;
				case 'p':
					Elements[ePredictedVelocity]=value;
					break;
				case 'e':
					Elements[eEncoderVelocity]=value;
					break;
				case 's':
					Elements[eCalibratedScaler]=value;
					TestColumn=ProcessColumn(Elements);
					if (!TestColumn)
						Error=true;
					break;
				}
				if (Error)
					break;
			}
			size_t ret= EqualPointer?((size_t)(EqualPointer-Source))-2:size;
			return ret;
		}
		Bitmap m_Bitmap;
		const double m_Velocity_Scaler,m_CS_Scaler;  //used to normalize the data
		double m_Amplitude[eNoColumns],m_Offset[eNoColumns];  //further tweaking of graphs
		size_t m_ColumnIndex;
		int m_SourceFileHandle,m_DestFileHandle;
		bool m_Error;
};

void main(int argc,const char **argv)
{
	if (argc < 2)
		printf("usage: MapPidDump <source> <dest> \n");
	else
	{
		MapPidDump instance(argv[1]); //instantiate with source
		//Check for error
		if (!instance.IsError())
			instance(argc==3?argv[2]:NULL);  //perform the operation with optional file to write
	}
}
