#pragma once

namespace GG_Framework
{
	namespace Base
	{

class FRAMEWORK_BASE_API Stream_In
{	public:			//! Read data from the IO stream
					virtual size_t ReadData(void *Data,size_t Size)=0;

					//! The destructor !
					virtual ~Stream_In(void) {}
};

//**********************************************************************************************************************************************
//! This is how data is sent out
class FRAMEWORK_BASE_API Stream_Out
{	public:			//! Write data to the IO stream
					virtual size_t WriteData(const void *Data,size_t Size)=0;	

					//! The destructor !
					virtual ~Stream_Out(void) {}
};


class FRAMEWORK_BASE_API MemoryStream_In : public Stream_In
{	private:		//! the pointer to the memory segment
					char *m_MemorySegment;
					char *m_MemoryOffset;

					//! Stuff
					virtual size_t ReadData(void *Data,size_t Size);

	public:			//! Free and Get the memory
					void *GetMemory(void);
					void FreeMemory(void);
					void SetData(void *Data);
					
					//! Constructor
					MemoryStream_In(void);
					MemoryStream_In(void *Memory) 
						{	SetData(Memory); 
						}

					virtual ~MemoryStream_In(void) 
						{	
						}
};

//**********************************************************************************************************************************************
#define DAVB_MemoryStream_Out_ExpandSize		512

class FRAMEWORK_BASE_API MemoryStream_Out : public Stream_Out
{	private:		//! the pointer to the memory segment
					char *m_MemorySegment;

					//! The amount of memory currently allocated
					size_t m_MemorySize;
					size_t m_TrueMemorySize;

					//! Stuff 
					virtual size_t WriteData(const void *Data,size_t Size);

	public:			//! Free and Get the memory
					void *GetMemory(void);
					void FreeMemory(void);

					//! Get the ammount of memory that has been written
					size_t GetMemorySize(void);
					
					//! Constructor
					MemoryStream_Out(void);
					virtual ~MemoryStream_Out(void);
};


//**********************************************************************************************************************************************
//! THis declaration is for a stream of data entering the application
class FRAMEWORK_BASE_API FileStream_In : public Stream_In
{	protected:		//! The file handle
					HANDLE m_Handle;
					std::wstring	m_fileName;

	public:			//! Standard stuff
					HANDLE GetFile(void);

					bool SetFilename(const wchar_t *FN);

					const wchar_t* GetFileName(void)
						{	return m_fileName.c_str();
						}

					void Close(void);

					//! Read data from the IO stream
					virtual size_t ReadData(void *Data,size_t Size);

					//! C'tor and D'tor
					FileStream_In (void);
					FileStream_In (wchar_t *FN);
					~FileStream_In(void);
};

//**********************************************************************************************************************************************
//! THis declaration is for a stream of data leaving the application
class FRAMEWORK_BASE_API FileStream_Out : public Stream_Out
{	protected:		//! The file handle
					HANDLE m_Handle;
					std::wstring	m_fileName;

	public:			//! Standard stuff
					HANDLE GetFile(void);

					bool SetFilename(const wchar_t *FN);

					const wchar_t* GetFileName(void)
						{	return m_fileName.c_str();
						}

					void Close(void);

					//! Write data to the IO stream
					virtual size_t WriteData(const void *Data,size_t Size);

					//! C'tor and D'tor
					FileStream_Out (void);
					FileStream_Out (wchar_t *FN);
					~FileStream_Out(void);
};




class LoadData
{	protected:			//! The interface for writing data to the drive
						Stream_In	*m_ReadDataInterface;
						bool		m_ErrorOccurredReading;


	public:				//! My casting operator
						operator Stream_In* (void) { return m_ReadDataInterface; }

						//! Read data from the source
						template<class L>
						bool Get(L &Data)
							{	if (m_ErrorOccurredReading) return false;
								if ( m_ReadDataInterface->ReadData(&Data,sizeof(L))!=sizeof(L) )  
								{	m_ErrorOccurredReading=true; 
									return false; 
								}
								return true;
							}

						template<class L>
						bool GetArray(L *Data,unsigned Num)
							{	if (m_ErrorOccurredReading) return false;
								if ( m_ReadDataInterface->ReadData(Data,sizeof(L)*Num)!=sizeof(L)*Num )  
								{	m_ErrorOccurredReading=true; 
									return false; 
								}
								return true;
							}

						bool GetRaw(void *Data,unsigned Num)
							{	if (m_ErrorOccurredReading) return false;
								if ( m_ReadDataInterface->ReadData(Data,Num)!=Num )  
								{	m_ErrorOccurredReading=true; 
									return false; 
								}
								return true;
							}

						bool GetString(char *String,unsigned MaxStringLength=0xffffffff)
							{	if (m_ErrorOccurredReading) return false;
								unsigned StringLength; 
								if (!Get(StringLength)) return false;
								if (!StringLength || (StringLength>MaxStringLength))
									{ m_ErrorOccurredReading=true; return false; }
								return GetArray(String,StringLength);
							}

