#pragma once

namespace Football
{

enum GameType
{
	eFootball_6601,
	eFootball_6611,
	eFootball_6103,
	eFootball_6104,
	eFootball_6105,
	eFootball_6501,
	eFootball_6604,
	eFootball_6612,
	eNumGameTypes
};

enum MasterFieldList
{	//** These values were reverse engineered and are different from feed description in PDF. **
	eClock,			//[7+1];
	eClockStatus,	//[1+1];
	ePlayClock,		//[2+1];
	eHomeScore,		//[3+1];
	eGuestScore,	//[3+1];
	eHomeTOFull,	//[1+1];
	eHomeTOPart,	//[1+1]; ** not used?
	eHomeTOTotal,	//[1+1];
	eGuestTOFull,	//[1+1];
	eGuestTOPart,	//[1+1]; ** not used?
	eGuestTOTotal,	//[1+1];
	eQuarter,		//[1+1];
	eMinutes,
	eSeconds,
	eMilli

	/*
	//The values below which are not in the current feed are not transmitted via the serial
	// port on our reference model. (From PDF)
	eClock,			//[5+1];
	eHomeTeamName,	//[10+1]; //not supported by current serial port feed.
	eGuestTeamName,	//[10+1]; //not supported by current serial port feed.
	eHomeScore,		//[2+1];
	eGuestScore,	//[2+1];
	eQuarter,		//[1+1];
	eBallOn,		//[2+1]; //not supported by current serial port feed.
	eDown,			//[1+1]; //not supported by current serial port feed.
	eToGo,			//[2+1]; //not supported by current serial port feed.
	eHomePossess,	//[1+1]; //not supported by current serial port feed.
	eGuestPossess,	//[1+1]; //not supported by current serial port feed.
	ePlayClock,		//[2+1];
	eHomeTO,		//[1+1];
	eGuestTO,		//[1+1];
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

const Game_Element c_Football_6601[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};
	
//The following is the Football definition as per the Daktronics feed PDF but it
// does not agree with the reverse engineered feed from our Daktronics AllSport 5000.
//The values below which are not in the current feed are not transmitted via the serial
// port on our reference model. (From PDF)
/*	{ eClock,			5,	"DakClock"			},
	{ eHomeTeamName,	10, "DakHomeTeamName"	},
	{ eGuestTeamName,	10, "DakGuestTeamName"	},
	{ eHomeScore,		2,	"DakHomeScore"		},
	{ eGuestScore,		2,	"DakGuestScore"		},
	{ eQuarter,			1,	"DakQuarter"		},
	{ eBallOn,			2,	"DakBallOn"			},
	{ eDown,			1,	"DakDown"			},
	{ eToGo,			2,	"DakToGo"			},
	{ eHomePossess,		1,	"DakHomePossess"	},
	{ eGuestPossess,	1,	"DakGuestPossess"	},
	{ ePlayClock,		2,	"DakPlayClock"		},
	{ eHomeTO,			1,	"DakHomeTO"			},
	{ eGuestTO,			1,	"DakGuestTO"		},	//The last four are discerned from the time value.
	{ eHours,			0,	"DakHhr"			},
	{ eMinutes,			0,	"DakMin"			},
	{ eSeconds,			0,	"DakSec"			},
	{ eMilli,			0,	"DakTen"			}
*/


const Game_Element c_Football_6611[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};

const Game_Element c_Football_6103[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};

const Game_Element c_Football_6104[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};

const Game_Element c_Football_6105[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};

const Game_Element c_Football_6501[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};

const Game_Element c_Football_6604[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};

const Game_Element c_Football_6612[] = {
	{ eClock,			1,	7,	"DakClock"			},
	{ eClockStatus,		0,	1,	"DakClockStatus"	},
	{ ePlayClock,		0,	2,	"DakPlayClock"		},
	{ eHomeScore,		0,	3,	"DakHomeScore"		},
	{ eGuestScore,		0,	3,	"DakGuestScore"		},
	{ eHomeTOFull,		4,	1,	"DakHomeTOFull"		},
	{ eHomeTOPart,		0,	1,	"DakHomeTOPart"		},
	{ eHomeTOTotal,		0,	1,	"DakHomeTOTotal"	},
	{ eGuestTOFull,		0,	1,	"DakGuestTOFull"	},
	{ eGuestTOPart,		0,	1,	"DakGuestTOPart"	},
	{ eGuestTOTotal,	0,	1,	"DakGuestTOTotal"	},
	{ eQuarter,			0,	1,	"DakQuarter"		},
	{ eMinutes,			0,	0,	"DakMin"			},
	{ eSeconds,			0,	0,	"DakSec"			},
	{ eMilli,			0,	0,	"DakTen"			}
};


}