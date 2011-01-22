#include "stdafx.h"

FilePropertiesDlg::FilePropertiesDlg()
{
	m_skinLocal = NULL;
	m_layout = NULL;
	m_files = NULL;
	m_currentFile = NULL;
	m_pathEdit = NULL;
	m_fileNameEdit = NULL;
	m_aliasEdit = NULL;
	m_groupToggle = NULL;
	m_fileIcon = NULL;

	m_addButton = NULL;
	m_deleteButton = NULL;
	m_previousButton = NULL;
	m_nextButton = NULL;
	m_OKButton = NULL;
	m_cancelButton = NULL;
	
	m_scrollBarMin.Set(0.0f);
	m_scrollBarMax.Set(1.0f);
	m_widthBarVar.Set(0.3f);
	m_group.Set(0);
	m_group.AddDependant(this, (long)&m_group);

	// Initialize dynamic variables
	m_button.Set(FilePropertiesDlg_NoButton);
	m_button.AddDependant(this, (long)(&m_button));

	m_skinFileProps = NULL;

	SetSkinRoot(FindFiles_FindFile(FINDFILE_SKINS,FileProperties_SkinRoot));
}
///////////////////////////////////////////////////////////////////////////////////////////////////
FilePropertiesDlg::~FilePropertiesDlg()
{
	if (m_files)
		delete m_files;

	if (m_currentFile)
	{
		m_currentFile->DeleteDependant(this);
		FileProperties::ReleaseHandle(m_currentFile);
		m_currentFile = NULL;
	}

	m_button.DeleteDependant(this);
	m_group.DeleteDependant(this);

	if (m_skinLocal) _throw("FilePropertiesDlg::DestroyWindow not called properly");
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::DestroyWindow(void)
{
	// Don't listen to skin changes for Asset Buttons any more
	if (m_skinFileProps)
	{
		this->ListenToAssetList(NULL);
		m_skinFileProps->ReleaseHandle();
		m_skinFileProps = NULL;
	}

	// Get rid of the dependencies to the Control I created
	if (m_aliasEdit){m_aliasEdit->DeleteDependant(this);m_aliasEdit=NULL;}
	if (m_pathEdit){m_pathEdit->DeleteDependant(this);m_pathEdit=NULL;}
	if (m_fileNameEdit){m_fileNameEdit->DeleteDependant(this);m_fileNameEdit=NULL;}

	// Call the base class
	AutoSkinControl::DestroyWindow();

	// delete the local skin
	if (m_skinLocal)
		delete m_skinLocal;
	m_skinLocal = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::InitialiseWindow(void)
{
	// Create the internal skin and set it as my resource
	m_skinLocal = new Auto_StretchySkin();
	this->Canvas_SetResource(m_skinLocal);	

	// Call the base class version of this
	AutoSkinControl::InitialiseWindow();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::InitializeSubControls()
{
	// Use this as a temporary pointer for creating all controls
	SkinControl_SubControl* MyControl;
	// Create the scrollbar & arrows
	{
		HWND hWndScrollbar=OpenChild(FileProps_Color_ScrollBar,"UtilLib_Slider");
		if (!hWndScrollbar) _throw("FilePropertiesDlg::Could not create the scrollbar.");
		UtilLib_Slider* scrollBar=GetWindowInterface<UtilLib_Slider>(hWndScrollbar);
		if (!scrollBar) _throw("FilePropertiesDlg::scrollBar could not case to UtilLib_Slider.");

		// Set the screen objects
		scrollBar->Button_SetResource(Controls_Button_UnSelected, &m_sliderBitmaps[Controls_Button_UnSelected]);
		scrollBar->Button_SetResource(Controls_Button_Selected, &m_sliderBitmaps[Controls_Button_Selected]);
		scrollBar->Button_SetResource(Controls_Button_MouseOver, &m_sliderBitmaps[Controls_Button_MouseOver]);
		scrollBar->Button_SetResource(Controls_Button_MouseOverDn, &m_sliderBitmaps[Controls_Button_MouseOverDn]);

		// Set all of the Dynamics for the scroller
		scrollBar->Slider_SetVariable(&m_scrollBarVar);
		scrollBar->Slider_SetMinVariable(&m_scrollBarMin);
		scrollBar->Slider_SetMaxVariable(&m_scrollBarMax);
		scrollBar->Slider_SetSliderWidth(&m_scrollBarWidth);

		// get the FilePropertiesDlg_Up arrow
		MyControl=OpenChild_SubControl(FileProps_Color_ScrollBarUp,"SkinControl_SubControl_SliderButton");
		if (!MyControl) _throw("FileProps::Could not create SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *UpArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (!UpArrow) _throw("FileProps::UpArrow Could not case to SkinControl_SubControl_SliderButton");
		UpArrow->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		UpArrow->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		UpArrow->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		UpArrow->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		UpArrow->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		UpArrow->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		UpArrow->Slider_SetVariable(&m_scrollBarVar);
		UpArrow->Slider_SetMinVariable(&m_scrollBarMin);
		UpArrow->Slider_SetMaxVariable(&m_scrollBarMax);
		UpArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
		UpArrow->Set_SliderMult(-20.0f);

		// get the down arrow
		MyControl=OpenChild_SubControl(FileProps_Color_ScrollBarDn,"SkinControl_SubControl_SliderButton");
		if (!MyControl) _throw("FileProps::Could not create SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *DnArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (!DnArrow) _throw("FileProps::DnArrow Could not case to SkinControl_SubControl_SliderButton");
		DnArrow->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		DnArrow->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		DnArrow->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		DnArrow->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		DnArrow->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		DnArrow->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		DnArrow->Slider_SetVariable(&m_scrollBarVar);
		DnArrow->Slider_SetMinVariable(&m_scrollBarMin);
		DnArrow->Slider_SetMaxVariable(&m_scrollBarMax);
		DnArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
		DnArrow->Set_SliderMult(20.0f);
	}

	// Create the Width Bar
	{
		HWND hWndScrollbar=OpenChild(FileProps_Color_WidthSlider,"PropsWidthSlider");
		if (!hWndScrollbar) _throw("FilePropertiesDlg::Could not create the width scrollbar.");
		PropsWidthSlider* scrollBar=GetWindowInterface<PropsWidthSlider>(hWndScrollbar);
		if (!scrollBar) _throw("FilePropertiesDlg::width scrollBar could not case to PropsWidthSlider.");

		scrollBar->Button_SetResource(Controls_Button_UnSelected, &m_widthBitmaps[Controls_Button_UnSelected]);
		scrollBar->Button_SetResource(Controls_Button_Selected, &m_widthBitmaps[Controls_Button_Selected]);
		scrollBar->Button_SetResource(Controls_Button_MouseOver, &m_widthBitmaps[Controls_Button_MouseOver]);
		scrollBar->Button_SetResource(Controls_Button_MouseOverDn, &m_widthBitmaps[Controls_Button_MouseOverDn]);

		// Set all of the Dynamics for the scroller
		scrollBar->Slider_SetVariable(&m_widthBarVar);

		// Use my own Text
		scrollBar->PWS_SetTextItem(&m_propsWidthSliderText);
	}
	
	// Create the buttons
	{
		// Add
		MyControl=OpenChild_SubControl(FileProps_Color_Add,"SkinControl_SubControl_Button");
		if (!MyControl) _throw("FilePropertiesDlg::Could not create SkinControl_SubControl_Button");
		m_addButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!m_addButton) _throw("FilePropertiesDlg::m_addButton Could not case to SkinControl_SubControl_Button");
		m_addButton->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		m_addButton->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		m_addButton->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		m_addButton->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		m_addButton->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		m_addButton->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		m_addButton->Button_SetSelectedState(FilePropertiesDlg_Add);
		m_addButton->Button_UseVariable(&m_button);

		// Delete
		MyControl=OpenChild_SubControl(FileProps_Color_Delete,"SkinControl_SubControl_Button");
		if (!MyControl) _throw("FilePropertiesDlg::Could not create SkinControl_SubControl_Button");
		m_deleteButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!m_deleteButton) _throw("FilePropertiesDlg::m_deleteButton Could not case to SkinControl_SubControl_Button");
		m_deleteButton->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		m_deleteButton->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		m_deleteButton->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		m_deleteButton->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		m_deleteButton->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		m_deleteButton->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		m_deleteButton->Button_SetSelectedState(FilePropertiesDlg_Delete);
		m_deleteButton->Button_UseVariable(&m_button);

		// Cancel
		MyControl=OpenChild_SubControl(FileProps_Color_Cancel,"SkinControl_SubControl_Button");
		if (!MyControl) _throw("FilePropertiesDlg::Could not create SkinControl_SubControl_Button");
		m_cancelButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!m_cancelButton) _throw("FilePropertiesDlg::m_cancelButton Could not case to SkinControl_SubControl_Button");
		m_cancelButton->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		m_cancelButton->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		m_cancelButton->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		m_cancelButton->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		m_cancelButton->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		m_cancelButton->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		m_cancelButton->Button_SetSelectedState(FilePropertiesDlg_Cancel);
		m_cancelButton->Button_UseVariable(&m_button);

		// Next
		MyControl=OpenChild_SubControl(FileProps_Color_Next,"SkinControl_SubControl_Button");
		if (!MyControl) _throw("FilePropertiesDlg::Could not create SkinControl_SubControl_Button");
		m_nextButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!m_nextButton) _throw("FilePropertiesDlg::m_nextButton Could not case to SkinControl_SubControl_Button");
		m_nextButton->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		m_nextButton->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		m_nextButton->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		m_nextButton->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		m_nextButton->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		m_nextButton->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		m_nextButton->Button_SetSelectedState(FilePropertiesDlg_Next);
		m_nextButton->Button_UseVariable(&m_button);

		// Previous
		MyControl=OpenChild_SubControl(FileProps_Color_Previous,"SkinControl_SubControl_Button");
		if (!MyControl) _throw("FilePropertiesDlg::Could not create SkinControl_SubControl_Button");
		m_previousButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!m_previousButton) _throw("FilePropertiesDlg::m_previousButton Could not case to SkinControl_SubControl_Button");
		m_previousButton->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		m_previousButton->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		m_previousButton->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		m_previousButton->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		m_previousButton->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		m_previousButton->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		m_previousButton->Button_SetSelectedState(FilePropertiesDlg_Previous);
		m_previousButton->Button_UseVariable(&m_button);

		// OK
		MyControl=OpenChild_SubControl(FileProps_Color_OK,"SkinControl_SubControl_Button");
		if (!MyControl) _throw("FilePropertiesDlg::Could not create SkinControl_SubControl_Button");
		m_OKButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (!m_OKButton) _throw("FilePropertiesDlg::m_OKButton Could not case to SkinControl_SubControl_Button");
		m_OKButton->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		m_OKButton->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		m_OKButton->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		m_OKButton->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		m_OKButton->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		m_OKButton->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		m_OKButton->Button_SetSelectedState(FilePropertiesDlg_OK);
		m_OKButton->Button_UseVariable(&m_button);

		// Group Toggle Button
		MyControl=OpenChild_SubControl(FileProps_Color_Group,"SkinControl_SubControl_ToggleButton");
		if (!MyControl) _throw("FilePropertiesDlg::Could not create SkinControl_SubControl_ToggleButton");
		m_groupToggle=GetInterface<SkinControl_SubControl_ToggleButton>(MyControl);
		if (!m_groupToggle) _throw("FilePropertiesDlg::m_groupToggle Could not case to SkinControl_SubControl_ToggleButton");
		m_groupToggle->Button_SetResource(SkinControl_SubControl_Button_None, FilePropertiesDlg_Up);
		m_groupToggle->Button_SetResource(SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_Dn);
		m_groupToggle->Button_SetResource(SkinControl_SubControl_Button_Rollover, FilePropertiesDlg_UpRo);
		m_groupToggle->Button_SetResource(SkinControl_SubControl_Button_Rollover|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnRo);
		m_groupToggle->Button_SetResource(SkinControl_SubControl_Button_Disabled, FilePropertiesDlg_UpDis);
		m_groupToggle->Button_SetResource(SkinControl_SubControl_Button_Disabled|SkinControl_SubControl_Button_Pushed, FilePropertiesDlg_DnDis);
		m_groupToggle->Button_UseVariable(&m_group);
		m_groupToggle->Button_SetSelectedState(1);

	}
	// Open the different Editors and FileButton
	{
		m_pathEdit = GetWindowInterface<UtilLib_Edit>(OpenChild(FileProps_Color_PathEdit, "UtilLib_Edit"));
		if (!m_pathEdit)
			_throw("FilePropertiesDlg::Could not create m_pathEdit");
		m_pathEdit->Controls_Edit_GetItem()->SetAlignment(TextItem_LeftAlign|TextItem_CenterVAlign);
		m_pathEdit->Controls_Edit_GetItem()->AddDependant(this, (long)m_pathEdit);
		
		m_fileNameEdit = GetWindowInterface<UtilLib_Edit>(OpenChild(FileProps_Color_NameEdit, "UtilLib_Edit"));
		if (!m_fileNameEdit)
			_throw("FilePropertiesDlg::Could not create m_fileNameEdit");
		m_fileNameEdit->Controls_Edit_GetItem()->SetAlignment(TextItem_LeftAlign|TextItem_CenterVAlign);
		m_fileNameEdit->Controls_Edit_GetItem()->AddDependant(this, (long)m_fileNameEdit);

		m_aliasEdit = GetWindowInterface<UtilLib_Edit>(OpenChild(FileProps_Color_AliasEdit, "UtilLib_Edit"));
		if (!m_aliasEdit)
			_throw("FilePropertiesDlg::Could not create m_aliasEdit");
		m_aliasEdit->Controls_Edit_GetItem()->SetAlignment(TextItem_LeftAlign|TextItem_CenterVAlign);
		m_aliasEdit->Controls_Edit_GetItem()->AddDependant(this, (long)m_aliasEdit);

		m_fileIcon = GetWindowInterface<FileButton>(OpenChild(FileProps_Color_Icon, "FileButton_NoMenu"));
		if (!m_fileIcon)
			_throw("FilePropertiesDlg::Could not create m_fileIcon");
		m_fileIcon->m_internalViewMode.FileButton_ViewMode_SetViewMode(FileButton_ViewMode_IconOnly);

		EditNewFile(NULL);	// Starts us off looking at no file
	}


	m_layout = GetWindowInterface<AssetListLayout>(OpenChild(FileProps_Color_AssetList, "AssetListLayout"));
	if (m_layout)
	{
		m_layout->BaseWindowLayoutManager_Y_Slider_SetVariable(&m_scrollBarVar);
		m_layout->BaseWindowLayoutManager_Y_Slider_SetMaxVariable(&m_scrollBarMax);
		m_layout->BaseWindowLayoutManager_Y_Slider_SetMinVariable(&m_scrollBarMin);
		m_layout->BaseWindowLayoutManager_Y_Slider_SetSliderWidth(&m_scrollBarWidth);
		m_layout->SetNameWidthDynamic(&m_widthBarVar);
		m_layout->SetAssetButtonClient(this);
	}
	else _throw("FilePropertiesDlg::Could not create the AssetListLayout");
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::CreateNewAssetButton(SharedElement* p_sharedElement, AssetListLayout* p_layout)
{
	// Call the base class and set the scroll bar to the bottom
	AssetButtonClient::CreateNewAssetButton(p_sharedElement, p_layout);
	m_scrollBarVar = m_scrollBarMax;	// Places us down at the bottom
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	if (IsDeletion(String))
	{
		if (ItemChanging == m_pathEdit){m_pathEdit = NULL; return;}
		if (ItemChanging == m_aliasEdit){m_aliasEdit = NULL; return;}
		if (ItemChanging == m_fileNameEdit){m_fileNameEdit = NULL; return;}
	}
	if (ID == (long)(&m_button))
	{
		if (strcmp(String, Controls_Button_ReleasedOn) == 0)
		{
			int whichButton = m_button.Get();
			if (whichButton == FilePropertiesDlg_Add)
			{
				if (m_currentFile && m_files)
				{
					tList<char*> assetTypes;
					AssetDynamic::GetAllAssetTypes(&assetTypes);
					RECT menuRect = CreateMenu(&assetTypes);

					RECT addRect;
					if (this->m_skin->GetRect(FileProps_Color_Add, addRect))
					{
						// Find the center, top, and width;
						long center = (addRect.left+addRect.right) / 2;
						long top = addRect.bottom;
						long width = addRect.right - addRect.left;
						long menuWidth = menuRect.right - menuRect.left;
						if (menuWidth > width)
							width = menuWidth;
						menuRect.left = center - width/2 + GetWindowPosX();
						menuRect.right = menuRect.left + width;
						menuRect.top = top + GetWindowPosY();
						menuRect.bottom += top + GetWindowPosY();
					}
					else
					{
						// Get the position of the cursor
						POINT pt;
						GetCursorPos(&pt);
						menuRect.left += pt.x;
						menuRect.right += pt.x;
						menuRect.top += pt.y;
						menuRect.bottom += pt.y;
					}
					int menuRet = ShowMenu(menuRect);
					if (menuRet > -1)
						CreateNewAssetButton(m_files->CreateNewShared(assetTypes[menuRet]), m_layout);
				}
			}
			else if (whichButton == FilePropertiesDlg_Delete)
				m_layout->DeleteSelected();
			else if ((whichButton == FilePropertiesDlg_Previous) || (whichButton == FilePropertiesDlg_Next))
			{
				unsigned numInCollection = m_files->GetNumInCollection();
				if (m_files && (numInCollection > 1))
				{
					FileProperties* newFile = GetInterface<FileProperties>
						(m_files->GetNextList(m_currentFile, (whichButton == FilePropertiesDlg_Next)));
					if (newFile)
						EditNewFile(newFile->GetFileName());
					else
						EditNewFile(NULL);

					// We might be enabling or disabling the next or prev button
					int pos = m_files->GetPosition(newFile);
					m_previousButton->SetEnabled(pos > 0);
					m_nextButton->SetEnabled(pos < (numInCollection-1));
				}
			}
			else if (whichButton == FilePropertiesDlg_OK)
			{
				CommitChanges();
			}
			else if (whichButton == FilePropertiesDlg_Cancel)
			{
				CancelChanges();
			}

			m_button.Set(FileBin_NoButton);
			return;
		}
	}
	else if (strcmp(String, UtilLib_Edit_GetFocus) == 0)
	{
		m_OKButton->SetEnabled(m_currentFile);
		m_cancelButton->SetEnabled(m_currentFile);
		return;
	}
	else if (ID == (long)(&m_group))
	{
		if (m_group.Get())
		{
			// disable the prev and nextButtons
			m_previousButton->SetEnabled(false);
			m_nextButton->SetEnabled(false);
		}
		else
		{
			m_previousButton->SetEnabled(this->m_files->GetNumInCollection() > 1);
			m_nextButton->SetEnabled(this->m_files->GetNumInCollection() > 1);
		}
		CancelChanges();
		return;
	}
	else if (m_currentFile && (ID == (long)m_currentFile))
	{
		if (!m_group.Get())
			CancelChanges();
		return;
	}
	AutoSkinControl::DynamicCallback(ID, String, args,ItemChanging);
}
////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::EditTheseFiles(tList<FileProperties*>* p_fileList)
{
	// delete the old set of files
	if (m_files)
		delete m_files;

	if (p_fileList)
		m_files = new FileCollection(p_fileList, this, this->m_layout);
	else m_files = NULL;

	DeferredMessage(0);
}
////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::DeferredDelete(void)
{
	// Closing Cancels all of the changes made to the Filename and alias
	this->CancelChanges();
	AutoSkinControl::DeferredDelete();
}
////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::ReceiveDeferredMessage(unsigned ID1,unsigned ID2)
{
	if (ID1 == 0)
	{
		// Clear all of the existing visible assets
		m_layout->CloseAllChildren();
		if (m_files)
		{
			// Be sure my FileCollection knows what my layout is
			// TODO: Isn't there a call that already does this?
			// CreateNewAssetButton()
			m_files->m_layout = this->m_layout;

			// Create the new list with the elements
			for (unsigned long i = 0; i < m_files->m_compositeTable.NoItems(); i++)
			{
				SharedElement* thisElement = m_files->m_compositeTable[i];
				// Create a new AssetButton with this AssetElement
				AssetButton* thisButton = m_layout->NewAssetButton();
				if (thisButton)
				{
					if (!thisButton->GetAssetButtonClient())
						thisButton->SetAssetButtonClient(this);
					thisButton->SetSharedElement(thisElement);
				}
			}
			// Change the current file we are dealing with
			FileProperties* firstFile = GetInterface<FileProperties>(m_files->GetFirstList());
			if (firstFile)
				EditNewFile(firstFile->GetFileName());
			else
				EditNewFile(NULL);
		}
		else EditNewFile(NULL);
	}
	else
	{
		FileProperties* deleteMe = (FileProperties*)ID1;
		deleteMe->DeleteDependant(this);
		FileProperties::ReleaseHandle(deleteMe);
	}
	AutoSkinControl::ReceiveDeferredMessage(ID1,ID2);	
}
////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::EditNewFile(const char* p_newFile)
{
	if (m_currentFile)
	{
		// See if we should commit or cancel changes
		long currentIndex = m_files->GetPosition(m_currentFile);
		if ((currentIndex >= 0) && m_files->m_listening[currentIndex])
		{
			if (!m_group.Get()) 
				CommitChanges();
			else
				CancelChanges();
		}

		// See if we are really just looking at the same file
		if (p_newFile && (strcmp(p_newFile , m_currentFile->GetFileName()) == 0))
		{
			m_nextButton->SetEnabled(!m_group.Get() && (this->m_files->GetNumInCollection() > 1));
			m_previousButton->SetEnabled(!m_group.Get() && (this->m_files->GetNumInCollection() > 1));
			int myInt = m_group.Get();
			if (myInt && (m_files->GetNumInCollection() < 2))
				m_group.Set(0);
			m_groupToggle->SetEnabled(this->m_files->GetNumInCollection() > 1);
			m_group.Set((m_files->GetNumInCollection() > 1) ? 1 : 0);	// Set to group by default if more than one in collection
			return;
		}

		// Get rid of m_currentFile
		DeferredMessage((long)m_currentFile);
		m_currentFile = NULL;
	}
	if (p_newFile && p_newFile[0])
	{
		m_currentFile = FileProperties::GetHandle(p_newFile);
		m_currentFile->AddDependant(this, (long)m_currentFile);

		int myInt = m_group.Get();
		if (myInt && (m_files->GetNumInCollection() < 2))
			m_group.Set(0);

		m_addButton->SetEnabled(true);
		m_deleteButton->SetEnabled(true);
		m_cancelButton->SetEnabled(false);
		m_OKButton->SetEnabled(false);
		m_nextButton->SetEnabled(!m_group.Get() && (this->m_files->GetNumInCollection() > 1));
		m_previousButton->SetEnabled(!m_group.Get() && (this->m_files->GetNumInCollection() > 1));
		m_groupToggle->SetEnabled(this->m_files->GetNumInCollection() > 1);
		m_group.Set((m_files->GetNumInCollection() > 1) ? 1 : 0);	// Set to group by default if more than one in collection

		if (!m_group.Get())
		{
			char buffer[MAX_PATH]; strcpy(buffer, p_newFile); p_newFile = buffer;
			char* lastSlash = NewTek_GetLastSlash((char*)p_newFile);
			if (lastSlash)
			{
				*lastSlash = 0;
				char* nameOnly = lastSlash+1;
				m_pathEdit->Controls_Edit_GetItem()->SetText(p_newFile);
				m_fileNameEdit->Controls_Edit_GetItem()->SetText(nameOnly);
				*lastSlash = '\\';
			}
			else
			{
				m_pathEdit->Controls_Edit_GetItem()->SetText("");
				m_fileNameEdit->Controls_Edit_GetItem()->SetText(p_newFile);
			}
			m_aliasEdit->Controls_Edit_GetItem()->SetText(m_currentFile->GetAltFilename());
		}
		m_fileIcon->ChangeFileName(p_newFile);
		return;
	}
	m_group.Set(0);
	m_groupToggle->SetEnabled(false);
	m_OKButton->SetEnabled(false);
	m_cancelButton->SetEnabled(false);
	m_nextButton->SetEnabled(false);
	m_previousButton->SetEnabled(false);
	m_addButton->SetEnabled(false);
	m_deleteButton->SetEnabled(false);

	char* errorText = "****  No File Selected  ****";
	m_pathEdit->Controls_Edit_GetItem()->SetText(errorText);
	m_fileNameEdit->Controls_Edit_GetItem()->SetText(errorText);
	m_aliasEdit->Controls_Edit_GetItem()->SetText(errorText);
	m_fileIcon->ChangeFileName(FindFiles_FindFile(FINDFILE_SKINS,NOFILEICON));
}
////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::CommitChanges()
{
	if (m_currentFile)
	{
		// Set the name, pathname, and altname if we are not in group mode
		if (!m_group.Get())
		{
			// Here is where we change only this one file
			// Get the values from the different text edits
			const char* newPath = m_pathEdit->Controls_Edit_GetItem()->GetText();
			const char* newFileName = m_fileNameEdit->Controls_Edit_GetItem()->GetText();
			const char* newAltName = m_aliasEdit->Controls_Edit_GetItem()->GetText();
			char fullPath[MAX_PATH];
			sprintf(fullPath, "%s\\%s", newPath, newFileName);
			
			// Try the altName first
			m_currentFile->GetAltFilenameDynamic()->Set(newAltName);

			// Then move if we have to
			if (strcmp(fullPath, m_currentFile->GetFileName()))
			{
				if (!m_currentFile->FileMoveFile(fullPath))
					this->CancelChanges();
			}
		}
		else if (m_files)
		{
			// here is where we can make multiple changes
			const char* newPath = m_pathEdit->Controls_Edit_GetItem()->GetText();
			const char* newFileName = m_fileNameEdit->Controls_Edit_GetItem()->GetText();
			const char* newAltName = m_aliasEdit->Controls_Edit_GetItem()->GetText();

			if (strlen(newAltName))
				m_files->SetGroupAltName(newAltName);
			if (strlen(newPath))
			{
				if (strlen(newFileName))
				{
					char newFullPath[MAX_PATH];
					sprintf(newFullPath, "%s\\%s", newPath, newFileName);
					m_files->SetGroupFullPath(newFullPath);
				}
				else
					m_files->SetGroupPath(newPath);
			}
			else if (strlen(newFileName))
				m_files->SetGroupFilename(newFileName);
		}
		CancelChanges();
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::CancelChanges()
{
	if (m_currentFile)
	{
		if (m_group.Get())
		{
			m_pathEdit->Controls_Edit_GetItem()->SetText("");
			m_fileNameEdit->Controls_Edit_GetItem()->SetText("");
			m_aliasEdit->Controls_Edit_GetItem()->SetText("");
		}
		else
		{
			char fileName[MAX_PATH]; strcpy(fileName, m_currentFile->GetFileName());
			char* lastSlash = NewTek_GetLastSlash(fileName);
			if (lastSlash)
			{
				*lastSlash = 0;
				char* nameOnly = lastSlash+1;
				m_pathEdit->Controls_Edit_GetItem()->SetText(fileName);
				m_fileNameEdit->Controls_Edit_GetItem()->SetText(nameOnly);
				m_aliasEdit->Controls_Edit_GetItem()->SetText(m_currentFile->GetAltFilename());
				*lastSlash = '\\';
			}
		}
		m_OKButton->SetEnabled(false);
		m_cancelButton->SetEnabled(false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
bool FilePropertiesDlg::ExecuteCommand(	char *FunctionName,						// The string representation of the command to execute
										ScriptExecutionStack &InputParameters,	// The set of input parameters to your function
										ScriptExecutionStack &OutputParameters,	// You can use this to access variables as well
										char *&Error)							// If you supported the command but failed for some reason
																				// return a user string here
{	
	if (!strcmp(FunctionName,"LoadScrollBarBitmapV"))
	{
		if (InputParameters.GetSizeOfStack()<2)
		{
			Error = "LoadScrollBarBitmapV: Function Missing Parameters, aborted";
			return true;
		}
		if (InputParameters[0].GetType()==Script_STRING)
		{
			int i=InputParameters[1];
			if ((i >= 0) && (i < 4))
			{	char TempBuffer[MAX_PATH];
				if (!m_sliderBitmaps[i].ChangeFilename(NewTek_GetAbsolutePath(m_skinPath,InputParameters[0],TempBuffer), true))
					Error = "LoadScrollBarBitmapH: Failed to Load Layer Image";
				m_sliderBitmaps[i].SetAlignment(BitmapTile_StretchX+BitmapTile_StretchY);
				m_sliderBitmaps[i].ForceTo32BPP();
			}
			else Error = "LoadScrollBarBitmapH: Invalid Parameter 1 (Use 0-3), aborted";
		}
		else Error = "LoadScrollBarBitmapH: Invalid Parameter 0, aborted";
		return true;
	}
	else if (!strcmp(FunctionName,"LoadScrollBarBitmapH"))
	{
		if (InputParameters.GetSizeOfStack()<2)
		{
			Error = "LoadScrollBarBitmapH: Function Missing Parameters, aborted";
			return true;
		}
		if (InputParameters[0].GetType()==Script_STRING)
		{
			int i=InputParameters[1];
			if ((i >= 0) && (i < 4))
			{	char TempBuffer[MAX_PATH];
				if (!m_widthBitmaps[i].ChangeFilename(NewTek_GetAbsolutePath(m_skinPath,InputParameters[0],TempBuffer), true))
					Error = "LoadScrollBarBitmapV: Failed to Load Layer Image";
				m_widthBitmaps[i].SetAlignment(BitmapTile_StretchY);
				m_widthBitmaps[i].ForceTo32BPP();
			}
			else Error = "LoadScrollBarBitmapV: Invalid Parameter 1 (Use 0-3), aborted";
		}
		else Error = "LoadScrollBarBitmapV: Invalid Parameter 0, aborted";
		return true;
	}
	else if (!strcmp(FunctionName, "GetNamePropsTextItem"))
	{	
		ScriptExecutionStack *NewStack=ScriptExecutionStack::GetNewStack(&OutputParameters);
		NewStack->SetContext(&m_propsWidthSliderText);
		OutputParameters.Add()=NewStack;
		NewStack->Release();
		return true;				
	}
	else if (AssetButtonClient::ExecuteCommand(m_skinPath, FunctionName, InputParameters, OutputParameters, Error))
		return true;
	return AutoSkinControl::ExecuteCommand(FunctionName, InputParameters, OutputParameters, Error);
}
//////////////////////////////////////////////////////////////////////////////////////////
void FilePropertiesDlg::SkinChangedCallback(char* newSkinPath)
{
	FileProperties* oldProps = m_skinFileProps;
	m_skinFileProps = FileProperties::GetHandle(newSkinPath);
	this->ListenToAssetList(m_skinFileProps);
	if (oldProps) oldProps->ReleaseHandle();
}
//////////////////////////////////////////////////////////////////////////////////////////