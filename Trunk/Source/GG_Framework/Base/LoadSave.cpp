#include "stdafx.h"
#include "GG_Framework.Base.h"

namespace GG_Framework
{
	namespace Base
	{
  /****************************************************************************************/
 /*										MemoryStream									 */
/****************************************************************************************/


//***********************************************************************************************************************************************
size_t MemoryStream_In::ReadData(void *Data,size_t Size)
{	
#ifndef __DISABLE_EXCEPTION_HANDLING
	try	
#endif
		{	memcpy(Data,m_MemoryOffset,Size);
			m_MemoryOffset+=Size;
			return Size;
		}
#ifndef __DISABLE_EXCEPTION_HANDLING
	catch(...) { return 0; }
#endif
}

MemoryStream_In::MemoryStream_In(void) : m_MemorySegment(NULL), m_MemoryOffset(NULL)
{	
}

// Free and Get the memory
void *MemoryStream_In::GetMemory(void)
{	return (void *)m_MemorySegment;
}

void MemoryStream_In::FreeMemory(void)
{	free(m_MemorySegment);
	m_MemorySegment=NULL;
}					

void MemoryStream_In::SetData(void *Data)
{	m_MemorySegment=m_MemoryOffset=(char*)Data;
}

//***********************************************************************************************************************************************
size_t MemoryStream_Out::WriteData(const void *Data,size_t Size)
{	
#ifndef __DISABLE_EXCEPTION_HANDLING
	try	
#endif
	{		// CHeck that the segment fits
			if (m_MemorySize+Size > m_TrueMemorySize)
			{	m_TrueMemorySize=max(m_MemorySize+Size,m_TrueMemorySize+DAVB_MemoryStream_Out_ExpandSize);
				m_MemorySegment=(char*)realloc(m_MemorySegment,m_TrueMemorySize);
				if (!m_MemorySegment) return false;
			}

			memcpy(m_MemorySegment+m_MemorySize,Data,Size);
			m_MemorySize+=Size;

			return Size;
		}
#ifndef __DISABLE_EXCEPTION_HANDLING
	catch(...) { return 0; }
#endif
}
					
MemoryStream_Out::MemoryStream_Out(void)
{	m_MemorySegment=NULL;
	m_MemorySize=
	m_TrueMemorySize=0;
}

// Free and Get the memory
void *MemoryStream_Out::GetMemory(void)
{	return (void*)m_MemorySegment;
}

void MemoryStream_Out::FreeMemory(void)
{	if (m_MemorySegment)
		free(m_MemorySegment);
	m_MemorySegment=NULL;
	m_MemorySize=
	m_TrueMemorySize=0;
}

size_t MemoryStream_Out::GetMemorySize(void)
{	return m_MemorySize;
}

MemoryStream_Out::~MemoryStream_Out(void) 
{	FreeMemory();
}



  /****************************************************************************************/
 /*											FileStream									 */
/****************************************************************************************/

bool IsFileDrive(const wchar_t* filename)
{	if (!filename) return false;
	return (GetDriveTypeW(filename) != DRIVE_NO_ROOT_DIR);
}
 
bool IsFolder(const wchar_t* filename)
{	if (!filename) return false;
	bool ret = false;
	WIN32_FIND_DATAW findData;
	HANDLE findHandle = FindFirstFileW(filename, &findData);
	if(findHandle != INVALID_HANDLE_VALUE)
	{	if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			ret = true;
	}
	FindClose(findHandle);
	return ret;
}

FileStream_In::FileStream_In(wchar_t *FN) 
{	m_Handle=NULL; 
	if (!SetFilename(FN)) assert(false);//_throw("Could not open the file."); 
}

HANDLE FileStream_In::GetFile(void)
{	return m_Handle;
}

void FileStream_In::Close(void)
{	if (m_Handle) CloseHandle(m_Handle);
	m_Handle=NULL;
}

size_t FileStream_In::ReadData(void *Data,size_t Size)
{	if (m_Handle) 
	{	DWORD NoBytesWritten=(DWORD)Size;
		ReadFile(m_Handle,Data,NoBytesWritten,&NoBytesWritten,NULL);
		return (size_t)NoBytesWritten;
	}
	return 0;
}

bool FileStream_In::SetFilename(const wchar_t *FN)
{	
	Close();

		if ((IsFileDrive(FN))||(IsFolder(FN)))
	{	m_Handle=NULL;
		return false;
	}
	
	m_Handle=CreateFileW(	FN,GENERIC_READ,FILE_SHARE_WRITE|FILE_SHARE_READ,NULL,
							OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
							NULL);
	if (m_Handle==INVALID_HANDLE_VALUE) m_Handle=NULL;
	if (!m_Handle) return false;
	m_fileName=FN;
	return true;
}

FileStream_In::~FileStream_In(void)
{	Close();
}

FileStream_In::FileStream_In(void)
{	
	m_Handle=NULL;
}






FileStream_Out::FileStream_Out(wchar_t *FN) 
{	m_Handle=NULL; 
	if (!SetFilename(FN)) assert(false);//_throw("Could not open the file."); 
}

FileStream_Out::FileStream_Out(void)
{	
	m_Handle=NULL;
}

size_t FileStream_Out::WriteData(const void *Data,size_t Size)
{	if (m_Handle) 
	{	
		DWORD NoBytesWritten=0;
		WriteFile(m_Handle,Data,(DWORD)Size,&NoBytesWritten,NULL);
		return NoBytesWritten;
	}
	return 0;
}

bool FileStream_Out::SetFilename(const wchar_t *FN)
{
	Close();

	if ((IsFileDrive(FN))||(IsFolder(FN)))
	{	m_Handle=NULL;
		return false;
	}
		
	m_Handle=CreateFileW(	FN,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
							CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,
							NULL);
	if (m_Handle==INVALID_HANDLE_VALUE) m_Handle=NULL;
	if (!m_Handle) return false;
	m_fileName=FN;
	return true;
}

FileStream_Out::~FileStream_Out(void)
{	Close();
}


void FileStream_Out::Close(void)
{	if (m_Handle) CloseHandle(m_Handle);
	m_Handle=NULL;
}

HANDLE FileStream_Out::GetFile(void)
{	return m_Handle;
}



bool fio_SetFileTime(const wchar_t *Name,FileTimeCache &cache)
{
	HANDLE FileHandle=CreateFileW(Name,GENERIC_READ|GENERIC_WRITE,
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (FileHandle==INVALID_HANDLE_VALUE)
		return false;
	bool Ret=(SetFileTime(FileHandle,&cache.CreationTime,&cache.LastAccessTime,&cache.LastWriteTime)!=0);
	CloseHandle(FileHandle);
	return Ret;
}

bool fio_GetFileTime(const wchar_t *Name,FileTimeCache &cache)
{
	HANDLE FileHandle=CreateFileW(Name,GENERIC_READ,
		FILE_SHARE_READ,
		NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if (FileHandle==INVALID_HANDLE_VALUE)
		return false;
	bool Ret=(GetFileTime(FileHandle,&cache.CreationTime,&cache.LastAccessTime,&cache.LastWriteTime)!=0);
	CloseHandle(FileHandle);
	return Ret;
}

	}//end namespace Base
} //end namespace GG_Framework
