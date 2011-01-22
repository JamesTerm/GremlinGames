#pragma once

namespace Soccer
{

enum GameType
{
	eSoccer_7701,
	eSoccer_7711,
	eSoccer_7611,
	eSoccer_7601,
	eSoccer_7501,
	eSoccer_7604,
	eNumGameTypes
};

enum MasterFieldList
{
	eClock,			//[7+1];
	eClockStatus,	//[1+1];
	eHomeScore,		//[3+1];
	eGuestScore,	//[3+1];
	eHomeTOFull,	//[1+1];
	eHomeTOTotal,	//[1+1];
	eGuestTOFull,	//[1+1];
	eGuestTOTotal,	//[1+1];
	eHalf,			//[1+1];
	eMinutes,
	eSeconds,
	eMilli

	//The values below which are not in the current feed are not transmitted via the serial
	// port on our reference model. (From PDF)
	/*
	eClock,				//[5+1];
	eHomeTeamName,		//[10+1];
	eGuestTeamName,		//[10+1];
	eHomeScore,			//[2+1];
	eGuesScore,			//[2+1];
	eHalf,				//[1+1];
	eHomeShotsOnGoal,	//[2+1];
	eHomeSaves,			//[2+1];
	eHomeCornerKicks,	//[2+1];
	eGuestShotsOnGoal,	//[2+1];
	eGuestSaves,		//[2+1];
	eGuestCornerKicks,	//[2+1];
	eHomeFouls,			//[2+1];
	eGuestFouls,		//[2+1];
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

const Game_Element c_Soccer_7701[] = {
	{ eClock,			1,	7, "DakClock"		},
	{ eClockStatus,		0,	1, "DakClockStatus"	},
	{ eHomeScore,		2,	3, "DakHomeScore"	},
	{ eGuestScore,		0,	3, "DakGuestScore"	},
	{ eHomeTOFull,		4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,		1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,	1,	1, "DakGuestTOTotal"},
	{ eHalf,			0,	1, "DakHalf"		},	//The last four are discerned from the time value.
	{ eMinutes,			0,	0, "DakMin"			},
	{ eSeconds,			0,	0, "DakSec"			},
	{ eMilli,			0,	0, "DakTen"			}

//The following is the Soccer definition as per the Daktronics feed PDF but it
// does not agree with the reverse engineered feed from our Daktronics AllSport 5000.
//The values below which are not in the current feed are not transmitted via the serial
// port on our reference model. (From PDF)
/*
	{ eClock,			5,	"DakClock"				},
	{ eHomeTeamName,	10,	"DakHomeTeamName"		},
	{ eGuestTeamName,	10,	"DakGuestTeamName"		},
	{ eHomeScore,		2,	"DakHomeScore"			},
	{ eGuestScore,		2,	"DakGuestScore"			},
	{ eHalf,			1,	"DakHalf"				},
	{ eHomeShotsOnGoal,	2,	"DakHomeShotsOnGoal"	},
	{ eHomeSaves,		2,	"DakHomeSaves"			},
	{ eHomeCornerKicks,	2,	"DakHomeCornerKicks"	},
	{ eGuestShotsOnGoal,2,	"DakGuestShotsOnGoal"	},
	{ eGuestSaves,		2,	"DakGuestSaves"			},
	{ eGuestCornerKicks,2,	"DakGuestCornerKicks"	},
	{ eHomeFouls,		2,	"DakHomeFouls"			},
	{ eGuestFouls,		2,	"DakGuestFouls"			},
	{ eHours,			0,	"DakHhr"				},
	{ eMinutes,			0,	"DakMin"				},
	{ eSeconds,			0,	"DakSec"				},
	{ eMilli,			0,	"DakTen"				}
*/
};

const Game_Element c_Soccer_7711[] = {
	{ eClock,			1,	7, "DakClock"		},
	{ eClockStatus,		0,	1, "DakClockStatus"	},
	{ eHomeScore,		2,	3, "DakHomeScore"	},
	{ eGuestScore,		0,	3, "DakGuestScore"	},
	{ eHomeTOFull,		4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,		1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,	1,	1, "DakGuestTOTotal"},
	{ eHalf,			0,	1, "DakHalf"		},	//The last four are discerned from the time value.
	{ eMinutes,			0,	0, "DakMin"			},
	{ eSeconds,			0,	0, "DakSec"			},
	{ eMilli,			0,	0, "DakTen"			}
};

const Game_Element c_Soccer_7611[] = {
	{ eClock,			1,	7, "DakClock"		},
	{ eClockStatus,		0,	1, "DakClockStatus"	},
	{ eHomeScore,		2,	3, "DakHomeScore"	},
	{ eGuestScore,		0,	3, "DakGuestScore"	},
	{ eHomeTOFull,		4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,		1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,	1,	1, "DakGuestTOTotal"},
	{ eHalf,			0,	1, "DakHalf"		},	//The last four are discerned from the time value.
	{ eMinutes,			0,	0, "DakMin"			},
	{ eSeconds,			0,	0, "DakSec"			},
	{ eMilli,			0,	0, "DakTen"			}
};

const Game_Element c_Soccer_7601[] = {
	{ eClock,			1,	7, "DakClock"		},
	{ eClockStatus,		0,	1, "DakClockStatus"	},
	{ eHomeScore,		2,	3, "DakHomeScore"	},
	{ eGuestScore,		0,	3, "DakGuestScore"	},
	{ eHomeTOFull,		4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,		1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,	1,	1, "DakGuestTOTotal"},
	{ eHalf,			0,	1, "DakHalf"		},	//The last four are discerned from the time value.
	{ eMinutes,			0,	0, "DakMin"			},
	{ eSeconds,			0,	0, "DakSec"			},
	{ eMilli,			0,	0, "DakTen"			}
};

const Game_Element c_Soccer_7501[] = {
	{ eClock,			1,	7, "DakClock"		},
	{ eClockStatus,		0,	1, "DakClockStatus"	},
	{ eHomeScore,		2,	3, "DakHomeScore"	},
	{ eGuestScore,		0,	3, "DakGuestScore"	},
	{ eHomeTOFull,		4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,		1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,	1,	1, "DakGuestTOTotal"},
	{ eHalf,			0,	1, "DakHalf"		},	//The last four are discerned from the time value.
	{ eMinutes,			0,	0, "DakMin"			},
	{ eSeconds,			0,	0, "DakSec"			},
	{ eMilli,			0,	0, "DakTen"			}
};

const Game_Element c_Soccer_7604[] = {
	{ eClock,			1,	7, "DakClock"		},
	{ eClockStatus,		0,	1, "DakClockStatus"	},
	{ eHomeScore,		2,	3, "DakHomeScore"	},
	{ eGuestScore,		0,	3, "DakGuestScore"	},
	{ eHomeTOFull,		4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,		1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,	1,	1, "DakGuestTOTotal"},
	{ eHalf,			0,	1, "DakHalf"		},	//The last four are discerned from the time value.
	{ eMinutes,			0,	0, "DakMin"			},
	{ eSeconds,			0,	0, "DakSec"			},
	{ eMilli,			0,	0, "DakTen"			}
};


}