						bool GetString(wchar_t *String,unsigned MaxStringLength=0xffffffff)
							{	if (m_ErrorOccurredReading) return false;
								unsigned StringLength; 
								if (!Get(StringLength)) return false;
								if (!StringLength || (StringLength>MaxStringLength))
									{ m_ErrorOccurredReading=true; return false; }
								return GetArray(String,StringLength);
							}

						//! Determine whether an error occurred when reading the stream
						bool Error(void)
							{	return m_ErrorOccurredReading; 
							}

						//! Constructor for a memory source
						LoadData(void *Memory)
							{	m_ReadDataInterface=NULL;
								MemoryStream_In *NewStream=new MemoryStream_In;
								if (!NewStream) { m_ErrorOccurredReading=true; return; }
								NewStream->SetData(Memory);
								m_ReadDataInterface=NewStream;
								m_ErrorOccurredReading=false;
							}

						//! Constructor for a file source
						LoadData(const wchar_t *FileName)
							{	m_ReadDataInterface=NULL;
								FileStream_In *NewStream=new FileStream_In;
								if (!NewStream) 
									{	m_ErrorOccurredReading=true; 
										return; 
									}
								if (!NewStream->SetFilename(FileName)) 
									{	delete NewStream;
										m_ErrorOccurredReading=true; 
										return; 
									}
								m_ReadDataInterface=NewStream;
								m_ErrorOccurredReading=false;
							}

						//! Constructor for all other sources
						LoadData(Stream_In *Stream)
							{	m_ReadDataInterface=Stream;
								m_ErrorOccurredReading=false;
							}

						//! Destructor
						~LoadData(void)
							{	if (m_ReadDataInterface) 
										delete m_ReadDataInterface;
							}

};

//************************************************************************************************************************************************s
class SaveData
{	protected:			//! The interface for writing data to the drive
						Stream_Out	*m_WriteDataInterface;
						bool		m_ErrorOccurredReading;

						// Debugging !
						unsigned m_BytesWritten;

	public:				//! My casting operator
						operator Stream_Out* (void)    
							{	return m_WriteDataInterface; 
							}

						//! Read data from the source
						template<class L>
						bool Put(L Data)
							{	m_BytesWritten+=sizeof(L);
								if (m_ErrorOccurredReading) return false;
								if ( m_WriteDataInterface->WriteData(&Data,sizeof(L))!=sizeof(L) ) 
								{	m_ErrorOccurredReading=true; 
									return false; 
								}
								return true;
							}

						template<class L>
						bool PutArray(const L *Data,unsigned Num)
							{	m_BytesWritten+=Num;
								if (m_ErrorOccurredReading) return false;
								if ( m_WriteDataInterface->WriteData(Data,sizeof(L)*Num)!=sizeof(L)*Num ) 
								{	m_ErrorOccurredReading=true; return false; }
								return true;
							}

						bool PutRaw(const void *Data,unsigned Num)
							{	m_BytesWritten+=Num;
								if (m_ErrorOccurredReading) return false;
								if ( m_WriteDataInterface->WriteData(Data,Num)!=Num ) 
								{	m_ErrorOccurredReading=true; 
									return false; 
								}
								return true;
							}

						bool PutString(const char *Data)
							{	unsigned StringLength=(unsigned)(strlen(Data)+1);
								m_BytesWritten+=StringLength;
								if (!Put(StringLength)) return false;
								if (!PutArray(Data,StringLength)) return false;
								return true;
							}

						bool PutString(const wchar_t *Data)
							{	unsigned StringLength=(unsigned)(wcslen(Data)+1);
								m_BytesWritten+=StringLength*sizeof(wchar_t);
								if (!Put(StringLength)) return false;
								if (!PutArray(Data,StringLength)) return false;
								return true;
							}

						//! Determine whether an error occurred when reading the stream
						bool Error(void)
							{	return m_ErrorOccurredReading; 
							}

						//! Constructor for a memory source
						SaveData()
							{	m_WriteDataInterface=NULL;
								MemoryStream_Out *NewStream=new MemoryStream_Out;
								if (!NewStream) { m_ErrorOccurredReading=true; return; }
								m_WriteDataInterface=NewStream;
								m_ErrorOccurredReading=false;
								m_BytesWritten=0;
							}

						//! Constructor for a file source
						SaveData(const wchar_t *FileName)
							{	m_WriteDataInterface=NULL;
								FileStream_Out *NewStream=new FileStream_Out;
								if (!NewStream) 
									{	m_ErrorOccurredReading=true; 
										return; 
									}
								if (!NewStream->SetFilename(FileName)) 
									{	delete NewStream;
										m_ErrorOccurredReading=true; 
										return; 
									}
								m_WriteDataInterface=NewStream;
								m_ErrorOccurredReading=false;
								m_BytesWritten=0;
							}

						//! Constructor for all other sources
						SaveData(Stream_Out *Stream)
							{	m_WriteDataInterface=Stream;
								m_ErrorOccurredReading=false;
								m_BytesWritten=0;
							}

