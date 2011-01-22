#include "stdafx.h"
#include "Resource.h"

//HICON UtilLib_FileButton_ViewMode::s_loadingIcon=NULL;

void UtilLib_FileButton_ViewMode::OnProcessDetach(void)
{	//if (s_loadingIcon) DeleteObject(s_loadingIcon);
	//s_loadingIcon=NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void UtilLib_FileButton_ViewMode::OnProcessAttach(void)
{	//if (s_loadingIcon) DeleteObject(s_loadingIcon);
	//s_loadingIcon = LoadIcon(g_FileManagementhInst, MAKEINTRESOURCE(IDI_LOADING));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

UtilLib_FileButton_ViewMode::UtilLib_FileButton_ViewMode()
{
	// No sort yet
	m_sortString = NULL;

	// Set some default colors for the TextItems
	for (int i = 0; i < 4; i++)
	{
		m_textItemToUse[i] = &m_textItemLocal[i];
		m_textItemToUse[i]->Set_Background(true);
		m_textItemToUse[i]->SetUnderline(i&2);

		m_textItemToUse[i]->Set_BackgroundColor(FILEBUTTONCOLORS_BACKGROUND[i][0],
												FILEBUTTONCOLORS_BACKGROUND[i][1],
												FILEBUTTONCOLORS_BACKGROUND[i][2]);

		m_textItemToUse[i]->SetColor(	FILEBUTTONCOLORS_TEXT[i][0],
										FILEBUTTONCOLORS_TEXT[i][1],
										FILEBUTTONCOLORS_TEXT[i][2]
									);		

		m_textItemToUse[i]->SetFont(TextItem_DefaultFont);
		m_textItemToUse[i]->SetBold(TextItem_DefaultBoldState);
		m_textItemToUse[i]->SetOffset(5,0);

		m_textItemToUse[i]->SetFontSize(FileButton_Size_FontSize);
		m_textItemToUse[i]->AddDependant(this);
	}
	this->m_textItemPainted.MessagesOff();
	m_textItemPainted.Set_Background(true);
	m_textItemPainted.SetFont(TextItem_DefaultFont);
	m_textItemPainted.SetBold(TextItem_DefaultBoldState);
	m_textItemPainted.SetFontSize(FileButton_Size_FontSize);
	//m_relativePath = NULL;
	m_viewMode = FileButton_ViewMode_LargeIcon;
	m_iconAspect = ICON_ASPECT_4x3;
	//m_XSize = FileButton_Size_ControlLargeX;
	//m_YSize = FileButton_Size_ControlLargeY;	// Temporary Y based on Font size
	FileButton_ViewMode_SetViewMode(FileButton_ViewMode_LargeIcon,0,0);
	m_textItemPainted.SetAlignment();
	this->m_textItemPainted.MessagesOn();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

UtilLib_FileButton_ViewMode::~UtilLib_FileButton_ViewMode()
{
	//if (m_relativePath)
	//	delete[] m_relativePath;

	for (unsigned i = 0; i < 4; i++)
		m_textItemToUse[i]->DeleteDependant(this);

	if (m_sortString)
		delete[] m_sortString;
	m_sortString = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void  UtilLib_FileButton_ViewMode::UFBVM_SendSizeChanges()
{
	//Watch out for many layout updates, using the current parent
	BaseWindowLayoutManager* bwlm = GetInterface<BaseWindowLayoutManager>(m_parent);
	if (bwlm) bwlm->MultipleLayouts_Start();

	// Tell the parent there has been a change
	if (m_parent) m_parent->ViewModeSizeChanged(this);

	// Send a Changed Message, buttons will resize themselves here
	Changed(FileButton_ViewMode_SIZE_Changed);

	// Complete the layout ONE TIME!
	if (bwlm) bwlm->MultipleLayouts_Finished(true, false);
}


long UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetPreferedXSize(FileButton* fb)
{
	if (fb) 
	{
		if ((m_viewMode == FileButton_ViewMode_NameOnly) || (m_viewMode == FileButton_ViewMode_SmallIcon) ||
			(m_viewMode == FileButton_ViewMode_SuperLargeIcon))	// mwatkins
		{
			// Find out how wide the Text Area should be
			char textToPaint[MAX_PATH]={0};
			GetTextFromFileButton(textToPaint,MAX_PATH,fb);
			unsigned ptr = 0;
			while ((ptr < 110) && (*(textToPaint+ptr) != 0))
				ptr++;
			if (ptr > 100)
				int breakHere = 1;

			m_textItemPainted.SetText(textToPaint);
			FileProperties* fp = fb->GetProperties();
			m_textItemPainted.SetBold(fp && fp->FP_IsLockedInToasterApp());
			long prefTextWidth, prefTextHeight;
			m_textItemPainted.GetSize(prefTextWidth, prefTextHeight);
			if (prefTextWidth > 0)
			{
				prefTextWidth += 20;
				
				// Add some for the ICON if we are a small ICON
				if (m_viewMode == FileButton_ViewMode_SmallIcon ||
					m_viewMode == FileButton_ViewMode_SuperLargeIcon)	// mwatkins
				{
					long height = FileButton_ViewMode_GetPreferedYSize(fb);
					prefTextWidth += (height * m_iconAspect);
				}
				return prefTextWidth;
			}
		}
	}
	return this->m_XSize;
}
long UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetPreferedYSize(FileButton* fb){return m_YSize;}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Get the RECTS for where the ICON and text are, based on the entire controlRect
void UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetRects(RECT & iconRect, RECT & nameRect, RECT controlRect)
{
	if (	(m_viewMode == FileButton_ViewMode_NameOnly) ||
			(m_viewMode == FileButton_ViewMode_Details))
	{
		nameRect.left = controlRect.left;
		nameRect.right = controlRect.right;
		nameRect.top = controlRect.top;
		nameRect.bottom = controlRect.bottom;
		iconRect.left = iconRect.top = 0;
		iconRect.right = iconRect.bottom = -1;
	}
	else if (m_viewMode == FileButton_ViewMode_IconOnly)
	{
		iconRect.left = controlRect.left;
		iconRect.right = controlRect.right;
		iconRect.top = controlRect.top;
		iconRect.bottom = controlRect.bottom;
		nameRect.left = nameRect.top = 0;
		nameRect.right = nameRect.bottom = -1;
	}
	else
	{
		iconRect.left = controlRect.left;
		iconRect.top = controlRect.top;
		if ((m_viewMode == FileButton_ViewMode_SmallIcon) ||
			(m_viewMode == FileButton_ViewMode_DetailsIcon) ||
			 (m_viewMode == FileButton_ViewMode_SuperLargeIcon))	// mwatkins
		{
			iconRect.bottom = controlRect.bottom;
			long width = (iconRect.bottom - iconRect.top + 1) * m_iconAspect;

			// Special case
			if (iconRect.bottom-iconRect.top == FileButton_Size_IconLargeY)
					width = FileButton_Size_IconLargeX+1;
			else if (iconRect.bottom-iconRect.top == FileButton_Size_IconSmallY)
					width = FileButton_Size_IconSmallX+1;

			iconRect.right = iconRect.left + width - 1;
			if (iconRect.right > controlRect.right)
				iconRect.right = controlRect.right;
			nameRect.top = controlRect.top;
			nameRect.left = iconRect.right + 1;
		}
		else // Large ICON
		{
			iconRect.right = controlRect.right;
			//long height = (iconRect.right - iconRect.left + 1) / m_iconAspect;
			long height = controlRect.bottom-controlRect.top-16;
			iconRect.bottom = iconRect.top + height - 1;
			if (iconRect.bottom > controlRect.bottom)
				iconRect.bottom = controlRect.bottom;
			nameRect.left = controlRect.left;
			nameRect.top = iconRect.bottom + 1;
		}
		nameRect.bottom = controlRect.bottom;
		nameRect.right = controlRect.right;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void UtilLib_FileButton_ViewMode::CopyTextItemAttributes(TextItem* textItem, int buttonState, unsigned ColorTint)
{
	// Turn Message off for the Text Item
	textItem->MessagesOff();

	// Be sure we are looking at button states 0 - 3
	buttonState = buttonState % 4;

	// Make a copy of the correct TextItem
	TextItem* ti = this->m_textItemToUse[buttonState];

	textItem->SetFont(ti->GetFont());
	textItem->SetFontSize(ti->GetFontSize());
	textItem->SetBold(ti->GetBold());
	textItem->SetItalic(ti->GetItalic());
	textItem->SetUnderline(ti->GetUnderline());
	textItem->SetStrikeout(ti->GetStrikeout());
	long r,g,b;
	
	ti->GetOffset(r,g);
	textItem->SetOffset(r,g);
	ti->GetBorderColor(r,g,b);
	textItem->SetBorderColor(r,g,b);
	textItem->SetBorder(ti->GetBorder());
	
	// Check out the color tint relative to the assigned text item
	WindowPixel wp; 
	wp.bgra = ColorTint;

	ti->Get_BackgroundColor(r, g, b);			
	wp.a = (wp.a * 4) / 5;
	r = ((r * (255 - wp.a)) + (wp.a * wp.r)) / 255;
	g = ((g * (255 - wp.a)) + (wp.a * wp.g)) / 255;
	b = ((b * (255 - wp.a)) + (wp.a * wp.b)) / 255;
	textItem->Set_BackgroundColor(r, g, b);

	if (ColorTint!=0xffffff)  //only apply b/w logic if we use color tint
	{
		// Make the text black or white based on the value of the color
		long value = 2*r/10+6*g/10+2*b/10;
		if (value > 130)
			textItem->SetColor(	0, 0, 0 );
		else	
			textItem->SetColor(	220, 220, 220 );
	}
	else
		textItem->SetColor( FILEBUTTONCOLORS_TEXT[buttonState][0], FILEBUTTONCOLORS_TEXT[buttonState][1],
			FILEBUTTONCOLORS_TEXT[buttonState][2] );

	// Turn Messages Back on, no messages sent
	textItem->MessagesOn();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Drawing the FileButton
void ExcludeEdge(HDC hdc,int x1,int y1,int x2,int y2)
{	ExcludeClipRect(hdc,x1,y1,x2,y1+1); 
	ExcludeClipRect(hdc,x2,y1,x2+1,y2); 
	ExcludeClipRect(hdc,x1,y1,x1+1,y2); 
	ExcludeClipRect(hdc,x1,y2,x2+1,y2+1); 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void UtilLib_FileButton_ViewMode::UFBVM_AlterTextItemInProject(bool inProject, TextItem* ti)
{
	// Sets the Text Item bold if we are in a project
	if (ti)
		ti->SetBold(inProject);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void UtilLib_FileButton_ViewMode::UFBVM_DrawBorder_Default
	(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect)
{
	// Draw a border around everything
	// Draw a border around an icon that is not animating
	{	
		switch(buttonState)
		{	case Controls_Button_MouseOver:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONSHADOW_RO,false);
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONHILITE,false);
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					break;

			case Controls_Button_UnSelected:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONSHADOW,false);
					//ExcludeClipRect(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1); 
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONSHADOW,false); 
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					break;

			case Controls_Button_MouseOverDn:					
					if ((controlRect.right-controlRect.left>FBVM_SizeBeforeWiderEdge)&&
						(controlRect.bottom-controlRect.top>FBVM_SizeBeforeWiderEdge))
					{	NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE_RO,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
						//NewTek_DrawRectangleEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3);
					}
					else
					{	NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE_RO,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					}

					break;

			case Controls_Button_Selected:
					if ((controlRect.right-controlRect.left>FBVM_SizeBeforeWiderEdge)&&
						(controlRect.bottom-controlRect.top>FBVM_SizeBeforeWiderEdge))
					{	NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
						//NewTek_DrawRectangleEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3);
					}
					else
					{	NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					}
					break;
		}
	}
}

void RoundEdges(HDC hdc, RECT Rect,unsigned NoPixels,unsigned Col)
{	// Now we are going to display our very own lines
	HPEN hashPen=CreatePen(PS_SOLID,1,Col);
	HPEN OldPen	=(HPEN)SelectObject(hdc,hashPen);

	for(unsigned i=0;i<NoPixels;i++)
	{	MoveToEx(hdc,Rect.left,Rect.top+NoPixels-i-1,NULL);
		LineTo(hdc,Rect.left+i+1,Rect.top+NoPixels-i-1);
		//ExcludeClipRect(hdc,	Rect.left,
		//						Rect.top+NoPixels-i-1,
		//						Rect.left+i+1,
		//						Rect.top+NoPixels-i);

		MoveToEx(hdc,Rect.right-i-1,Rect.top+NoPixels-i-1,NULL);
		LineTo(hdc,Rect.right,Rect.top+NoPixels-i);
		//ExcludeClipRect(hdc,	Rect.right-i-1,
		//						Rect.top+NoPixels-i-1,
		//						Rect.right,
		//						Rect.top+NoPixels-i);

		MoveToEx(hdc,Rect.left,Rect.bottom-NoPixels+i,NULL);
		LineTo(hdc,Rect.left+i+1,Rect.bottom-NoPixels+i);
		//ExcludeClipRect(hdc,	Rect.left,
		//						Rect.bottom-NoPixels+i,
		//						Rect.left+i+1,
		//						Rect.bottom-NoPixels+i+1);

		MoveToEx(hdc,Rect.right-i-1,Rect.bottom-NoPixels+i,NULL);
		LineTo(hdc,Rect.right,Rect.bottom-NoPixels+i);
		//ExcludeClipRect(hdc,	Rect.right-i-1,
		//						Rect.bottom-NoPixels+i,
		//						Rect.right,
		//						Rect.bottom-NoPixels+i+1);

		//MoveToEx(hdc,Rect.right-i-1,Rect.top+NoPixels-i,NULL);
		//LineTo(hdc,Rect.right,Rect.top+NoPixels-i);
		//ExcludeClipRect(hdc,Rect.right-i-1,Rect.top+NoPixels-i,Rect.right,Rect.top+NoPixels-i+1);

		//MoveToEx(hdc,Rect.left,(Rect.bottom-i-1),NULL);
		//LineTo(hdc,Rect.left+i,(Rect.bottom-i-1));
		//ExcludeClipRect(hdc,Rect.left,(Rect.bottom-i-1),Rect.left+i+1,(Rect.bottom-i-1)+1);
		//MoveToEx(hdc,Rect.right-i-1,(Rect.bottom-i-1),NULL);
		//LineTo(hdc,Rect.right,(Rect.bottom-i-1));
		//ExcludeClipRect(hdc,Rect.right-i-1,(Rect.bottom-i-1),Rect.right,(Rect.bottom-i-1)+1);
	}

	// Delete the stuff
	SelectObject(hdc,OldPen);
	DeleteObject(hashPen);
}

void UtilLib_FileButton_ViewMode::UFBVM_DrawBorder_Rounded
		(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect)
{	// Draw a border around everything
	{	
		switch(buttonState)
		{	case Controls_Button_MouseOver:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONSHADOW_RO,false);										
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONHILITE,false);
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);					
					//RoundEdges(hdc,controlRect,5,FILEBUTTONSHADOW_RO);
					break;

			case Controls_Button_UnSelected:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONSHADOW,false);
					//ExcludeClipRect(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1); 
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONSHADOW,false); 
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					//RoundEdges(hdc,controlRect,5,FILEBUTTONSHADOW);
					break;

			case Controls_Button_MouseOverDn:					
					if ((controlRect.right-controlRect.left>FBVM_SizeBeforeWiderEdge)&&
						(controlRect.bottom-controlRect.top>FBVM_SizeBeforeWiderEdge))
					{	//RoundEdges(hdc,controlRect,1,FILEBUTTONSHADOW);
						NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE_RO,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
						//RoundEdges(hdc,controlRect,6,FILEBUTTONHILITE_RO);
						//NewTek_DrawRectangleEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3);						
						//RoundEdges(hdc,controlRect,7,FILEBUTTONSHADOW);
					}
					else
					{	//RoundEdges(hdc,controlRect,1,FILEBUTTONSHADOW);
						NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						//RoundEdges(hdc,controlRect,4,FILEBUTTONHILITE);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE_RO,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
						//RoundEdges(hdc,controlRect,5,FILEBUTTONSHADOW);
					}

					break;

			case Controls_Button_Selected:
					if ((controlRect.right-controlRect.left>FBVM_SizeBeforeWiderEdge)&&
						(controlRect.bottom-controlRect.top>FBVM_SizeBeforeWiderEdge))
					{	//RoundEdges(hdc,controlRect,1,FILEBUTTONSHADOW);
						NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
						//RoundEdges(hdc,controlRect,6,FILEBUTTONHILITE);
						//NewTek_DrawRectangleEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+2,controlRect.top+2,controlRect.right-3,controlRect.bottom-3);
						//RoundEdges(hdc,controlRect,7,FILEBUTTONSHADOW);
					}
					else
					{	//RoundEdges(hdc,controlRect,1,FILEBUTTONSHADOW);
						NewTek_DrawRectangleEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2,0,FILEBUTTONSHADOW,false);
						//ExcludeEdge(hdc,controlRect.left+1,controlRect.top+1,controlRect.right-2,controlRect.bottom-2);
						//RoundEdges(hdc,controlRect,4,FILEBUTTONHILITE);
						NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE,false);
						//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
						//RoundEdges(hdc,controlRect,5,FILEBUTTONSHADOW);
					}
					break;
		}
	}
}

