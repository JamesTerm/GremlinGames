#pragma once

namespace Basketball
{

	enum GameType
	{
		eBasketball_0,
		eNoGameTypes
	};

	enum MasterFieldList
	{
		eDSIClock,		//[7+1];
		eDSIShotClock,	//[2+1];
		eHours,
		eMinutes,
		eSeconds,
		eMilli
	};

	struct Game_Element
	{
		MasterFieldList field;
		size_t Width;
		const char *name;
	};

	const Game_Element c_Basketball_0[] = {
		{ eDSIClock,	7, "DSIClock"		},
		{ eDSIShotClock,2, "DSIShotClock"	},	//The last four are discerned from the time value.
		{ eHours,		0, "DSIHhr"			},
		{ eMinutes,		0, "DSIMin"			},
		{ eSeconds,		0, "DSISec"			},
		{ eMilli,		0, "DSITen"			}
	};

}