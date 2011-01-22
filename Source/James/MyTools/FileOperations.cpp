#include "StdAfx.h"


HANDLE OpenReadSeq(const char *filename) {
	HANDLE hf;
	hf=CreateFile(filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		//FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hf==INVALID_HANDLE_VALUE) hf=(void *)-1;
	return (hf);
	}

HANDLE OpenWriteSeq(const char *filename) {
	HANDLE hf;
	hf=CreateFile(filename,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		//FILE_ATTRIBUTE_NORMAL,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hf==INVALID_HANDLE_VALUE) hf=(void *)-1;
	return (hf);
	}


HANDLE OpenReadWriteSeq(const char *filename) {
	HANDLE hf;
	hf=CreateFile(filename,
		GENERIC_READ|GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		//FILE_FLAG_SEQUENTIAL_SCAN,
		NULL);
	if (hf==INVALID_HANDLE_VALUE) hf=(void *)-1;
	return (hf);
	}


__int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod) {
   LARGE_INTEGER li;
   li.QuadPart = distance;
   li.LowPart = SetFilePointer (hf,li.LowPart,&li.HighPart,MoveMethod);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
		}
	return li.QuadPart;
	}

__int64 myTell64 (HANDLE hf) {
   LARGE_INTEGER li;
   li.QuadPart = 0;
   li.LowPart = SetFilePointer (hf,li.LowPart,&li.HighPart,FILE_CURRENT);
	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		li.QuadPart = -1;
		}
	return li.QuadPart;
	}

DWORD myTell (HANDLE hf) {
	DWORD distance;
   distance = SetFilePointer (hf,0,NULL,FILE_CURRENT);
	if (distance == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		distance = -1;
		}
	return distance;
	}

DWORD mySeek (HANDLE hf,long distance,DWORD MoveMethod) {
	DWORD bytes_read;
   bytes_read=SetFilePointer (hf,distance,NULL,MoveMethod);
	if (bytes_read == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
		bytes_read = -1;
		}
	return (bytes_read);
	}
 
int myRead(HANDLE hf,void *buf,DWORD count) 
{
	DWORD bytes_read=-1;
	if (hf)
		if (!(ReadFile(hf,buf,count,&bytes_read,NULL))) bytes_read=-1;
	return ((int)bytes_read);
}

void Writef(HANDLE hf,const char *format, ... )
{
	char Temp[4096];
	va_list marker;
	va_start(marker,format);
		vsprintf(Temp,format,marker);
		if (hf)
			myWrite(hf,Temp,strlen(Temp));
		else
			OutputDebugString(Temp);
	va_end(marker); 
}

int myWrite(HANDLE hf,void *buf,DWORD count) 
{
	DWORD bytes_read=-1;
	if (hf)
		if (!(WriteFile(hf,buf,count,&bytes_read,NULL))) bytes_read=-1;
	return ((int)bytes_read);
}

int myClose(HANDLE hf) {
	int value=-1;
	if ((hf)&&CloseHandle(hf)) value=0;
	return (value);
	}

void ToolsFileOperations::Writef(const char *format, ... )
{
	char Temp[4096];
	va_list marker;
	va_start(marker,format);
		vsprintf(Temp,format,marker);
		if (m_hf)
			myWrite(m_hf,Temp,strlen(Temp));
		else
			OutputDebugString(Temp);
	va_end(marker); 
}


bool ToolsFileOperations::OpenRead (char *filename) {
	bool ret=false;
	if (!m_hf) {
		m_hf=OpenReadSeq(filename);
		ret=(((int)m_hf)!=-1);
		if (!ret)
			m_hf=NULL;
		}
	return ret;
	}

bool ToolsFileOperations::OpenWrite(char *filename) {
	bool ret=false;
	if (!m_hf) {
		m_hf=OpenWriteSeq(filename);
		ret=(((int)m_hf)!=-1);
		if (!ret)
			m_hf=NULL;
		}

	return ret;
	}