void UtilLib_FileButton_ViewMode::UFBVM_DrawWarning
	(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect)
{
	// Display a red cross on the corner if this item has been cut.
	// If I have been cut, or my file does not exist, draw an X over me to show
	if (fileButton->FileButton_ShouldDrawWarningBorder())
	{	
		unsigned Dst=2;
		controlRect.left	+=Dst;
		controlRect.top	+=Dst;
		controlRect.right	-=Dst;
		controlRect.bottom	-=Dst;

		switch(buttonState)
		{	case Controls_Button_MouseOver:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONSHADOW_RO_CUT,false);					
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONHILITE,false); 
					break;

			case Controls_Button_UnSelected:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONSHADOW_CUT,false);
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONSHADOW,false); 
					break;

			case Controls_Button_MouseOverDn:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE_RO_CUT,false);
					//ExcludeClipRect(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1); 
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONHILITE,false); 
					break;

			case Controls_Button_Selected:
					NewTek_DrawRectangleEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1,0,FILEBUTTONHILITE_CUT,false);
					//ExcludeClipRect(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1); 
					//ExcludeEdge(hdc,controlRect.left,controlRect.top,controlRect.right-1,controlRect.bottom-1);
					//NewTek_DrawBorder(hdc,controlRect,FILEBUTTONSHADOW,false); 
					break;
		}
	}	
}

