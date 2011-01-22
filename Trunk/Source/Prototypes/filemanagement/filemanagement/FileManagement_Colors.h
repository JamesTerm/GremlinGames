#ifndef __FileManagement_Colors__
#define __FileManagement_Colors__

const byte FILEBUTTONCOLORS_BACKGROUND[4][3]=	{	{	152, 156, 158 },//{	120,120,120	},													
													{	153,174,207	},													
													{	169, 173, 175 },//{	133,133,133	},													
													{	153+33,174+33,207+33 },
												};

const byte FILEBUTTONCOLORS_TEXT[4][3]=	{	{	0,0,0	},													
											{	0,0,0	},													
											{	0,0,0	},													
											{	0,0,0	}
										};


const byte VIDEOEDITOR_BACKGROUND[4][3]=	{	{	120,120,120	},													
												{	201,207,153	},													
												{	133,133,133	},													
												{	234,240,186 },
											};

const byte VIDEOEDITOR_TEXT[4][3]=	{	{	0,0,0	},													
										{	0,0,0	},													
										{	0,0,0	},													
										{	0,0,0	}
									};

#define FILEBUTTONHILITE			RGB(255,255,255)
#define FILEBUTTONSHADOW			RGB(0,0,0)
#define FILEBUTTONHILITE_RO			RGB(255,255,192)
#define FILEBUTTONSHADOW_RO			RGB(32,32,32)

#define FILEBUTTONHILITE_CUT		RGB(255,128,128)
#define FILEBUTTONSHADOW_CUT		RGB(255,0,0)
#define FILEBUTTONHILITE_RO_CUT		RGB(255,128,128)
#define FILEBUTTONSHADOW_RO_CUT		RGB(255,0,0)

#define FILEBUTTONCUT				RGB(255,192,192)
#define FILEBUTTONNOINFO			RGB(192,192,255)


const float	HotList_FontSize[] =		{	TL_GetFloat("VTNTAPI::HotListDefaultFontSize",-1.1f),
											TL_GetFloat("VTNTAPI::HotListDefaultFontSize",-1.1f),
											TL_GetFloat("VTNTAPI::HotListDefaultFontSize",-1.1f),
											TL_GetFloat("VTNTAPI::HotListDefaultFontSize",-1.1f)
										};

#define		HotList_Font				TL_GetString("VTNTAPI::HotListDefaultFont",VT_Default_Font)

const int	HotList_BGColor[4][3]=		{ {120,120,120},{133,133,133},{153,174,207},{153+33,174+33,207+33} };
const int	HotList_FGColor[4][3]=		{ {0,0,0},{0,0,0},{0,0,0},{0,0,0} };
const bool	HotList_Underline[] =		{ false,true,true,true };
const bool	HotList_Bold[] =			{ true,true,true,true };
const bool	HotList_Italic[] =			{ false,false,false,false };

#endif __FileManagement_Colors__