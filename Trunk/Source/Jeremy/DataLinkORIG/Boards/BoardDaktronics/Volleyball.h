#pragma once

namespace Volleyball
{

enum GameType
{
	eVolleyball_2101,
	eVolleyball_2111,
	eVolleyball_2103,
	eVolleyball_2104,
	eVolleyball_2105,
	eNumGameTypes
};

enum MasterFieldList
{
	eClock,					//[7+1];
	eClockStatus,			//[1+1];
	eHomeServiceIndicator,	//[2+1];
	eHomeScore,				//[2+1];
	eGuestScore,			//[2+1];
	eHomeTOFull,			//[1+1];
	eHomeTOTotal,			//[1+1];
	eGuestTOFull,			//[1+1];
	eGuestTOTotal,			//[1+1];
	eGameNumber,			//[1+1];
	eMinutes,
	eSeconds,
	eMilli

	//The values below which are not in the current feed are not transmitted via the serial
	// port on our reference model. (From PDF)
	/*
	eClock,					//[7+1];
	eClockStatus,			//[1+1];
	eHomeGameScore,			//[2+1];
	eGuestGameScore,		//[2+1];
	eHomeTO,				//[1+1];
	eGuestTO,				//[1+1];
	eHomeServiceIndicator,	//[1+1];
	eGuestServiceIndicator,	//[1+1];
	eHomeGamesWon,			//[1+1];
	eGuestGamesWon,			//[1+1];
	eGameNumber,			//[1+1];
	eHomeGameScore1,		//[2+1];
	eHomeGameScore2,		//[2+1];
	eHomeGameScore3,		//[2+1];
	eHomeGameScore4,		//[2+1];
	eGuestGameScore1,		//[2+1];
	eGuestGameScore2,		//[2+1];
	eGuestGameScore3,		//[2+1];
	eGuestGameScore4,		//[2+1];
	eHours,
	eMinutes,
	eSeconds,
	eMilli
	*/
};

struct Game_Element
{
	MasterFieldList field;
	size_t offset;
	size_t width;
	const char *name;
};

const Game_Element c_Volleyball_2101[] = {
	{ eClock,				1,	7,	"DakClock"					},
	{ eClockStatus,			0,	1,	"DakClockStatus"			},
	{ eHomeServiceIndicator,0,	2,	"DakHomeServiceIndicator"	},
	{ eHomeScore,			1,	2,	"DakHomeScore"				},
	{ eGuestScore,			1,	2,	"DakGuestScore"				},
	{ eHomeTOFull,			4,	1,	"DakHomeTOFull"				},
	{ eHomeTOTotal,			1,	1,	"DakHomeTOTotal"			},
	{ eGuestTOFull,			0,	1,	"DakGuestTOFull"			},
	{ eGuestTOTotal,		1,	1,	"DakGuestTOTotal"			},
	{ eGameNumber,			0,	1,	"DakGameNumber"				},	//The last four are discerned from the time value.
	{ eMinutes,				0,	0,	"DakMin"					},
	{ eSeconds,				0,	0,	"DakSec"					},
	{ eMilli,				0,	0,	"DakTen"					}

//The eMillifollowing is the Hockey definition as per the Daktronics feed PDF but it
// does not agree with the reverse engineered feed from our Daktronics AllSport 5000.
//The values below which are not in the current feed are not transmitted via the serial
// port on our reference model. (From PDF)
/*
	{ eClock,					7,	"DakClock"					},
	{ eClockStatus,				1,	"DakClockStatus"			},
	{ eHomeGameScore,			2,	"DakHomeGameScore"			},
	{ eGuestGameScore,			2,	"DakGuestGameScore"			},
	{ eHomeTO,					1,	"DakHomeTO"					},
	{ eGuestTO,					1,	"DakGuestTO"				},
	{ eHomeServiceIndicator,	1,	"DakHomeServiceIndicator"	},
	{ eGuestServiceIndicator,	1,	"DakGuestServiceIndicator"	},
	{ eHomeGamesWon,			1,	"DakHomeGamesWon"			},
	{ eGuestGamesWon,			1,	"DakGuestGamesWon"			},
	{ eGameNumber,				1,	"DakGameNumber"				},
	{ eHomeGameScore1,			2,	"DakHomeGameScore1"			},
	{ eHomeGameScore2,			2,	"DakHomeGameScore2"			},
	{ eHomeGameScore3,			2,	"DakHomeGameScore3"			},
	{ eHomeGameScore4,			2,	"DakHomeGameScore4"			},
	{ eGuestGameScore1,			2,	"DakGuestGameScore1"		},
	{ eGuestGameScore2,			2,	"DakGuestGameScore2"		},
	{ eGuestGameScore3,			2,	"DakGuestGameScore3"		},
	{ eGuestGameScore4,			2,	"DakGuestGameScore4"		},
	{ eHours,					0,	"DakHhr"					},
	{ eMinutes,					0,	"DakMin"					},
	{ eSeconds,					0,	"DakSec"					},
	{ eMilli,					0,	"DakTen"					}
*/
};

const Game_Element c_Volleyball_2111[] = {
	{ eClock,				1,	7,	"DakClock"					},
	{ eClockStatus,			0,	1,	"DakClockStatus"			},
	{ eHomeServiceIndicator,0,	2,	"DakHomeServiceIndicator"	},
	{ eHomeScore,			1,	2,	"DakHomeScore"				},
	{ eGuestScore,			1,	2,	"DakGuestScore"				},
	{ eHomeTOFull,			4,	1,	"DakHomeTOFull"				},
	{ eHomeTOTotal,			1,	1,	"DakHomeTOTotal"			},
	{ eGuestTOFull,			0,	1,	"DakGuestTOFull"			},
	{ eGuestTOTotal,		1,	1,	"DakGuestTOTotal"			},
	{ eGameNumber,			0,	1,	"DakGameNumber"				},	//The last four are discerned from the time value.
	{ eMinutes,				0,	0,	"DakMin"					},
	{ eSeconds,				0,	0,	"DakSec"					},
	{ eMilli,				0,	0,	"DakTen"					}
};

const Game_Element c_Volleyball_2103[] = {
	{ eClock,				1,	7,	"DakClock"					},
	{ eClockStatus,			0,	1,	"DakClockStatus"			},
	{ eHomeServiceIndicator,0,	2,	"DakHomeServiceIndicator"	},
	{ eHomeScore,			1,	2,	"DakHomeScore"				},
	{ eGuestScore,			1,	2,	"DakGuestScore"				},
	{ eHomeTOFull,			4,	1,	"DakHomeTOFull"				},
	{ eHomeTOTotal,			1,	1,	"DakHomeTOTotal"			},
	{ eGuestTOFull,			0,	1,	"DakGuestTOFull"			},
	{ eGuestTOTotal,		1,	1,	"DakGuestTOTotal"			},
	{ eGameNumber,			0,	1,	"DakGameNumber"				},	//The last four are discerned from the time value.
	{ eMinutes,				0,	0,	"DakMin"					},
	{ eSeconds,				0,	0,	"DakSec"					},
	{ eMilli,				0,	0,	"DakTen"					}
};

const Game_Element c_Volleyball_2104[] = {
	{ eClock,				1,	7,	"DakClock"					},
	{ eClockStatus,			0,	1,	"DakClockStatus"			},
	{ eHomeServiceIndicator,0,	2,	"DakHomeServiceIndicator"	},
	{ eHomeScore,			1,	2,	"DakHomeScore"				},
	{ eGuestScore,			1,	2,	"DakGuestScore"				},
	{ eHomeTOFull,			4,	1,	"DakHomeTOFull"				},
	{ eHomeTOTotal,			1,	1,	"DakHomeTOTotal"			},
	{ eGuestTOFull,			0,	1,	"DakGuestTOFull"			},
	{ eGuestTOTotal,		1,	1,	"DakGuestTOTotal"			},
	{ eGameNumber,			0,	1,	"DakGameNumber"				},	//The last four are discerned from the time value.
	{ eMinutes,				0,	0,	"DakMin"					},
	{ eSeconds,				0,	0,	"DakSec"					},
	{ eMilli,				0,	0,	"DakTen"					}
};

const Game_Element c_Volleyball_2105[] = {
	{ eClock,				1,	7,	"DakClock"					},
	{ eClockStatus,			0,	1,	"DakClockStatus"			},
	{ eHomeServiceIndicator,0,	2,	"DakHomeServiceIndicator"	},
	{ eHomeScore,			1,	2,	"DakHomeScore"				},
	{ eGuestScore,			1,	2,	"DakGuestScore"				},
	{ eHomeTOFull,			4,	1,	"DakHomeTOFull"				},
	{ eHomeTOTotal,			1,	1,	"DakHomeTOTotal"			},
	{ eGuestTOFull,			0,	1,	"DakGuestTOFull"			},
	{ eGuestTOTotal,		1,	1,	"DakGuestTOTotal"			},
	{ eGameNumber,			0,	1,	"DakGameNumber"				},	//The last four are discerned from the time value.
	{ eMinutes,				0,	0,	"DakMin"					},
	{ eSeconds,				0,	0,	"DakSec"					},
	{ eMilli,				0,	0,	"DakTen"					}
};


}