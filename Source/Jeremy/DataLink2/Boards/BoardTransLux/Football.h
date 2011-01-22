#pragma once

namespace Football
{

enum GameType
{
	eFootball_0,
	eNumGameTypes
};

enum MasterFieldList
{
	eClock,
	eQuarter,
	eHomeScore,
	eVisitorScore,
	eDown,
	eToGo,
	eBallOn,
	eFieldTimer,
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

const Game_Element c_Football_0[] = {
	{ eClock,			2,	7, "TLFPClock"			},
	{ eQuarter,			2,	1, "TLFPQuarter"		},
	{ eHomeScore,		8,	2, "TLFPHomeScore"		},
	{ eVisitorScore,	5,	3, "TLFPVisitorScore"	},
	{ eDown,			2,	1, "TLFPDown",			},
	{ eToGo,			6,	2, "TLFPToGo",			},
	{ eBallOn,			11,	2, "TLFPBallOn",		},
	{ eFieldTimer,		3,	2, "TLFPFieldTimer",	},
	{ eMinutes,			0,	0, "TLFPMin"			},
	{ eSeconds,			0,	0, "TLFPSec"			},
	{ eMilli,			0,	0, "TLFPTen"			}
};

}