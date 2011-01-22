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
	eWWPeriod,
	eWWClock,
	eWWHomeScore,
	eWWAwayScore,
	eWWShotClock,
	eMinutes,
	eSeconds,
	eMilli,
	eWWNumFields
};

struct Game_Element
{
	MasterFieldList field;
	size_t offset;
	size_t width;
	const char *name;
};

const Game_Element c_Basketball_0[] = {
	{ eWWPeriod,		5,	1,	"WWPeriod"		},
	{ eWWClock,			0,	5,	"WWClock"		},
	{ eWWHomeScore,		0,	2,	"WWHomeScore"	},
	{ eWWAwayScore,		0,	2,	"WWAwayScore"	},
	{ eWWShotClock,		0,	2,	"WWShotClock"	},
	{ eMinutes,			0,	0,	"WWMin"			},
	{ eSeconds,			0,	0,	"WWSec"			},
	{ eMilli,			0,	0,	"WWTen"			}
};

}