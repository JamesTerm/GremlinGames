#include "stdafx.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
Details_ViewMode::Details_ViewMode()
{
	m_DetailsView_Info=NULL;
	m_findThisColsSizeNextRePaint = -1;
	m_maxColumnSize = 0;
	m_lastFileButtonWidth = 0;
	for (unsigned i = 0; i < 3; ++i)
		m_checkBoxScreenObjects[i] = NULL;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

Details_ViewMode::~Details_ViewMode()
{
	if (m_DetailsView_Info) 
		m_DetailsView_Info->DeleteDependant(this);
	for (unsigned i = 0; i < 3; ++i)
	{
		if (m_checkBoxScreenObjects[i])
		{
			m_checkBoxScreenObjects[i]->DeleteDependant(this);
			m_checkBoxScreenObjects[i] = NULL;
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

void Details_ViewMode::DVM_SetDetailsInfo(DetailsView_Info* detailsInfo)
{
	if (detailsInfo != m_DetailsView_Info)
	{
		if (m_DetailsView_Info)
			m_DetailsView_Info->DeleteDependant(this);
		m_DetailsView_Info = detailsInfo;
		m_columnPositions.DeleteAll();
		if (m_DetailsView_Info)
		{
			m_DetailsView_Info->DVI_GetColumnPositions(m_columnPositions);
			m_DetailsView_Info->DVI_GetShownColumnList(m_viewColumnList);
			m_DetailsView_Info->AddDependant(this);
			DynamicCallback(-1, DVI_Changed_Sort, NULL, m_DetailsView_Info);
		}
		else FileButton_ViewMode_SetSortString(NULL);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Set the screen objects the check box will use
void Details_ViewMode::DVM_SetCheckBoxScreenObject(eDetails_ViewMode_Checkbox check, ScreenObject* so)
{
	// Set the proper screen object and listen to it
	if (m_checkBoxScreenObjects[check])
		m_checkBoxScreenObjects[check]->DeleteDependant(this);
	m_checkBoxScreenObjects[check] = so;
	if (so)
		so->AddDependant(this);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

void Details_ViewMode::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	if (ItemChanging == m_DetailsView_Info)
	{
		if (IsDeletion(String))
		{
			m_DetailsView_Info = NULL;
			FileButton_ViewMode_SetSortString(NULL);
		}
		else if (!stricmp(String, DVI_Changed_Sort))
		{
			// The sort has changed, get the sort string
			DetailsView_ColumnInfo* ci = m_DetailsView_Info->DVI_GetSortColumn();
			FileButton_ViewMode_SetSortString(ci ? ci->DVCI_GetMenuString() : NULL);

			// Send the Changed Message along for the layout
			Changed(DVI_Changed_Sort);
		}
		else if (	(!stricmp(String, DVI_Changed_ColumnOrder)) ||
					(!stricmp(String, DVI_Changed_ColumnPos))	)
		{
			m_columnPositions.DeleteAll();
			m_viewColumnList.DeleteAll();
	
			if (m_DetailsView_Info)
			{
				m_DetailsView_Info->DVI_GetColumnPositions(m_columnPositions);
				m_DetailsView_Info->DVI_GetShownColumnList(m_viewColumnList);
			}

			// Send the Changed message if we are looking at details
			if (m_viewMode & FileButton_ViewMode_Details)
				Changed(FileButton_ViewMode_ViewMode_Changed);
			else if ((m_viewMode == FileButton_ViewMode_SmallIcon) || (m_viewMode == FileButton_ViewMode_NameOnly))
			{
				// We will re-compute the layout of the windows if the size of the FileButtons has changed
				if (m_lastFileButtonWidth != FileButton_ViewMode_GetPreferedXSize())
				{
					// Get the layout to re-compute
					UtilLib_SimpleGrid* grid = GetInterface<UtilLib_SimpleGrid>(m_parentWindow);
					if (grid)
						grid->Layout_PerformLayout(grid->GetWindowHandle(), false);
				}
			}
		}
		else if (!stricmp(String, DVCI_Changed_FindColumnWidth))
		{
			// Be sure we are looking at some info in the details view
			if ((m_viewMode == FileButton_ViewMode_SmallIcon) || 
				(m_viewMode == FileButton_ViewMode_NameOnly) || 
				(m_viewMode & FileButton_ViewMode_Details))
			{
				if (m_DetailsView_Info)
				{
					// Grab the list of all visible columns
					tList<DetailsView_ColumnInfo*> visCols;
					m_DetailsView_Info->DVI_GetShownColumnList(visCols);
					
					// Get what index we are trying to increase
					void* Args = args;
					int colIndex = NewTek_GetArguement<int>(Args);
					if ((colIndex >= 0) && (colIndex < visCols.NoItems()))
					{
						// Look at this column the next time there is a RePaint
						m_findThisColsSizeNextRePaint = colIndex;
						m_maxColumnSize = 0;

						// Force a RePaint of all buttons to get the proper sizes
						Changed(FileButton_ViewMode_ViewMode_Changed);

						// Do not get values any more
						m_findThisColsSizeNextRePaint = -1;

						// Set the Column to this new preferred Size
						visCols[colIndex]->DVCI_GetCurrentWidthDyn()->Set(m_maxColumnSize);
						m_maxColumnSize = 0;
					}
				}
			}
		}
		return;
	}
	// Listen for deletion of my Screen Objects
	if (IsDeletion(String))
	{
		for (unsigned i = 0; i < 3; ++i)
		{
			if (ItemChanging == m_checkBoxScreenObjects[i])
				m_checkBoxScreenObjects[i] = NULL;
		}
	}
	HeightWidth_ViewMode::DynamicCallback(ID, String, args, ItemChanging);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

long Details_ViewMode::FileButton_ViewMode_GetPreferedXSize(FileButton* fb)
{
	if ((m_viewMode == FileButton_ViewMode_NameOnly) || 
		(m_viewMode == FileButton_ViewMode_SmallIcon) ||
		(m_viewMode == FileButton_ViewMode_SuperLargeIcon))	// mwatkins
	{
		if (m_parentWindow && ShouldIMatchWidth(fb))
		{
			// We should also adjust for spacing in a simple grid
			UtilLib_SimpleGrid* grid = GetInterface<UtilLib_SimpleGrid>(m_parentWindow);
			long spacing = grid ? grid->UtilLib_SimpleGrid_GetSpacingX() : 0;
			
			// What is the parent's available width (Subtract out 1 spacing
			long parentWidth = m_parentWindow->GetWindowWidth() - spacing;

			// Find the size of the currently selected column
			DetailsView_Info* dvInfo = DVM_GetDetailsInfo();
			DetailsView_ColumnInfo*	dvCol = dvInfo ? dvInfo->DVI_GetSortColumn() : NULL;
			if (dvCol)
			{
				// Here is the item the user has chosen in the field, represent its length
				long dvColLength = dvCol->DVCI_GetCurrentWidthDyn()->Get();

				// Add The icon width to the rec. if we are in small icon mode
				if (m_viewMode == FileButton_ViewMode_SmallIcon)
					dvColLength += FileButton_Size_IconSmallX;
				else if (m_viewMode == FileButton_ViewMode_SuperLargeIcon)	// mwatkins
					dvColLength += FileButton_Size_IconLargeX;	

				// Be sure we are not way over-sized anyway
				if (dvColLength >= (parentWidth - spacing))
					return parentWidth - spacing;
				{
					// We might be able to fit more than one column
					int numCols = parentWidth / dvColLength;

					// How long is each column if I spread them out?
					long colWidth = parentWidth / numCols;

					// Return this width, minus spacing
					return colWidth - spacing;
				}
			}
		}
	}
	return HeightWidth_ViewMode::FileButton_ViewMode_GetPreferedXSize(fb);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void Details_ViewMode::TweakTextItem(FileButton* fileButton, TextItem* textItem, bool missingRef)
{
	// If we are missing the reference, italicize
	if (textItem)
		textItem->SetItalic(missingRef);
	HeightWidth_ViewMode::TweakTextItem(fileButton, textItem, missingRef);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

char* Details_ViewMode::FileButton_ViewMode_GetColumnAtPoint(POINT point, FileButton* fb)
{
	if (m_DetailsView_Info && (m_viewMode & FileButton_ViewMode_Details))
	{
		// We are in some type of details mode
		// PEDANTIUC
		if (!fb) return NULL;

		// Return NULL if we are sitting where the icon was
		if (	(point.x >= fb->m_lastIconRect.left) &&
				(point.x <= fb->m_lastIconRect.right) &&
				(point.y >= fb->m_lastIconRect.top) &&
				(point.y <= fb->m_lastIconRect.bottom))
				return NULL;

		// Find out which colujmn this thing falls in
		unsigned numCols = m_viewColumnList.NoItems();
		for (unsigned i = 0; i < numCols; i++)
		{
			// Return the 
			long rightEdge = m_columnPositions[i*2+1];
			if (i == (numCols-1))
				rightEdge = fb->GetWindowWidth();
			if (point.x <= rightEdge)
				return m_viewColumnList[i]->DVCI_GetMenuString();
		}

		// PEDANTIC, should never reach this
		return NULL;
	}
	else return HeightWidth_ViewMode::FileButton_ViewMode_GetColumnAtPoint(point, fb);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	Details_ViewMode::FileButton_ViewMode_GetEditingRect(RECT& rect,const char *columnName, FileButton* fb)
{
	// Watch to see if we are in details view
	if (m_DetailsView_Info && (m_viewMode & FileButton_ViewMode_Details))
	{
		// Pedantic Check
		if (!fb || !columnName) return false;

		// Find out which column this thing falls in
		unsigned numCols = m_viewColumnList.NoItems();
		for (unsigned i = 0; i < numCols; i++)
		{
			if (!stricmp(columnName, m_viewColumnList[i]->DVCI_GetMenuString()))
			{
				rect.top = 2;
				rect.bottom = fb->GetWindowHeight()-2;
				rect.left = m_columnPositions[i*2];
				rect.right = m_columnPositions[i*2+1];
				return true;
			}
		}

		// If an invalid column was passed, do not do it
		return false;
	}
	else return HeightWidth_ViewMode::FileButton_ViewMode_GetEditingRect(rect, columnName, fb);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void Details_ViewMode::FileButton_ViewMode_Draw
(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect)
{
	m_lastFileButtonWidth = controlRect.right - controlRect.left;
	if (m_DetailsView_Info && (m_viewMode & FileButton_ViewMode_Details))
	{
		tList<RECT> DeferredBorders;
		tList<int> DeferredButtonStates;

		// If this thing has no size, don't even draw it
		if ((controlRect.bottom < 1) || (controlRect.right < 1))
			return;
		if (!fileButton) return;

		// Be sure we are looking at button states 0 - 3
		buttonState = buttonState % 4;

		// Initialize the button to be sure assets are read
		fileButton->DVR_Initialize();
		
		// Get the list of visible columns
		unsigned numCols = m_viewColumnList.NoItems();
		bool shownIcon = false;
		unsigned i = 0;
		for (i = 0; i < numCols; i++)
		{
			// See where this thing should be drawn
			long leftEdge = m_columnPositions[i*2];
			long rightEdge = m_columnPositions[i*2+1];

			if ((rightEdge > controlRect.left) && (leftEdge < controlRect.right))
			{
				// Set the alignment of the Text Item before we paint it
				long alignment = m_viewColumnList[i]->DVCI_GetTextAlignment();
				m_textItemPainted.SetAlignment(alignment);

				// Create the Rect we are drawing in
				RECT thisRect = controlRect;
				thisRect.left = leftEdge;
				thisRect.right = ((i == numCols-1) && (alignment & TextItem_LeftAlign)) ? 
					controlRect.right : rightEdge;
				thisRect.top = controlRect.top;
				thisRect.bottom = controlRect.bottom;
				long iconWidth = 0;

				// Draw the icon on the far left edge
				if (!shownIcon && (leftEdge >= controlRect.left))
				{
					// We need to draw the icon AND the text string, because we are the first column visible
					RECT iconRect, nameRect;
					FileButton_ViewMode_GetRects(iconRect, nameRect, thisRect);
					thisRect = nameRect;
					
					bool drawingIcon = (iconRect.right > iconRect.left) && (iconRect.bottom > iconRect.top);

					if (drawingIcon)
					{
						iconWidth = iconRect.right - iconRect.left;
						UFBVM_DrawIcon(hWnd, hdc, fileButton, buttonState, ColorTint, iconRect);
					}
					// mwatkins - added to fix VT[4.5] bug #6475
					else if (fileButton)
					{
						::SetRect(&(fileButton->m_lastIconRect), 0, 0, 0, 0);
					}

					// We don't need to show the icon again
					shownIcon = true;
				}

				// Grab this column name
				char* thisColumnName = m_viewColumnList[i]->DVCI_GetMenuString();
				bool missingRef = true;
				char textFromFileName[MAX_PATH] = {0};
				bool drawOwnName = true;
				long prefTextWidth;

				// Grab the associated column element
				DetailsView_Element* columnElement = fileButton->DVR_GetDetailsViewElement(thisColumnName);
				if (columnElement)
				{
					// Init
					columnElement->DVE_Initialize();

					// We have a reference
					missingRef = false;

					long* ptw = (i == m_findThisColsSizeNextRePaint) ? &prefTextWidth : NULL;

					// Let the element try to draw itself
					if (columnElement->DVE_DrawItem
						(this, fileButton, buttonState, ColorTint, hWnd, hdc, thisRect, ptw))
						drawOwnName = false;
					else	// Just get the string and draw it ourselves 
						columnElement->DVE_GetTextString(textFromFileName,MAX_PATH);
					if (!textFromFileName || !textFromFileName[0])
					{
						missingRef = true;
						textFromFileName[0] = 0;
					}

					//James was here...
					if ((fileButton->DragAndDrop_AmISelected())&&(columnElement->DVE_AllowPopupEdit())) { //if the row is selected too...
						//If this element is selected draw a selection border
						const char *ElementName=columnElement->DVE_GetColumnName();
						if (fileButton->GetDetailsColumnSelection() && ElementName) {
							if (!(stricmp(fileButton->GetDetailsColumnSelection(),ElementName))) {
								RECT SelectionRect=thisRect;
								SelectionRect.left+=1;
								SelectionRect.right-=1;
								//SelectionRect.top+=1;
								//SelectionRect.bottom-=1;

								//RECT *newrect=new RECT();
								//memcpy(newrect,&SelectionRect,sizeof(RECT));
								DeferredBorders.Add(SelectionRect);
								DeferredButtonStates.Add(buttonState);
								//UFBVM_DrawBorder(hWnd, hdc, fileButton, buttonState, ColorTint, SelectionRect);
								}
							}
						}

					// Clean up
					columnElement->DVE_Cleanup();
				}

				// If we still need to draw the name ourselves...
				if (drawOwnName)
				{
					UFBVM_DrawName(hWnd, hdc, fileButton, textFromFileName, missingRef, buttonState, ColorTint, thisRect);
					if (i == m_findThisColsSizeNextRePaint)
					{
						long prefTextHeight;
						m_textItemPainted.GetSize(prefTextWidth,prefTextHeight);
						long xoffset, yoffset; m_textItemPainted.GetOffset(xoffset, yoffset);
						prefTextWidth += 2*xoffset;
					}
				}

				// Find out the preferred size
				if (i == m_findThisColsSizeNextRePaint)
				{
					long prefSize = iconWidth + prefTextWidth;
					if (prefSize > m_maxColumnSize)
						m_maxColumnSize = prefSize;
				}

				// Draw a black space at the end if the last element is not left aligned
				if ((i == numCols-1) && !(alignment & TextItem_LeftAlign))
				{
					RECT spaceRect = thisRect;
					spaceRect.left = thisRect.right;
					spaceRect.right = controlRect.right;
					UFBVM_DrawName(hWnd, hdc, fileButton, "", false, buttonState, ColorTint, spaceRect);
				}

				// Draw the spacer
				if (i < numCols-1)
				{
					thisRect.left = thisRect.right;
					thisRect.right = m_columnPositions[i*2+2];
					//UFBVM_DrawBorder(hWnd, hdc, fileButton, buttonState, ColorTint, thisRect);
					//RECT *newrect=new RECT();
					//memcpy(newrect,&thisRect,sizeof(RECT));
					DeferredBorders.Add(thisRect);
					DeferredButtonStates.Add(((buttonState==1)||(buttonState==3))?0:buttonState);
					UFBVM_DrawName(hWnd, hdc, fileButton, "", false, buttonState, ColorTint, thisRect);
				}
			}
		}

		// Draw the border and warnings
		UFBVM_DrawBorder(hWnd, hdc, fileButton, buttonState, ColorTint, controlRect);
		UFBVM_DrawWarning(hWnd, hdc, fileButton, buttonState, ColorTint, controlRect);

		for (i=0;i<DeferredBorders.NoItems();i++) 
			UFBVM_DrawBorder(hWnd, hdc, fileButton, DeferredButtonStates[i], ColorTint, DeferredBorders[i] );

		// All done with the FileButton, let it clean up
		fileButton->DVR_Cleanup();
	}
	else
	{
		HeightWidth_ViewMode::FileButton_ViewMode_Draw(hWnd, hdc, fileButton, buttonState, ColorTint, controlRect);

		// We can grab the width we want, m_textItemPainted should have painted this button last
		if ((m_findThisColsSizeNextRePaint != -1) && 
			((m_viewMode == FileButton_ViewMode_SmallIcon) || (m_viewMode == FileButton_ViewMode_NameOnly) ||
			 (m_viewMode == FileButton_ViewMode_SuperLargeIcon)))	// mwatkins TODO: ???
		{
			long prefTextWidth, prefTextHeight;
			m_textItemPainted.GetSize(prefTextWidth,prefTextHeight);
			long xoffset, yoffset; m_textItemPainted.GetOffset(xoffset, yoffset);
			prefTextWidth += 2*xoffset;

			long prefSize = fileButton->m_lastIconRect.right - fileButton->m_lastIconRect.left + prefTextWidth;
			if (prefSize > m_maxColumnSize)
				m_maxColumnSize = prefSize;
		}
	}
}

char*		Details_ViewMode::FileButton_ViewMode_GetColumnAtIndex(unsigned index, FileButton* fb)
{
	if (m_DetailsView_Info && (m_viewMode & FileButton_ViewMode_Details))
	{
		return (index < m_viewColumnList.NoItems()) ? m_viewColumnList[index]->DVCI_GetMenuString() : NULL;
	}
	else return HeightWidth_ViewMode::FileButton_ViewMode_GetColumnAtIndex(index, fb);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned	Details_ViewMode::FileButton_ViewMode_GetColumnIndex(const char* columnName, FileButton* fb)
{
	if (m_DetailsView_Info && (m_viewMode & FileButton_ViewMode_Details))
	{
		for (unsigned i = 0; i < m_viewColumnList.NoItems(); i++)
		{
			if (!stricmp(m_viewColumnList[i]->DVCI_GetMenuString(), columnName))
				return i;
		}
		return (unsigned)-1;
	}
	else return HeightWidth_ViewMode::FileButton_ViewMode_GetColumnIndex(columnName, fb);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned	Details_ViewMode::FileButton_ViewMode_GetNumColumns(FileButton* fb)
{
	if (m_DetailsView_Info && (m_viewMode & FileButton_ViewMode_Details))
		return m_viewColumnList.NoItems();
	else return HeightWidth_ViewMode::FileButton_ViewMode_GetNumColumns(fb);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

BaseFile_DetailsView_Element::BaseFile_DetailsView_Element(FileButton* fb, unsigned type) :
	UtilLib_DetailsView_Element((char*)BasicFile_DVElem_TypeArray[type].name, false, 
		BasicFile_DVElem_TypeArray[type].min, BasicFile_DVElem_TypeArray[type].max, BasicFile_DVElem_TypeArray[type].init)
{
	m_fileButton = fb;
	m_type = type;
}

bool	BaseFile_DetailsView_Element::DVE_AllowPopupEdit()
{	
	// We can always edit the Alternate file name
	if (m_type == BasicFile_DVElem_AltName)
		return true;

	// Allow name editing of files and folders
	if (		(m_type == BasicFile_DVElem_NameOnly) ||
				(m_type == BasicFile_DVElem_FullName))
	{
		if (m_fileButton)
		{
			long type = m_fileButton->GetStatus();
			if ((type == FileProperties_File) || (type == FileProperties_Directory))
				return true;
		}
	}

	// Nothing else should be edited
	return false;
}

void	BaseFile_DetailsView_Element::DVE_PopupEdit
(BaseWindowClass* parent, RECT& inRect, bool& allowCopyToSelectedSisterWindows, long& reasonForExit,const char *InitString, POINT* mouseRelease, bool HighLightInitString)
{
	// We are never going to allow the Base to copy to all selected
	allowCopyToSelectedSisterWindows = false;
	reasonForExit = Popup_Escape_EscapeKey;

	// Either we are working on the alias OR the actual FileName
	bool editAlias = (m_type == BasicFile_DVElem_AltName);

	// Do some PEDANTIC checks, these should have already been done when we asked
	if (!m_fileButton) return;
	if (!parent) return;
	long type = m_fileButton->GetStatus();
	if (!editAlias && (type != FileProperties_File) && (type != FileProperties_Directory))
		return;

	// Get the string we are going to place in the text editor
	char TempBuf [MAX_PATH];
	if (editAlias) {
		const char *source=m_fileButton->FileButton_GetAlias(false);
		if (source) {
			strcpy(TempBuf,source);
			}
		}
	else {
		DVE_GetTextString(BasicFile_DVElem_FullName,TempBuf,MAX_PATH);
		}
	
	if (!InitString) {
		InitString=TempBuf;
		}
	// Bring up my little text editor
	char* NewText = NewTek_GetTextFromPopupWindow("", const_cast<char *>(InitString), *parent, reasonForExit, NULL, &inRect, 
		TextItem_LeftAlign | TextItem_CenterVAlign | EditPopupWindow_AutoGrow_Expand, "SimpleEditPopupWindow",HighLightInitString);

	// Check for NewText from the user
	if (NewText)
	{
		// Set the text here, be sure there really is a change
		if ((reasonForExit != Popup_Escape_EscapeKey) && strcmp(const_cast<char *>(InitString), NewText))
		{
			// If we are editing the alias, and actually giving it a value, just do it to this one
			if (editAlias && NewText[0])
				m_fileButton->FileButton_SetAlias(NewText);
			else
			{
				// Here is where the job is REALLY done, Get a list of all of the files in selection order
				tList<FileButton*> selectedButtons;
				m_fileButton->GrabSelectedButtons(selectedButtons);
				unsigned numSelected = selectedButtons.NoItems();
				for (unsigned i = 0 ; i < numSelected; i++)
				{
					if (editAlias)	// The alias calls the virtual function, which is good for StoryBOard
						selectedButtons[i]->FileButton_SetAlias(NewText);
					else
					{
						// This part is file specific
						FileProperties* fp = selectedButtons[i]->GetProperties();
						fp->FileChangeFilename(NewText, true);
					}
				}
			}
		}
		NewTek_free(NewText);
	}
}


//James was here >:)

unsigned BaseFile_DetailsView_Element::DVE_GetTextString(unsigned type,char *DestBuffer,unsigned LengthOfBuffer) {
	const char *source=NULL;
	do { //This scope will determine what the source pointer is to be...
		// Pedantic
		if (!m_fileButton) break;

		// Try for alt name, look for name only if it does not work
		if (type == BasicFile_DVElem_AltName) {
			source=m_fileButton->FileButton_GetAlias(false);
			break;
			}

		// Pedantic Check
		FileProperties* fp = m_fileButton->GetProperties();
		if (!fp) break;

		// Use this static buffer to muck about with the file name
		char TempBuffer[MAX_PATH];
			
		if ((type == BasicFile_DVElem_Path) || 
			(type == BasicFile_DVElem_PathOnly)) {
			// Try to grab a relative path
			const char *FullFileName=m_fileButton->GetFileName();
			if (type==BasicFile_DVElem_Path) {
				source=FullFileName;
				break;
				}
			else {
				char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
				_splitpath(FullFileName,drive,dir,fname,ext);
				// Compose the path
				TempBuffer[0]=0;
				strcat(TempBuffer,drive);
				strcat(TempBuffer,dir);
				source=TempBuffer;
				break;
				}
			}
		else 
			if (type == BasicFile_DVElem_Type) {
				source=fp->FP_GetTypeString();
				}
		else {
			const char* filename = fp->GetFileName();
			if (!filename) {
				source="";
				break;
				}
			char* lastSlash = NewTek_GetLastSlash((char*)filename);
			if (lastSlash && lastSlash[1]) filename = lastSlash + 1;

			// What part of the file name do we want?
			//Assert filename=true (checked above)
			strcpy(TempBuffer, filename);
			char* lastDot = (char*)NewTek_GetLastDot(TempBuffer);
			char* retFileName = TempBuffer;

			if (type == BasicFile_DVElem_NameOnly) {
				// Terminate the string to strip the extension
				if (lastDot) *lastDot = 0;
				source=retFileName;
				break;
				}
			if (type == BasicFile_DVElem_FullName) {
				source=retFileName;
				break;
				}
			if (type == BasicFile_DVElem_ExtOnly) {
				// Just use the extension if there is one
				if (lastDot && lastDot[1])
					retFileName = lastDot+1;
				else retFileName = NULL;	// Show that this is missing
				source=retFileName;
				break;
				}
			
			// Last of all we have the file name itself
			source=retFileName;
			}
		} while(false);

	unsigned NumBytesCopied=0;
	if (source) {
		NumBytesCopied=strlen(source)+1;

		if ((DestBuffer)&&(NumBytesCopied)) {
			NumBytesCopied=min(LengthOfBuffer,NumBytesCopied);
			memcpy(DestBuffer,source,NumBytesCopied);
			}
		}


	return NumBytesCopied;
	}