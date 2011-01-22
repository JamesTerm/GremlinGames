class FileCollection;

#ifndef __FileCollectionH_
#define __FileCollectionH_

/*	FileCollection
Keeps track of a list of files.  Keeps track of a composite table.  
Can Make all of the changes to the files.  Passes on changes made in other places.
Deals with one FilePropertiesDlg.
*/
class FilePropertiesDlg;
class FileManagementDLL FileCollection : public AssetCollection
{
	friend FilePropertiesDlg;
protected:
	//! Here is my own dialog, so I can notify when I am done.
	FilePropertiesDlg*			m_parent;

	virtual AssetList* DeleteEntry(unsigned long p_index);

public:
	FileCollection(tList<FileProperties*>* p_fileNames, FilePropertiesDlg* p_parent, AssetListLayout* p_layout);
	~FileCollection();

	//! Make the changes to the group of files
	void	SetGroupPath(const char* p_path);
	void	SetGroupFilename(const char* p_filename);
	void	SetGroupFullPath(const char* p_fullPath);
	void	SetGroupAltName(const char* p_altName);

	//! Add and remove files from my list
	void	AddFile(FileProperties* p_newFile);
	void	RemoveFile(FileProperties* p_oldFile);

	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);
};


#endif //! #ifndef __FileCollectionH_