void UtilLib_FileButton_ViewMode::UFBVM_DrawName
	(HWND hWnd, HDC hdc, FileButton* fileButton, char* text, bool missingRef,
	int buttonState, unsigned ColorTint, RECT nameRect)
{
	// Be sure the Text Item isn't trying to send changes
	m_textItemPainted.MessagesOff();

	// Copy the text item attributes
	CopyTextItemAttributes(&m_textItemPainted, buttonState, ColorTint);
	
	// Get the correct Text for the filename
	m_textItemPainted.SetText(text);
	FileProperties* fp = fileButton->GetProperties();
	UFBVM_AlterTextItemInProject(fp && fp->FP_IsLockedInToasterApp(), &m_textItemPainted);

	// If the name is too long, make us left aligned, center V aligned
	if (!(m_viewMode & FileButton_ViewMode_Details))
	{
		long nameWidth = nameRect.right - nameRect.left;
		if (	(nameWidth > m_textItemPainted.ScreenObject_GetPreferedXSize()) &&
				(m_viewMode == FileButton_ViewMode_LargeIcon))
			m_textItemPainted.SetAlignment(TextItem_CenterHAlign | TextItem_CenterVAlign);
		else m_textItemPainted.SetAlignment(TextItem_LeftAlign | TextItem_CenterVAlign);
	}

	// I want to be sure I can see the Background
	m_textItemPainted.Set_Background(true);
	
	// Watch for right alignment offsets
	long currentAlignment = m_textItemPainted.GetAlignement();
	long currentOffset, coy;m_textItemPainted.GetOffset(currentOffset, coy);
	if (currentAlignment & TextItem_RightAlign)
	{
		if (currentOffset > 0)
			m_textItemPainted.SetOffset(currentOffset*-1, coy);
	}
	else if (currentAlignment & TextItem_LeftAlign)
	{
		if (currentOffset < 0)
			m_textItemPainted.SetOffset(currentOffset*-1, coy);
	}

	// Any final tweaking of the TextItem?
	TweakTextItem(fileButton, &m_textItemPainted, missingRef);

	// Finally draw the textItem
	m_textItemPainted.ScreenObject_DrawItem(&nameRect, hdc);

	// Draw a rectangle around the text
	NewTek_DrawRectangleEdge(hdc,nameRect.left,nameRect.top,nameRect.right-1,nameRect.bottom-1,0,FILEBUTTONSHADOW,false);

	// Set the offset back
	m_textItemPainted.SetOffset(currentOffset, coy);

	// Be sure the Text Item isn't trying to send changes
	m_textItemPainted.MessagesOn();
}

