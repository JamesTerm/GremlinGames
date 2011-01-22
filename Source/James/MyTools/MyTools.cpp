#include "StdAfx.h"
#include <Imagehlp.h>

#pragma  comment (lib,"ImageHlp")

HANDLE g_WriteFile=NULL;


struct FileTimeCache
{
	FILETIME CreationTime;
	FILETIME LastAccessTime;
	FILETIME LastWriteTime;
};

FileTimeCache &GetFileTimeCache()
{
	static FileTimeCache cache;
	return cache;
}


SYSTEMTIME *MyGet_SYSTime()
{
	static SYSTEMTIME SYSTime;
	return &SYSTime;
}

void UpdateTouchTime()
{
	GetSystemTime(MyGet_SYSTime());
}

bool MySetFileTime(const char *Name,FileTimeCache &cache)
{
	HANDLE FileHandle=CreateFile(Name,GENERIC_READ|GENERIC_WRITE,
	                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
	                             NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (FileHandle==INVALID_HANDLE_VALUE)
		return false;
	bool Ret=(SetFileTime(FileHandle,&cache.CreationTime,&cache.LastAccessTime,&cache.LastWriteTime)!=0);
	CloseHandle(FileHandle);
	return Ret;
}

bool MyGetFileTime(const char *Name,FileTimeCache &cache)
{
	HANDLE FileHandle=CreateFile(Name,GENERIC_READ,
	                             FILE_SHARE_READ,
	                             NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (FileHandle==INVALID_HANDLE_VALUE)
		return false;
	bool Ret=(GetFileTime(FileHandle,&cache.CreationTime,&cache.LastAccessTime,&cache.LastWriteTime)!=0);
	CloseHandle(FileHandle);
	return Ret;
}

bool My_TouchFile(const char *Name,SYSTEMTIME *SYSTime)
{
	HANDLE FileHandle=CreateFile(Name,GENERIC_READ|GENERIC_WRITE,
	                             FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
	                             NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (FileHandle==INVALID_HANDLE_VALUE)
		return false;
	bool Ret=(TouchFileTimes(FileHandle,SYSTime)!=0);
	CloseHandle(FileHandle);
	return Ret;
}

void *NewTek_malloc(unsigned int Size)
{
	return malloc(Size);
}

void  NewTek_free(void* Ptr)
{
	free(Ptr);
}

void *NewTek_realloc(void *Ptr,unsigned int Size)
{
	return realloc(Ptr,Size);
}

char *NewTek_malloc(const char *Mem)
{
	if (!Mem)
		return NULL;
	unsigned CharLength=strlen(Mem)+1;
	char *Temp=(char*)NewTek_malloc(CharLength);
	if (!Temp)
		return NULL;
	memcpy(Temp,Mem,CharLength);
	return Temp;
}


void Printf(const char *format, ... )
{
	char Temp[4096];
	va_list marker;
	va_start(marker,format);
	vsprintf(Temp,format,marker);
	if (g_WriteFile)
		myWrite(g_WriteFile,Temp,strlen(Temp));
	else
	{
		printf(Temp);
		OutputDebugString(Temp);
	}
	va_end(marker);
}

void reverse(register char *s)
{
	long c,i,j;
	for (i=0,j=strlen(s)-1;i<j;i++,j--)
	{
		c=s[i];
		s[i]=s[j];
		s[j]=(char)c;
	}
}

long Hex2Long (char *string)
{
	long value;
	//convert string to a long value
	char *hexstring=strrchr(string,'x');
	//todo check for zero before x
	if (hexstring)
	{
		value=0;
		unsigned index=1;
		char Digit;
		while (Digit=hexstring[index++])
		{
			value=value<<4;
			Digit-='0';
			if (Digit>16)
				Digit-=7;
			if (Digit>16)
				Digit-=32;
			value+=Digit;
		}
	}
	else
		value=atol(string);
	return value;
}

__int64 Hex2Int64 (char *string)
{
	__int64 value;
	//convert string to a long value
	char *hexstring=strrchr(string,'x');
	//todo check for zero before x
	if (hexstring)
	{
		value=0;
		unsigned index=1;
		char Digit;
		while (Digit=hexstring[index++])
		{
			value=value<<4;
			Digit-='0';
			if (Digit>16)
				Digit-=7;
			if (Digit>16)
				Digit-=32;
			value+=Digit;
		}
	}
	else
		value=atol(string); //TODO may want a 64bit conversion here for
	return value;
}

void Hex2Ascii (char *string)
{
	long value=Hex2Long(string);

	//while I'm at use this to print negative integer values
	printf("Decimal value=%d\n",value);

	//convert long value to ascii
	char buffer[4];
	memset(buffer,' ',4);
	unsigned index=0;
	char asciichar;
	while ((value!=0)&&(value!=-1))
	{
		asciichar=value&0xff;
		value=value>>8;
		buffer[index++]=asciichar;
	}
	printf("%s\n",buffer);
	reverse(buffer);
	printf("%s\n",buffer);
}

void mySqrt(char *string)
{
	long value=Hex2Long(string);
	double Answer=sqrt((double)value);
	printf("Answer=%f %x\n",Answer,(long)Answer);
}

__int64 av_gcd(__int64 a, __int64 b)
{
	if(b) return av_gcd(b, a%b);
	else  return a;
}


struct AVRational
{
	int num; ///< numerator
	int den; ///< denominator
};

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))

int av_reduce(int *dst_num, int *dst_den, __int64 num, __int64 den, __int64 max)
{
	AVRational a0={0,1}, a1={1,0};
	int sign= (num<0) ^ (den<0);
	__int64 gcd= av_gcd(FFABS(num), FFABS(den));

	if(gcd){
		num = FFABS(num)/gcd;
		den = FFABS(den)/gcd;
	}
	if(num<=max && den<=max)
	{
		a1.num=num,a1.den=den;
		den=0;
	}

	while(den){
		__int64 x      = num / den;
		__int64 next_den= num - den*x;
		__int64 a2n= x*a1.num + a0.num;
		__int64 a2d= x*a1.den + a0.den;

		if(a2n > max || a2d > max){
			if(a1.num) x= (max - a0.num) / a1.num;
			if(a1.den) x= min(x, (max - a0.den) / a1.den);

			if (den*(2*x*a1.den + a0.den) > num*a1.den)
				a1.num=x*a1.num + a0.num;
				a1.den=x*a1.den + a0.den;
			break;
		}

		a0= a1;
		a1.num=a2n,a1.den=a2d;
		num= den;
		den= next_den;
	}
	assert(av_gcd(a1.num, a1.den) <= 1U);

	*dst_num = sign ? -a1.num : a1.num;
	*dst_den = a1.den;

	return den==0;
}

void ExtFileBytes(char *DestFile,char *SourceFile,char *Soffset,char *Slength)
{
	unsigned long offset=Hex2Long(Soffset);
	unsigned long length=Hex2Long(Slength);
	printf("Saving 0x%x bytes to %s...\n",length,DestFile);
	bool Success=false;

	HANDLE readfile=NULL,writefile=NULL;
	do
	{
		//Open Files
		readfile=OpenReadSeq(SourceFile);
		if (readfile==(HANDLE)-1)
		{
			readfile=NULL;
			break;
		}
		writefile=OpenWriteSeq(DestFile);
		if (writefile==(HANDLE)-1)
		{
			writefile=NULL;
			break;
		}
		//Seek to correct read position
		mySeek(readfile,offset,FILE_BEGIN);
		{//Main loop
			char Buffer[1024];
			unsigned eoi=length/1024,i;
			for (i=0;i<eoi;i++)
			{
				myRead(readfile,Buffer,1024);
				myWrite(writefile,Buffer,1024);
			}
			eoi=length%1024;
			if (eoi)
			{
				myRead(readfile,Buffer,eoi);
				myWrite(writefile,Buffer,eoi);
			}
		}
		Success=true;
	}
	while(false);

	//Close files
	if (readfile)
		myClose(readfile);
	if (writefile)
		myClose(writefile);
	if (Success)
	{
		printf("Successful!\n");
	}
	else
	{
		printf("UnSuccessful! (error Detected)\n");
	}
}

unsigned RawDataToAscii(char *Dest,char *Source,unsigned count)
{
	unsigned BytesRead=0;
	if (!count) return BytesRead;

	do 
	{
		char NewChar;
		
		while ((count)&&((NewChar=*Source++)<32))
			--count;
		if (!count) break;
		*Dest++=NewChar;
		BytesRead++;

	} while(--count);
	return BytesRead;
}

unsigned RawDataToHexByteText(char *Dest,char *Source,unsigned count)
{
	unsigned BytesRead=0;
	if (!count)
		return BytesRead;
	unsigned valuelo,valuehi;
	size_t groupcount=0;
	do
	{
		valuehi=(*Source++)&0xff;
		valuelo=valuehi&0x0f;
		valuehi>>=4;
		valuehi&=0x0f;
		*Dest++='0';
		*Dest++='x';
		*Dest++=valuehi+'0'+((valuehi>9)*7);
		*Dest++=valuelo+'0'+((valuelo>9)*7);
		*Dest++=',';
		*Dest++=32;
		//*Dest++=13+((((count-1)&0x0f)!=0)*19); //13 cr or 32 space
		BytesRead+=6;
		groupcount++;
		if (groupcount>7)
		{
			*Dest++='\t';
			for (size_t i=8;i>0;i--)
			{
				char Value=*(Source-i);
				*Dest++=(Value<32)?'.':Value;
			}
			*Dest++='\n';
			groupcount=0;
			BytesRead +=10;
		}
	}
	while (--count);

	*Dest++=13;
	BytesRead++;
	*Dest=0;
	return BytesRead;
}

void ExtFileBytestoText(char *DestFile,char *SourceFile,char *Soffset,char *Slength,bool AsciiOnly=false)
{
	unsigned long offset=Hex2Long(Soffset);
	unsigned long length=Hex2Long(Slength);
	printf("Saving 0x%x bytes to %s...\n",length,DestFile);
	bool Success=false;

	HANDLE readfile=NULL,writefile=NULL;
	do
	{
		//Open Files
		readfile=OpenReadSeq(SourceFile);
		if (readfile==(HANDLE)-1)
		{
			readfile=NULL;
			break;
		}
		writefile=OpenWriteSeq(DestFile);
		if (writefile==(HANDLE)-1)
		{
			writefile=NULL;
			break;
		}
		//Seek to correct read position
		mySeek(readfile,offset,FILE_BEGIN);
		{//Main loop
			char Buffer[1024];
			char StringLine[1024*8];
			unsigned eoi=length/1024,i;
			for (i=0;i<eoi;i++)
			{
				myRead(readfile,Buffer,1024);

				unsigned BytesRead;
				if (AsciiOnly)
					BytesRead=RawDataToAscii(StringLine,Buffer,1024);
				else
					BytesRead=RawDataToHexByteText(StringLine,Buffer,1024);

				myWrite(writefile,StringLine,BytesRead);
			}
			eoi=length%1024;
			if (eoi)
			{
				myRead(readfile,Buffer,eoi);

				unsigned BytesRead;
				if (AsciiOnly)
					BytesRead=RawDataToAscii(StringLine,Buffer,eoi);
				else
					BytesRead=RawDataToHexByteText(StringLine,Buffer,eoi);
				
				myWrite(writefile,StringLine,BytesRead);
			}
		}
		Success=true;
	}
	while(false);

	//Close files
	if (readfile)
		myClose(readfile);
	if (writefile)
		myClose(writefile);
	if (Success)
	{
		printf("Successful!\n");
	}
	else
	{
		printf("UnSuccessful! (error Detected)\n");
	}
}


//length must be 8 byte aligned....
bool CompareMemory32bit(DWORD *Memory1,DWORD *Memory2,unsigned length,__int64 offsetindex=0,bool BreakIfUnsuccessful=true)
{
	bool success=true;
	unsigned index=0;
	if (length%4)
		return false;
	while (length)
	{
		if (Memory1[index]!=Memory2[index])
		{
			Printf("0x%x offset ",index+offsetindex);
			Printf("%.8x != ",Memory1[index]);
			Printf("%.8x\n",Memory2[index]);
			if (BreakIfUnsuccessful)
				return false;
			else
				success=false;
		}
		index++;
		length-=4;
	}
	return success;
}

bool CompareMemory8bit(byte *Memory1,byte *Memory2,unsigned length,bool BreakIfUnsuccessful=true)
{
	bool success=true;
	unsigned index=0;
	while (length)
	{
		if (Memory1[index]!=Memory2[index])
		{
			Printf("0x%x offset %.8x != %.8x\n",index,Memory1[index],Memory2[index]);
			if (BreakIfUnsuccessful)
				return false;
			else
				success=false;
		}

		index++;
		length--;
	}
	return success;
}

bool CompareMemory(char *sMemory1,char *sMemory2,char *sLength)
{
	byte *Memory1=(byte *)Hex2Long(sMemory1);
	byte *Memory2=(byte *)Hex2Long(sMemory2);
	unsigned long length=Hex2Long(sLength);
	unsigned remainder=length%4;
	unsigned length32=length-remainder;
	bool Test1=CompareMemory32bit((DWORD *)Memory1,(DWORD *)Memory2,length32);
	bool Test2=CompareMemory8bit(Memory1,Memory2,remainder);
	return (Test1 && Test2);
}

void AllocateMemory(char *sLength)
{
	unsigned length=Hex2Long(sLength);
	byte *memory=(byte *)malloc(length);
	memset(memory,0,length);
	printf("0x%x bytes allocated @ 0x%x \n",length,memory);
}

void FreeMemory(char *sMemory)
{
	byte *memory=(byte *)Hex2Long(sMemory);
	free(memory);
	printf("Freeing 0x%x\n",memory);
}


bool CompareFiles(const char *File1,const char *File2,bool BreakIfUnsuccessful=true)
{
	bool Success=false;
	byte *Buffer=(byte *)NewTek_malloc(1048576);
	byte *Buffer2=(byte *)NewTek_malloc(1048576);

	HANDLE file1=NULL,file2=NULL;
	{
		if ((!Buffer)||(!Buffer2))
			goto ExitLoop;

		//Open Files
		file1=OpenReadSeq(File1);
		if (file1==(HANDLE)-1)
		{
			file1=NULL;
			goto ExitLoop;
		}
		file2=OpenReadSeq(File2);
		if (file2==(HANDLE)-1)
		{
			file2=NULL;
			goto ExitLoop;
		}

		LARGE_INTEGER li_length1,li_length2;
		fpos_t length1=0,length2=0;
		BOOL Test1=GetFileSizeEx(file1,&li_length1);
		BOOL Test2=GetFileSizeEx(file1,&li_length2);

		if ((Test1!=0)&&(Test2!=0))
		{
			length1=li_length1.QuadPart;
			length2=li_length2.QuadPart;
		}

		if (length1!=length2)
		{
			printf("filesize difference %x != %x\n",length1,length2);
			if (BreakIfUnsuccessful)
				goto ExitLoop;
		}

		fpos_t length=min(length1,length2);
		{//Main loop
			fpos_t eoi64=length/1048576,i;
			for (i=0;i<eoi64;i++)
			{
				myRead(file1,Buffer,1048576);
				myRead(file2,Buffer2,1048576);
				if ((i & 0xf)==0) //only update progress every so often
					printf("%x     \r",(i>>4));
				if ((!CompareMemory32bit((DWORD *)Buffer,(DWORD *)Buffer2,1048576,i*0x40000,BreakIfUnsuccessful))&&(BreakIfUnsuccessful))
					break;
			}

			unsigned long eoi=(unsigned long)(length%1048576);
			if (eoi)
			{
				myRead(file1,Buffer,eoi);
				myRead(file2,Buffer2,eoi);
				unsigned long remainder=eoi;
				unsigned offset=0;
				if (eoi>4)
				{
					remainder=eoi%4;
					CompareMemory32bit((DWORD *)Buffer,(DWORD *)Buffer2,eoi-remainder,(eoi64 * 0x40000),BreakIfUnsuccessful);
					offset=eoi-remainder;
				}
				CompareMemory8bit(Buffer+offset,Buffer2+offset,remainder,BreakIfUnsuccessful);
			}
		}
		//All finished with success
		Success=true;
	}
ExitLoop:
	;

	//Close files
	if (file1)
		myClose(file1);
	if (file2)
		myClose(file2);

	if (Buffer)
		NewTek_free(Buffer);
	if (Buffer2)
		NewTek_free(Buffer2);

	if (Success)
		printf("Successful!\n");
	else
		printf("UnSuccessful! (error Detected)\n");
	return Success;
}


bool CompareFolders(const char *SearchPath,const char *CompareToPath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE FindFileHandle;
	char WildCardSearch[MAX_PATH];

	strcpy(WildCardSearch,SearchPath);
	strcat(WildCardSearch,"*");

	char SearchBuffer[MAX_PATH];
	char CompareToBuffer[MAX_PATH];

	bool success=true;
	size_t counter=0;

	FindFileHandle = FindFirstFile(WildCardSearch,&FindFileData);

	if (FindFileHandle!=INVALID_HANDLE_VALUE)
	{


		do
		{
			strcpy(SearchBuffer,SearchPath);
			strcat(SearchBuffer,FindFileData.cFileName);
			//Now to construct the CompareTo path
			strcpy(CompareToBuffer,CompareToPath);
			strcat(CompareToBuffer,FindFileData.cFileName);

			printf("%d Comparing %s...\n",++counter,SearchBuffer,FindFileData.cFileName);
			if (!CompareFiles(SearchBuffer,CompareToBuffer))
				success=false;
		}
		while(FindNextFile(FindFileHandle,&FindFileData));

	}

	FindClose(FindFileHandle);

	return success;

}



void load(char *filename)
{
	bool Success=false;
	HANDLE file=NULL;
	do
	{
		file=OpenReadSeq(filename);
		if (file==(HANDLE)-1)
		{
			file=NULL;
			break;
		}
		DWORD HighPart;
		unsigned long length=GetFileSize(file,&HighPart);
		if (HighPart)
		{
			printf("Only files less than 2 gig are supported\n");
			break;
		}
		byte *memory=(byte *)malloc(length);
		myRead(file,memory,length);
		printf("0x%x bytes loaded to memory @ 0x%x\n",length,memory);
		Success=true;
	}
	while (false);
	//Close files
	if (file)
		myClose(file);
	if (Success)
		printf("Successful!\n");
	else
		printf("UnSuccessful! (error Detected)\n");
}

bool split(const char *FileName,fpos_t splitsize)
{
	bool Success=false;
	char Path[MAX_PATH];
	char FNameExt[_MAX_FNAME];
	char FName[_MAX_FNAME];
	char Drive[_MAX_DRIVE];
	char Directory[_MAX_DIR];
	char Extension[_MAX_EXT];

	_splitpath(FileName,Drive,Directory,FName,Extension);
	//Now to merge together some of the fields
	strcpy(Path,Drive);
	strcat(Path,Directory);

	strcpy(FNameExt,FName);
	strcat(FNameExt,Extension);

	HANDLE readfile=NULL;
	HANDLE writefile=NULL;
	{
		readfile=OpenReadSeq(FileName);
		if (readfile==(HANDLE)-1)
		{
			readfile=NULL;
			printf("Unable to open the file to split\n");
			goto exitmain;
		}

		//Get the file size
		LARGE_INTEGER li_length;
		fpos_t fposlength=0;
		BOOL Test=GetFileSizeEx(readfile,&li_length);

		if (Test==0)
		{
			printf("Unable to get the file size\n");
			goto exitmain;
		}
		else
			fposlength=li_length.QuadPart;

		if (splitsize>fposlength)
		{
			printf("Aborted: splitsize>fposlength\n");
			goto exitmain;
		}

		//Now then figure out how many files will need to be made
		size_t FullPieces=(size_t) (fposlength/splitsize);
		fpos_t BytesRemain= fposlength%splitsize;
		//Main full piece loop
		char DestFile[MAX_PATH];
		byte Buffer[1024]; //Ram transfer buffer

		for (size_t i=0;i<FullPieces;i++)
		{
			//Now to construct the new filename
			sprintf(DestFile,"%s%.3d",FileName,i);
			printf("Writing %s%.3d... (%lx bytes)\n",FNameExt,i,splitsize);

			//Create the new file
			writefile=OpenWriteSeq(DestFile);
			if (writefile==(HANDLE)-1)
			{
				writefile=NULL;
				printf("Unable to open %s\n",DestFile);
				goto exitmain;
			}
			fpos_t eoj=splitsize/1024,j;
			for (j=0;j<eoj;j++)
			{
				myRead(readfile,Buffer,1024);
				myWrite(writefile,Buffer,1024);
				if ((j & 0xffff)==0) //only update progress every so often
					printf("%lx     \r",(j>>16));
			}
			size_t RemainingChunk=(size_t)(splitsize%1024);
			if (RemainingChunk)
			{
				myRead(readfile,Buffer,RemainingChunk);
				myWrite(writefile,Buffer,RemainingChunk);
			}

			myClose(writefile);
		}
		if (BytesRemain)
		{
			sprintf(DestFile,"%s%.3d",FileName,FullPieces);
			printf("Writing %s%.3d...  (%lx bytes)\n",FNameExt,FullPieces,BytesRemain);

			//Create the new file
			writefile=OpenWriteSeq(DestFile);
			if (writefile==(HANDLE)-1)
			{
				writefile=NULL;
				printf("Unable to open %s\n",DestFile);
				goto exitmain;
			}

			fpos_t eoj=BytesRemain/1024,j;
			for (j=0;j<eoj;j++)
			{
				myRead(readfile,Buffer,1024);
				myWrite(writefile,Buffer,1024);
				if ((j & 0xffff)==0) //only update progress every so often
					printf("%lx     \r",(j>>16));
			}
			size_t RemainingChunk=(size_t)(BytesRemain%1024);
			if (RemainingChunk)
			{
				myRead(readfile,Buffer,RemainingChunk);
				myWrite(writefile,Buffer,RemainingChunk);
			}

			myClose(writefile);
			writefile=NULL;
			Success=true;
		}
	} exitmain:;

	if (readfile)
		myClose(readfile);
	//this one would only have a pointer if an error has occurred
	if (writefile)
		myClose(writefile);

	return Success;
}

int JoinCompare( const void *arg1, const void *arg2 )
{
	return stricmp((const char *)arg1,(const char *)arg2);
}


//If NULL it is assumed to be the Source File name without the numbers
bool join(const char *SourceFileName,const char *DestFileName=NULL)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE FindFileHandle;

	char Path[MAX_PATH];
	char FNameExt[_MAX_FNAME];
	char FName[_MAX_FNAME];
	char Drive[_MAX_DRIVE];
	char Directory[_MAX_DIR];
	char Extension[_MAX_EXT];

	_splitpath(SourceFileName,Drive,Directory,FName,Extension);
	//Now to merge together some of the fields
	strcpy(Path,Drive);
	strcat(Path,Directory);

	strcpy(FNameExt,FName);
	strcat(FNameExt,Extension);


	char WildCardSearch[MAX_PATH];

	strcpy(WildCardSearch,SourceFileName);
	strcat(WildCardSearch,"*");

	char SearchBuffer[MAX_PATH];

	bool success=true;

	tList<char *> FileList;

	FindFileHandle = FindFirstFile(WildCardSearch,&FindFileData);

	HANDLE readfile=NULL;
	HANDLE writefile=NULL;

	if (FindFileHandle!=INVALID_HANDLE_VALUE)
	{
		do
		{
			strcpy(SearchBuffer,Path);
			strcat(SearchBuffer,FindFileData.cFileName);

			//This file must have some numbers appended to it
			if (stricmp(SearchBuffer,SourceFileName)==0)
				continue;

			FileList.Add(NewTek_malloc(SearchBuffer));
		}
		while(FindNextFile(FindFileHandle,&FindFileData));

		if (FileList.NoItems())
			qsort(&FileList[0],FileList.NoItems(),sizeof(char*),JoinCompare);

		{	//Now to iterate through the list and join them
			byte Buffer[1024]; //Ram transfer buffer
			writefile=OpenWriteSeq(DestFileName?DestFileName:SourceFileName);
			if (writefile==(HANDLE)-1)
			{
				writefile=NULL;
				goto exitmain;
			}

			for (size_t i=0;i<FileList.NoItems();i++)
			{
				printf("Joining %s...\n",FileList[i]);
				//Open it
				readfile=OpenReadSeq(FileList[i]);
				if (readfile==(HANDLE)-1)
				{
					readfile=NULL;
					goto exitmain;
				}

				//Get the file size
				LARGE_INTEGER li_length;
				fpos_t fposlength=0;
				BOOL Test=GetFileSizeEx(readfile,&li_length);

				if (Test==0)
				{
					printf("Unable to get the file size\n");
					goto exitmain;
				}
				else
					fposlength=li_length.QuadPart;


				//Now then figure out how many buffer reads will need to be made
				fpos_t eoj=fposlength/1024,j;
				for (j=0;j<eoj;j++)
				{
					myRead(readfile,Buffer,1024);
					myWrite(writefile,Buffer,1024);
					if ((j & 0xffff)==0) //only update progress every so often
						printf("%lx     \r",(j>>16));
				}
				size_t RemainingChunk=(size_t)(fposlength%1024);
				if (RemainingChunk)
				{
					myRead(readfile,Buffer,RemainingChunk);
					myWrite(writefile,Buffer,RemainingChunk);
				}

				myClose(readfile);
				readfile=NULL;
			}

			myClose(writefile);
			writefile=NULL;
		}
	} exitmain:;

	if (writefile)
		myClose (writefile);
	if (readfile)
		myClose(readfile);
	FindClose(FindFileHandle);
	//close all the filenames in the list
	for (size_t i=0;i<FileList.NoItems();i++)
	{
		NewTek_free(FileList[i]);
	}

	return success;
}


