#pragma once

#ifndef SHARED_PTR
//#define SHARED_PTR std::tr1::shared_ptr
#define SHARED_PTR osg::ref_ptr
#endif

//Here is some example code:
/*
class TestClass;

void main()
{
	static FileSharedPointer<TestClass> s_TestPointer;
	SHARED_PTR<TestClass> MyPointer(s_TestPointer.GetSharedPointer(L"TestFileA"));
	{
		SHARED_PTR<TestClass> MyPointer2;
		//we can delay the association too
		MyPointer2=s_TestPointer.GetSharedPointer(L"TestFileB");
		s_TestPointer.Release(MyPointer2.get());
	}
	s_TestPointer.Release(L"TestFileA");
}
*/
//Typically speaking the FileSharedPointer will be static where each pointer of the same type can reference their own
//smart pointer association to the one retrieved from this class

namespace UtilityCode
{

/// This is a template class which allows you to associate a shared pointer to a filename.  It can store and retrieve in log-n time to support
/// a large database of filenames.  The shared pointer itself (documented in boost) allows multiple clients to share the same pointer safely,
/// and is designed to work across multiple threads (i.e. is threadsafe).  \see GetFileVariableBlock
/// \note If you are using this class most likely your pointer may be shared across different threads
///Please take care and put critical sections when modifying the shared value.  To be safe
///put critical sections around all reads and writes of this pointer

template <class T>
class FileSharedPointer  //throws exception (but should be an impossible case)
{
public:
	FileSharedPointer() : m_FileList()
	{
	}

	///This will return the smart pointer associated with this filename (instantiates implicitly)
	/// \param InitializeZerod when a new instance is implicitly created setting to true will zero out the new memory.  You should not
	/// use this for classes where it would be the constructor's responsibility to initialize.  However, this is ideal for pure data types
	/// (e.g. structs)
	SHARED_PTR<T> GetSharedPointer(const char *FileName,bool InitializeZerod=false);

	///This is a handy call to know if the file already has been entered or not.  This is pretty much the same functionality as it was for tList
	bool Exists(const char *FileName) const;
	///This is optional! You may call this when client is finished with the pointer, and it will (eventually) remove the entry from the list
	///If your client chooses to accumulate as you go (e.g. caching solution) you need not call this.
	/// \note the destruction of this class will implicitly cleanup the list
	void Release(const char *FileName);
	///This may be more convenient to use \see Release(const char *FileName)
	void Release(T *pPointer);

	///Use this to protect all shared pointers of the same file type.  This way all clients which share the same pointer can call this, and you 
	///may establish critical sections per multiples references of the same file across threads.
	OpenThreads::Mutex *GetFileVariableBlock(const char *FileName);
private:
	void Release_Internal(const char *FileName=NULL,T *pPointer=NULL);

	struct FileListInfo
	{
		FileListInfo()	: m_SharedPointer(new T) //,m_BlockPointer(new OpenThreads::Mutex)
		{
			m_Cnt=0;
		}
		~FileListInfo()
		{
			//Boost shared_ptr does not support release, but will dereference automatically upon destruction of m_SharedPointer
			/*
			if (m_SharedPointer.get()!=NULL)
				m_SharedPointer=NULL; //This will dereference this instance
			if (m_BlockPointer.get()!=NULL)
				m_BlockPointer=NULL; //This will dereference this instance
			*/
		}
		void Init(const char *FileName_,bool InitializeZerod=false)
		{
			FileName=FileName_;
			//Convert into lowercase to make case insensitive
			std::transform(FileName.begin(),FileName.end(),FileName.begin(),std::ptr_fun<wint_t,wint_t>(towlower));
			//Boost can't support late binding.. this is now done in constructor
			//m_SharedPointer=new T;
			if (InitializeZerod)
				memset(m_SharedPointer.get(),0,sizeof(T)); //we'll initialize what its pointing to
			//Boost can't support late binding.. this is now done in constructor
			//m_BlockPointer=new FrameWork::Threads::CriticalSection;
		}
		std::string FileName;
		SHARED_PTR<T> m_SharedPointer;
		//Keep track of how many references are used to this file
		LONG m_Cnt;
		//This is the ideal critical section to block all pointers using the same file
		//SHARED_PTR<OpenThreads::Mutex> m_BlockPointer;  referenced includes this
		//TODO: find out why wstring < operator is not working
		//The wcscmp will work fine for now 
		bool operator() (FileListInfo A,FileListInfo B) const
		{	return (A.FileName<B.FileName);		//ascending order
		}
	};

	struct FilePointerRefInfo
	{
		FileListInfo *Element;
		T *PointerKey;
		bool operator() (FilePointerRefInfo A,FilePointerRefInfo B) const
		{ return (A.PointerKey<B.PointerKey);		//ascending order
		}
	};

