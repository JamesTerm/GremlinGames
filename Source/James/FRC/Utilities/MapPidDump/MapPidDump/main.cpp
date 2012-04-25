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
	{0x80,0x80,0x80}, //Gray
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
	char Buffer[16];
	//copy it all... then we'll chop off the space
	memcpy(Buffer,source,16);
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
		//Note the CS Scaler is inverted as this is more readable
		MapPidDump(const char Source[],double Velocity_Scaler,double CS_Scaler,double YPos_Scaler,bool UseEncoderOffset) : 
		  m_Bitmap(c_X_Resolution,c_Y_Resolution),m_Velocity_Scaler(Velocity_Scaler),m_CS_Scaler(-CS_Scaler),m_YPos_Scaler(YPos_Scaler),
		  m_ColumnIndex(0),m_SourceFileHandle(-1),m_DestFileHandle(-1),m_UseEncoderOffset(UseEncoderOffset)
		{
			m_SourceFileHandle=_open(Source, _O_RDONLY );
			m_Error= (m_SourceFileHandle==-1);
			if (m_Error)
				printf("source %s not found \n",Source);
			for (size_t i=0;i<eNoItemsToGraph;i++)
			{
				m_Amplitude[i]=1.0;
				m_Offset[i]=0.0;
				m_ElementsColumn[i]=0.0;
			}
			//This (for now) toggles between the new and old way to represent the cs
			if (CS_Scaler==1.0/3.0)
				m_Offset[eCalibratedScaler]=CS_Scaler;  //Note: this is positive since m_CS_Scaler is inverted
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
			AddMarkers();
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
						memcpy(buffer,buffer+count,splice_offset);
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
			eVoltage, ePredictedVelocity, eEncoderVelocity, eCalibratedScaler, eYPos, eNoItemsToGraph
		};

		int GetYPos(double y)
		{
			const int YRes=m_Bitmap.yres();
			const int HalfYRes=YRes>>1;
			return (int)((float)(HalfYRes-1)*y)+HalfYRes;
		}

		void AddMarkers()
		{

			enum Markers
			{
				eMarker_Top,
				eMarker_Top_75,
				eMarker_Top_50,
				eMarker_Top_25,
				eMarker_0,
				eMarker_Bottom_25,
				eMarker_Bottom_50,
				eMarker_Bottom_75,
				eMarker_Bottom,
				eMarker_NoItems
			};
			int Position[eMarker_NoItems];
			Position[eMarker_Top]=GetYPos(1.0);			
			Position[eMarker_Top_75]=GetYPos(0.75);			
			Position[eMarker_Top_50]=GetYPos(0.5);			
			Position[eMarker_Top_25]=GetYPos(0.25);			
			Position[eMarker_0]=GetYPos(0.0);			
			Position[eMarker_Bottom_25]=GetYPos(-0.25);			
			Position[eMarker_Bottom_50]=GetYPos(-0.50);			
			Position[eMarker_Bottom_75]=GetYPos(-0.75);			
			Position[eMarker_Bottom]=GetYPos(-1.0);			
			for (size_t x=0;x<5;x++)
			{
				for (size_t i=0;i<eMarker_NoItems;i++)
				{
					pixel_bgr_u8 &pixel=m_Bitmap(x,Position[i]);
					pixel=Pixel_Color[5];  
				}
			}
			for (size_t x=5;x<10;x++)
			{
				for (size_t i=0;i<eMarker_NoItems;i+=2)
				{
					pixel_bgr_u8 &pixel=m_Bitmap(x,Position[i]);
					pixel=Pixel_Color[5];  
				}
			}
		}
		bool ProcessColumn(const double *Elements)
		{
			const int XRes=m_Bitmap.xres();
			const int YRes=m_Bitmap.yres();
			const int HalfYRes=YRes>>1;

			for (size_t i=0;i<eNoItemsToGraph;i++)
			{
				//simply plot each point
				double NormalizedValue=Elements[i];
				//conduct the normalizing
				switch (i)
				{
					case eYPos:
						NormalizedValue*=m_YPos_Scaler;
						break;
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
			return (m_ColumnIndex<(size_t)XRes);
		}
		//All processing here, returns number of bytes processed... client will manage merging remainder for next iteration
		//if returns 0... then we cannot process any more and client will need to stop calling
		size_t Process(const char * Source,size_t size)
		{
			const char *EqualPointer=Source;
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
				case 'y':
					m_ElementsColumn[eYPos]=value;
					break;
				case 'v':
					m_ElementsColumn[eVoltage]=value;
					break;
				case 'p':
					m_ElementsColumn[ePredictedVelocity]=value;
					break;
				case 'e':
					m_ElementsColumn[eEncoderVelocity]=value;
					break;
				case 's':
				case 'o':
					if	(((!m_UseEncoderOffset)&&(Command=='s')) ||
						((m_UseEncoderOffset)&&(Command=='o')))
					m_ElementsColumn[eCalibratedScaler]=value;
					TestColumn=ProcessColumn(m_ElementsColumn);
					if (!TestColumn)
						Error=true;
					break;
				}
				if (Error)
					break;
			}
			if (EqualPointer)
			{
				//Ensure we get the whole command
				while((*EqualPointer!=' ')&&(*EqualPointer!='\n'))
					EqualPointer--;
				EqualPointer++;
			}
			size_t ret= EqualPointer?((size_t)(EqualPointer-Source)):size;
			return ret;
		}
		Bitmap m_Bitmap;
		const double m_Velocity_Scaler,m_CS_Scaler;  //used to normalize the data
		const double m_YPos_Scaler;
		double m_Amplitude[eNoItemsToGraph],m_Offset[eNoItemsToGraph];  //further tweaking of graphs
		double m_ElementsColumn[eNoItemsToGraph];
		size_t m_ColumnIndex;
		int m_SourceFileHandle,m_DestFileHandle;
		bool m_UseEncoderOffset;
		bool m_Error;
};

void main(int argc,const char **argv)
{
	if (argc < 2)
	{
		printf("usage: MapPidDump <source> <dest> [Velocity Scaler] [CS Scaler] [Y Scaler] [Use Encoder Offset=false]\n");
		printf("Defaults: Velocity=400, CS=3, Y=5\n");
	}
	else
	{
		double Velocity_Scaler=argc>3?(1.0/atof(argv[3])):1.0/400.0;
		double CS_Scaler=argc>4?(1.0/atof(argv[4])):1.0/3.0;
		double YPos_Scaler=argc>5?(1.0/atof(argv[5])):1.0/5.0;
		bool UseEncoderOffset=argc>6? (atoi(argv[6])==0?false:true) : false;
		MapPidDump instance(argv[1],Velocity_Scaler,CS_Scaler,YPos_Scaler,UseEncoderOffset); //instantiate with source
		//Check for error
		if (!instance.IsError())
			instance(argc>2?argv[2]:NULL);  //perform the operation with optional file to write
	}
}