inline int GetFrameNumber(double seconds,double Num,double Den)
{
	if (seconds<0)
		return (int)(-0.5+(seconds/(Den/Num)));
	else
		return (int)(0.5+(seconds/(Den/Num)));
}
inline double GetSeconds(int framenumber,double Num,double Den)
{
	return ((double)framenumber*Den/Num);
}

bool ConvertSecondstoTimecodeString(char *Timecode,double seconds,double num,double den,bool outpointmode)
{
	unsigned hrs=0,mins=0,sec;
	double dec;
	unsigned frames;

	//decrease the seconds by 1 frame for out point mode
	if (outpointmode)
		seconds-=(den/num);

	if (seconds<0)
	{
		Timecode[0]='-';
		Timecode++;
		seconds=-seconds;
	}

	sec=(unsigned int)seconds;
	dec=seconds-sec;
	if (seconds>=60)
	{
		mins=sec/60;
		sec-=mins*60;
		if (seconds>=3600)
		{
			hrs=mins/60;
			mins-=hrs*60;
		}
	}
	unsigned roundup= (dec > 0.0) ? (unsigned int)(0.5+(dec*(num/den))) : (unsigned int)(-0.5+(dec*(num/den)));
	frames=(unsigned int)(roundup);

	sprintf(Timecode,"%.2d:%.2d:%.2d:%.2d",hrs,mins,sec,frames);
	//Success
	return true;
}

