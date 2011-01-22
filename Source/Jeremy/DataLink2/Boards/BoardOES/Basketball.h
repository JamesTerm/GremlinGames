#pragma once

namespace Basketball
{

enum GameType
{
	eBasketball_0,
	eNumGameTypes
};

enum MasterFieldList
{
	eOESClock,		
	eOESPeriod,		
	eOESHomeScore,	
	eOESAwayScore,	
	eOESHomeTOFull,	
	eOESHomeTOPart,	
	eOESAwayTOFull,	
	eOESAwayTOPart,	
	eOESHomeFouls,	
	eOESAwayFouls,	
	eOESShotClock,	
	eMinutes,			
	eSeconds,			
	eMilli,			
};

struct Game_Element
{
	MasterFieldList field;
	size_t offset;
	size_t width;
	const char *name;
};

const Game_Element c_Basketball_0[] = {
	{ eOESClock,		2,	4,	"OESClock"		},
	{ eOESPeriod,		0,	1,	"OESPeriod"		},
	{ eOESHomeScore,	0,	2,	"OESHomeScore"	},
	{ eOESAwayScore,	0,	2,	"OESAwayScore"	},
	{ eOESHomeTOFull,	0,	1,	"OESHomeTOFull"	},
	{ eOESHomeTOPart,	0,	0,	"OESHomeTOPart"	},
	{ eOESAwayTOFull,	0,	1,	"OESAwayTOFull"	},
	{ eOESAwayTOPart,	0,	0,	"OESAwayTOPart"	},
	{ eOESHomeFouls,	0,	1,	"OESHomeFouls"	},
	{ eOESAwayFouls,	0,	1,	"OESAwayFouls"	},
	{ eOESShotClock,	3,	3,	"OESShotClock"	},
	{ eMinutes,			0,	0,	"OESMin"		},
	{ eSeconds,			0,	0,	"OESSec"		},
	{ eMilli,			0,	0,	"OESTen"		}
};

}