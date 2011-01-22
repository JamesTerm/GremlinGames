#include "stdafx.h"

/*	FileCollection
Keeps track of a list of files.  Keeps track of a composite table.  
Can Make all of the changes to the files.  Passes on changes made in other places.
Deals with one FilePropertiesDlg.
class FilePropertiesDlg;
class AssetManagementDLL FileCollection : public DynamicListener
{
	friend FilePropertiesDlg;
	friend SharedElement;
protected:
	tList<FileProperties*>		m_assetLists;
	tList<SharedElement*>		m_compositeTable;

	// Here is my own dialog, so I can notify when I am done.
	FilePropertiesDlg*	m_parent;
};
*/
void FileCollection::AddFile(FileProperties* p_newFile)
{
	if (!p_newFile) return;
	if (!m_assetLists.Exists(p_newFile))
	{
		p_newFile->GetHandle();
		p_newFile->AddToDialog();
		AddList(p_newFile);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
FileCollection::FileCollection(tList<FileProperties*>* p_fileNames, FilePropertiesDlg* p_parent, AssetListLayout* p_layout)
: AssetCollection(p_parent, p_layout)
{
	if (!p_fileNames) throw "FileCollection::FileCollection() Bad pointer to p_fileNames";
	if (!p_parent) throw "FileCollection::FileCollection() Bad pointer to p_parent";
	for (unsigned long i = 0; i < p_fileNames->NoItems(); i++)
		AddFile((*p_fileNames)[i]);
	m_parent = p_parent;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
FileCollection::~FileCollection()
{
	for (unsigned long i = 0; i < m_assetLists.NoItems(); i++)
	{
		FileProperties* thisFile = GetInterface<FileProperties>(m_assetLists[i]);
		if (thisFile)
		{
			m_listening[i] = false;
			thisFile->DeleteDependant(this);
			thisFile->RemoveFromDialog();
			FileProperties::ReleaseHandle(thisFile);
			m_assetLists[i] = NULL;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
AssetList* FileCollection::DeleteEntry(unsigned long p_index)
{
	FileProperties* thisFile = GetInterface<FileProperties>(AssetCollection::DeleteEntry(p_index));
	if (thisFile)
	{
		// This file is no longer in my dialog
		thisFile->RemoveFromDialog();
		thisFile->ReleaseHandle();

		if ((this->GetNumInCollection() < 2) || (thisFile == m_parent->m_currentFile))
		{
			FileProperties* editThis = GetInterface<FileProperties>(this->GetFirstList());
			if (editThis)
				m_parent->EditNewFile( editThis->GetFileName() );
			else
				m_parent->EditNewFile(NULL);
		}
	}
	return thisFile;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void FileCollection::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	// Don't do anything if I am making the changes myself.
	if (m_amIChanging) return;

	int index = m_assetLists.GetPosition((AssetList*)ID);
	if (!m_listening[index]) return;

	// I am only looking to see if a file has been deleted, or properties are added or deleted
	if (strcmp(String, FileProperties_Changed_FileDeletedInfo) == 0)
		DeleteEntry(index);
	else AssetCollection::DynamicCallback(ID,String,args,ItemChanging);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileCollection::SetGroupPath(const char* p_path)
{
	if (!p_path) return;
	for (unsigned long i = 0; i < m_assetLists.NoItems(); i++)
	{
		FileProperties* thisFile = GetInterface<FileProperties>(m_assetLists[i]);
		if (thisFile)
			thisFile->FileChangePath(p_path);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileCollection::SetGroupFilename(const char* szFilename)
{
	// Check the FN
	if (!szFilename || !szFilename[0]) return;
	
	// Make a copy of the filename to work with, so Andrew doesn't pick on me about modifying in place :)
	char buffer[MAX_PATH] = { 0 };
	strcpy(buffer, szFilename);
	char *p_filename = buffer;

	// Get how many files are being renamed
	unsigned numFiles = m_assetLists.NoItems();

	// How many hash marks do we need?
	int numHashNeeded = (int)(log10((float)numFiles)) + 1;

	// How many hash marks are there?
	char* firstHash = strchr(p_filename, '#');
	const char* lastDot = NewTek_GetLastDot(p_filename);
	char* nextChar = firstHash;
	char format[MAX_PATH];
	if (firstHash)
	{
		while (nextChar && *nextChar && (*nextChar == '#'))
			nextChar++;
		if ((nextChar-firstHash) > numHashNeeded)
			numHashNeeded = nextChar-firstHash;
		*firstHash = 0;
		sprintf(format, "%s%%0.%id%s", p_filename, numHashNeeded, nextChar);
		*firstHash = '#';
	}
	
	for (int i = 0; i < numFiles; i++)
	{
		char newName[MAX_PATH];
		if (firstHash)
			sprintf(newName, format, i);
		else strcpy(newName, p_filename);

		FileProperties* thisFile = GetInterface<FileProperties>(m_assetLists[i]);
		if (thisFile)
			thisFile->FileChangeFilename(newName, (firstHash == NULL));
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileCollection::SetGroupFullPath(const char* p_fullPath)
{
	if (!p_fullPath) return;
	for (unsigned long i = 0; i < m_assetLists.NoItems(); i++)
	{
		char* newName = new char[strlen(p_fullPath+1)];
		strcpy(newName, p_fullPath);
		// Find out how many * there are
		char* firstHash = strchr(newName, '#');
		if (firstHash)
		{
			// find out how many hash symbols
			long numHash = 1;
			while(*(firstHash+numHash) == '#')
				numHash++;

			// replace each hash with the a number
			for (long thisPos = 0; thisPos < numHash; thisPos++)
			{
				long divisor = (long)pow(10.0, (double)(numHash-thisPos-1));
				long thisDig = i / divisor;
				thisDig = thisDig % 10;	// To get the single digit
				*firstHash = (thisDig + '0');
				firstHash++;
			}
		}
		FileProperties* thisFile = GetInterface<FileProperties>(m_assetLists[i]);
		if (thisFile)
			thisFile->FileMoveFile(newName);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileCollection::SetGroupAltName(const char* p_altName)
{
	if (!p_altName) return;
	for (unsigned long i = 0; i < m_assetLists.NoItems(); i++)
	{
		FileProperties* thisFile = GetInterface<FileProperties>(m_assetLists[i]);
		if (thisFile)
			thisFile->FileChangeAltName(p_altName);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void	FileCollection::RemoveFile(FileProperties* p_oldFile)
{
	if (!p_oldFile) return;
	p_oldFile->RemoveFromDialog();
	int index = m_assetLists.GetPosition(p_oldFile);
	if (index >= 0)
	{
		this->DeleteEntry(index);
		p_oldFile->ReleaseHandle();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