static double g_FrameRateNum=30000.0;
static double g_FrameRateDen=1001.0;

void flt2tc(char *sfloat)
{
	char tc[32];
	double seconds=atof(sfloat);
	ConvertSecondstoTimecodeString(tc,seconds,g_FrameRateNum,g_FrameRateDen,false);
	printf("TimeCode=%s\n",tc);
}

void tc2flt(unsigned long BCD)
{
	double Seconds;
	TimecodeConverters::Timecode_BCDToSeconds(BCD,Seconds,g_FrameRateNum/g_FrameRateDen);
	printf("Seconds=%.17g\n",Seconds);
}

void Frame2TC(char *sframe)
{
	char tc[32];
	unsigned long frame=Hex2Long(sframe);
	double FrameRate=g_FrameRateNum/g_FrameRateDen;
	double seconds=(double)frame/FrameRate;
	//ConvertSecondstoTimecodeString(tc,seconds,g_FrameRateNum,g_FrameRateDen,false);
	TimecodeConverters::Timecode_ConvertToString(tc,seconds,FrameRate);
	printf("TimeCode=%s\n",tc);
}

void TC2Frame(unsigned long BCD)
{
	double Value;
	double FrameRate=g_FrameRateNum/g_FrameRateDen;
	TimecodeConverters::Timecode_BCDToSeconds(BCD,Value,FrameRate);
	printf("Frames=%d\n",GetFrameNumber(Value,FrameRate,1));
}

