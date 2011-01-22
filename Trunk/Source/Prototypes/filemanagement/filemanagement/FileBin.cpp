#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////////////////
unsigned StretchyFileBin::s_favoritesRefCt = 0;

StretchyFileBin::StretchyFileBin()
{	
	FavoriteFileList::GetInstance();
	s_favoritesRefCt++;
	
	m_scrollBarMin.Set(0.0f);
	m_scrollBarMax.Set(1.0f);
	m_initializeWithDirectory[0] = NULL;
	m_fileLayout = NULL;
	m_slider = NULL;
	m_columnHeader=NULL;
	m_viewModeLabelButton = NULL;
	m_ScreenObject_StretchySkin = NULL;
	SetSkinRoot(FindFiles_FindFile(FINDFILE_SKINS,FileBin_ScriptRoot));

	// Set up scroll bar dynamics
	m_scrollBarMin.Set(0.0f);
	m_scrollBarMax.Set(1.0f);
	m_scrollBarWidth.Set(0.3f);
	m_tabScrollBarMin.Set(0.0f);
	m_tabScrollBarMax.Set(1.0f);
	m_tabScrollBarWidth.Set(30.0f);
	m_tabScrollBarVar.Set(0.0);

	for (unsigned i = 0; i < 4; i++)
	{
		TextItem* ti = m_buttonLabelText + i;
		ti->MessagesOff();
		ti->SetAlignment(TextItem_LeftAlign | TextItem_CenterVAlign);
		ti->SetFontSize(12);
		//ti->SetColor(255, 255, 255);
		if ((i == Controls_Button_UnSelected) || (i == Controls_Button_MouseOver))
			ti->SetColor(255, 255, 255);
		ti->SetBold(false);
		ti->SetOffset(4, 0);
		// ti->SetFont();
		ti->MessagesOn(false);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
StretchyFileBin::~StretchyFileBin()
{
	s_favoritesRefCt--;
	if (0 == s_favoritesRefCt)
		FavoriteFileList::GetInstance()->FFL_WriteList();
		
	// Remove dependancies on dynamics
	m_button.DeleteDependant(this);
	m_viewMode.DeleteDependant(this);

	if (m_ScreenObject_StretchySkin)
		_throw("StretchyFileBin::DestroyWindow not called properly");
}
////////////////////////////////////////////////////////////////////////////////////////////
const char* StretchyFileBin::GetDirectoryLayoutType(){return "DirectoryLayout";}
////////////////////////////////////////////////////////////////////////////////////////////
void StretchyFileBin::InitializeSubControls()
{
	// Initialize dynamic variables
	m_button.Set(FileBin_NoButton);
	m_button.AddDependant(this, (long)(&m_button));
	m_viewMode.AddDependant(this, (long)(&m_viewMode));

	// Use this as a temporary pointer for creating all controls
	SkinControl_SubControl* MyControl;

	// Create the Toolbar buttons
	{
		// Cut
		MyControl=OpenChild_SubControl(FileBin_Color_Cut,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *CutButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (CutButton)
		{
			CutButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			CutButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			CutButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			CutButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			CutButton->Button_SetSelectedState(FileBin_Cut);
			CutButton->Button_UseVariable(&m_button);
		}

		// Copy
		MyControl=OpenChild_SubControl(FileBin_Color_Copy,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *CopyButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (CopyButton)
		{
			CopyButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			CopyButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			CopyButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			CopyButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			CopyButton->Button_SetSelectedState(FileBin_Copy);
			CopyButton->Button_UseVariable(&m_button);
		}

		// Paste
		MyControl=OpenChild_SubControl(FileBin_Color_Paste,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *PasteButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (PasteButton)
		{
			PasteButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			PasteButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			PasteButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			PasteButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			PasteButton->Button_SetSelectedState(FileBin_Paste);
			PasteButton->Button_UseVariable(&m_button);
		}

		// Delete
		MyControl=OpenChild_SubControl(FileBin_Color_Delete,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *DeleteButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (DeleteButton)
		{
			DeleteButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			DeleteButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			DeleteButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			DeleteButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			DeleteButton->Button_SetSelectedState(FileBin_Delete);
			DeleteButton->Button_UseVariable(&m_button);		
		}

		// NewFolder
		MyControl=OpenChild_SubControl(FileBin_Color_NewFolder,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *NewFolderButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (NewFolderButton)
		{
			NewFolderButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			NewFolderButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			NewFolderButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			NewFolderButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			NewFolderButton->Button_SetSelectedState(FileBin_NewFolder);
			NewFolderButton->Button_UseVariable(&m_button);
		}

		// Edit
		MyControl=OpenChild_SubControl(FileBin_Color_Edit,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *EditButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (EditButton)
		{
			EditButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			EditButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			EditButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			EditButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			EditButton->Button_SetSelectedState(FileBin_Edit);
			EditButton->Button_UseVariable(&m_button);
		}

		// ListIcon
		MyControl=OpenChild_SubControl(FileBin_Color_ListIcon,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *ListIconButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (ListIconButton)
		{
			ListIconButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			ListIconButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			ListIconButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			ListIconButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			ListIconButton->Button_SetSelectedState(FileButton_ViewMode_NameOnly);
			ListIconButton->Button_UseVariable(&m_viewMode);
		}

		// SmallIcon
		MyControl=OpenChild_SubControl(FileBin_Color_SmallIcon,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *SmallIconButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (SmallIconButton)
		{
			SmallIconButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			SmallIconButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			SmallIconButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			SmallIconButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			SmallIconButton->Button_SetSelectedState(FileButton_ViewMode_SmallIcon);
			SmallIconButton->Button_UseVariable(&m_viewMode);
		}

		// LargeIcon
		MyControl=OpenChild_SubControl(FileBin_Color_LargeIcon,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *LargeIconButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (LargeIconButton)
		{
			LargeIconButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			LargeIconButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			LargeIconButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			LargeIconButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			LargeIconButton->Button_SetSelectedState(FileButton_ViewMode_LargeIcon);
			LargeIconButton->Button_UseVariable(&m_viewMode);
		}

		// Back
		MyControl=OpenChild_SubControl(FileBin_Color_Back,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *BackButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (BackButton) 
		{
			BackButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			BackButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			BackButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			BackButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			BackButton->Button_SetSelectedState(FileBin_Back);
			BackButton->Button_UseVariable(&m_button);
		}

		// Forward
		MyControl=OpenChild_SubControl(FileBin_Color_Forward,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *ForwardButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (ForwardButton)
		{
			ForwardButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			ForwardButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			ForwardButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			ForwardButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			ForwardButton->Button_SetSelectedState(FileBin_Forward);
			ForwardButton->Button_UseVariable(&m_button);
		}

		// Favorites
		MyControl=OpenChild_SubControl(FileBin_Color_Favorites,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *FavoritesButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (FavoritesButton)
		{
			FavoritesButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			FavoritesButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			FavoritesButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			FavoritesButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			FavoritesButton->Button_SetSelectedState(FileBin_Favorites);
			FavoritesButton->Button_UseVariable(&m_button);
		}

		// AddFav
		MyControl=OpenChild_SubControl(FileBin_Color_AddFav,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *AddFavButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (AddFavButton)
		{
			AddFavButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			AddFavButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			AddFavButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			AddFavButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			AddFavButton->Button_SetSelectedState(FileBin_AddFav);
			AddFavButton->Button_UseVariable(&m_button);
		}

		// DeleteFav
		MyControl=OpenChild_SubControl(FileBin_Color_DeleteFav,"SkinControl_SubControl_Button");
		SkinControl_SubControl_Button *DeleteFavButton=GetInterface<SkinControl_SubControl_Button>(MyControl);
		if (DeleteFavButton)
		{
			DeleteFavButton->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			DeleteFavButton->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			DeleteFavButton->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			DeleteFavButton->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			DeleteFavButton->Button_SetSelectedState(FileBin_DeleteFav);
			DeleteFavButton->Button_UseVariable(&m_button);
		}

		// ButtonViewMenu
		HWND hWndListButton=OpenChild(FileBin_Color_ButtonViewMenu,"UtilLib_ButtonLabel");
		m_viewModeLabelButton=GetWindowInterface<UtilLib_ButtonLabel>(hWndListButton);
		if (m_viewModeLabelButton)

		{
			m_viewModeLabelButton->Button_SetSelectedState(FileBin_ButtonViewMenu);
			m_viewModeLabelButton->Button_UseVariable(&m_button);
			m_viewModeLabelButton->AddDependant(this);
			for (unsigned i = 0; i < 4; i++)
			{
				TextItem* to = m_viewModeLabelButton->ButtonLabel_GetTextItem(i);
				TextItem* from = m_buttonLabelText + i;
				*to = *from;
			}
		}
	}

	// Create the scrollbar & arrows
	{
		HWND hWndScrollbar=OpenChild(FileBin_Color_ScrollBar,"UtilLib_Slider");
		m_slider=GetWindowInterface<UtilLib_Slider>(hWndScrollbar);
		if (m_slider)
		{
			// Set all of the Dynamics for the scroller
			m_slider->Slider_SetVariable(&m_scrollBarVar);
			m_slider->Slider_SetMinVariable(&m_scrollBarMin);
			m_slider->Slider_SetMaxVariable(&m_scrollBarMax);
			m_slider->Slider_SetSliderWidth(&m_scrollBarWidth);

			m_slider->Button_SetResource(Controls_Button_UnSelected, &m_sliderBitmaps[Controls_Button_UnSelected]);
			m_slider->Button_SetResource(Controls_Button_Selected, &m_sliderBitmaps[Controls_Button_Selected]);
			m_slider->Button_SetResource(Controls_Button_MouseOver, &m_sliderBitmaps[Controls_Button_MouseOver]);
			m_slider->Button_SetResource(Controls_Button_MouseOverDn, &m_sliderBitmaps[Controls_Button_MouseOverDn]);
		}

		// get the Up arrow
		MyControl=OpenChild_SubControl(FileBin_Color_ScrollBarUp,"SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *UpArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (UpArrow)
		{
			UpArrow->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			UpArrow->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			UpArrow->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			UpArrow->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			UpArrow->Slider_SetVariable(&m_scrollBarVar);
			UpArrow->Slider_SetMinVariable(&m_scrollBarMin);
			UpArrow->Slider_SetMaxVariable(&m_scrollBarMax);
			UpArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
			UpArrow->Set_SliderMult(-10.0f);
		}

		// get the Down arrow
		MyControl=OpenChild_SubControl(FileBin_Color_ScrollBarDn,"SkinControl_SubControl_SliderButton");
		SkinControl_SubControl_SliderButton *DnArrow=GetInterface<SkinControl_SubControl_SliderButton>(MyControl);
		if (DnArrow)
		{
			DnArrow->Button_SetResource(Controls_Button_UnSelected, Controls_Button_UnSelected);
			DnArrow->Button_SetResource(Controls_Button_Selected, Controls_Button_Selected);
			DnArrow->Button_SetResource(Controls_Button_MouseOver, Controls_Button_MouseOver);
			DnArrow->Button_SetResource(Controls_Button_MouseOverDn, Controls_Button_MouseOverDn);
			DnArrow->Slider_SetVariable(&m_scrollBarVar);
			DnArrow->Slider_SetMinVariable(&m_scrollBarMin);
			DnArrow->Slider_SetMaxVariable(&m_scrollBarMax);
			DnArrow->Slider_SetSliderWidth(&m_scrollBarWidth);
			DnArrow->Set_SliderMult(10.0f);
		}
	}

	//************  Working with the Details View Mode
	DetailsView_ColumnLayout* columnHeader = NULL;
	{
		// Create the controls
		columnHeader = GetWindowInterface<DetailsView_ColumnLayout>
			(OpenChild(FileBin_Color_ColumnHeader,"DetailsView_ColumnLayout"));
		if (columnHeader)
		{
			// Set Up the Text Spacing
			columnHeader->SBL_SetTextSpacingDyn(&m_textSpacing);
			
			// Set up the tabScrolling dynamics
			columnHeader->BaseWindowLayoutManager_X_Slider_SetMaxVariable(&m_tabScrollBarMax);
			columnHeader->BaseWindowLayoutManager_X_Slider_SetMinVariable(&m_tabScrollBarMin);
			columnHeader->BaseWindowLayoutManager_X_Slider_SetSliderWidth(&m_tabScrollBarWidth);
			columnHeader->BaseWindowLayoutManager_X_Slider_SetVariable(&m_tabScrollBarVar);

			// Setup the screen objects
			columnHeader->SBL_AssignScreenObjects(Controls_Button_UnSelected, &m_DetailsViewBtnBitmaps[Controls_Button_UnSelected]);
			columnHeader->SBL_AssignScreenObjects(Controls_Button_Selected, &m_DetailsViewBtnBitmaps[Controls_Button_Selected]);
			columnHeader->SBL_AssignScreenObjects(Controls_Button_MouseOver, &m_DetailsViewBtnBitmaps[Controls_Button_MouseOver]);
			columnHeader->SBL_AssignScreenObjects(Controls_Button_MouseOverDn, &m_DetailsViewBtnBitmaps[Controls_Button_MouseOverDn]);

			// Spinner to scroll the layout
			JumpSpinner* spinner=GetWindowInterface<JumpSpinner>
				(OpenChild(RGBA(0,165,0),"JumpSpinner"));
			if (spinner)
			{
				spinner->JS_SetLayout(columnHeader);
				spinner->Spinner_UseVariable(&m_tabScrollBarVar);
				spinner->Spinner_SetMinVariable(&m_tabScrollBarMin);
				spinner->Spinner_SetMaxVariable(&m_tabScrollBarMax);
				spinner->Spinner_SetWidth(&m_tabScrollBarWidth);
				spinner->SetAttr(UtilLib_Spinner_LeftRight);
			}
		}


		// Create the list of visible columns by default
		DetailsView_ColumnInfo* columns[BasicFile_DVElem_NumItems];
		for (unsigned i = 0; i < BasicFile_DVElem_NumItems; i++)
		{
			columns[i] = m_BLS_DetailsView_Info.DVI_AddColumnString
				((char*)BasicFile_DVElem_TypeArray[i].name, BasicFile_DVElem_TypeArray[i].min, BasicFile_DVElem_TypeArray[i].max, BasicFile_DVElem_TypeArray[i].init, false);
		}

		// We will show the Full Name type, and modified date by default
		tList<DetailsView_ColumnInfo*> shownColumns;
		shownColumns.Add(columns[BasicFile_DVElem_FullName]);
		shownColumns.Add(columns[BasicFile_DVElem_Type]);

		DetailsView_ColumnInfo* addModifiedColumn=m_BLS_DetailsView_Info.DVI_AddColumnString("Modified",100,-1,150,false,TextItem_LeftAlign|TextItem_CenterVAlign,DVCI_SortType_Numeric);
		shownColumns.Add(addModifiedColumn);
		
		// Show this list
		m_BLS_DetailsView_Info.DVI_SetShownColumnList(shownColumns);
		m_BLS_DetailsView_Info.DVI_SortByColumn(columns[BasicFile_DVElem_FullName], 1);

		//use this detailsviewinfo... instead of the default...
		columnHeader->DVCL_SetDetailsViewInfo(&m_BLS_DetailsView_Info);
		m_columnHeader=columnHeader;
	}


	// Create the DirectoryLayout in a scroller window and create the pathLayout
	{
		// mwatkins TODO - rid ourselves of this const cast bullshit!!!
		HWND hWndScrollwindow=OpenChild(FileBin_Color_Scrollwindow,const_cast<char*>(GetDirectoryLayoutType()));
		if (!hWndScrollwindow) _throw("StretchyFileBin::Could not create the DirectoryLayout.");
		m_fileLayout=GetWindowInterface<DirectoryLayout>(hWndScrollwindow);
		if (!m_fileLayout) _throw("StretchyFileBin::scrollwindow could not case to DirectoryLayout.");

		// Setup the sliders
		m_fileLayout->BaseWindowLayoutManager_Y_Slider_SetVariable(&m_scrollBarVar);
		m_fileLayout->BaseWindowLayoutManager_Y_Slider_SetMaxVariable(&m_scrollBarMax);
		m_fileLayout->BaseWindowLayoutManager_Y_Slider_SetMinVariable(&m_scrollBarMin);
		m_fileLayout->BaseWindowLayoutManager_Y_Slider_SetSliderWidth(&m_scrollBarWidth);

		// Listen to the ViewMode of the Directory Layout
		m_fileLayout->m_viewMode->AddDependant(this);
		DynamicCallback(-1, "", NULL, m_fileLayout->m_viewMode);

	// Create the PathLayout for the DirectoryLayout
		HWND hWndPathWindow = OpenChild(FileBin_Color_PathWindow, "PathLayout");
		if (!hWndPathWindow) _throw("StretchyFileBin::Could not create the PathLayout.");
		m_pathLayout=GetWindowInterface<PathLayout>(hWndPathWindow);
		if (!m_pathLayout) _throw("StretchyFileBin::pathLayout could not case to PathLayout.");

		m_fileLayout->m_layoutToInform = m_pathLayout;
		//Give the column layout the filelayout as the asset list provider
		columnHeader->DVCL_SetDetailsViewRowCollection(m_fileLayout);

		// We want to defer drag and drop to this layout
		Control_DragNDrop_RemapTo(m_fileLayout);

		//********************************
		// Initial dir stuff

		const char* newDir = this->GetDefaultDirectory();

		//Make sure that the default dir exists
		if (!newDir || !::NewTek_DoesFolderExist(newDir))
			newDir = FILEBIN_MYCOMPUTER;

		if (m_initializeWithDirectory[0])
			newDir = m_initializeWithDirectory;

		//m_fileLayout->SetDirectory(newDir);
		//m_pathLayout->SetNewPath(newDir, m_fileLayout);
		
		m_fileLayout->SetInitialDirectory(newDir);
		m_pathLayout->SetNewPath(m_fileLayout->GetDirectory(), m_fileLayout);

		//********************************

	}

	// Create the favorites panel last
	OpenChild(FileBin_Color_FavTabPanel , (char*)GetFileBinFavouritesType() );
	AutoSkinControl::InitializeSubControls();

	if (NewTek_IsPopup( GetWindowHandle() ) )
		SetKBFocus();

	// Set the viewmode
	m_viewMode.Changed();
}
////////////////////////////////////////////////////////////////////////////////////////////
const char* StretchyFileBin::GetDefaultDirectory()
{
	return FILEBIN_MYCOMPUTER;
}
////////////////////////////////////////////////////////////////////////////////////////////
void StretchyFileBin::SkinChangedCallback(char* newSkinPath)
{
	// TODO, place all of this in CallBack_AssetChanged, listening to Assets properly
		// We need AssetFile to do this properly
}
////////////////////////////////////////////////////////////////////////////////////////////
bool	StretchyFileBin::StretchyFileBin_InitDirectory(const char* p_dir,bool PassiveToBLS)
{
	if (!p_dir)
		return false;

	bool Abort = PassiveToBLS;
	if (PassiveToBLS && m_fileLayout)
	{
		const char *Path = m_fileLayout->GetDirectory();
		assert(Path); //DynamicStrings are not allowed to be NULL
		
		// We'll plan to abort unless it equals the FILEBIN_MYCOMPUTER in which case we implement our default path
		Abort = (0 != ::strcmp(Path, FILEBIN_MYCOMPUTER));
	}
	if (Abort) 
		return false;

	if (m_fileLayout)
		return m_fileLayout->SetInitialDirectory(p_dir);
	else
	{
		strcpy(m_initializeWithDirectory, p_dir);	// Hold on to this path until we are initialized
		return true;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////
const char* StretchyFileBin::StretchyFileBin_GetDirectory(char* p_dir)
{
	if (p_dir)
		return m_fileLayout->GetDirectory(p_dir);
	else return m_fileLayout->GetDirectory();
}
/////////////////////////////////////////////////////////////////////////////////////////
void	StretchyFileBin::StretchyFileBin_SetDirectory(const char* p_dir)
{
	if (!p_dir) 
		return;

	if (m_fileLayout)
		m_fileLayout->SetDirectory(p_dir);
	else 
		strcpy(m_initializeWithDirectory, p_dir);	// Hold on to this path until we are initialized
}
/////////////////////////////////////////////////////////////////////////////////////////
void StretchyFileBin::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanged)
{
	if (ID == (long)(&m_viewMode))
	{
		if (strcmp(String, Controls_Button_ReleasedOn) == 0)
		{	if (( m_fileLayout ) && (m_fileLayout->m_viewMode) )
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(m_viewMode.Get());
		}
	}
	else if (strcmp(String, Controls_Button_ReleasedOn) == 0)
	{
		int whichButton = m_button.Get();
		if ((whichButton == FileBin_Favorites)&&(IsWindowVisible()))
		{	
			FavoriteFileList* favList = SFB_GetFavoriteFileList();
			if (favList)
			{	
				tList<TextItem*> Favourites[4];
				unsigned i=0;
				for(i=0;i<4;i++)
					favList->GetTextItemList(&Favourites[i]);
				for(i=0;i<Favourites[0].NoItems();i++)
				{	for(unsigned j=0;j<4;j++)
					{	Favourites[j][i]->SetFont(HotList_Font);
						Favourites[j][i]->SetFontSize(HotList_FontSize[j]);
						//WindowPixel Temp=Favourites[j][i]->GetColorWPX();
						Favourites[j][i]->Set_Background(true);
						Favourites[j][i]->Set_BackgroundColor(HotList_BGColor[j][0],HotList_BGColor[j][1],HotList_BGColor[j][2]);
						Favourites[j][i]->SetColor(HotList_FGColor[j][0],HotList_FGColor[j][1],HotList_FGColor[j][2]);
						Favourites[j][i]->SetUnderline(HotList_Underline[j]);
						Favourites[j][i]->SetBold(HotList_Bold[j]);
						Favourites[j][i]->SetItalic(HotList_Italic[j]);
						Favourites[j][i]->SetAlignment(TextItem_LeftAlign);
						//Favourites[j][i]->SetBorder(true);
						//Favourites[j][i]->SetBorderColor(0,0,0);
					}					
				}
				// Now go ahread and make the stretchy menu list
				tList<StretchyMenuGroup*>	m_StretchyMenuGroup;
				for(i=0;i<Favourites[0].NoItems();i++)
				{	m_StretchyMenuGroup.Add(new StretchyMenuGroup);
					for(unsigned j=0;j<4;j++)
						m_StretchyMenuGroup[m_StretchyMenuGroup.NoItems()-1]->m_screenObjects[j]=Favourites[j][i];
				}

				RECT menuRect = CreateMenu(&m_StretchyMenuGroup);
				if (menuRect.left == 0)
				{
					RECT favRect;
					if (m_ScreenObject_StretchySkin->GetRect(FileBin_Color_Favorites, favRect))
					{
						// Find the center, top, and width;
						long center = (favRect.left+favRect.right) / 2;
						//long top = favRect.bottom;
						long width = menuRect.right - menuRect.left;
						long height = menuRect.bottom - menuRect.top;
						//long menuWidth = menuRect.right - menuRect.left;
						//width = max(menuWidth,width*2);
						menuRect.left   = favRect.left/*center - width/2*/ + GetWindowPosX();
						menuRect.right  = menuRect.left+width;
						menuRect.top    = favRect.bottom+GetWindowPosY();
						menuRect.bottom = menuRect.top+height;
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
					//SetKBFocus();
					if (menuRet > -1)
						m_fileLayout->SetDirectory((*favList)[menuRet]);

					// Fix for KB focus being assigned elsewhere
					SetKBFocus();
				}

				// Free memory
				for(i=0;i<Favourites[0].NoItems();i++)
				{	delete m_StretchyMenuGroup[i];				
					for(unsigned j=0;j<4;j++)
						delete Favourites[j][i];
				}
			}
		}
		else if ((whichButton == FileBin_ButtonViewMenu) && (this->m_fileLayout))
		{
			tList<char*> viewModes;
			viewModes.Add( TL_GetString( "%View Modes") );
			viewModes.Add( TL_GetString( "Full" ) );	// mwatkins - Added this here and bumped case statements below and added handling for view mode changed to select proper cb text
			viewModes.Add( TL_GetString( "Large Icon") );
			viewModes.Add( TL_GetString( "Small Icon") );
			viewModes.Add( TL_GetString( "Name Only") );
			viewModes.Add( TL_GetString( "Icon Only") );
			viewModes.Add( TL_GetString( "Details") );
			viewModes.Add( TL_GetString( "Details Only") );

			RECT menuRect = CreateMenu(&viewModes);
			if (menuRect.left == 0)
			{
				RECT favRect;
				if (m_ScreenObject_StretchySkin->GetRect(FileBin_Color_ButtonViewMenu, favRect))
				{
					// Find the center, top, and width;
					long center = (favRect.left+favRect.right) / 2;
					long top = favRect.bottom;
					long width = favRect.right - favRect.left;
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
				long width = m_fileLayout->GetWindowWidth();
				switch (menuRet)
				{
				case 1:	// mwatkins - Added this and bumped case statements below (need to pass width like small icon does)
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(FileButton_ViewMode_SuperLargeIcon, width);
					break;
				case 2:
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(FileButton_ViewMode_LargeIcon);
					break;
				case 3:
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(FileButton_ViewMode_SmallIcon, width);
					break;
				case 4:
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(FileButton_ViewMode_NameOnly, width);
					break;
				case 5:
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(FileButton_ViewMode_IconOnly);
					break;
				case 6:
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(FileButton_ViewMode_DetailsIcon);
					break;
				case 7:
					m_fileLayout->m_viewMode->FileButton_ViewMode_SetViewMode(FileButton_ViewMode_Details);
					break;
				}

				// Fix for KB focus being assigned elsewhere
				SetKBFocus();
			}
		}
		else if (whichButton == FileBin_AddFav)
		{
			FavoriteFileList* favList = SFB_GetFavoriteFileList();
			if (favList)
			favList->Add( m_fileLayout->GetDirectory() );
		}
		else if (whichButton == FileBin_DeleteFav)
		{
			FavoriteFileList* favList = SFB_GetFavoriteFileList();
			if (favList)
			favList->Delete(m_fileLayout->GetDirectory());
		}
		else if (whichButton == FileBin_Forward)
		{
			m_fileLayout->SetCurrentPosition(m_fileLayout->GetCurrentPosition()+1);
		}
		else if (whichButton == FileBin_Back)
		{
			unsigned cp = m_fileLayout->GetCurrentPosition();
			if (cp > 0)
				m_fileLayout->SetCurrentPosition(cp-1);
		}
		else if (whichButton == FileBin_NewFolder)
		{
			m_fileLayout->DirLayout_CreateNewFolder();
		}
		else if (whichButton == FileBin_Edit)
		{
			m_fileLayout->EditProperties();
		}
		else if (whichButton == FileBin_Cut)
		{
			FBin_Cut();
		}
		else if (whichButton == FileBin_Copy)
		{
			FBin_Copy();
		}
		else if (whichButton == FileBin_Paste)
		{
			FBin_Paste();
		}
		else if (whichButton == FileBin_Delete)
		{
			FBin_Delete();
		}
		m_button.Set(FileBin_NoButton);
	}
	else if (m_fileLayout && (ItemChanged == m_fileLayout->m_viewMode))
	{
		m_viewMode.Set(m_fileLayout->m_viewMode->FileButton_ViewMode_GetViewMode());
		if (m_viewModeLabelButton)
		{
			int viewMode = m_fileLayout->m_viewMode->FileButton_ViewMode_GetViewMode();
			char* viewModeString = NULL;
			switch (viewMode)
			{
			case FileButton_ViewMode_SuperLargeIcon:	// mwatkins - Select proper CB item
				viewModeString = "Full"; break;
			case FileButton_ViewMode_LargeIcon:
				viewModeString = "Large Icon";	break;
			case FileButton_ViewMode_SmallIcon:
				viewModeString = "Small Icon";	break;
			case FileButton_ViewMode_NameOnly:
				viewModeString = "Name Only";	break;
			case FileButton_ViewMode_Details:
				viewModeString = "Details Only";	break;
			case FileButton_ViewMode_DetailsIcon:
				viewModeString = "Details";	break;
			default:
				viewModeString = "Icon Only";	break;
			}
			char viewString[32];

			//m_viewModeLabelButton->
			//sprintf(viewString, "%s - %s", viewModeString, nameModeString);
			sprintf(viewString, "%s", viewModeString);

			m_viewModeLabelButton->ButtonLabel_SetTextSelection(viewString);
			m_viewModeLabelButton->RePaint();	// mwatkins was here
		}
	}
	else
		AutoSkinControl::DynamicCallback(ID,String,args,ItemChanged);
}
////////////////////////////////////////////////////////////////////////////////////////////

bool StretchyFileBin::FBin_Cut()
{
	return m_fileLayout->EditCut();
	// TODO: Place warning message on failed Cut
}
////////////////////////////////////////////////////////////////////////////////////////////

bool StretchyFileBin::FBin_Copy()
{
	return m_fileLayout->EditCopy();
	// TODO: Place warning message on failed Copy
}
////////////////////////////////////////////////////////////////////////////////////////////

bool StretchyFileBin::FBin_Paste()
{
	return m_fileLayout->EditPaste();
	// TODO: Place warning message on failed Paste
}
////////////////////////////////////////////////////////////////////////////////////////////

bool StretchyFileBin::FBin_Delete()
{
	return m_fileLayout->EditDelete();
	// TODO: Place warning message on failed Delete
}
////////////////////////////////////////////////////////////////////////////////////////////


void StretchyFileBin::InitialiseWindow(void)
{	
	m_ScreenObject_StretchySkin=GetInterface<Auto_StretchySkin>(NewTek_New("Auto_StretchySkin"));
	if (!m_ScreenObject_StretchySkin) _throw("Cannot create the stretchy skin !");
	Canvas_SetResource(m_ScreenObject_StretchySkin);

	AutoSkinControl::InitialiseWindow();
}
////////////////////////////////////////////////////////////////////////////////////////////
void StretchyFileBin::DestroyWindow(void)
{	RemoveAllDependantsToThis();

	//force details column layout to update the layout (back to default viewinfo) early before closing any
	//of the other windows
	if (m_columnHeader) m_columnHeader->DVCL_SetDetailsViewInfo(NULL);

	AutoSkinControl::DestroyWindow();
	if (m_ScreenObject_StretchySkin) 
			NewTek_Delete(m_ScreenObject_StretchySkin);
	m_ScreenObject_StretchySkin = NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
// From Control_DragNDrop
WindowLayout_Item* StretchyFileBin::DragNDrop_DropItemsHere
								(	int Width,int Height,				// Window size
									int MousePosnX,int MousePosnY,		// Mouse posn
									WindowLayout_Item *ObjectsInWindow,	// The objects already in the window
									WindowLayout_Item *ObjectsDropped,	// The objects that have been dropped in the window
									bool Resizing,bool DroppedHere
									)
{
	if (DroppedHere)
	{
		// We scan through all the items, and find out how many we have
		WindowLayout_Item *Scan=ObjectsDropped;
		unsigned numFilesDropped = 0;
		FileButton* firstDropped = NULL;
		while(Scan && (numFilesDropped < 2))
		{	// We need to get the BaseWindowClass interface
			FileButton			*FB=GetWindowInterface<FileButton>(Scan->hWnd);
			if (FB && FB->DragAndDrop_AmISelected())
			{
				firstDropped = FB;
				numFilesDropped++;
			}
			Scan=Scan->Next;
		}

		// If there is only One FileButton, try to set the directory with it.
		bool copyAll = false;
		if (numFilesDropped == 1)
			copyAll = !(m_fileLayout->DeferredSetDirectory(firstDropped->GetFileName()));

		// If more than one was selected, or the directory change didn't work, try copying
		if (copyAll || (numFilesDropped > 1))
		{
			// Check to see if the CTRL key is pressed
			long ctrlPressed = 0;
			if (GetAsyncKeyState(VK_CONTROL)&((short)(1<<15)))
				ctrlPressed = 1;

			// Get the name of the folder we are copying to
			const char* folderName = m_fileLayout->GetDirectory();

			// Scan throught the dropped items
			Scan=ObjectsDropped;
			while(Scan)
			{	// We need to get the BaseWindowClass interface
				FileButton			*FB=GetWindowInterface<FileButton>(Scan->hWnd);
				if (FB && FB->DragAndDrop_AmISelected())
				{
					if (ctrlPressed)
						FB->CopyFileTo(folderName);
					else
						FB->MoveFileTo(folderName);
				}
				Scan=Scan->Next;
			}
		}

		// We scan through all the items and delete them ALL!
		Scan=ObjectsDropped;
		while(Scan)
		{	
			BaseWindowClass* bwc = GetWindowInterface<BaseWindowClass>(Scan->hWnd);
			NewTek_Delete<BaseWindowClass>(bwc);
			Scan->hWnd = NULL;
			Scan=Scan->Next;
		}
	}
	return Control_DragNDrop::DragNDrop_DropItemsHere
		(Width,Height,MousePosnX,MousePosnY,ObjectsInWindow,ObjectsDropped,Resizing,DroppedHere);
}
/////////////////////////////////////////////////////////////////////////////////////////
bool StretchyFileBin::DragNDrop_CanItemBeDroppedHere(HWND hWnd,Control_DragNDrop_DropInfo *Dropped)
{
	// Do not allow if not a FileButton
	FileButton* thisButton = GetWindowInterface<FileButton>(Dropped->hWnd);
	if (!thisButton) return false;

	long status = thisButton->GetStatus();
	if (status == FileProperties_NoFile)
		return false;

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////
bool StretchyFileBin::ExecuteCommand(	char *FunctionName,						// The string representation of the command to execute
										ScriptExecutionStack &InputParameters,	// The set of input parameters to your function
										ScriptExecutionStack &OutputParameters,	// You can use this to access variables as well
										char *&Error)							// If you supported the command but failed for some reason
																				// return a user string here
{	if (!strcmp(FunctionName,"AddFavorite"))
	{	FavoriteFileList* favList = SFB_GetFavoriteFileList();
		if (favList) favList->Add(m_fileLayout->GetDirectory());
		return true;
	}
	else if (!strcmp(FunctionName,"ShowFavorites"))
	{	m_button.Set(FileBin_Favorites,Controls_Button_ReleasedOn);
		return true;
	}
	else if (!strcmp(FunctionName,"DeleteFavorite"))
	{	FavoriteFileList* favList = SFB_GetFavoriteFileList();
		if (favList) favList->Delete(m_fileLayout->GetDirectory());
		return true;
	}
	else if (!strcmp(FunctionName,"Forward"))
	{	if (m_fileLayout) m_fileLayout->SetCurrentPosition(m_fileLayout->GetCurrentPosition()+1);
		return true;
	}
	else if (!strcmp(FunctionName,"Backward"))
	{	unsigned cp = m_fileLayout->GetCurrentPosition();
		if (cp>0)
		{
			if (m_fileLayout) m_fileLayout->SetCurrentPosition(cp-1);
		}
		return true;
	}
	else if (!strcmp(FunctionName,"NewFolder"))
	{	
		char* newName = NULL;
		if ((InputParameters.GetSizeOfStack()>0) &&
			(InputParameters[0].GetType()==Script_STRING))
			newName = InputParameters[0];
		if (m_fileLayout) m_fileLayout->DirLayout_CreateNewFolder(newName);
		return true;
	}
	else if (!strcmp(FunctionName,"SetDirectoryPath"))
	{    if (InputParameters.GetSizeOfStack()!=1) 
            {    Error="Not enough parameters.";
                  return false;
            }

        // Set the directory here
        if (m_fileLayout) 
            m_fileLayout->SetDirectory((char*)InputParameters[0]);
        
        return true;
	}
	else if (!strcmp(FunctionName,"FileProperties"))
	{	if (m_fileLayout) m_fileLayout->EditProperties();

		return true;
	}
	else if (!strcmp(FunctionName,"CutFile"))
	{	if (m_fileLayout) m_fileLayout->EditCut();
		return true;
	}
	else if (!strcmp(FunctionName,"CopyFile"))
	{	if (m_fileLayout) m_fileLayout->EditCopy();
		return true;
	}
	else if (!strcmp(FunctionName,"PasteFile"))
	{	if (m_fileLayout) m_fileLayout->EditPaste();
		return true;
	}
	else if (!strcmp(FunctionName,"DeleteFile"))
	{	if (m_fileLayout) m_fileLayout->EditDelete();
		return true;
	}
	else if (!strcmp(FunctionName,"UpFolder"))
	{	char Temp[MAX_PATH];
		strcpy(Temp,StretchyFileBin_GetDirectory());
		char *Slash1=NewTek_GetLastSlash(Temp);
		if (Slash1) Slash1[0]=0;
		else strcpy(Temp,FILEBIN_MYCOMPUTER);
		StretchyFileBin_SetDirectory(Temp);
		return true;
	}
	else if (!strcmp(FunctionName,"SetViewMode_LargeIcon"))
	{
		m_viewMode.Set(FileButton_ViewMode_LargeIcon, Controls_Button_ReleasedOn);
		return true;
	}
	// mwatkins - TODO: Script handling for Super Large Icon view mode
	else if (!strcmp(FunctionName,"SetViewMode_SmallIcon"))
	{
		m_viewMode.Set(FileButton_ViewMode_SmallIcon, Controls_Button_ReleasedOn);
		return true;
	}
	else if (!strcmp(FunctionName,"SetViewMode_NameOnly"))
	{
		m_viewMode.Set(FileButton_ViewMode_NameOnly, Controls_Button_ReleasedOn);
		return true;
	}
	else if (!strcmp(FunctionName,"SetViewMode_IconOnly"))
	{
		m_viewMode.Set(FileButton_ViewMode_IconOnly, Controls_Button_ReleasedOn);
		return true;
	}
	else if (!strcmp(FunctionName,"SetViewMode_Details"))
	{
		m_viewMode.Set(FileButton_ViewMode_Details, Controls_Button_ReleasedOn);
		return true;
	}
	else if (!strcmp(FunctionName,"SetViewMode_DetailsIcon"))
	{
		m_viewMode.Set(FileButton_ViewMode_DetailsIcon, Controls_Button_ReleasedOn);
		return true;
	}
	else if (!strcmp(FunctionName,"LoadScrollBarBitmap"))
	{
		if (InputParameters.GetSizeOfStack()<2)
		{
			Error = "LoadScrollBarBitmap: Function Missing Parameters, aborted";
			return true;
		}
		if (InputParameters[0].GetType()==Script_STRING)
		{
			int i=InputParameters[1];
			if ((i >= 0) && (i < 4))
			{	char TempBuffer[MAX_PATH];
				if (!m_sliderBitmaps[i].ChangeFilename(NewTek_GetAbsolutePath(m_skinPath,InputParameters[0],TempBuffer), true ))
					Error = "LoadScrollBarBitmap: Failed to Load Layer Image";
				m_sliderBitmaps[i].SetAlignment(BitmapTile_StretchX+BitmapTile_StretchY);
				m_sliderBitmaps[i].ForceTo32BPP();
			}
			else Error = "LoadScrollBarBitmap: Invalid Parameter 1 (Use 0-3), aborted";
		}
		else Error = "LoadScrollBarBitmap: Invalid Parameter 0, aborted";
		return true;
	}
	else if (!strcmp(FunctionName,"GetLayout"))
	{	ScriptExecutionStack *NewStack=ScriptExecutionStack::GetNewStack(&OutputParameters);
		NewStack->SetContext(m_fileLayout);
		OutputParameters.Add()=NewStack;
		NewStack->Release();
		return true;
	}
	else if (!strcmp(FunctionName,"SetDropDownTextItem"))
	{
		if (InputParameters.GetSizeOfStack()!=1)
		{	Error = "SetDropDownTextItem: Function Missing Parameters, aborted";
			return true;
		}

		int i=InputParameters[0];
		if ((i >= 0) && (i < 4))
		{	ScriptExecutionStack *NewStack=ScriptExecutionStack::GetNewStack(&OutputParameters);
			NewStack->SetContext(&m_buttonLabelText[i]);
			OutputParameters.Add()=NewStack;
			NewStack->Release();
			return true;				
		}
		else Error = "SetDropDownTextItem: Invalid Parameter 0 (Use 0-3), aborted";
		return true;
	}
	if (!strcmp(FunctionName,"LoadDetailsBtnBmp"))
	{
		int stackSize = InputParameters.GetSizeOfStack();
		if (stackSize<2)
		{
			Error = "LoadDetailsBtnBmp: Function Missing Parameters, aborted";
			return true;
		}
		if (InputParameters[0].GetType()==Script_STRING)
		{
			int i=InputParameters[1];

			// Get the buffers on the sides
			long l = (stackSize >= 3) ? InputParameters[2] : 0;
			long r = (stackSize >= 4) ? InputParameters[3] : 0;
			long t = (stackSize >= 5) ? InputParameters[4] : 0;
			long b = (stackSize >= 6) ? InputParameters[5] : 0;

			if ((i >= 0) && (i < 4))
			{	char TempBuffer[MAX_PATH];
				if (!m_DetailsViewBtnBitmaps[i].ChangeFilename(NewTek_GetAbsolutePath(m_skinPath,InputParameters[0],TempBuffer), true ))
					Error = "LoadDetailsBtnBmp: Failed to Load Layer Image";
				m_DetailsViewBtnBitmaps[i].SetAlignment(BitmapTile_StretchX|BitmapTile_StretchY);
				m_DetailsViewBtnBitmaps[i].ForceTo32BPP();
				m_DetailsViewBtnBitmaps[i].SOSB_SetNonStretchRegions(l,r,t,b);

				// Set the text spacing they will all look at
				if (l > m_textSpacing.Get())
					m_textSpacing.Set(l);
				if (r > m_textSpacing.Get())
					m_textSpacing.Set(r);
			}
			else Error = "LoadDetailsBtnBmp: Invalid Parameter 1 (Use 0-3), aborted";
		}
		else Error = "LoadDetailsBtnBmp: Invalid Parameter 0, aborted";
		return true;
	}
	return AutoSkinControl::ExecuteCommand(FunctionName, InputParameters, OutputParameters, Error);
}
//////////////////////////////////////////////////////////////////////////////////////////
void StretchyFileBin::BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion)
{	MajorVersion=1;
	MinorVersion=2;
}
//////////////////////////////////////////////////////////////////////////////////////////
bool StretchyFileBin::BaseLoadSave_SaveData(SaveData *SaveContext)
{
	// The file name
	if (!SaveContext->PutString(StretchyFileBin_GetDirectory())) return false;

	// The view mode we are using
	if (!SaveContext->Put(m_viewMode.Get())) return false;

	// The base class
	BaseLoadSave_SaveData_Inhereted(AutoSkinControl,SaveContext);

	// Save the proper column info
	BaseLoadSave* colInfo = &m_BLS_DetailsView_Info;
	BaseLoadSave_SaveData_Member(colInfo, SaveContext);

	return !SaveContext->Error();
}
//////////////////////////////////////////////////////////////////////////////////////////
bool StretchyFileBin::BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion)
{
	// Pedantic Checks
	if (MajorVersion != 1) return false;
	if (MinorVersion > 2) return false;
	if (!LoadContext) return false;
	
	
	// new path
	char newPath[MAX_PATH];
	if (!LoadContext->GetString(newPath, MAX_PATH)) return false;
	
	// the view mode, watch for old BLS that havea wrong view mode
	int viewMode;
	if (!LoadContext->Get(viewMode)) return false;
	if (!viewMode) viewMode = FileButton_ViewMode_LargeIcon;
	
	// the name mode
	int nameMode;
	if (MinorVersion < 2)
	{
		if (!LoadContext->Get(nameMode)) 
			return false;
	}
	
	// The base class
	BaseLoadSave_LoadData_Inhereted(AutoSkinControl,LoadContext);
	
	// Set all of the values
	this->StretchyFileBin_InitDirectory(newPath);

	m_viewMode.Set(viewMode, Controls_Button_ReleasedOn);
	
	// Details View Columns
	if (MinorVersion > 0)
	{
		BaseLoadSave* colInfo = &m_BLS_DetailsView_Info;
		BaseLoadSave_LoadData_Member(colInfo, LoadContext);
	}
	
	return !LoadContext->Error();
}
//////////////////////////////////////////////////////////////////////////////////////////
void StretchyFileBin::OnProcessAttach()
{
}
void StretchyFileBin::OnProsessDetach()
{
}
//////////////////////////////////////////////////////////////////////////////////////////
/*void StretchyFileBin::MouseRButtonRelease(long Flags,long x,long y)
{
	// Create a Context Menu
	tList<BaseWindowClass*>	Windows;
	// Windows.Add(this);
	Windows.Add(m_fileLayout);
	NewTek_ExecuteContextMenu(&Windows);
}*/
//////////////////////////////////////////////////////////////////////////////////////////