void UtilLib_FileButton_ViewMode::UFBVM_DrawIcon
	(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT iconRect)
{
	// remember the last drawn rect
	fileButton->m_lastIconRect = iconRect;

	ScreenObject* icon = fileButton->GetFileIcon();
	if (icon)
		icon->ScreenObject_DrawItem(&iconRect, hdc);

	//if (fileButton->FB_GetThreadReadStatus() == FileButton_ThreadRead_Reading)
	//	::DrawIcon(hdc, iconRect.right - ICON_RES_WIDTH - 5, iconRect.top + 5, s_loadingIcon);
}

void UtilLib_FileButton_ViewMode::FileButton_ViewMode_Draw
	(HWND hWnd, HDC hdc, FileButton* fileButton, int buttonState, unsigned ColorTint, RECT controlRect)
{
	// If this thing has no size, don't even draw it
	if ((controlRect.bottom < 1) || (controlRect.right < 1))
		return;
	if (!fileButton) return;

	// Be sure we are looking at button states 0 - 3
	buttonState = buttonState % 4;

	// Find out what we are drawing
	RECT iconRect, nameRect;
	FileButton_ViewMode_GetRects(iconRect, nameRect, controlRect);
	bool drawingName = (nameRect.right > nameRect.left) && (nameRect.bottom > nameRect.top);
	bool drawingIcon = (iconRect.right > iconRect.left) && (iconRect.bottom > iconRect.top);

	if (drawingName)
	{
		bool missingRef = false; double d;
		char textFromFileName[MAX_PATH]={0};
		GetSortTextFromFilename(textFromFileName,MAX_PATH,fileButton, missingRef, d);
		UFBVM_DrawName(hWnd, hdc, fileButton, textFromFileName, missingRef, buttonState, ColorTint, nameRect);
	}

	if (drawingIcon)
		UFBVM_DrawIcon(hWnd, hdc, fileButton, buttonState, ColorTint, iconRect);
	else 
		fileButton->m_lastIconRect.left = fileButton->m_lastIconRect.right = fileButton->m_lastIconRect.top = fileButton->m_lastIconRect.bottom = 0;

	// Draw each part
	UFBVM_DrawBorder(hWnd, hdc, fileButton, buttonState, ColorTint, controlRect);
	UFBVM_DrawWarning(hWnd, hdc, fileButton, buttonState, ColorTint, controlRect);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void UtilLib_FileButton_ViewMode::FileButton_ViewMode_SetTextItem(TextItem* textItem, unsigned buttonState)
{
	buttonState %= 4;
	if (!textItem) textItem = &m_textItemLocal[buttonState];
	if (textItem == m_textItemToUse[buttonState]) return;
	m_textItemToUse[buttonState]->DeleteDependant(this);
	m_textItemToUse[buttonState] = textItem;
	m_textItemToUse[buttonState]->AddDependant(this);
	DynamicCallback(-1,"",NULL,m_textItemToUse[buttonState]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Setting up the naming modes
/*void UtilLib_FileButton_ViewMode::FileButton_ViewMode_SetRelativePath(char* relativePath)
{
	bool changed = false;
	if (relativePath)
	{
		int relPathLen = strlen(relativePath);
		if (m_relativePath && strcmp(relativePath, m_relativePath))
		{
			changed = true;
			if (strlen(m_relativePath) != strlen(relativePath))
			{
				delete[] m_relativePath;
				m_relativePath = new char[relPathLen+1];
			}
			strcpy(m_relativePath, relativePath);
		}
		if (!m_relativePath)
		{
			changed = true;
			m_relativePath = new char[relPathLen+1];
			strcpy(m_relativePath, relativePath);
		}
	}
	else if (m_relativePath)
	{
		changed = true;
		delete[] m_relativePath;
		m_relativePath = NULL;
	}
	
	// ** Set the Search String based on this name
}*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void	UtilLib_FileButton_ViewMode::FileButton_ViewMode_SetSortString(char* sortString)
{
	if (sortString == m_sortString)
		return;
	if (m_sortString && sortString && (!strcmp(m_sortString, sortString)))
		return;

	if (m_sortString)
		delete[] m_sortString;
	m_sortString = NULL;
	if (sortString)
	{
		m_sortString = new char[strlen(sortString)+1];
		strcpy(m_sortString, sortString);
	}
	// Send a change like the name mode
	Changed(FileButton_ViewMode_NameMode_Changed);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

long UtilLib_FileButton_ViewMode::GetTextAreaHeight()
{
	// Create a Dummy Text Item with my current Settings
	TextItem tempTI;
	CopyTextItemAttributes(&tempTI, 3, 0);	// Look at the OnRO state
	tempTI.SetText("Ty");
	long ht = tempTI.ScreenObject_GetPreferedYSize();
	return ht + 4;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Use -1 for XSize and YSize to use the default sizes listed above
void UtilLib_FileButton_ViewMode::FileButton_ViewMode_SetViewMode(int viewMode, long XSize, long YSize)
{
	if (XSize < 0) XSize = 0;
	if (YSize < 0) YSize = 0;

	float iconDiff = (m_iconAspect / ICON_ASPECT_4x3);
	switch(viewMode)
	{
	case FileButton_ViewMode_LargeIcon:
		if (!XSize) XSize = FileButton_Size_ControlLargeX * iconDiff + 0.5f;
		if (!YSize) YSize = GetTextAreaHeight() + FileButton_Size_IconLargeY;
		break;
	case FileButton_ViewMode_SuperLargeIcon:	// mwatkins - Proper sizing for super large icon view mode
		if (!XSize) XSize = FileButton_Size_ControlSuperLargeX;//FileButton_Size_IconLargeX * iconDiff + 0.5f + FileButton_Size_ControlListX;
		if (!YSize) YSize = FileButton_Size_ControlSuperLargeY;
		break;
	case FileButton_ViewMode_DetailsIcon:
	case FileButton_ViewMode_SmallIcon:
		if (!XSize) XSize = FileButton_Size_ControlSmallX;
		if (!YSize) YSize = FileButton_Size_ControlSmallY;
		break;
	case FileButton_ViewMode_NameOnly:
	case FileButton_ViewMode_Details:
		if (!XSize) XSize = FileButton_Size_ControlListX;
		if (!YSize) YSize = GetTextAreaHeight();
		break;
	default:
		if (!XSize) XSize = FileButton_Size_IconLargeX * iconDiff + 0.5f;
		if (!YSize) YSize = FileButton_Size_IconLargeY;
	}

	if ((m_XSize != XSize) || (m_YSize != YSize))
	{
		m_XSize = XSize;
		m_YSize = YSize;
		m_viewMode = viewMode;
		UFBVM_SendSizeChanges();
	}
	else if (viewMode != m_viewMode)
	{
		m_viewMode = viewMode;
		Changed(FileButton_ViewMode_ViewMode_Changed);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// We can actually change the aspect ratio of the icon! (this might change the size based on what mode we are in!)
void UtilLib_FileButton_ViewMode::FileButton_ViewMode_SetIconAspect(float iconAspect)
{
	if (iconAspect == m_iconAspect) return;	// Same value
	if (iconAspect <= 0.2) return;			// Bad Value
	float aspectChange = iconAspect / m_iconAspect;
	m_iconAspect = iconAspect;

	switch (m_viewMode)
	{
	case FileButton_ViewMode_SmallIcon:
	case FileButton_ViewMode_SuperLargeIcon:	// mwatkins
		Changed(FileButton_ViewMode_IconAspect_Changed);	// We will not change the size
		break;
	case FileButton_ViewMode_NameOnly:
		return;	// No change is necessary, FileButtons do not need a re-paint
		break;
	default:
		m_XSize *= aspectChange;
		UFBVM_SendSizeChanges();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// We listen for changes in our text items and pass those changes on to you!
void UtilLib_FileButton_ViewMode::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	if (IsDeletion(String))
	{
		for (unsigned i = 0; i < 4; i++)
			if (ItemChanging == m_textItemToUse[i])
				FileButton_ViewMode_SetTextItem(NULL, i);
		return;
	}
	if (ItemChanging == m_textItemToUse[0])
		Changed(FileButton_ViewMode_State0_Changed);
	if (ItemChanging == m_textItemToUse[1])
		Changed(FileButton_ViewMode_State1_Changed);
	if (ItemChanging == m_textItemToUse[2])
		Changed(FileButton_ViewMode_State2_Changed);
	if (ItemChanging == m_textItemToUse[3])
		Changed(FileButton_ViewMode_State3_Changed);
}


//James was here >:)
unsigned UtilLib_FileButton_ViewMode::GetTextFromFileButton(char *DestBuffer,unsigned LengthOfBuffer,FileButton* fb, char* sortString, bool allowBlankIfNoAsset) {
	char SourceBuffer[MAX_PATH]={0};
	if (fb) {
		bool b; double sv;
		GetSortTextFromFilename(SourceBuffer,MAX_PATH,fb, b, sv, sortString, allowBlankIfNoAsset);

		if (SourceBuffer[0]==0) {
			const char *source=fb->FileButton_GetAlias();
			if (source) {
				strcpy(SourceBuffer,source);
				}
			}
		}

	unsigned NumBytesCopied=strlen(SourceBuffer)+1;

	if ((DestBuffer)&&(NumBytesCopied)) {
		NumBytesCopied=min(LengthOfBuffer,NumBytesCopied);
		memcpy(DestBuffer,SourceBuffer,NumBytesCopied);
		}

	return NumBytesCopied;
	}


//James was here >:)
unsigned UtilLib_FileButton_ViewMode::GetSortTextFromFilename (char *DestBuffer,unsigned LengthOfBuffer,
	FileButton* fb, bool & missingAsset, double & sortValue, char* sortString, bool allowBlankIfNoAsset) {

	char SourceBuffer[MAX_PATH]={0};
	do {
		// Initialize reference params
		missingAsset = false;
		sortValue = 0.0;
		
		//Assert fb is true
		if (!fb) {
			_dthrow("UtilLib_FileButton_ViewMode::GetSortTextFromFilename fb==NULL\n");
			break;
			}

		// Get a good sort string
		if (!sortString)
			sortString = m_sortString;
		if (!sortString)
			sortString = FileButton_Naming_AltName;

		// Be sure all is read in ok
		fb->DVR_Initialize();

		// Find the element to use for a string
		missingAsset = true;
		DetailsView_Element* dvElement = fb->DVR_GetDetailsViewElement(sortString);
		if (dvElement) {
			dvElement->DVE_Initialize();

			// Grab the string
			dvElement->DVE_GetTextString(SourceBuffer,LengthOfBuffer);
			if (!dvElement->DVE_DoesSortNumeric(sortValue))
				sortValue = 0.0;
			missingAsset = (SourceBuffer[0] == 0);

			dvElement->DVE_Cleanup();
			}

		// If we are missing an asset
		if (missingAsset && !allowBlankIfNoAsset) {
			const char *source=fb->FileButton_GetAlias();
			if (source)
				strcpy(SourceBuffer,source);
			}

		// All read in ok
		fb->DVR_Cleanup();

		} while(false);

	unsigned NumBytesCopied=strlen(SourceBuffer)+1;

	if ((DestBuffer)&&(NumBytesCopied)) {
		NumBytesCopied=min(LengthOfBuffer,NumBytesCopied);
		memcpy(DestBuffer,SourceBuffer,NumBytesCopied);
		}

	return NumBytesCopied;
	}


void UtilLib_FileButton_ViewMode::BaseLoadSave_GetVersionNumber(unsigned &MajorVersion,unsigned &MinorVersion)
{
	MajorVersion = 1;
	MinorVersion = 2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UtilLib_FileButton_ViewMode::BaseLoadSave_SaveData(SaveData *SaveContext)
{
	if (!SaveContext) return false;
	if (!SaveContext->PutString(m_sortString ? m_sortString : FileButton_Naming_NameOnly)) return false;
	if (!SaveContext->Put(m_viewMode)) return false;
	if (!SaveContext->Put(m_XSize)) return false;
	if (!SaveContext->Put(m_YSize)) return false;
	if (!SaveContext->Put(m_iconAspect)) return false;
	//if (m_relativePath)
	//{
	//	if (!SaveContext->Put(strlen(m_relativePath))) return false;
	//	if (!SaveContext->PutString(m_relativePath)) return false;
	//}
	//else if (!SaveContext->Put(0)) return false;

	for (unsigned i = 0; i < 4; i++)
	{
		if (!SaveContext->PutString(m_textItemLocal[i].GetFont())) return false;
		if (!SaveContext->Put(m_textItemLocal[i].GetFontSize())) return false;
		if (!SaveContext->Put(m_textItemLocal[i].GetBold())) return false;
		if (!SaveContext->Put(m_textItemLocal[i].GetItalic())) return false;
		if (!SaveContext->Put(m_textItemLocal[i].GetUnderline())) return false;
		if (!SaveContext->Put(m_textItemLocal[i].GetStrikeout())) return false;
		if (!SaveContext->Put(m_textItemLocal[i].GetBorder())) return false;
		long r,g,b;
		m_textItemLocal[i].GetColor(r,g,b);
		if (!SaveContext->Put(r)) return false;
		if (!SaveContext->Put(g)) return false;
		if (!SaveContext->Put(b)) return false;
		m_textItemLocal[i].GetBorderColor(r,g,b);
		if (!SaveContext->Put(r)) return false;
		if (!SaveContext->Put(g)) return false;
		if (!SaveContext->Put(b)) return false;
		m_textItemLocal[i].Get_BackgroundColor(r,g,b);
		if (!SaveContext->Put(r)) return false;
		if (!SaveContext->Put(g)) return false;
		if (!SaveContext->Put(b)) return false;
		m_textItemLocal[i].GetOffset(r,g);
		if (!SaveContext->Put(r)) return false;
		if (!SaveContext->Put(g)) return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UtilLib_FileButton_ViewMode::BaseLoadSave_LoadData(LoadData *LoadContext,unsigned MajorVersion,unsigned MinorVersion)
{
	// Pedantic Checks
	if (!LoadContext) return false;
	if (MajorVersion != 1) return false;
	if (MinorVersion > 2) return false;

	if (MinorVersion < 1)
	{
		// Old version had an int for the naming style
		int nameMode;
		if (!LoadContext->Get(nameMode)) return false;
	}
	else
	{
		// New Version always sets the string
		char nameString[128];
		if (!LoadContext->GetString(nameString, 128)) return false;
		FileButton_ViewMode_SetSortString(nameString);
	}

	if (!LoadContext->Get(m_viewMode)) return false;
	long oldx = m_XSize;
	long oldy = m_YSize;
	if (!LoadContext->Get(m_XSize)) return false;
	if (!LoadContext->Get(m_YSize)) return false;
	if (!LoadContext->Get(m_iconAspect)) return false;

	if (MinorVersion<2)
	{	char RelativePath[MAX_PATH];
		int pathLen;
		if (!LoadContext->Get(pathLen)) return false;		
		if (pathLen)
		{	if (!LoadContext->GetString(RelativePath,pathLen+1)) return false;
		}
	}

	for (unsigned i = 0; i < 4; i++)
	{
		DynamicTalkerMessageDefer messageDefer(&m_textItemLocal[i]);
		char str[64];
		bool boolVal;
		float floatVal;
		if (!LoadContext->GetString(str, 64)) return false;
		m_textItemLocal[i].SetFont(str);
		if (!LoadContext->Get(floatVal)) return false;m_textItemLocal[i].SetFontSize(floatVal);
		if (!LoadContext->Get(boolVal)) return false;m_textItemLocal[i].SetBold(boolVal);
		if (!LoadContext->Get(boolVal)) return false;m_textItemLocal[i].SetItalic(boolVal);
		if (!LoadContext->Get(boolVal)) return false;m_textItemLocal[i].SetUnderline(boolVal);
		if (!LoadContext->Get(boolVal)) return false;m_textItemLocal[i].SetStrikeout(boolVal);
		if (!LoadContext->Get(boolVal)) return false;m_textItemLocal[i].SetBorder(boolVal);
		
		long r,g,b;
		if (!LoadContext->Get(r)) return false;
		if (!LoadContext->Get(g)) return false;
		if (!LoadContext->Get(b)) return false;
		m_textItemLocal[i].SetColor(r,g,b);
		
		if (!LoadContext->Get(r)) return false;
		if (!LoadContext->Get(g)) return false;
		if (!LoadContext->Get(b)) return false;
		m_textItemLocal[i].SetBorderColor(r,g,b);
		
		if (!LoadContext->Get(r)) return false;
		if (!LoadContext->Get(g)) return false;
		if (!LoadContext->Get(b)) return false;
		m_textItemLocal[i].Set_BackgroundColor(r,g,b);
		
		if (!LoadContext->Get(r)) return false;
		if (!LoadContext->Get(g)) return false;
		m_textItemLocal[i].SetOffset(r,g);
	}

	if ((oldx != m_XSize) || (oldy != m_YSize))
	{
		UFBVM_SendSizeChanges();
	}
	else Changed(FileButton_ViewMode_Loaded);
	return !LoadContext->Error();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

char*	UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetColumnAtPoint(POINT point, FileButton* fb)
{
	// PEDANTIUC
	if (!fb) return NULL;

	// Find the Current Rect for the File Button
	RECT controlRect;
	controlRect.left = controlRect.top = 0;
	controlRect.right = fb->GetWindowWidth();
	controlRect.bottom = fb->GetWindowHeight();
	if ((controlRect.bottom < 1) || (controlRect.right < 1))
		return NULL;

	RECT nameRect, iconRect;
	this->FileButton_ViewMode_GetRects(iconRect, nameRect, controlRect);

	// If we are in the name rect, return whatever we are sorting by
	// ADJC : or the icon rect - otherwuise you cannot rename in large icon view !!!
	if ( ( (point.x >= nameRect.left) && (point.x <= nameRect.right) && (point.y >= nameRect.top) && (point.y <= nameRect.bottom) ) ||
		 ( (point.x >= iconRect.left) && (point.x <= iconRect.right) && (point.y >= iconRect.top) && (point.y <= iconRect.bottom) ) )
			return m_sortString ? m_sortString : FileButton_Naming_AltName;
	else return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetEditingRect
(RECT& rect,const char *columnName, FileButton* fb)
{
	// This view mode just gets the name rect if it exists
	// PEDANTIC
	if (!fb) return false;

	// Find the Current Rect for the File Button
	RECT controlRect;
	controlRect.left = controlRect.top = 0;
	controlRect.right = fb->GetWindowWidth();
	controlRect.bottom = fb->GetWindowHeight();
	if ((controlRect.bottom < 1) || (controlRect.right < 1))
		return false;

	RECT iconRect;
	this->FileButton_ViewMode_GetRects(iconRect, rect, controlRect);

	// Watch for the border top and bottom, left and right
	rect.left += 2;
	rect.right -= 2;
	rect.top += 2;
	rect.bottom -= 2;
	
	// Only return true if the rect is large enough to do something with
	return ((rect.right > (rect.left+2)) && (rect.bottom > (rect.top+5)));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
char*		UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetColumnAtIndex(unsigned index, FileButton* fb)
{
	if ((m_viewMode == FileButton_ViewMode_IconOnly) || (index != 0))
		return NULL;
	else return m_sortString ? m_sortString : FileButton_Naming_AltName;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned	UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetColumnIndex(const char* columnName, FileButton* fb)
{
	char* onlyColumn = FileButton_ViewMode_GetColumnAtIndex(0, fb);
	if (!onlyColumn || !columnName || (stricmp(columnName, onlyColumn)))
		return (unsigned)-1;
	else return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned	UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetNumColumns(FileButton* fb)
{
	char* onlyColumn = FileButton_ViewMode_GetColumnAtIndex(0, fb);
	return onlyColumn ? 1 : (unsigned)-1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

void FileButton_ViewMode_Parent::FileButton_DeletedProperties(FileButton* fb)
{
	fb->DeferredDelete();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

HeightWidth_ViewMode::~HeightWidth_ViewMode()
{
	if (m_parentWindow) m_parentWindow->DeleteDependant(this);
}
HeightWidth_ViewMode::HeightWidth_ViewMode()
{
	m_parentWindow = NULL;
	m_matchHeight = false;
	m_matchWidth = false;
	m_oldSpacerX = -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void HeightWidth_ViewMode::SetMatchingParentWindow(BaseWindowClass* parentWindow)
{
	if (m_parentWindow) m_parentWindow->DeleteDependant(this);
	m_parentWindow = parentWindow;
	if (m_parentWindow) m_parentWindow->AddDependant(this);
}

bool HeightWidth_ViewMode::ShouldIMatchHeight(FileButton* fb){return m_matchHeight;}
bool HeightWidth_ViewMode::ShouldIMatchWidth(FileButton* fb){return m_matchWidth;}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

long HeightWidth_ViewMode::FileButton_ViewMode_GetPreferedXSize(FileButton* fb)
{
	if ((m_viewMode == FileButton_ViewMode_NameOnly) || 
		(m_viewMode == FileButton_ViewMode_SmallIcon) ||
		(m_viewMode & FileButton_ViewMode_Details) ||
		(m_viewMode == FileButton_ViewMode_SuperLargeIcon))	// mwatkins
	{
		if (m_parentWindow && ShouldIMatchWidth(fb))
		{
			// We should also adjust for spacing in a simple grid
			UtilLib_SimpleGrid* grid = GetInterface<UtilLib_SimpleGrid>(m_parentWindow);
			long spacing = grid ? grid->UtilLib_SimpleGrid_GetSpacingX() : 0;
			return m_parentWindow->GetWindowWidth() - 2*spacing;
		}
	}
	return UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetPreferedXSize(fb);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

long HeightWidth_ViewMode::FileButton_ViewMode_GetPreferedYSize(FileButton* fb)
{
	if (m_parentWindow && ShouldIMatchHeight(fb))
	{
		// We should also adjust for spacing in a simple grid
		UtilLib_SimpleGrid* grid = GetInterface<UtilLib_SimpleGrid>(m_parentWindow);
		long spacing = grid ? grid->UtilLib_SimpleGrid_GetSpacingY() : 0;
		return m_parentWindow->GetWindowHeight() - 2*spacing;
	}
	return UtilLib_FileButton_ViewMode::FileButton_ViewMode_GetPreferedYSize(fb);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void HeightWidth_ViewMode::DynamicCallback(long ID,char *String,void *args,DynamicTalker *ItemChanging)
{
	if (ItemChanging == m_parentWindow)
	{
		if (IsDeletion(String))
			m_parentWindow = NULL;
		else if (!strcmp(String, BaseWindowClass_Sized))
		{
			if (m_matchHeight || m_matchWidth)
			{
				if ((m_viewMode == FileButton_ViewMode_NameOnly) || 
					(m_viewMode == FileButton_ViewMode_SmallIcon) ||
					(m_viewMode & FileButton_ViewMode_Details) ||
					(m_viewMode == FileButton_ViewMode_SuperLargeIcon))	// mwatkins
					UFBVM_SendSizeChanges();
			}
		}
	}
	UtilLib_FileButton_ViewMode::DynamicCallback(ID, String, args, ItemChanging);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void HeightWidth_ViewMode::FileButton_ViewMode_SetViewMode(int viewMode, long XSize, long YSize)
{
	// watch for conflicting views
	UtilLib_SimpleGrid* grid = GetInterface<UtilLib_SimpleGrid>(m_parentWindow);
	if (grid) grid->MultipleLayouts_Start();
	if (grid && ShouldIMatchWidth(NULL))
	{
		int oldViewMode = FileButton_ViewMode_GetViewMode();
		bool newWide = (viewMode & FileButton_ViewMode_Details);
		bool wasWide = (oldViewMode & FileButton_ViewMode_Details);

		if (wasWide && !newWide && (m_oldSpacerX > -1))
		{
			grid->UtilLib_SimpleGrid_PutSpacingX(m_oldSpacerX);
			m_oldSpacerX = -1;
		}
		else if (!wasWide && newWide)
		{
			m_oldSpacerX = grid->UtilLib_SimpleGrid_GetSpacingX();
			grid->UtilLib_SimpleGrid_PutSpacingX(0);
		}
	}
	UtilLib_FileButton_ViewMode::FileButton_ViewMode_SetViewMode(viewMode, XSize, YSize);
	if (grid) grid->MultipleLayouts_Finished(true);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////