	mutable OpenThreads::Mutex m_BlockListAccess;
	typedef std::set<FileListInfo,FileListInfo> FileListSet;
	FileListSet m_FileList;
	//Find the element by using the pointer
	typedef std::set<FilePointerRefInfo,FilePointerRefInfo> PointerRefSet;
	PointerRefSet m_PointerRef;
};

template <class T>
SHARED_PTR<T> FileSharedPointer<T>::GetSharedPointer(const char *FileName,bool InitializeZerod)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> FunctionBlock(m_BlockListAccess);
	FileListInfo Key;
	Key.Init(FileName,InitializeZerod); //Note: if key exists the new pointer will be removed at end of function scope
	FileListSet::iterator Posn=m_FileList.find(Key);
	if (Posn==m_FileList.end())
	{
		//No entries yet... insert a new *copy*... which adds a reference to this new key pointer
		std::pair<FileListSet::iterator,bool> result=m_FileList.insert(Key);

		//Now also insert a pointer reference
		FilePointerRefInfo Reference;
		Reference.Element=&(*result.first);
		Reference.PointerKey=Reference.Element->m_SharedPointer.get();
		::InterlockedIncrement(&Reference.Element->m_Cnt);
		m_PointerRef.insert(Reference);

		ASSERT (result.second);  //TODO address this if this is really a problem

		return (*result.first).m_SharedPointer;
	}
	else
	{
		//entry found use that pointer (the temp key one will be deleted)
		FileListInfo &Entry=*Posn;
		::InterlockedIncrement(&Entry.m_Cnt);
		return Entry.m_SharedPointer;
	}
}

template <class T>
bool FileSharedPointer<T>::Exists(const char *FileName) const
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> FunctionBlock(m_BlockListAccess);
	FileListInfo Key;
	Key.Init(FileName); //Note: if key exists the new pointer will be removed at end of function scope
	FileListSet::const_iterator Posn=m_FileList.find(Key);
	return (Posn!=m_FileList.end());
}

template <class T>
void FileSharedPointer<T>::Release_Internal(const char *FileName,T *pPointer)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> FunctionBlock(m_BlockListAccess);
	class Internals
	{
		public:
			Internals(FileSharedPointer<T> * const parent) :  _(parent) {}

			bool GetPointerRefIterator(T *pPointer,PointerRefSet::iterator &Posn)
			{
				FilePointerRefInfo Key;
				Key.PointerKey=pPointer;
				Posn=_->m_PointerRef.find(Key);
				return Posn!=_->m_PointerRef.end();
			}
		
			bool GetFileListIterator(const char *FileName,FileListSet::iterator &Posn)
			{
				FileListInfo Key;
				Key.Init(FileName); //Note: if key exists the new pointer will be removed at end of function scope
				Posn=_->m_FileList.find(Key);
				return Posn!=_->m_FileList.end();
			}
		private:
			FileSharedPointer<T> * const _;
	}_(this);

	ASSERT(FileName||pPointer);
	PointerRefSet::iterator PointerRefTable_Posn;
	if (pPointer)
	{
		if (!FileName)
		{
			if (_.GetPointerRefIterator(pPointer,PointerRefTable_Posn))
			{
				FilePointerRefInfo &Entry=*PointerRefTable_Posn;
				//Now will just grab the filename
				FileName=Entry.Element->FileName.c_str();
			}
		}
	}

	if (FileName)
	{
		FileListSet::iterator FileInfoPosn;

		if (_.GetFileListIterator(FileName,FileInfoPosn))
		{
			FileListInfo &Entry=*FileInfoPosn;
			bool GotPointerRefIterator=(pPointer!=NULL);
			if (!GotPointerRefIterator)
			{
				pPointer=Entry.m_SharedPointer.get();
				//This line is why those are functors.... ;)
				GotPointerRefIterator=_.GetPointerRefIterator(pPointer,PointerRefTable_Posn);
			}
			if (::InterlockedDecrement(&Entry.m_Cnt)==0 )
			{
				//No more references found... remove file entry from list
				m_FileList.erase(FileInfoPosn);
				if (GotPointerRefIterator)
					m_PointerRef.erase(PointerRefTable_Posn);
			}
		}
	}
}


template <class T>
void FileSharedPointer<T>::Release(const char *FileName)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> FunctionBlock(m_BlockListAccess);
	Release_Internal(FileName);
}


template <class T>
void FileSharedPointer<T>::Release(T *pPointer)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> FunctionBlock(m_BlockListAccess);
	Release_Internal(NULL,pPointer);
}

template <class T>
OpenThreads::Mutex *FileSharedPointer<T>::GetFileVariableBlock(const char *FileName)
{
	OpenThreads::ScopedLock<OpenThreads::Mutex> FunctionBlock(m_BlockListAccess);
	FileListInfo Key;
	Key.Init(FileName); //Note: if key exists the new pointer will be removed at end of function scope
	FileListSet::iterator Posn=m_FileList.find(Key);
	if (Posn==m_FileList.end())
	{
		ASSERT(false);
		throw("GetFileVariableBlock failed");
	}
	else
	{
		//entry found use that pointer (the temp key one will be deleted)
		FileListInfo &Entry=*Posn;
		return Entry.m_SharedPointer->getRefMutex();
	}
}



}		//end namespace UtilityCode