void DisplayHelp()
{
	//"mkfile <filename> [filesize_MB] [bufsize_kB] - create file\n"
	printf(
	    "Hex2Ascii <hexnumber|decimal> \n"
	    "sqrt <hexnumber|decimal> \n"
		"reduce <num> <den>\n"
	    "quit    - quit this program\n"
	    "ExtFileBytes <DestFile> <SourceFile> Offset length \n"
	    "ExtFileText <DestFile> <SourceFile> Offset length \n"
		"ExtFileAscii <DestFile> <SourceFile> Offset length \n"
	    "CompareFiles <File1> <File2> [1=continue if mismatch]\n"
	    "CompareFolders <Path\\> <compare to path\\>\n"
	    "CompareMemory Memory1 Memory2 Length\n"
	    "[ length allocates memory\n"
	    "] memory frees memory\n"
	    "Load <file> allocates memory with file (use ] to free memory)\n"
	    "Split <file> <splitsize in kilobytes> \n"
	    "Join <file (no appending numbers)> <[dest file name]> \n"
		"SetFrameRate <double num> <double den=1.0> \n"
	    "flt2tc <double> \n"
	    "tc2flt <BCD> \n"
	    "Frame2TC <frame> \n"
	    "TC2Frame <BCD> \n"
	    "AddTC <BCD1> <BCD2>\n"
	    "SubTC <BCD1> <BCD2>\n"
	    "mem2flt <raw memory>\n"
	    "---------------------File Operations---------------\n"
	    "OpenRead  <FileName>\n"
	    "OpenWrite <FileName>\n"
	    "Close \n"
	    "Seek <hex position> [c || e] (default beginning)\n"
	    "Tell\n"
	    "ReadAscii <count>\n"
	    "ReadHex <count>\n"
	    "WriteHexBytes <0x01020304> <count>\n"
	    "WriteString <string>\n"
	    "Touch <FileName>\n"
	    "UpdateTime\n"
	    "CaptureFileTime\n"
	    "SetFileTime\n"
	);

}

