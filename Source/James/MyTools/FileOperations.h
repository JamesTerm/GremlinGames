#ifndef __ToolsFileOperations__
#define	__ToolsFileOperations__

extern HANDLE OpenReadSeq(const char *filename);
extern HANDLE OpenWriteSeq(const char *filename);
extern HANDLE OpenReadWriteSeq(const char *filename);

extern __int64 mySeek64 (HANDLE hf,__int64 distance,DWORD MoveMethod);
extern __int64 myTell64 (HANDLE hf);

extern DWORD myTell (HANDLE hf);
extern DWORD mySeek (HANDLE hf,long distance,DWORD MoveMethod);
 
extern int myRead(HANDLE hf,void *buf,DWORD count);
extern int myWrite(HANDLE hf,void *buf,DWORD count);
extern void Writef(HANDLE hf,const char *format, ... );
extern int myClose(HANDLE hf);



class ToolsFileOperations 
{
	public:
		ToolsFileOperations() {m_hf=NULL;}
		~ToolsFileOperations() {Close();}
		bool OpenRead (char *filename);
		bool OpenWrite(char *filename);
		DWORD Tell() {return myTell(m_hf);}
		__int64 Tell64() {return myTell64(m_hf);}
		DWORD Seek(long distance,DWORD MoveMethod) {return mySeek(m_hf,distance,MoveMethod);}
		__int64 Seek64(__int64 distance,DWORD MoveMethod) {return mySeek64(m_hf,distance,MoveMethod);}
		int Read(void *buf,DWORD count) {return myRead(m_hf,buf,count);}
		int Write(void *buf,DWORD count) {return myWrite(m_hf,buf,count);}
		void Writef(const char *format, ... );
		int Close() {int ret=0;if (m_hf) {ret=myClose(m_hf);m_hf=NULL;} return ret;}
		HANDLE GetHandle() {return m_hf;}
	private:
		HANDLE m_hf;
};

#endif	__ToolsFileOperations__