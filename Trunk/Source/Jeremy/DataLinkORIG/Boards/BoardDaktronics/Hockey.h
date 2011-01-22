#pragma once

namespace Hockey
{

enum GameType
{
	eHockey_4000,
	eHockey_4401,
	eHockey_4402,
	eHockey_4102,
	eHockey_4103,
	eHockey_4104,
	eHockey_4105,
	eHockey_4601,
	eNumGameTypes
};

enum MasterFieldList
{
	eClock,			//[7+1];
	eClockStatus,	//[1+1];
	eShotClock,		//[2+1];
	eHomeScore,		//[3+1];
	eGuestScore,	//[3+1];
	eHomeTOFull,	//[1+1];
	eHomeTOTotal,	//[1+1];
	eGuestTOFull,	//[1+1];
	eGuestTOTotal,	//[1+1];
	ePeriod,		//[1+1];
	eMinutes,
	eSeconds,
	eMilli

	//The values below which are not in the current feed are not transmitted via the serial
	// port on our reference model. (From PDF)
	/*
	eClock,						//[7+1];
	eClockStatus,				//[1+1];
	eHomeScore,					//[2+1];
	eGuestScore,				//[2+1];
	eHomeTO,					//[1+1];
	eGuestTO,					//[1+1];
	eHomeShotsOnGoal,			//[2+1];
	eGuestShotsOnGoal,			//[2+1];
	ePeriod,					//[2+1];
	eHomePenalty1_PlayerNum,	//[2+1];
	eHomePenalty1_PenaltyTime,	//[5+1];
	eHomePenalty2_PlayerNum,	//[2+1];
	eHomePenalty2_PenaltyTime,	//[5+1];
	eGuestPenalty1_PlayerNum,	//[2+1];
	eGuestPenalty1_PenaltyTime,	//[5+1];
	eGuestPenalty2_PlayerNum,	//[2+1];
	eGuestPenalty2_PenaltyTime,	//[5+1];
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

const Game_Element c_Hockey_4402[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}

//The following is the Hockey definition as per the Daktronics feed PDF but it
// does not agree with the reverse engineered feed from our Daktronics AllSport 5000.
//The values below which are not in the current feed are not transmitted via the serial
// port on our reference model. (From PDF)
/*
	{ eClock,						7,	"DakClock"						},
	{ eClockStatus,					1,	"DakClockStatus"				},
	{ eHomeScore,					2,	"DakHomeScore"					},
	{ eGuestScore,					2,	"DakGuestScore"					},
	{ eHomeTO,						1,	"DakHomeTO"						},
	{ eGuestTO,						1,	"DakGuestTO"					},
	{ eHomeShotsOnGoal,				2,	"DakHomeShotsOnGoal"			},
	{ eGuestShotsOnGoal,			2,	"DakGuestShotsOnGoal"			},
	{ ePeriod,						2,	"DakPeriod"						},
	{ eHomePenalty1_PlayerNum,		2,	"DakHomePenalty1_PlayerNum"		},
	{ eHomePenalty1_PenaltyTime,	5,	"DakHomePenalty1_PenaltyTime"	},
	{ eGuestPenalty1_PlayerNum,		2,	"DakGuestPenalty1_PlayerNum"	},
	{ eGuestPenalty1_PenaltyTime,	5,	"DakGuestPenalty1_PenaltyTime"	},
	{ eHomePenalty2_PlayerNum,		2,	"DakHomePenalty2_PlayerNum"		},
	{ eHomePenalty2_PenaltyTime,	5,	"DakHomePenalty2_PenaltyTime"	},
	{ eGuestPenalty2_PlayerNum,		2,	"DakGuestPenalty2_PlayerNum"	},
	{ eGuestPenalty2_PenaltyTime,	5,	"DakGuestPenalty2_PenaltyTime"	},
	{ eHours,						0,	"DakHhr"						},
	{ eMinutes,						0,	"DakMin"						},
	{ eSeconds,						0,	"DakSec"						},
	{ eMilli,						0,	"DakTen"						}
*/
};

const Game_Element c_Hockey_4000[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Hockey_4401[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Hockey_4102[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Hockey_4103[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Hockey_4104[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Hockey_4105[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Hockey_4601[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeTOFull,	4,	1, "DakHomeTOFull"	},
	{ eHomeTOTotal,	1,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOTotal,1,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};


}