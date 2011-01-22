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
	eDSIClock,		//[7+1];
	eDSIShotClock,	//[2+1];
	eMinutes,
	eSeconds,
	eMilli
};

struct Game_Element
{
	MasterFieldList field;
	size_t offset;
	size_t width;
	const char *name;
};

const Game_Element c_Basketball_0[] = {
	{ eDSIClock,	4,	7, "DSIClock"		},
	{ eDSIShotClock,4,	2, "DSIShotClock"	},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DSIMin"			},
	{ eSeconds,		0,	0, "DSISec"			},
	{ eMilli,		0,	0, "DSITen"			}
};

}