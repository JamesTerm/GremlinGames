#include "stdafx.h"

FileButton_PreviewInterface				*FileButton::m_FileButton_PreviewInterface=NULL;
FileButton_MouseOverPreviewInterface	*FileButton::m_FileButton_MouseOverPreviewInterface=NULL;
char FileButton::filePath[MAX_PATH];

////////////////////////////////////////////////
void FileButton::DragAndDrop_Started(void)
{	m_BeingDragged = true;
	StopRolloverAnimation();
}

void FileButton::DragAndDrop_Finished(void)
{	m_BeingDragged = false;
	// Figure out the FileButton Parent
	SetFileButtonParent(GetWindowInterface<FileButton_ViewMode_Parent>(GetParent()));
}

RECT FileButton::GetRolloverAnimationPosition(void)
{
	return m_lastIconRect;
}

void FileButton::StartRolloverAnimation(void)
{	// Never start if we are being dragged
	if (m_BeingDragged) return;
	
	// Is there an animation file ?
	char AnimationFile[MAX_PATH];
	FileProperties::GetAnimatedIconFromFileName(GetFileName(),AnimationFile);
	if (NewTek_DoesFileExist(AnimationFile))
	{	// Use the animation as the file
		if (m_FileButton_MouseOverPreviewInterface)
				m_FileButton_MouseOverPreviewInterface->GeneratePreview
					(AnimationFile,this,GetRolloverAnimationPosition());
	}
	else if (GetStatus() == FileProperties_File)
	{	// Use the animation as the file
		if (m_FileButton_MouseOverPreviewInterface)
				m_FileButton_MouseOverPreviewInterface->GeneratePreview
					(GetFileName(),this,GetRolloverAnimationPosition());
	}
}

void FileButton::StopRolloverAnimation(bool WaitUntilStopped)
{	// I am no longe being previewed
	if (m_FileButton_MouseOverPreviewInterface)
	{	m_FileButton_MouseOverPreviewInterface->RemoveAllPreviews(this);
		if (WaitUntilStopped)	m_FileButton_MouseOverPreviewInterface->GeneratePreview
										(NULL,NULL,GetRolloverAnimationPosition());
	}
}

int FileButton::FileButton_MouseLButtonDblClick_compare( const void *arg1, const void *arg2 )
{	FileButton **Item1=(FileButton**)arg1;
	FileButton **Item2=(FileButton**)arg2;

	if ((*Item1)->DragNDrop_SelectedBefore(*Item2)) return -1;
	return 1;
}

void FileButton::MouseRButtonClick(long Flags,long x,long y)
{	StopRolloverAnimation(true);
	UtilLib_DragButton::MouseRButtonClick(Flags,x,y);
}

bool FileButton::FileButton_ShouldIDisplayAnimations(void)
{	// If the viewmode exludes animated icons !
	if (!m_currentViewMode->FileButton_ViewMode_IsAnimated(this)) return false;
	if (m_properties && m_properties->m_propsGenerated && m_properties->m_infoExists)
	{
		AssetElement* assetElement = NULL;
		bool newAsset = !m_properties->AddAsset(assetElement, "Animate Icon", "Boolean", true, false);
		if (assetElement)
		{
			AssetBool* animateIcon = GetInterface<AssetBool>(assetElement->GetProperty());
			if (animateIcon)
			{
				if (newAsset)
					animateIcon->Set(true);
				else if (!animateIcon->Get())
					return false;
			}
		}
	}

	// The file seems to think it is ok to animate the icon, check to be sure it is not the BLS of a TEd TimeLine project
	const char* filename = this->GetFileName();
	if (filename)
	{
		const char* lastDot = NewTek_GetLastDot((char*)filename);
		if	(lastDot && 
				(	( !stricmp(lastDot, ".ToasterEdit Project") ) ||
					( !stricmp(lastDot, ".VT-Edit Project") ) ||
					( !stricmp(lastDot, TLF_BASE_FILEEXT ) )					
				)
			)
		{
			// We have a Toaster Edit project, Check preferences to see if we should animate them
			if (!Preferences::GetValue_Bool("Rollover Previews for SpeedEDIT Projects", false, "Enabled","Disabled"))
				return false;
		}
	}

	// Nothis says we shouldn't
	return true;
}

void FileButton::MouseEnter(void)
{	// We put it up in the priority chain on output rendering
	if ((m_waitingForImage)&&(m_properties)) 
				ThreadReadIconFile(m_properties, m_waitingForImage);
	// If we are allowed to display animations
	if (FileButton_ShouldIDisplayAnimations())
		StartRolloverAnimation();	

	// Call my predecessor
	UtilLib_DragButton::MouseEnter();
}

void FileButton::MouseExit(void)
{	// Stop the Animations and the Timers
	StopRolloverAnimation();
	// Call my predecessor
	UtilLib_DragButton::MouseExit();
}

void FileButton::MouseLButtonDblClick(long Flags,long x,long y)
{
	if (!m_parent || m_parent->ShouldFileButtonExecuteOnDoubleClick(this)) {
		ExecuteFile();
		}
}
void FileButton::GrabSelectedButtons(tList<FileButton*>& Windows)
{
	// Get the list of parent windows	
	BaseWindowClass *BWC=GetParentBWC();

	// Sort the items into selection order and execute them
	if (BWC)
	{
		tList<HWND> ChildrenList;
		BWC->GetChildren(ChildrenList);

		for(unsigned i=0;i<ChildrenList.NoItems();i++)
		{	FileButton *FB=GetWindowInterface<FileButton>(ChildrenList[i]);
			if ((FB)&&(FB->DragAndDrop_AmISelected()) && (::IsWindowVisible(FB->GetWindowHandle())))
				Windows.Add(FB);
		}

		if (Windows.NoItems())
				qsort(	&Windows[0],
						Windows.NoItems(),
						sizeof(FileButton*),
						FileButton_MouseLButtonDblClick_compare);
	}
}
void FileButton::ExecuteFile()
{
	tList<FileButton*> Windows;
	GrabSelectedButtons(Windows);
	tList<const char*> WindowNames;
	for(unsigned i=0;i<Windows.NoItems();i++)
	{
		const char* fn = Windows[i]->GetFileName();
		if (fn && fn[0])
			WindowNames.Add(fn);
	}
	WindowNames.Add(NULL);

	NewTek_FileExecute( GetWindowHandle() , (char**)&WindowNames[0] );
}

void FileButton::MouseRButtonRelease(long Flags,long x,long y)
{
	// Remember the point where the Context Menu is clicked
	m_contextMenuClickPoint.x = x;
	m_contextMenuClickPoint.y = y;

	// Here is where we do the Context Menu thing
	BaseWindowClass *BWC=GetParentBWC();

	tList<BaseWindowClass*>	Windows;
	PathLayout* pl = GetInterface<PathLayout>(BWC);

	// Sort the items into selection order and execute them
	if (BWC && !pl)
	{	
		if (!DragAndDrop_AmISelected())
		{
			tList<HWND> ChildrenList;
			BWC->GetChildren(ChildrenList);
			for(unsigned i=0;i<ChildrenList.NoItems();i++)
			{	FileButton *FB=GetWindowInterface<FileButton>(ChildrenList[i]);
				if ((FB)&&(FB->DragAndDrop_AmISelected())&&(FB->IsWindowVisible()))
					FB->DragAndDrop_SelectMe(false);
			}
			DragAndDrop_SelectMe(true);
			Windows.Add(this);
		}
		else
		{
			tList<HWND> ChildrenList;
			BWC->GetChildren(ChildrenList);
			for(unsigned i=0;i<ChildrenList.NoItems();i++)
			{	FileButton *FB=GetWindowInterface<FileButton>(ChildrenList[i]);
				if ((FB)&&(FB->DragAndDrop_AmISelected())&&(FB->IsWindowVisible()))
					Windows.Add(FB);
			}
		}

		qsort(&Windows[0],Windows.NoItems(),sizeof(FileButton*),FileButton_MouseLButtonDblClick_compare);
	}
	else Windows.Add(this);

	unsigned OldNoPopups=BWC_GetNoPopupWindows();
	NewTek_ExecuteContextMenu(&Windows, this);
	// We do this so that KB shortcut focus is returned to myself after the menu closes
	if (OldNoPopups==BWC_GetNoPopupWindows())
			SetKBFocus();

	// If we are told to edit a column from the Context Menu, do it now
	if (m_startEditingColumnAfterContextMenu)
	{
		FB_DVR_StartEditingColumn(m_startEditingColumnAfterContextMenu);
		NewTek_free(m_startEditingColumnAfterContextMenu);
		m_startEditingColumnAfterContextMenu = NULL;
	}
}

