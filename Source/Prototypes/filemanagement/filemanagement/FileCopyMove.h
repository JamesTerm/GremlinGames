/*!	FileCopyMove.h
	Handles copying/moving files on another thread.  Also displays a dialog box w/ a progress bar.
	Mike Watkins
	NewTek
	10/30/2002
*/

#ifndef __FILECOPYMOVE_H__
#define __FILECOPYMOVE_H__

////////////////////////////////////////////////////////////////////////////////////////////////

namespace FileCopyMove_Globals
{
	FileManagementDLL bool IsInFolder(const char* szFolderPath, const char* szTestPath, char* szRelativePath = NULL);
	FileManagementDLL void GetParentDirectoryName(const char* szPath, char szDirName[]);
	FileManagementDLL bool IsFolderEmpty(const char* szPath, bool bIgnoreEmptyNewTekInfoFolder=false);
};

////////////////////////////////////////////////////////////////////////////////////////////////

/*!	Thread derived class that handles the file copy/move and updates the gas gauge dynamic
	variable that the corresponding progress bar is listening to
*/
//!	AutoSkinControl derived class that implements the dialog containing the progress bar
class FileProperties;
enum FileProperties_Status;

class FileManagementDLL FileCopyMoveHandler : public AutoSkinControl, public WorkerThread
{	
	//! Interface...

public:
	FileCopyMoveHandler();
	virtual ~FileCopyMoveHandler();

	static void StartAsyncCopy(const tList<FileProperties*> &sourceFilesList, const char* szDestPath, bool bMove);

	//! Callback that updates the gas gauge
	static DWORD CALLBACK CopyProgressRoutine(
												LARGE_INTEGER TotalFileSize,          // file size
												LARGE_INTEGER TotalBytesTransferred,  // bytes transferred
												LARGE_INTEGER StreamSize,             // bytes in stream
												LARGE_INTEGER StreamBytesTransferred, // bytes transferred for stream
												DWORD dwStreamNumber,                 // current stream
												DWORD dwCallbackReason,               // callback reason
												HANDLE hSourceFile,                   // handle to source file
												HANDLE hDestinationFile,              // handle to destination file
												LPVOID lpData                         // from CopyFileEx
												);

	//! Performs file copies on a separate thread
	virtual void ThreadProcessor();

	//deferred msgs sent by worker thread to notify dialog that a file has completed copying/moving
	virtual void ReceiveDeferredMessage(unsigned ID1, unsigned ID2);

	//used to show the UI and update the progressbar
	virtual void OnTimer(unsigned long TimerID);

	virtual void ClosingWindow();

	//! Implementation...

protected:

	//! Facilitator functions...

	void InitCopy(const tList<FileProperties*> &sourceFilesList, const char* szDestPath, bool bMove);
	
	virtual void InitialiseWindow();	//! Window Initialisation routine
	virtual void DestroyWindow();		//! Window Destruction routine

	void EnumerateFoldersAndFiles(FileProperties* sourceFileProp, const char* szDestPath);

	inline BOOL GetCancelCopyFlagVal() const;
	inline void SetCancelCopyFlagVal(BOOL bVal);

	bool PrepareNextCopy();
	unsigned SkipAllSourceSubPaths(const char* szFolderPath, unsigned listIdx, unsigned &numFilesSkipped);
	

	inline bool VerifyCopyFile(char* szSourcePath, char* szDestPath, FileProperties_Status fileType);
	inline void FinishedCopyFile(unsigned listIdx);
	void CheckFinishedFolderCopy(unsigned listIdx);

	//! Thread syncronization functions
	void SignalNextCopyReady();
	bool WaitNextCopyReady(char szSourcePath[], char szDestPath[]);
	void SignalCopyFinished(BOOL bSuccess);

	void FinishUp();

	//! Data...

	//! Copy/Move functionality data...

	bool m_bMove;
	std::stack<char*> m_SourceFoldersBeingCopied;
	std::stack<char*> m_DestFoldersBeingCopied;
	
	tList<char*> m_SourceFoldersAndFilesList;
	tList<char*> m_DestFoldersAndFilesList;
	Locked<unsigned> m_CurrFoldersAndFilesListIdx;
	LONG m_NextCopyReadyLock;	//! Thread syncronization lock

	bool m_bReplaceAll;
	bool m_bReadOnlyMoveAll;
	
	BOOL m_bCancelCopyFlag;
	mutable VariableBlock m_CancelCopyFlagLock;

	unsigned m_cAtomicOps;
	Locked<unsigned> m_currAtomicOpNum;
	Locked<double> m_PercentageProcessed;	//! For gas gauge

	Locked<DWORD> m_CopyErrorCode;

	//! UI data...

	Auto_StretchySkin *m_ScreenObject_StretchySkin;	//! The skin
	
	UtilLib_GasGauge *m_UtilLib_GasGauge;	//! The gas gauge
	UtilLib_TextItem *m_UtilLib_FileNameTextItem;	//! The text item that displays the filename being copied
	UtilLib_TextItem *m_UtilLib_SourceDirTextItem;	//! Text item that displays source dir of current copy
	UtilLib_TextItem *m_UtilLib_DestDirTextItem;	//! Text item that displays dest dir of current copy

private:

	//! Disallowed

	FileCopyMoveHandler(const FileCopyMoveHandler& other) { }
	void operator = (const FileCopyMoveHandler& other) { }
};

#endif __FILECOPYMOVE_H__