void CommandLineInterface()
{
	printf("\nReady.\n");
	UpdateTouchTime();
	ToolsFileOperations fileops;
	static char input_line[128];

	while (printf(">"),gets(input_line))
	{
		static char		command[32];
		static char		str_1[64];
		static char		str_2[64];
		static char		str_3[64];
		static char		str_4[64];

		static char		g_char = 0;

		command[0] = '\0';
		str_1[0] = '\0';
		str_2[0] = '\0';
		str_3[0] = '\0';
		str_4[0] = '\0';

		if (sscanf( input_line,"%s %s %s %s %s",command,str_1,str_2,str_3,str_4)>=1)
		{

			if (!strnicmp( input_line, "Hex2Ascii", 9 ))
			{
				//file_test( rtc->rtc_contig_smp, str_1, param_2, param_3, _BOING_CREATE );
				Hex2Ascii(str_1);
			}
			else if (!strnicmp( input_line, "sqrt", 4 ))
			{
				mySqrt(str_1);
			}
			else if (!strnicmp( input_line, "reduce", 6))
			{
				__int64 num=Hex2Int64(str_1);
				__int64 den=Hex2Int64(str_2);
				int Num,Den;
				int result=av_reduce(&Num,&Den,num,den,1<<30);
				assert(result==1);
				printf("%d/%d\n",Num,Den);
			}
			else if (!strnicmp(input_line,"ExtFileBytes",12))
			{
				ExtFileBytes(str_1,str_2,str_3,str_4);
			}
			else if (!strnicmp(input_line,"ExtFileText",11))
			{
				ExtFileBytestoText(str_1,str_2,str_3,str_4);
			}
			else if (!strnicmp(input_line,"ExtFileAscii",12)) 
			{
				ExtFileBytestoText(str_1,str_2,str_3,str_4,true);
			}
			else if (!strnicmp(input_line,"CompareFiles",12))
			{
				bool breakifunsuccess=str_3[0]?atol(str_3)==0:true;
				CompareFiles(str_1,str_2,breakifunsuccess);
			}
			else if (!strnicmp(input_line,"CompareFolders",14))
			{
				if (CompareFolders(str_1,str_2))
					printf("Finished CompareFolders Successfully\n");
				else
					printf("Finished CompareFolders --Unsuccessful :(\n");

			}
			else if (!strnicmp(input_line,"[",1))
			{
				AllocateMemory(str_1);
			}
			else if (!strnicmp(input_line,"]",1))
			{
				FreeMemory(str_1);
			}
			else if (!strnicmp(input_line,"CompareMemory",13))
			{
				CompareMemory(str_1,str_2,str_3);
			}
			else if (!strnicmp(input_line,"Load",4))
			{
				load(str_1);
			}
			else if (!strnicmp(input_line,"Split",5))
			{
				if (split(str_1,((fpos_t)Hex2Long(str_2)) * 1024))
					printf("Finished split Successfully\n");
				else
					printf("Finished split --Unsuccessful :(\n");

			}
			else if (!strnicmp(input_line,"Join",4))
			{
				if (join(str_1,str_2))
					printf("Finished join Successfully\n");
				else
					printf("Finished join --Unsuccessful :(\n");

			}
			else if (!strnicmp(input_line,"SetFrameRate",12))
			{
				g_FrameRateNum=atof(str_1);
				g_FrameRateDen=(str_2[0])?atof(str_2):1.0;
				printf("Num=%f Den=%f Framerate=%f width=%f\n",g_FrameRateNum,g_FrameRateDen,g_FrameRateNum/g_FrameRateDen,g_FrameRateDen/g_FrameRateNum);
			}
			else if (!strnicmp(input_line,"flt2tc",6))
			{
				flt2tc(str_1);
			}
			else if (!strnicmp(input_line,"tc2flt",6))
			{
				tc2flt(Hex2Long(str_1));
			}
			else if (!strnicmp(input_line,"Frame2TC",8))
			{
				Frame2TC(str_1);
			}
			else if (!strnicmp(input_line,"TC2Frame",8))
			{
				TC2Frame(Hex2Long(str_1));
			}
			else if (!strnicmp(input_line,"AddTC",5))
			{
				double FrameRate=g_FrameRateNum/g_FrameRateDen;
				double Value1;
				TimecodeConverters::Timecode_BCDToSeconds(Hex2Long(str_1),Value1,FrameRate);
				double Value2;
				TimecodeConverters::Timecode_BCDToSeconds(Hex2Long(str_2),Value2,FrameRate);
				double Result=Value1+Value2;
				char TimeCode[32];
				TimecodeConverters::Timecode_ConvertToString(TimeCode,Result,FrameRate);
				printf("Result = %s\n",TimeCode);
			}
			else if (!strnicmp(input_line,"SubTC",5))
			{
				double FrameRate=g_FrameRateNum/g_FrameRateDen;
				double Value1;
				TimecodeConverters::Timecode_BCDToSeconds(Hex2Long(str_1),Value1,FrameRate);
				double Value2;
				TimecodeConverters::Timecode_BCDToSeconds(Hex2Long(str_2),Value2,FrameRate);
				double Result=Value1-Value2;
				char TimeCode[32];
				TimecodeConverters::Timecode_ConvertToString(TimeCode,Result,FrameRate);
				printf("Result = %s\n",TimeCode);
			}
			else if (!strnicmp(input_line,"mem2flt",7))
			{
				long Value=Hex2Long(str_1);
				long *bufptr=(long *)&Value;

				printf("Result = %f\n",*((float *)&Value));

				__asm {
				    mov edx,[bufptr]
					mov eax,[edx]
				    bswap eax
					mov [edx],eax
				}

				printf("Flipped Result = %f\n",*((float *)&Value));
			}

			//File Operations area---------------------------------------------------------
			else if (!strnicmp(input_line,"OpenRead",8))
			{
				fileops.Close();
				if (fileops.OpenRead(str_1))
					printf("Success\n");
				else
					printf("Unable to open\n");
			}
			else if (!strnicmp(input_line,"OpenWrite",9))
			{
				fileops.Close();
				if (fileops.OpenWrite(str_1))
				{
					g_WriteFile=fileops.GetHandle();
					printf("Success\n");
				}
				else
					printf("Unable to open\n");
			}

			else if (!strnicmp(input_line,"Close",5))
			{
				printf("result=%d\n",fileops.Close());
				g_WriteFile=NULL;
			}

			else if (!strnicmp(input_line,"Seek",4))
			{
				DWORD movemethod=FILE_BEGIN;
				if (str_2[0]=='c')
					movemethod=FILE_CURRENT;
				else if (str_2[0]=='e')
					movemethod=FILE_END;
				__int64 result=fileops.Seek64(Hex2Int64(str_1),movemethod);
				printf("result= high %x, ",result>>32);
				printf("low %x\n",result);
			}

			else if (!strnicmp(input_line,"Tell",4))
			{
				//printf("Current Position=%x\n",fileops.Tell());
				__int64 result=fileops.Tell64();
				printf("Current Position= high %x, ",result>>32);
				printf("low %x\n",result);
			}

			else if (!strnicmp(input_line,"ReadAscii",9))
			{
				unsigned BufferSize=min(1000,Hex2Long(str_1));
				char *Buffer=(char *)malloc(BufferSize+1);
				memset(Buffer,0,BufferSize+1);
				fileops.Read(Buffer,BufferSize);
				//Now to replace all non-ascii chars with periods
				for (size_t i=0;i<BufferSize;i++)
				{
					char currentchar=Buffer[i];
					if (currentchar<32)
						Buffer[i]='.';
				}
				printf("Beginning\n%s\nEnd\n",Buffer);
				free(Buffer);
			}
			else if (!strnicmp(input_line,"ReadHex",7))
			{
				unsigned BufferSize=min(1000,Hex2Long(str_1));
				char *Buffer=(char *)malloc(BufferSize+1);
				char *DestBuffer=(char *)malloc((BufferSize+1) * 8);
				memset(Buffer,0,BufferSize+1);
				fileops.Read(Buffer,BufferSize);
				RawDataToHexByteText(DestBuffer,Buffer,BufferSize);
				printf("Beginning\n%s\nEnd\n",DestBuffer);
				free(DestBuffer);
				free(Buffer);
			}

			else if (!strnicmp(input_line,"WriteHexBytes",13))
			{
				char Buffer[4];
				long *bufptr=(long *)Buffer;
				int count=4;
				if (str_2[0])
					count=Hex2Long(str_2);
				*bufptr=Hex2Long(str_1);
				__asm {
				    mov edx,[bufptr]
				    mov eax,[edx]
				    bswap eax
				    mov [edx],eax
				}
				printf("Bytes written=%d\n",fileops.Write(Buffer,count));
			}

			else if (!strnicmp(input_line,"WriteString",11))
			{
				printf("Bytes written=%d\n",fileops.Write(str_1,strlen(str_1)));
			}
			//Updates files to current time
			else if (!strnicmp(input_line,"Touch",5))
			{
				My_TouchFile(str_1,MyGet_SYSTime());
				printf("Updated date for %s\n",str_1);
			}
			else if (!strnicmp(input_line,"UpdateTime",9))
			{
				UpdateTouchTime();
				//Now to show the local time
				SYSTEMTIME time;
				TIME_ZONE_INFORMATION tz;
				GetTimeZoneInformation(&tz);
				SystemTimeToTzSpecificLocalTime(&tz,MyGet_SYSTime(),&time);
				printf("Updated system time\n");
				printf("To: %d/%d/%d at %.2d:%.2d %.2d\n",time.wMonth,time.wDay,time.wYear,time.wHour,time.wMinute,time.wSecond);
			}
			else if (!strnicmp(input_line,"CaptureFileTime",15))
			{
				FileTimeCache &cache=GetFileTimeCache();
				if (MyGetFileTime(str_1,cache))
				{
					printf("Captured %s time\n",str_1);
					SYSTEMTIME Created,LastAccess,LastWrite;
					FileTimeToSystemTime(&cache.CreationTime,&Created);
					FileTimeToSystemTime(&cache.LastAccessTime,&LastAccess);
					FileTimeToSystemTime(&cache.LastWriteTime,&LastWrite);

					TIME_ZONE_INFORMATION tz;
					GetTimeZoneInformation(&tz);
					SystemTimeToTzSpecificLocalTime(&tz,&Created,&Created);
					SystemTimeToTzSpecificLocalTime(&tz,&LastAccess,&LastAccess);
					SystemTimeToTzSpecificLocalTime(&tz,&LastWrite,&LastWrite);

					printf("Created: %d/%d/%d at %.2d:%.2d %.2d\n",Created.wMonth,Created.wDay,Created.wYear,Created.wHour,Created.wMinute,Created.wSecond);
					printf("Last Accessed: %d/%d/%d at %.2d:%.2d %.2d\n",LastAccess.wMonth,LastAccess.wDay,LastAccess.wYear,LastAccess.wHour,LastAccess.wMinute,LastAccess.wSecond);
					printf("Last Written: %d/%d/%d at %.2d:%.2d %.2d\n",LastWrite.wMonth,LastWrite.wDay,LastWrite.wYear,LastWrite.wHour,LastWrite.wMinute,LastWrite.wSecond);
				}
				else
					printf("Unsuccessful.\n");
			}
			else if (!strnicmp(input_line,"SetFileTime",11))
			{
				FileTimeCache &cache=GetFileTimeCache();
				if (MySetFileTime(str_1,cache))
				{
					printf("Set %s time to:\n",str_1);
					SYSTEMTIME Created,LastAccess,LastWrite;
					FileTimeToSystemTime(&cache.CreationTime,&Created);
					FileTimeToSystemTime(&cache.LastAccessTime,&LastAccess);
					FileTimeToSystemTime(&cache.LastWriteTime,&LastWrite);

					TIME_ZONE_INFORMATION tz;
					GetTimeZoneInformation(&tz);
					SystemTimeToTzSpecificLocalTime(&tz,&Created,&Created);
					SystemTimeToTzSpecificLocalTime(&tz,&LastAccess,&LastAccess);
					SystemTimeToTzSpecificLocalTime(&tz,&LastWrite,&LastWrite);

					printf("Created: %d/%d/%d at %.2d:%.2d %.2d\n",Created.wMonth,Created.wDay,Created.wYear,Created.wHour,Created.wMinute,Created.wSecond);
					printf("Last Accessed: %d/%d/%d at %.2d:%.2d %.2d\n",LastAccess.wMonth,LastAccess.wDay,LastAccess.wYear,LastAccess.wHour,LastAccess.wMinute,LastAccess.wSecond);
					printf("Last Written: %d/%d/%d at %.2d:%.2d %.2d\n",LastWrite.wMonth,LastWrite.wDay,LastWrite.wYear,LastWrite.wHour,LastWrite.wMinute,LastWrite.wSecond);
				}
				else
					printf("Unsuccessful.\n");
			}

			else if (!strnicmp(input_line,"help",4))
			{
				DisplayHelp();
			}
			else if (!strnicmp( input_line, "quit", 4))
			{
				break;
			}
			else
			{
				printf("huh? - try \"help\"\n");
			}
		}
	}
	fileops.Close();
}



void main ()
{
	DisplayHelp();
	CommandLineInterface();
}
