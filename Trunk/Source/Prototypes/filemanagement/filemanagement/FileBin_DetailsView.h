#ifndef __FileBin_DetailsViewH_
#define __FileBin_DetailsViewH_

//! The details ViewMode may hold the screen objects used for drawing check boxes
enum eDetails_ViewMode_Checkbox
{
	eDetails_ViewMode_Checkbox_False,
	eDetails_ViewMode_Checkbox_True,
	eDetails_ViewMode_Checkbox_Invalid,
};

//! The Special View Mode used by DirectoryLayout to have the postential of showing Details View
class FileManagementDLL Details_ViewMode : public HeightWidth_ViewMode
{
public:	// Member functions
	// Set up the Details View Info
	void DVM_SetDetailsInfo(DetailsView_Info* detailsInfo);
	DetailsView_Info* DVM_GetDetailsInfo(){return m_DetailsView_Info;}
	

public:	// SDK Callbacks
	Details_ViewMode();
	virtual ~Details_ViewMode();

	// Listen for changes in the Info for Sorting
	virtual void DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging);

	//! Drawing the FileButton
	virtual void FileButton_ViewMode_Draw
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect);
	
	//! Base Small Icon and Name only widths on the currently selected column
	virtual long FileButton_ViewMode_GetPreferedXSize(FileButton* fb = NULL);

	// The proper column at the point for Details View
	virtual char*	FileButton_ViewMode_GetColumnAtPoint(POINT point, FileButton* fb);
	virtual bool	FileButton_ViewMode_GetEditingRect(RECT& rect,const char *columnName, FileButton* fb);
	virtual char*		FileButton_ViewMode_GetColumnAtIndex(unsigned index, FileButton* fb);
	virtual unsigned	FileButton_ViewMode_GetColumnIndex(const char* columnName, FileButton* fb);
	virtual unsigned	FileButton_ViewMode_GetNumColumns(FileButton* fb);

	ScreenObject* DVM_GetCheckBoxScreenObject(eDetails_ViewMode_Checkbox check)
		{return m_checkBoxScreenObjects[check];}

	//! Set the screen objects the check box will use
	void DVM_SetCheckBoxScreenObject(eDetails_ViewMode_Checkbox check, ScreenObject* so);

protected:	// Member Variables
	// Keep a list of all of the column widths
	tList<long>	m_columnPositions;
	DetailsView_Info* m_DetailsView_Info;
	tList<DetailsView_ColumnInfo*>	m_viewColumnList;
	
	// Make the text item italic if we are missing the reference
	virtual void TweakTextItem(FileButton* fileButton, TextItem* textItem, bool missingRef);
	
	// These work together for finding the preferred column size
	int	m_findThisColsSizeNextRePaint;
	long m_maxColumnSize;

	// Keep track of the last size I drew a FileButton at
	long m_lastFileButtonWidth;

	// Check Box Screen Objects
	ScreenObject*	m_checkBoxScreenObjects[3];
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum BasicFile_DVElem_Enum
{
	BasicFile_DVElem_AltName,
	BasicFile_DVElem_NameOnly,
	BasicFile_DVElem_FullName,
	BasicFile_DVElem_ExtOnly,
	BasicFile_DVElem_Path,
	BasicFile_DVElem_PathOnly,
	BasicFile_DVElem_Type,

	BasicFile_DVElem_NumItems,
};

class FileManagementDLL DVElem_TypeStruct
{
public:
	char* name;
	long min, max, init;
};

const DVElem_TypeStruct BasicFile_DVElem_TypeArray[] =
{
	{FileButton_Naming_AltName, 100, -1, 150},
	{FileButton_Naming_NameOnly, 100, -1, 150},
	{FileButton_Naming_FullName, 100, -1, 150},
	{FileButton_Naming_ExtOnly, 50, -1, 50},
	{FileButton_Naming_Path, 100, -1, 300},
	{FileButton_Naming_PathOnly, 100, -1, 300},
	{FileButton_Naming_Type, 50, -1, 100}
};

//! DetailsView_Element classes dealing with basic File Management
class FileManagementDLL BaseFile_DetailsView_Element : public UtilLib_DetailsView_Element
{
public:
	BaseFile_DetailsView_Element(FileButton* fb, unsigned type);

	virtual unsigned DVE_GetTextString(char *DestBuffer,unsigned LengthOfBuffer) {return DVE_GetTextString(m_type,DestBuffer,LengthOfBuffer);}
	virtual unsigned DVE_GetTextString(unsigned type,char *DestBuffer,unsigned LengthOfBuffer);

	virtual bool	DVE_CanThisElementBeRemoved(){return false;}
	virtual bool	DVE_AllowPopupEdit();
	
	unsigned	BFDVE_GetType(){return m_type;}
	FileButton* BFDVE_GetFileButton(){return m_fileButton;}

	/*!	This is called when the user clicks in or tabs into the RECT, a click will be provided with mouseRelease
		hWnd is provided as the parent window
		hdc is provided if you only want to draw on the control rather than create a new control
		inRect is the rect where this paerticular element is found in hWnd
		set copyToSelectedSisterWindows if this element should be copied to other similarly selected rows after editing */
	virtual void	DVE_PopupEdit
		(BaseWindowClass* parent, RECT& inRect, bool& allowCopyToSelectedSisterWindows, long& reasonForExit, 
		const char *InitString=NULL,POINT* mouseRelease = NULL, bool HighLightInitString=true);

protected:
	FileButton*		m_fileButton;
	unsigned		m_type;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif	// __FileBin_DetailsViewH_