////////////////////////////////////////////////
// Constructors & Destructors
FileButton::FileButton() : m_AmIPainting(false)
{	m_BeingDragged = false;
	m_Ref = 1;
	m_iconLoaded = false;
	m_properties = NULL;
	m_currentViewMode = NULL;
	m_parent = NULL;
	m_repaintMe = true;
	m_haveIbeenSet = false;
	m_waitingForImage = NULL;
	FB_SetThreadReadStatus(FileButton_ThreadRead_Unknown);
	m_lastIconRect.left = m_lastIconRect.right = m_lastIconRect.top = m_lastIconRect.bottom = 0;

	// Create the base file Details View Elements
	for (unsigned i = 0; i < BasicFile_DVElem_NumItems; i++)
		m_baseFile_DetailsView_Elements[i] = new BaseFile_DetailsView_Element(this, i);

	m_icon.AddDependant(this, (long)(&m_icon));
	m_icon.SetAlignment(BitmapTile_StretchX|BitmapTile_StretchY);

	// I need to listen to myself for DragNDropContext_DragStarted messages
	AddDependant(this, (long)this);
}
//////////////////////////////////////////////////////////////////////////////////////////
FileButton::~FileButton()
{	// Delete my resources
	if (m_currentViewMode)
		m_currentViewMode->DeleteDependant(this);

	if (m_waitingForImage)
		delete[] m_waitingForImage;
	m_waitingForImage = NULL;

	// Delete the base file Details View Elements
	for (unsigned i = 0; i < BasicFile_DVElem_NumItems; i++)
		delete m_baseFile_DetailsView_Elements[i];

	if (m_properties)
	{
		m_properties->DeleteDependant(this);
		m_properties->m_interestedButtons.Delete(this);
		FileProperties::ReleaseHandle(m_properties);
	}
	m_icon.DeleteDependant(this);
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::PaintWindow(HWND hWnd,HDC hdc)
{
	//This bool is used to avoid RePaint recursion
	m_AmIPainting=true;

	// Load the icon if it has not been loaded yet
	if ((!m_iconLoaded)&&(GetProperties()))
	{	FB_LoadIcon(GetProperties());
		m_iconLoaded=true;
	}

	// Determine what state we are in
	int State=Controls_Button_UnSelected;
	if(Button_IsPressed()) State=Controls_Button_Selected;	

	if (RollOver)
	{	if (State==Controls_Button_UnSelected)
				State=Controls_Button_MouseOver;
		else	State=Controls_Button_MouseOverDn;
	}
	LastButtonState=State;

	RECT rect;
	rect.left = rect.top = 0;
	rect.right = GetWindowWidth();
	rect.bottom = GetWindowHeight();
	if (m_currentViewMode)
		m_currentViewMode->FileButton_ViewMode_Draw(hWnd, hdc, this, State, ColorTinting_GetTintColor(), rect);
	else
		int breakHere = 1;

	m_AmIPainting=false;
}
/////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
// View Modes
long FileButton::GetStatus()
{
	if (m_properties)
		return m_properties->GetFileStatus();
	else return FileProperties_NoFile;
}
/////////////////////////////////////////////////////////////////////////////////////////
const char* FileButton::GetFileName(void)
{	if (m_properties) return m_properties->GetFileName();
	else return "";
}
//////////////////////////////////////////////////////////////////////////////////////////
const char* FileButton::GetDirectory(void)
{	if (!m_properties) return NULL;
	
	long status = m_properties->GetFileStatus();
	if ((status == FileProperties_File) || (status == FileProperties_Directory))
		return m_properties->GetParentDir();
	return "";
}
//////////////////////////////////////////////////////////////////////////////////////////
FileProperties*	FileButton::GetProperties(){return m_properties;}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::MoveFile(const char* p_newFileName)
{	StopRolloverAnimation(true);
	// Check for a bad pointer
	if (!p_newFileName || !m_properties) return false;

	if (!m_properties->FileMoveFile(p_newFileName))
		return false;
	ChangeFileName(m_properties->GetFileName());
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::MoveFileTo(const char* p_newDirectory)
{	StopRolloverAnimation(true);
	if (m_properties && p_newDirectory)
	{
		if (m_properties->FileChangePath(p_newDirectory))
		{
			ChangeFileName(m_properties->GetFileName());
			return true;
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::CopyFile(const char* p_newFileName)
{	StopRolloverAnimation(true);
	// Check for a bad pointer
	if (!p_newFileName || !m_properties) return false;

	if (!m_properties->FileCopyFile(p_newFileName))
		return false;
	ChangeFileName(m_properties->GetFileName());
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::CopyFileTo(const char* p_newDirectory)
{	StopRolloverAnimation(true);
	if (m_properties && p_newDirectory)
	{
		const char* nameOnly = NewTek_GetFileNameFromPath((char*)m_properties->GetFileName());
		if (nameOnly)
		{
			char newPath[MAX_PATH];
			sprintf(newPath, "%s\\%s", p_newDirectory, nameOnly);
			return CopyFile(newPath);
		}
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::ChangeFileName(const char* p_newFileName)
{
	if (!p_newFileName || !m_properties || (strcmp(GetFileName(), p_newFileName)))
	{
		FileProperties* newProps = FileProperties::GetHandle(p_newFileName);
		SetFileButtonProperties(newProps, false);
		StopRolloverAnimation();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::ShowWindow_Callback(long Flags)
{	StopRolloverAnimation();
	UtilLib_DragButton::ShowWindow_Callback(Flags);
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::SetFileButtonProperties(FileProperties* newProps, bool addReference, bool redraw)
{
	// Be sure they are not the same
	if (m_haveIbeenSet && (m_properties == newProps)) return;

	// Stop Thread Reading if we are
	FB_StopThreadReading();

	// I have been set now
	m_haveIbeenSet = true;

	// Only repaint if told to do so
	m_repaintMe = redraw;

	// Get rid of my old properties here
	FileProperties* oldProperties = m_properties;
	
	// Add to the reference count if told to do so
	if (addReference && newProps)
		newProps->GetHandle();

	// If I am not already in the list of buttons, make sure I am now
	if (newProps && !newProps->m_interestedButtons.Exists(this))
		newProps->m_interestedButtons.Add(this);

	// This is my new Props!!
	m_properties = newProps;
	if (m_properties)
		m_properties->AddDependant(this, (long)m_properties);

	// Look at this new file for the icon
	FB_SetThreadReadStatus(FileButton_ThreadRead_Unknown);
	//FB_LoadIcon(m_properties);
	m_iconLoaded=false;

	Changed(FileButton_Changed_FileName);
	if (m_properties)
		FileButton_FileNameChanged(m_properties->GetFileName());
	else FileButton_FileNameChanged(NULL);

	m_repaintMe = true;
	if (redraw)
		RePaintMe(true);

	if (oldProperties)
	{
		oldProperties->DeleteDependant(this);
		oldProperties->m_interestedButtons.Delete(this);
		FileProperties::ReleaseHandle(oldProperties);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
// Layout manager hints
long FileButton::LayoutHints_GetPreferedXSize(void)
{	
	return -1;
}
long FileButton::LayoutHints_GetPreferedYSize(void)
{	
	return -1;
}



void FileButton::MouseLButtonRelease(long Flags,long x,long y) {
	do {
		POINT mouse;
		mouse.x = x; mouse.y = y;
		const char* columnName = DVR_GetDetailsViewColumnName(mouse);
		DetailsView_Element* element = columnName ? DVR_GetDetailsViewElement(columnName) : NULL;
		const char *ElementName=element?element->DVE_GetColumnName():NULL;

		// Watch for single button executes
		if (m_parent) {
			if (m_parent->ShouldFileButtonExecuteOnSingleClick(this)) {
				ExecuteFile();
				DragAndDrop_SelectMe(false);
				break;
				}
			}

		//Only perform click in selection for Details view...
		FileButton_ViewMode *currentviewmode=GetCurrentViewMode();
		UtilLib_FileButton_ViewMode *ufbvm=GetInterface<UtilLib_FileButton_ViewMode>(currentviewmode);
		bool detailsVM=false;
		if (ufbvm) {
			if (ufbvm->FileButton_ViewMode_GetViewMode() & FileButton_ViewMode_Details)
				detailsVM=true;
			}

		if (!detailsVM) break;


		//click edit support will only be for EditOnFirstClick items only (i.e. toggle)
		if (element && element->DVE_EditOnFirstClick()) {

			FileButton_ViewMode* viewMode = GetCurrentViewMode();
			//we'll keep this row selectiong check in here just to be safe... (it should always be true)
			if (this->DragAndDrop_AmISelected()) {

				if (element->DVE_AllowPopupEdit()) {
					FB_DVR_StartEditingColumn(columnName, &mouse);
					break;
					}
				}
			}
		
		} while(false);
	}


bool FileButton::KeyboardShortCut_IsKeyboardConfigurationLocal(byte *m_KeyboardMap_256Elements) {
	bool ret=false;
	
	//Only perform type in selection for Details view...
	FileButton_ViewMode *currentviewmode=GetCurrentViewMode();
	UtilLib_FileButton_ViewMode *ufbvm=GetInterface<UtilLib_FileButton_ViewMode>(currentviewmode);
	bool detailsVM=false;
	if (ufbvm) {
		if (ufbvm->FileButton_ViewMode_GetViewMode() & FileButton_ViewMode_Details)
			detailsVM=true;
		}

	//Only intercept keystrokes if this is selected with details element selected
	if ((DragAndDrop_AmISelected())&&(detailsVM)) {
		char *ColumnName=GetDetailsColumnSelection();
		DetailsView_Element *element = ColumnName ? DVR_GetDetailsViewElement(ColumnName) : NULL;
		KeyboardShortCut_Selective_Interface *kscsi=GetInterface<KeyboardShortCut_Selective_Interface>(element);
		if (kscsi) {
			bool EnterEditMode;
			EnterEditMode=kscsi->KeyboardShortCut_IsKeyboardConfigurationLocal(m_KeyboardMap_256Elements);
			bool F2KeyPressed=(m_KeyboardMap_256Elements[VK_F2]!=0);
			bool SpacePressed=(m_KeyboardMap_256Elements[VK_SPACE]!=0);
			if (F2KeyPressed)
				EnterEditMode=true;
			bool ShiftKeyPressed=(m_KeyboardMap_256Elements[VK_SHIFT]!=0);
			bool LeftKeyPressed=(m_KeyboardMap_256Elements[VK_LEFT]!=0)||
				((m_KeyboardMap_256Elements[VK_TAB]!=0)&&(ShiftKeyPressed));
			bool RightKeyPressed=(m_KeyboardMap_256Elements[VK_RIGHT]!=0)||
				((m_KeyboardMap_256Elements[VK_TAB]!=0)&&(!ShiftKeyPressed));
			bool UpKeyPressed=(m_KeyboardMap_256Elements[VK_UP]!=0);
			bool DownKeyPressed=(m_KeyboardMap_256Elements[VK_DOWN]!=0);
			bool CTRLkeyPressed=(m_KeyboardMap_256Elements[VK_CONTROL]!=0);
			bool DeleteKeyPressed=(m_KeyboardMap_256Elements[VK_DELETE]!=0);
			bool ReturnKeyPressed=(m_KeyboardMap_256Elements[VK_RETURN]!=0);
			if (LeftKeyPressed||RightKeyPressed||UpKeyPressed||DownKeyPressed||CTRLkeyPressed||DeleteKeyPressed||ReturnKeyPressed)  {
				EnterEditMode=false;
				}

			if (EnterEditMode) {
				ret=true;  //We'll handle the key locally
				WORD TempBuf[4]= {0,0,0,0};
				const char *KeyPressed=(char *)TempBuf;
				if ((!F2KeyPressed)&&(!SpacePressed)) {
					//Translate the virtual key code to an unshifted character code
					if (ShiftKeyPressed) {
						//ensure this is shown as high bit set
						m_KeyboardMap_256Elements[VK_SHIFT]=0x80;
						}
					for(unsigned i=0x30;i<0xFF;i++) {
						if (m_KeyboardMap_256Elements[i]) {
							int KeyStatus=ToAscii(i,0,m_KeyboardMap_256Elements,TempBuf,0);
							if (KeyStatus) //This ensures we skip over the "dead keys" to get to the keys which count
								break;
							}
						}

					if (element->DVE_IsNumericEntry()) {
						if ((TempBuf[0]<'0')||(TempBuf[0]>'9')) {
							TempBuf[0]=0;
							ret=false;
							}
						}
					}
				else {
					if (F2KeyPressed) {
						 //For F2, This will implicitly have the editpopup use the pre-existing string
						KeyPressed=NULL;
						}
					if (SpacePressed) {
						//We basically check for space exclusively to avoid having to filter out the range from 0x21-0x2f
						TempBuf[0]=VK_SPACE;

						//TODO: This is a temporary fix... we should probably have the element specify whether it needs to use
						//the space for keyboard shortcuts... this fix will ensure mark in/out spacebar key works correctly
						if (element->DVE_IsNumericEntry()) {
							TempBuf[0]=0;
							ret=false;
							}

						}
					}

				//Note: Modifier keys will be ignored...
				//only F2 will enter into edit
				if ((TempBuf[0]!=0)||(F2KeyPressed))
					FB_DVR_StartEditingColumn(ColumnName,NULL,KeyPressed,F2KeyPressed);
				}
			else { //handle special keys here (Arrow keys)
				if (LeftKeyPressed||RightKeyPressed||UpKeyPressed||DownKeyPressed||ReturnKeyPressed) {
					ret=true; //We'll handle the key locally
					Popup_Escape direction=Popup_Escape_ForwardTab;
					if (LeftKeyPressed) {
						direction=Popup_Escape_BackwardsTab;
						}
					else if (UpKeyPressed) {
						direction=Popup_Escape_ArrowUp;
						}
					else if (DownKeyPressed) {
						direction=Popup_Escape_ArrowDown;
						}
					else if ((ReturnKeyPressed)&&(!ShiftKeyPressed)) {
						direction=Popup_Escape_ReturnKey;
						}
					else if ((ReturnKeyPressed)&&(ShiftKeyPressed)) {
						direction=Popup_Escape_ShiftReturnKey;
						}
					FB_SelectNextEditableElement(ColumnName,direction);
					}
				}
			}
		}
	return ret;
	}

//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::MouseLButtonClick(long Flags,long x,long y) {
	}

char *FileButton::GetDetailsColumnSelection() {
	char *ret=NULL;
	DetailsView_RowCollection* collection = GetWindowInterface<DetailsView_RowCollection>(this->GetParent());
	if (collection) {
		ret=collection->DVRC_GetDetailsColumnSelection();
		}
	return ret;
	}

void FileButton::SetDetailsColumnSelection(const char *ElementName,bool refresh) {
	DetailsView_RowCollection* collection = GetWindowInterface<DetailsView_RowCollection>(this->GetParent());
	if (collection) {
		collection->DVRC_SetDetailsColumnSelection(ElementName,refresh);
		}
	//if (refresh) RePaint();
	}


bool FileButton::DragNDrop_ShouldIDragAndDrop(HWND hWnd,bool ActualDragOperation,bool MouseOperation) {
	bool ret=true;
	// Watch for single click execution
	if (m_parent && m_parent->ShouldFileButtonExecuteOnSingleClick(this)) {
		DragAndDrop_SelectMe(true);
		ret=false;
		}

	//if we haven't taken over the DND operation yet here is one last check
	if (ret) {
		_CodeBlockStart;
			// Watch for clicking in a column to edit
			POINT mouse;
			GetCursorPos(&mouse);
			ScreenToClient(GetWindowHandle(), &mouse);
			char* columnName = DVR_GetDetailsViewColumnName(mouse);
			DetailsView_Element* element = columnName ? DVR_GetDetailsViewElement(columnName) : NULL;

			if (!(element && element->DVE_AllowPopupEdit() && MouseOperation && ActualDragOperation))
				_CodeBlockExit;
			const char *ElementName=element->DVE_GetColumnName();
			bool WasColumnSelected=true;

			if (!(GetDetailsColumnSelection()&&(ElementName))) {
				WasColumnSelected=false;
				}
			else {
				if (stricmp(GetDetailsColumnSelection(),ElementName))
					WasColumnSelected=false;
				}
				
			bool WasRowSelected=DragAndDrop_AmISelected();
			if (!WasRowSelected)
				WasColumnSelected=false;

			bool ShiftKeyPressed=(bool)(GetAsyncKeyState(VK_SHIFT)&((short)(1<<15)));
			bool ControlKeyPressed=(bool)(GetAsyncKeyState(VK_CONTROL)&((short)(1<<15)));
			if ((!ShiftKeyPressed)&&(!ControlKeyPressed)) {
				if (!WasRowSelected) {
					// De-Select all other children and select myself
					BaseWindowClass* parentBWC = GetParentBWC();
					if (parentBWC) {
						parentBWC->DragAndDrop_SelectAllChildren(false);
						}
					DragAndDrop_SelectMe(true);
					SetKBFocus();
					}
				}
			bool editonfirstclick=element->DVE_EditOnFirstClick();
			if ((!WasColumnSelected)||(editonfirstclick)) {
				if (editonfirstclick) {
					//this will give immediate control to edit... (these elements can not be used for dnd operations)
					ret=false; 
					}
				SetDetailsColumnSelection(ElementName,true);
				}
			else {
				ret=true;
				}
		_CodeBlockEnd;
		}
	// Otherwise, FileButtons are Drag Buttons that respond to DnD
	return ret;
	}



void FileButton::CopyTo(FileButton* p_CopyButton)
{
	p_CopyButton->ChangeFileName(GetFileName());
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{
	return CanItemBeDroppedOnProperties(GetProperties(), Dropped, this);
}
//////////////////////////////////////////////////////////////////////////////////////////
WindowLayout_Item* FileButton::DragNDrop_DropItemsHere
							(	int Width,int Height,				// Window size
								int MousePosnX,int MousePosnY,		// Mouse posn
								WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
								WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
								bool Resizing,bool DroppedHere
								)
{
	// Place all of the elements where they cannot be seen
	long newXPos = GetWindowWidth() + 100;
	long newYPos = GetWindowHeight() + 100;
		
	// We scan through all the items
	WindowLayout_Item *Scan=ObjectsDropped;
	while(Scan)
	{
		Scan->XPosn = newXPos;
		Scan->YPosn = newYPos;		

		// Look at the next item
		Scan=Scan->Next;
	}

	if (DroppedHere)
	{
		// Get a handle on my own dorectory
		FileProperties* fp = this->GetProperties();

		// Do the work of dropping it in to my directory
		DropOntoProperties(fp, ObjectsDropped, this);

		// Delete ALL Windows dropped in to me, they will be re-created
		WindowLayout_Item* scan = ObjectsDropped;
		while (scan)
		{
			BaseWindowClass* bwc = GetWindowInterface<BaseWindowClass>(scan->hWnd);
			NewTek_Delete(bwc);
			scan->hWnd = NULL;
			scan = scan->Next;
		}
	}

	// Drop them in the correct positions
	return Control_DragNDrop::DragNDrop_DropItemsHere(Width,Height,MousePosnX,MousePosnY,ObjectsInWindow,ObjectsDropped,Resizing,DroppedHere);

}
//////////////////////////////////////////////////////////////////////////////////////////

// Deferred callback
// This function works out the icon that is read on a seperate thread
void FileButton::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{
	if (ID1 == (unsigned)(&m_icon))
	{
		if (m_waitingForImage)
			delete[] m_waitingForImage;
		m_waitingForImage = NULL;

		// Pedantic
		if (m_properties)
		{	
#ifdef VT5_GENERATEFILEPROPERTIES_ON_THE_FLY

			// Try settomg the assets
			VideoToaster_WindowsMedia_File_Info	FileInfo;
			if (FileInfo.LoadInfo( (char*)m_properties->GetFileName() ))
				// Write the assets
				WriteVideoFileAssets( m_properties, &FileInfo );

#endif VT5_GENERATEFILEPROPERTIES_ON_THE_FLY
			
			if (ID2)
			{
				// Where to load the little version
				char autoName[MAX_PATH];
				FileProperties::GetIconFromFileName(m_properties->GetFileName(), autoName);
				//DebugOutput("Reading file into icon for: %s, %s\n", m_properties->GetFileName(), autoName);
				BitmapItem_Cache_Item_Update( autoName, false );
				m_icon.ReadBitmapFile(autoName);
				FB_SetThreadReadStatus(FileButton_ThreadRead_Completed);

				// I am pretty sure I need to RePaint.
				RePaint();

				// The asset management already reads true for being able to load the file
			}
			else
			{
				// We just need to set the asset to false
				//AssetElement* ae = m_properties->GetAsset("Loadable Into Editor");
				//DebugOutput("Failed to read icon: %s\n", m_properties->GetFileName());
				//if (ae)
				//{
				//	AssetBool* ab = GetInterface<AssetBool>(ae->GetProperty());
				//	if (ab) ab->Set(false);
				//}
				FB_SetThreadReadStatus(FileButton_ThreadRead_Invalid);
				RePaint();
			}
		}
	}
	else if (ID1 == FILEBUTTON_DEFERRED_EDIT)
	{
		// We are going to start editing a field
			FileButton_ViewMode* viewMode = GetCurrentViewMode();
		if (viewMode)
		{
			// Grab the column name at the index ID2 and start editing it
			char* columnName = viewMode->FileButton_ViewMode_GetColumnAtIndex(ID2, this);
			if (columnName)
				this->FB_DVR_StartEditingColumn(columnName, NULL);
		}
	}
	UtilLib_DragButton::ReceiveDeferredMessage(ID1,ID2);	
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	if (!(strcmp(String,BaseWindowClass_KeyDown))) {
		return;
		}
	if (ItemChanging == m_properties)
	{
		if (strcmp(String, FileProperties_Changed_AltName) == 0)
			RePaintMe(true);
		else if (!stricmp(String, FileProperties_Changed_MovedFile))
			RePaintMe(true);
		else if (!stricmp(String, AssetList_Changed_Properties))
			RePaintMe(true);
		else if (!stricmp(String, AssetList_Changed_ElementRemoved))
			RePaintMe(true);
		else if (!stricmp(String, AssetList_Changed_ElementAdded))
			RePaintMe(true);
		else if (strcmp(String, FileProperties_Changed_Cut) == 0)
			RePaintMe(false);
		else if (strcmp(String, FileProperties_Changed_ReCreated) == 0)
			RePaintMe(false);
		else if (strcmp(String, FileProperties_Changed_ColorTint) == 0)
			RePaintMe(false);
		else if (!stricmp(String, FileProperties_Changed_InProject))
			RePaintMe(false);
		else if ((strcmp(String, FileProperties_Changed_FileDeletedInfo) == 0) ||
				(strcmp(String, FileProperties_Changed_FileDeletedNotInfo) == 0))
		{
			// If I am waiting for an image, stop.
			FB_StopThreadReading();


			// See if I have a parent that wants to delete me
			if (m_parent)
				m_parent->FileButton_DeletedProperties(this);
			else	// Do a deferred Delete
				DeferredDelete();

		}
		return;
	}
	else if (ItemChanging == &m_icon)
	{
		StopRolloverAnimation();
		RePaintMe(false);
		return;
	}
	else if (ID == (long)this)
	{
		if (!strcmp(String, DragNDropContext_DragStarted))
			StopRolloverAnimation();
	}
	else if (ItemChanging == m_currentViewMode)
	{
		if (IsDeletion(String))
		{
			// Be sure to set the current view mode to NULL so it doens't crash
			// When removing the dependancy
			m_currentViewMode = NULL;
			SetFileButtonParent(NULL);
		}
		else if (!strcmp(String, FileButton_ViewMode_SIZE_Changed))
		{
			if (!m_parent || m_parent->ShouldFileButtonResizeOnViewModeChange(this))
			{
				// Watch to see if there really is a change
				RECT rect;
				GetClientRect(m_hWnd,&rect);
				long oldWidth = rect.right;
				long oldHeight = rect.bottom;
				long Width = m_currentViewMode->FileButton_ViewMode_GetPreferedXSize(this);
				long Height = m_currentViewMode->FileButton_ViewMode_GetPreferedYSize(this);
				if ((oldWidth != Width) || (oldHeight != Height))
				{
					SetWindowSize(Width, Height);	
				}
				else // Our parent will set our size appropriately
					RePaintMe(false);
			}
			else RePaintMe(false);
		}
		else if (!strcmp(String, FileButton_ViewMode_State0_Changed))
		{
			if (LastButtonState == 0) RePaintMe(false);
		}
		else if (!strcmp(String, FileButton_ViewMode_State1_Changed))
		{
			if (LastButtonState == 1) RePaintMe(false);
		}
		else if (!strcmp(String, FileButton_ViewMode_State2_Changed))
		{
			if (LastButtonState == 2) RePaintMe(false);
		}
		else if (!strcmp(String, FileButton_ViewMode_State3_Changed))
		{
			if (LastButtonState == 3) RePaintMe(false);
		}
		else RePaintMe(false);
		return;
	}
	UtilLib_DragButton::DynamicCallback(ID, String, args, ItemChanging);
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion)
{	MajorVersion=1;
	MinorVersion=0;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::BaseLoadSave_SaveData(SaveData *SaveContext)
{
	if (!SaveContext->Put(DragAndDrop_AmISelected())) return false;
	if (!SaveContext->PutString(GetFileName())) return false;

	BaseLoadSave* blsPTR = &m_internalViewMode;
	BaseLoadSave_SaveData_Member(blsPTR, SaveContext);

	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion)
{
	if ((MajorVersion==1)&&(MinorVersion==0))
	{
		bool selected; if (!LoadContext->Get(selected)) return false;
		char fileName[MAX_PATH]; if(!LoadContext->GetString(fileName, MAX_PATH)) return false;

		ChangeFileName(fileName);
		DragAndDrop_SelectMe(selected);

		BaseLoadSave* blsPTR = &m_internalViewMode;
		BaseLoadSave_LoadData_Member(blsPTR, LoadContext);
		return !LoadContext->Error();
	}
	else return false; // Unknown version number to load
}
//////////////////////////////////////////////////////////////////////////////////////////
void		FileButton::ColorTinting_SetTintColor(unsigned RGBA)
{
	if (m_properties)
		m_properties->SetTintColor(RGBA);
}
//////////////////////////////////////////////////////////////////////////////////////////
unsigned	FileButton::ColorTinting_GetTintColor(void)
{
	// Grab the current File Properties
	FileProperties* fp = this ? this->GetProperties() : NULL;

#ifdef VTNT25_FileButton_RecursiveColorTint
	if (fp)
	{
		// Loop until we find a non clear color tint
		fp->GetHandle();
		while (fp && (fp->GetTintColor().a == 0))
		{
			FileProperties* parent = fp->GetParentHandle();
			fp->ReleaseHandle();
			fp = parent;
		}
	}
#endif

	// Did we find a non clear color tint?
	if (fp)
	{
		unsigned ret = fp->GetTintColor().bgra;
		fp->ReleaseHandle();
		return ret;
	}
	else
	{
		// Return a default color that will bring out a 0 alpha
		return 0;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
bool		FileButton::ColorTinting_IsColorTintClear()
{
	if (m_properties) return (m_properties->GetTintColor().a == 0);
	else return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::DestroyWindow(void)
{	
	// If I am waiting for an image, stop.
	FB_StopThreadReading();

	RemoveAllDependantsToThis();
	// Stop the animation
	StopRolloverAnimation();
	// Close the window
	UtilLib_DragButton::DestroyWindow();
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::InitialiseWindow(void)
{
	// Figure out the FileButton Parent
	SetFileButtonParent(GetWindowInterface<FileButton_ViewMode_Parent>(GetParent()));
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::SetFileButtonParent(FileButton_ViewMode_Parent* newParent)
{
	FileButton_ViewMode* newViewMode = NULL;
	if (newParent) newViewMode = newParent->Get_FileButton_ViewMode(this);
	if (SetCurrentViewMode(newViewMode))
	{
		// Should I resize myself to this new parent
		if (!newParent || newParent->ShouldFileButtonResizeOnParentChange(this))
		{
			// Check the sizes
			long newX = m_currentViewMode->FileButton_ViewMode_GetPreferedXSize(this);
			long newY = m_currentViewMode->FileButton_ViewMode_GetPreferedYSize(this);

			long currX = GetWindowWidth();
			long currY = GetWindowHeight();

			if ((newX!=currX)||(newY!=currY))
					SetWindowSize(newX, newY);
			else	RePaintMe(false);
		}
	}
	m_parent = newParent;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::SetCurrentViewMode(FileButton_ViewMode* newViewMode)
{
	if (!newViewMode) 
		newViewMode = &m_internalViewMode;
	if (newViewMode != m_currentViewMode)
	{
		if (m_currentViewMode) m_currentViewMode->DeleteDependant(this);
		m_currentViewMode = newViewMode;
		m_currentViewMode->AddDependant(this);

		Changed(FileButton_Changed_ViewMode);
		return true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::RePaintMe(bool checkSize)
{
	if (m_repaintMe && m_currentViewMode && (!m_AmIPainting))
	{
		bool checkForRescale = (checkSize && !(m_parent && !m_parent->ShouldFileButtonResizeIndividually(this)));
		if (checkForRescale)
		{
			// Find Out my own size
			long currentWidth = GetWindowWidth();
			long currentHeight = GetWindowHeight();
			
			long prefWidth = m_currentViewMode->FileButton_ViewMode_GetPreferedXSize(this);
			if (prefWidth <= 0) prefWidth = currentWidth;
			long prefHeight = m_currentViewMode->FileButton_ViewMode_GetPreferedYSize(this);
			if (prefHeight <= 0) prefHeight = currentHeight;

			if ((prefWidth != currentWidth) || (prefHeight != currentHeight))
			{
				SetWindowSize(prefWidth, prefHeight);
				return;
			}
		}
		RePaint();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////
const char* FileButton::FileButton_GetAlias(bool getFileNameIfBlank)
{
	FileProperties* props = GetProperties();
	if (!props) return NULL;
	const char* ret = props->GetAltFilename();
	if (getFileNameIfBlank && (!ret || !ret[0]))
	{
		// Return the filename past the last slash
		const char* filename = props->GetFileName();
		ret = NewTek_GetFileNameFromPath((char*)filename);
	}
	return ret;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::Persitance_ShouldBeUsed_Load(void) 
{
	if (m_parent) 
		return m_parent->Persitance_ShouldBeUsed_Load(this); 
	else return true;
}
bool FileButton::Persitance_ShouldBeUsed_Save(void) 
{
	if (m_parent) 
		return m_parent->Persitance_ShouldBeUsed_Save(this); 
	else return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
/*bool FileButton::FB_IsFileReadable(FileProperties* fp)
{
	// Do some checks
	bool readableFile = false;
	if (fp && (fp->GetFileStatus() == FileProperties_File))
	{
		// See if Properties will let me read in my own file
		readableFile = true;
		AssetElement* ae;
		bool newAsset = fp->AddAsset(ae,"Loadable Into Editor", "Boolean", true, true);
		if (ae)
		{
			AssetBool* ab = GetInterface<AssetBool>(ae->GetProperty());
			if (ab)
			{
				if (newAsset) ab->Set(true);
				else readableFile = ab->Get();
			}
		}
	}
	return readableFile;
}*/
//////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::FB_ReadDefaultIcon(FileProperties* fp)
{
	// Is this a non file, or MyComputer
	if (m_icon.ReadBitmapFile(ReadInvalidIconFile(fp))) return false;

	// Is this a drive?
	if (m_icon.ReadBitmapFile(ReadDriveIconFile(fp))) return false;

	// Is This a folder, We might still be able to load it
	if (m_icon.ReadBitmapFile(ReadFolderIconFile(fp))) return true;
	
	// It might still be something readable since we are cleaning up everything else
	m_icon.ReadBitmapFile(ReadDefaultIconFile(fp));
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////
void FileButton::FB_LoadIcon(FileProperties* fp)
{
	if (fp)
	{
		// Try to read in the avi file if it is there
		bool readAVI = false;
		const char* baseFileName = fp->GetFileName();
		FileProperties::GetAnimatedIconFromFileName(baseFileName, filePath);
		if (NewTek_DoesFileExist(filePath))
		{
			readAVI = true;
			ThreadReadIconFile(fp, filePath);
			DebugOutput("AVI mini file read for %s: %s\n", baseFileName, filePath);
		}
		
		// Try the special icon in the info folder
		char* specialIconFile = ReadSpecialIconFile(fp);
		if (NewTek_DoesFileExist(specialIconFile) &&
			!NewTek_DoesPreviewNeedToBeGenerated((char*)baseFileName, specialIconFile) &&
			m_icon.ReadBitmapFile(specialIconFile))
		{
			FB_SetThreadReadStatus(FileButton_ThreadRead_Cached);
			return;
		}

		// Try reading the default types
		bool stillRead = FB_ReadDefaultIcon(fp);

		// If we think we might be able to read the file itself, so go ahead and try
		if ( !readAVI && stillRead )
			ThreadReadIconFile(fp, (char*)baseFileName);
	}
	else
	// Load the default item for no properties or one that has no where else to go
		m_icon.ReadBitmapFile(ReadDefaultIconFile(fp));
}
//////////////////////////////////////////////////////////////////////////////////////////
char* FileButton::ReadDriveIconFile(FileProperties* fileProps)
{
	if (fileProps->GetFileStatus() == FileProperties_Drive)
	{
		const char* fullFileName = fileProps->GetFileName();
		char* fn = NULL;

		UINT driveInfo = GetDriveType(fullFileName);
		switch (driveInfo)
		{
		case DRIVE_REMOVABLE:
			fn = FindFiles_FindFile(FINDFILE_SKINS,REMOVABLEDRIVEICON);
				if (fn) return fn;
			break;
		case DRIVE_FIXED:
			fn = FindFiles_FindFile(FINDFILE_SKINS,FIXEDDRIVEICON);
				if (fn) return fn;
			break;
		case DRIVE_REMOTE:
			fn = FindFiles_FindFile(FINDFILE_SKINS,NETWORKDRIVEICON);
				if (fn) return fn;
			break;
		case DRIVE_CDROM:
			fn = FindFiles_FindFile(FINDFILE_SKINS,CDROMDRIVEICON);
				if (fn) return fn;
			break;
		case DRIVE_RAMDISK:
			fn = FindFiles_FindFile(FINDFILE_SKINS,RAMDISKDRIVEICON);
				if (fn) return fn;
			break;
		}

		return FindFiles_FindFile(FINDFILE_SKINS,UNKNOWNDRIVEICON);
	}
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileButton::ReadFolderIconFile(FileProperties* fileProps)
{
	if (fileProps->GetFileStatus() == FileProperties_Directory)
		return FindFiles_FindFile(FINDFILE_SKINS,FOLDERICON);

	else return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileButton::ReadInvalidIconFile(FileProperties* fileProps)
{
	if (fileProps->GetFileStatus() == FileProperties_MyComputer)
		return (FindFiles_FindFile(FINDFILE_SKINS,MYCOMPUTERICON));
	else if (fileProps->GetFileStatus() == FileProperties_NoFile)
		return (FindFiles_FindFile(FINDFILE_SKINS,NOFILEICON));
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileButton::ReadSpecialIconFile(FileProperties* fileProps)
{
	FileProperties::GetIconFromFileName(fileProps->GetFileName(), filePath);
	if (NewTek_DoesFileExist(filePath))
		return filePath;
	else return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
char* FileButton::ReadDefaultIconFile(FileProperties* fileProps)
{
	if (fileProps)
	{
		const char* thisChar = NewTek_GetLastDot((char*)fileProps->GetFileName());
		if (thisChar)
		{
			thisChar++;
			sprintf(filePath, "File Icons\\%s.%s", thisChar, ICONEXTENSION);
			char* ret = FindFiles_FindFile(FINDFILE_SKINS,filePath);
			if (ret) return ret;
		} 
	}
	return FindFiles_FindFile(FINDFILE_SKINS,DEFAULTICON);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::FB_RefreshIcon( bool OnlyIfInvalid )
{	// Be sure we have a file to look at
	FileProperties* fp = this->GetProperties();
	if (fp)
	{	// If we only want to invalidate files that might no be up to date
		if (OnlyIfInvalid)
		{	// Where to save the little version
			char* specialIconFileName = ReadSpecialIconFile(fp);

			// We only want to save out the auto file if it is older than fileToRead
			// or if it does not exist
			if (!NewTek_DoesPreviewNeedToBeGenerated((char*)fp->GetFileName(), specialIconFileName))
				return;
		}

		// Stop any reading that might already be happening
		FB_StopThreadReading();

		// We need to get rid of Asset Management they might prejudice the decision
		//AssetElement* ae = m_properties->GetAsset("Loadable Into Editor");
		//if (ae)
		//{
		//	AssetBool* ab = GetInterface<AssetBool>(ae->GetProperty());
		//	if (ab) ab->Set(true);
		//}

		// Set the internal thread state
		FB_SetThreadReadStatus(FileButton_ThreadRead_Unknown);

		// Remove the file that has already been saved if there was one
		static char cachedImage[MAX_PATH];
		FileProperties::GetIconFromFileName(fp->GetFileName(), cachedImage);
		NewTek_DeleteFile(cachedImage, false);
	}

	// Re-Read the file
	//FB_LoadIcon(fp);
	m_iconLoaded=false;

	// Make sure I'm repainted so that the icon is regenerated
	RePaint();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::ThreadReadIconFile(FileProperties* fp, char* fileToRead)
{
	//DebugOutput("FileButton::ThreadReadIconFile for %s. %s\n", fp->GetFileName(), fileToRead);
	
	// Where to save the little version
	char fileToSave[MAX_PATH];
	FileProperties::GetIconFromFileName(fp->GetFileName(), fileToSave);

	// We only want to save out the auto file if it is older than fileToRead
	// or if it does not exist
	if (!NewTek_DoesPreviewNeedToBeGenerated(fileToRead, fileToSave))
	{
		FB_SetThreadReadStatus(FileButton_ThreadRead_Cached);
		return;
	}

	// Be sure the preferences allow a Thread Read
	if (!FB_DoesUserPreferThreadRead()) return;

	//DebugOutput("FileButton::ThreadReadIconFile saving to %s\n", fileToSave);

	// The resolution of the little version
	long XRes = FILEICONSIZEX;
	long YRes = FILEICONSIZEY;

	// The ID we want passed to our Deferred Callback
	unsigned ID1 = (unsigned)(&m_icon);

	// The success and failure ID's
	unsigned success = 1;
	unsigned failure = 0;

	// Do we want an image overlayed
	char* overlayImage = NULL;
	if (fp->GetFileStatus() == FileProperties_Directory)
		overlayImage = FindFiles_FindFile(FINDFILE_SKINS,IMAGEFOLDERICON);

	// I am now waiting for an Image
	if (!m_waitingForImage)
	{
		m_waitingForImage = new char[strlen(fileToRead) + 1];
		strcpy(m_waitingForImage, fileToRead);
	}

	if ((m_FileButton_PreviewInterface)&&(fileToSave))
	{	// DebugOutput("Requesting Preview Source=%s Destination=%s\n",fileToRead,fileToSave);
		FB_SetThreadReadStatus(FileButton_ThreadRead_Reading);
		if (!m_FileButton_PreviewInterface->GeneratePreview(
						fileToRead,
						fileToSave,
						XRes, YRes,
						ID1, success, 
						ID1, failure,
						overlayImage,
						this,			// The BaseWindowClass to recv. the Deferred Callback
						m_iconLoaded))
				DeferredMessage(ID1,failure);
		//RePaint();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::SetFileButtonPreviewInterfaceToUse(FileButton_PreviewInterface *ToUse)
{	m_FileButton_PreviewInterface=ToUse;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::SetFileButtonMouseOverPreviewInterface(FileButton_MouseOverPreviewInterface *ToUse)
{	m_FileButton_MouseOverPreviewInterface=ToUse;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::FB_StopThreadReading()
{
	if (m_waitingForImage)
	{	if ((m_properties)&&(m_FileButton_PreviewInterface))
			m_FileButton_PreviewInterface->RemovePreview(m_waitingForImage,this);
		delete[] m_waitingForImage;
		m_waitingForImage = NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::FB_DoesUserPreferThreadRead()
{
	// If the user has specified that we don't want to read it, return false
	int status = FB_GetThreadReadStatus();
	if (status == FileButton_ThreadRead_PreferedNo)
		return false;

	// If we don't know yet, grab the info from Preferences
	if ((status == FileButton_ThreadRead_Unknown) || (status == FileButton_ThreadRead_Cached))
	{
		bool prefered = Preferences::GetValue_Bool("Use Preview Generation", 1, 
			"Use preview generation", "Do not use preview generation.");

		if (!prefered)
		{
			FB_SetThreadReadStatus(FileButton_ThreadRead_PreferedNo);
			return false;
		}
	}

	// anything else is OK
	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::FB_ThreadReadIcon()
{
	// This is only valid if the preferences for the file have already read false
	if (FB_GetThreadReadStatus() == FileButton_ThreadRead_PreferedNo)
	{
		// Set to prefered Yes and try to load the Icon again
		FB_SetThreadReadStatus(FileButton_ThreadRead_PreferedYes);
		//FB_LoadIcon(this->GetProperties());
		m_iconLoaded=false;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::FB_SetThreadReadStatus(int newStatus)
{
	if (m_threadReadStatus != newStatus)
		m_threadReadStatus = newStatus;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void FileButton::FB_CancelThreadReading()
{
	if (m_threadReadStatus == FileButton_ThreadRead_Reading)
	{
		FB_StopThreadReading();
		FB_SetThreadReadStatus(FileButton_ThreadRead_PreferedNo);
		RePaint();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
bool FileButton::FileButton_ShouldDrawWarningBorder()
{
	return (!GetProperties() || 
			GetProperties()->AmICut() || 
			!GetProperties()->DoesFileExist());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

DetailsView_Element* FileButton::DVR_GetDetailsViewElement(unsigned index)
{
	// Be sure we have file properties to look at
	FileProperties *fp=GetProperties();
	if (!fp) return NULL;

	// Watch for all of the base types
	if (index < BasicFile_DVElem_NumItems)
	{
		// Get the relative path from the view mode
		return m_baseFile_DetailsView_Elements[index];
	}

	// Now check on Asset Management
	index -= BasicFile_DVElem_NumItems;
	if (index < fp->GetNumAssets())
		return fp->GetAsset(index);

	// We are all done now
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

DetailsView_Element* FileButton::DVR_GetDetailsViewElement(const char* columnName)
{
	// Be sure we have file properties to look at
	FileProperties *fp=GetProperties();
	if (!fp) return NULL;

	// Pedantic Check
	if (!columnName) return NULL;

	// Check for the base elements
	for (unsigned i = 0; i < BasicFile_DVElem_NumItems; i++)
	{
		if (!stricmp(columnName, BasicFile_DVElem_TypeArray[i].name))
			return m_baseFile_DetailsView_Elements[i];
	}

	// Now check on Asset Management
	AssetElement* ae = fp ? fp->GetAsset(columnName) : NULL;
	if (ae)
		return ae;

	// Nothing found by that name
	return NULL;
	
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void FileButton::DVR_Cleanup()
{
	// Just drop these properties from the DLG list
	FileProperties *fp=GetProperties();
	if (fp) fp->RemoveFromDialog();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void FileButton::DVR_Initialize()
{
	// Just drop these properties from the DLG list
	FileProperties *fp=GetProperties();
	if (fp) fp->AddToDialog();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void FileButton::DVR_RemoveElement(char* columnName)
{
	// Be sure we have FileProperties (pedantic)
	FileProperties *fp=GetProperties();
	if (!fp || !columnName) return;

	// Let Asset Management remove this Asset
	fp->RemoveAsset(columnName);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void FileButton::DVR_AddElement(char* columnName)
{
	// Be sure we have FileProperties (pedantic)
	FileProperties *fp=GetProperties();
	if (!fp || !columnName) return;

	// Always initialize and clean up
	DVR_Initialize();
	
	// This element should not have this particular element, check to be sure that it does not
	DetailsView_Element* element = this->DVR_GetDetailsViewElement(columnName);
	if (element)
	{
		element->DVE_Initialize();
		element->DVE_Cleanup();
	}
	else
	{
		// OK, now we can add something, but WHAT?  
		// We need to find some other file button in the list that has this Asset to copy
		AssetElement* elementToCopy = NULL;

		// First look at the selected elements in order
		tList<FileButton*> selectedButtons;
		GrabSelectedButtons(selectedButtons);
		unsigned numSelected = selectedButtons.NoItems();
		for (unsigned i = 0; i < numSelected && !elementToCopy; i++)
		{
			FileProperties* fp = selectedButtons[i]->GetProperties();
			elementToCopy = fp ? fp->GetAsset(columnName) : NULL;
		}

		if (!elementToCopy)
		{
			// If there was nothing there, look at the other Files in the same parent window	
			BaseWindowClass *BWC=GetParentBWC();

			// Sort the items into selection order and execute them
			if (BWC)
			{
				tList<HWND> ChildrenList;
				BWC->GetChildren(ChildrenList);

				unsigned numBrothers = ChildrenList.NoItems();
				for(unsigned i=0;i<numBrothers && !elementToCopy;i++)
				{	
					FileButton *FB=GetWindowInterface<FileButton>(ChildrenList[i]);
					if ((FB) && (::IsWindowVisible(FB->GetWindowHandle())) && (!selectedButtons.Exists(FB)))
					{
						FileProperties* fp = FB->GetProperties();
						elementToCopy = fp ? fp->GetAsset(columnName) : NULL;
					}
				}
			}
		}

		// Do I FINALLY have an element to copy from?
		if (elementToCopy)
		{
			// YEAH!
			AssetDynamic* dynamicToCopy = elementToCopy->GetProperty();
			if (dynamicToCopy)
			{
				AssetElement* newElement = fp->AddAsset(columnName, dynamicToCopy->AssetDynamic_GetAssetType());
				AssetDynamic* newDynamic = newElement ? newElement->GetProperty() : NULL;
				if (newDynamic)
				{
					// Copy from the master
					newDynamic->AssetDynamic_Copy(dynamicToCopy);
				}
			}
		}
		else
		{
			// Just make a string one with the proper name, Nothing to copy
			fp->AddAsset(columnName, "String");
		}
	}

	// Always Cleanup afterward
	this->DVR_Cleanup();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void	FileButton::FileButton_SetAlias(const char* newAlias)
{
	// Just set my own properties alt filename
	FileProperties* myProps = this->GetProperties();
	if (myProps)
		myProps->GetAltFilenameDynamic()->Set(newAlias ? newAlias : "");
}

void FileButton::DVR_CopyElementFrom(DetailsView_Element* masterElement,unsigned CopiedIndexCount)
{
	// Pedantic Check
	if (!masterElement) return;

	// See if this is an alias or FileName to be copied
	BaseFile_DetailsView_Element* baseElement = GetInterface<BaseFile_DetailsView_Element>(masterElement);
	if (baseElement)
	{
		// Grab the master file button and properties
		FileButton* masterButton = baseElement->BFDVE_GetFileButton();
		FileProperties* masterProps = masterButton ? masterButton->GetProperties() : NULL;
		FileProperties* thisProps = GetProperties();
		if (baseElement->BFDVE_GetType() == BasicFile_DVElem_AltName)
		{
			// Working with the alias, note that this technique WILL NOT work for SB
			// StoryBoard must override to properly set the CI alias
			if (thisProps && masterElement)
				thisProps->GetAltFilenameDynamic()->Set(masterProps->GetAltFilename());
		}
		else
		{
			// We are going to set the filename for this button based on the master
			// We want to use the filename with the extension and set it for all elements
			if (thisProps && masterProps) {
				char TempBuffer[MAX_PATH]= {0};
				baseElement->DVE_GetTextString(BasicFile_DVElem_FullName,TempBuffer,MAX_PATH);
				thisProps->FileChangeFilename(TempBuffer, true);
				}
		}
	}

	// Be sure we have FileProperties (pedantic)
	FileProperties *fp=GetProperties();
	if (!fp) return;

	// We only deal with Assets for the moment
	AssetElement* elementToCopy = GetInterface<AssetElement>(masterElement);
	if (elementToCopy)
	{
		AssetDynamic* dynamicToCopy = elementToCopy->GetProperty();
		if (dynamicToCopy)
		{
			// Add Asset will return an existing Asset if there is one
			AssetElement* newElement = fp->AddAsset(elementToCopy->GetName(), dynamicToCopy->AssetDynamic_GetAssetType());
			AssetDynamic* newDynamic = newElement ? newElement->GetProperty() : NULL;
			if (newDynamic)
			{
				// Copy from the master
				newDynamic->AssetDynamic_Copy(dynamicToCopy);
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

char* FileButton::DVR_GetDetailsViewColumnName(POINT mousePoint)
{
	// Find the name reactangle area for the view mode
	FileButton_ViewMode* viewMode = GetCurrentViewMode();
	return viewMode ? viewMode->FileButton_ViewMode_GetColumnAtPoint(mousePoint, this) : NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void  FileButton::FB_DVR_StartEditingColumn(const char* columnName, POINT* clickPoint, const char *InitString,bool HighLightInitString)
{
	// Pedantic Check
	if (!columnName) return;

	// Always initialize
	this->DVR_Initialize();

	// Grab the column we want to work with
	DetailsView_Element* element = this->DVR_GetDetailsViewElement(columnName);
	if (element)
	{
		// We have an element, be sure its initialization is handled
		element->DVE_Initialize();

		// Be sure the element will allow editing, PEDANTIC if called from the Context Menu
		if (element->DVE_AllowPopupEdit())
		{
			// Be sure this window is visible if the parent is a Layout Manager
			BaseWindowLayoutManager* wlm = GetWindowInterface<BaseWindowLayoutManager>(GetParent());
			if (wlm)
			{
				tList<HWND> list;
				list.Add(this->GetWindowHandle());
				wlm->CenterOnWindows(list);
			}

			// Be sure the column is all the way visible as well
			DetailsView_RowCollection* collection = GetWindowInterface<DetailsView_RowCollection>(this->GetParent());
			DetailsView_ColumnLayout* columnHeader = collection ? collection->DVRC_GetColumnHeader() : NULL;
			if (columnHeader)
				columnHeader->DVCL_CenterOnColumn(columnName);

			// Get the proper rect for the column
			RECT editingRect;
			FileButton_ViewMode* viewMode = GetCurrentViewMode();
			if (viewMode && viewMode->FileButton_ViewMode_GetEditingRect(editingRect, columnName, this)) {
				long reasonForExit = Popup_Escape_ClickAway;

				
				{ //Start the actual edit change
					if (collection)		// This could be NULL!!!
						collection->DVR_StartBatchCopy();
					//-------------------------------

					// We have a good rect to start with, Send it to the element for editing
					bool allowCopying = true;
					element->DVE_PopupEdit(this, editingRect, allowCopying, reasonForExit, InitString, clickPoint,HighLightInitString);
					

					// If we are going to allow copying, copy to selected elements here
					tList<FileButton*> selectedButtons;
					if (allowCopying) {

						GrabSelectedButtons(selectedButtons);
						unsigned numSelected = selectedButtons.NoItems();
						for (unsigned i = 0; i < numSelected; i++) {
							if (selectedButtons[i] != this)
								selectedButtons[i]->DVR_CopyElementFrom(element,i);
							}

						}

					if (collection)	// This could be NULL!!!
						collection->DVR_EndBatchCopy();
					//-----------------------------
					} //End the actual edit change

				FB_SelectNextEditableElement(columnName,(Popup_Escape)reasonForExit);
				}
			}
		// Always cleanup after yourself
		element->DVE_Cleanup();
		}
	// Always cleanup when done
	this->DVR_Cleanup();
	}


void FileButton::FB_SelectNextEditableElement(const char *ColumnName,Popup_Escape direction) {
	DetailsView_RowCollection* collection = GetWindowInterface<DetailsView_RowCollection>(this->GetParent());
	DetailsView_Element* element = this->DVR_GetDetailsViewElement(ColumnName);
	FileButton_ViewMode* viewMode = GetCurrentViewMode();
		
	// Here is where we pass on the Editing Baton if the user hit Tabs or arrows
	if ((collection)&&(element)) {
		// See if there is a new Row and Element
		DetailsView_Row* nextRow = NULL;
		DetailsView_Element* nextElement = NULL;
		collection->DVRC_GetNextEditableElement(direction, this, element, nextRow, nextElement);
		
		// We will only start on next if it was a FileButton
		FileButton *nextFB = (nextRow && nextElement) ? GetInterface<FileButton>(nextRow) : NULL;
		if (nextFB) {
			// Find the index of the element we are wanting to edit next
			unsigned index = viewMode->FileButton_ViewMode_GetColumnIndex(nextElement->DVE_GetColumnName(), nextFB);
			if (index != (unsigned)-1) {
				if (nextFB!=this) {
					// Be sure the button is the only one selected for the next pass
					BaseWindowClass* parentBWC = GetInterface<BaseWindowClass>(collection);
					bool ShiftKeyPressed=(bool)(GetAsyncKeyState(VK_SHIFT)&((short)(1<<15)));
					bool ReturnKeyPressed=(bool)(GetAsyncKeyState(VK_RETURN)&((short)(1<<15)));
					bool ShiftSelect=false;
					if ((ShiftKeyPressed)&&(!ReturnKeyPressed))
						ShiftSelect=true;
					if ((parentBWC)&&(!ShiftSelect))
						parentBWC->DragAndDrop_SelectAllChildren(false);
					nextFB->DragAndDrop_SelectMe(true);
					nextFB->SetKBFocus();
					FB_SelectedThisButton(nextFB);
					}

				// We want to edit the button with a deferred message, 
				//nextFB->DeferredMessage(FILEBUTTON_DEFERRED_EDIT, index);
				nextFB->SetDetailsColumnSelection(nextElement->DVE_GetColumnName(),true);
				}
			}

		// Clean up everything
		if (nextElement && (nextElement != element))
			nextElement->DVE_Cleanup();
		if (nextRow && (nextRow != this))
			nextRow->DVR_Cleanup();
		}
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////

DetailsView_Element* FileButton::DVR_GetNextEditableDVE(DetailsView_Element* adjoiningThis, bool forward)
{
	// My View Mode is what knows about this
	FileButton_ViewMode* viewMode = GetCurrentViewMode();
	if (viewMode)
	{
		// Find out which element this is and how many elements there are
		unsigned numElements = viewMode->FileButton_ViewMode_GetNumColumns(this);
		if (numElements != (unsigned)-1)
		{
			// Which index do I start at?
			int startIndex = adjoiningThis ? viewMode->FileButton_ViewMode_GetColumnIndex(adjoiningThis->DVE_GetColumnName(), this) : -1;
			if (startIndex == -1)
				startIndex = forward ? 0 : numElements-1;
			else 
				startIndex = forward ? startIndex+1 : startIndex-1;

			while ((startIndex >= 0) && (startIndex < numElements))
			{
				// Get the columnName for this element
				char* columnName = viewMode->FileButton_ViewMode_GetColumnAtIndex(startIndex, this);
				DetailsView_Element* thisElement = this->DVR_GetDetailsViewElement(columnName);
				if (thisElement)
				{
					// Initialize the element and see if it is the one
					thisElement->DVE_Initialize();
					if (thisElement->DVE_AllowPopupEdit())
						return thisElement;
					else thisElement->DVE_Cleanup();
				}
				// Next Loop
				startIndex = forward ? startIndex+1 : startIndex-1;
			}
		}
	}

	// Nothing worked
	return NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////

void FileButton::DVR_Refresh() {
	if (DragAndDrop_AmISelected())
		RePaint();
	}