						//! Destructor
						~SaveData(void)
							{	
								if (m_WriteDataInterface) 
										delete m_WriteDataInterface;
							}

};


class LoadSave_Interface
{	public:				/*! Get the version of this implementation
							Eg, for version 1.2 
								MajorVersion=1
								MinorVersion=2*/
						virtual void LoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion)=0;

						//! Save the data for this item
						virtual bool LoadSave_SaveData(SaveData *SaveContext)=0;

						/*!	Load the data. You are passed the version
							number of the plugin that was saved. You can use
							this for intelligent future revision control */
						virtual bool LoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion)=0;
};

inline void LoadSave_Failed_Load(LoadSave_Interface *a)
{	
	GG_Framework::Base::DebugOutput("LoadSave::Load failed : [0x%lx] [%s] [%d]\n",a,__FILE__,__LINE__);
}

inline void LoadSave_Failed_Save(LoadSave_Interface *a)
{	
	GG_Framework::Base::DebugOutput("LoadSave::Save failed : [0x%lx] [%s] [%d]\n",a,__FILE__,__LINE__);
}


inline bool LoadSave_LoadData_Member(LoadSave_Interface *Variable,LoadData *LoadContext)
{	
	unsigned l_MajorVersion; LoadContext->Get(l_MajorVersion);
	unsigned l_MinorVersion; LoadContext->Get(l_MinorVersion);
	if (LoadContext->Error()) 
	{
		LoadSave_Failed_Load(Variable);
		return false;
	}
	if (!(Variable)->LoadSave_LoadData(LoadContext,l_MajorVersion,l_MinorVersion))
	{
		LoadSave_Failed_Load(Variable);
		return false;
	}
	return true;
}


inline bool LoadSave_SaveData_Member(LoadSave_Interface *Variable,SaveData *SaveContext)
{	unsigned l_MajorVersion,l_MinorVersion;
	(Variable)->LoadSave_GetVersionNumber(l_MajorVersion,l_MinorVersion);
	SaveContext->Put(l_MajorVersion); SaveContext->Put(l_MinorVersion);
	if (SaveContext->Error()) 
	{
		LoadSave_Failed_Save(Variable);
		return false;
	}
	if (!(Variable)->LoadSave_SaveData(SaveContext))
	{
		LoadSave_Failed_Save(Variable);
		return false;
	}
	return true;
}

template <class T>
inline bool LoadSave_LoadData_Inhereted(T *Variable,LoadData *LoadContext)
{	
	unsigned l_MajorVersion; LoadContext->Get(l_MajorVersion);
	unsigned l_MinorVersion; LoadContext->Get(l_MinorVersion);
	if (LoadContext->Error())
		return false;
	if (!Variable->T::LoadSave_LoadData(LoadContext,l_MajorVersion,l_MinorVersion))
		return false;
	return true;
}

template <class T>
inline bool LoadSave_SaveData_Inhereted(T *Variable,SaveData *SaveContext)
{	
	unsigned l_MajorVersion,l_MinorVersion;
	Variable->T::LoadSave_GetVersionNumber(l_MajorVersion,l_MinorVersion);
	SaveContext->Put(l_MajorVersion); SaveContext->Put(l_MinorVersion);
	if (SaveContext->Error())
		return false;
	if (!Variable->T::LoadSave_SaveData(SaveContext))
		return false;
	return true;
}


//TODO:  if I write my own fileIO routines... these will need to be moved
struct FileTimeCache
{
	FILETIME CreationTime;
	FILETIME LastAccessTime;
	FILETIME LastWriteTime;
};

//Here is a handy way to obtain the file times information
FRAMEWORK_BASE_API bool fio_SetFileTime(const wchar_t *Name,FileTimeCache &cache);
FRAMEWORK_BASE_API bool fio_GetFileTime(const wchar_t *Name,FileTimeCache &cache);

//Here is a handy utility to verify a file hasn't changed
inline bool SaveTimeStamp(const wchar_t *FileName,SaveData *SaveContext)
{
	FileTimeCache FTcache;
	if (!fio_GetFileTime(FileName,FTcache)) return false;
	if (!SaveContext->Put(FTcache.LastWriteTime)) return false;
	return true;
}

inline bool LoadTimeStamp(const wchar_t *FileName,LoadData *LoadContext)
{
	FileTimeCache FTcache,blsFTcache;
	if (!fio_GetFileTime(FileName,FTcache)) return false;

	if (!LoadContext->Get(blsFTcache.LastWriteTime)) return false;
	//returns true (success) if the file dates are the same as before
	return  (
		(FTcache.LastWriteTime.dwLowDateTime ==blsFTcache.LastWriteTime.dwLowDateTime) &&
		(FTcache.LastWriteTime.dwHighDateTime==blsFTcache.LastWriteTime.dwHighDateTime)
		);
}


	}//end namespace Base
} //end namespace GG_Framework

	
		
