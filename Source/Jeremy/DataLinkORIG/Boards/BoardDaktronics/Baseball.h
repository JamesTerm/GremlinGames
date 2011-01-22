#pragma once

namespace Baseball
{

enum GameType
{
	eBaseball_5501,
	eBaseball_5601,
	eBaseball_5602,
	eNumGameTypes
};

enum MasterFieldList
{	//** These values were reverse engineered and are different from feed description in PDF. **
	eClock,			//[7+1];
	eClockStatus,	//[1+1];
	eHomeHits,		//[1+1];
	eHomeScore,		//[2+1];
	eGuestScore,	//[2+1];
	eInning,		//[1+1];
	eMinutes,
	eSeconds,
	eMilli

	//The values below which are not in the current feed are not transmitted via the serial
	// port on our reference model. (From PDF)
	/*
	eHomeScore,				//[4+1];
	eGuestScore,			//[4+1];
	eInning,				//[2+1];
	eInningText,			//[4+1]; (1st, 2nd, 3rd...)
	eInningDescription,		//[12+1]; (Top of 1st...)
	eHomeAtBat,				//[1+1]; (on/off indicator)
	eGuestAtBat,			//[1+1]; (on/off indicator)
	eHomeHits,				//[2+1];
	eHomeErrors,			//[2+1];
	eHomeLeftOnBase,		//[2+1];
	eGuestHits,				//[2+1];
	eGuestErrors,			//[2+1];
	eGuestLeftOnBase,		//[2+1];
	eBatterNumber,			//[2+1];
	eBatterAverage,			//[5+1]; (#.###)
	eBall,					//[1+1];
	eStrike,				//[1+1];
	eOut,					//[1+1];
	eHit,					//[1+1]; (H)it indicator
	eError,					//[1+1]; (E)rror indicator
	eHitErrorText,			//[5+1]; 'Hit' or 'Error'
	eErrorPosition,			//[2+1];
	eInningLabel1,			//[2+1];
	eInningLabel2,			//[2+1];
	eInningLabel3,			//[2+1];
	eInningLabel4,			//[2+1];
	eInningLabel5,			//[2+1];
	eInningLabel6,			//[2+1];
	eInningLabel7,			//[2+1];
	eInningLabel8,			//[2+1];
	eInningLabel9,			//[2+1];
	eInningLabel10,			//[2+1];
	eInningLabel11,			//[2+1];
	eInningLabel12,			//[2+1];
	eHomeInningScore1,		//[2+1];
	eHomeInningScore2,		//[2+1];
	eHomeInningScore3,		//[2+1];
	eHomeInningScore4,		//[2+1];
	eHomeInningScore5,		//[2+1];
	eHomeInningScore6,		//[2+1];
	eHomeInningScore7,		//[2+1];
	eHomeInningScore8,		//[2+1];
	eHomeInningScore9,		//[2+1];
	eHomeInningScore10,		//[2+1];
	eHomeInningScore11,		//[2+1];
	eHomeInningScore12,		//[2+1];
	eGuestInningScore1,		//[2+1];
	eGuestInningScore2,		//[2+1];
	eGuestInningScore3,		//[2+1];
	eGuestInningScore4,		//[2+1];
	eGuestInningScore5,		//[2+1];
	eGuestInningScore6,		//[2+1];
	eGuestInningScore7,		//[2+1];
	eGuestInningScore8,		//[2+1];
	eGuestInningScore9,		//[2+1];
	eGuestInningScore10,	//[2+1];
	eGuestInningScore11,	//[2+1];
	eGuestInningScore12,	//[2+1];
	eHomePitcherNum,		//[2+1];
	eHomePitchesBalls,		//[3+1];
	eHomePitchesStrikes,	//[3+1];
	eHomePitchesFoulBall,	//[3+1];
	eHomePitchesInPlay,		//[3+1];
	eHomePitchesTotal,		//[3+1];
	eGuestPitcherNum,		//[2+1];
	eGuestPitchesBalls,		//[3+1];
	eGuestPitchesStrikes,	//[3+1];
	eGuestPitchesFoulBall,	//[3+1];
	eGuestPitchesInPlay,	//[3+1];
	eGuestPitchesTotal,		//[3+1];
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

const Game_Element c_Baseball_5501[] = {
	{ eClock,		1,	7,	"DakClock"			},
	{ eClockStatus,	0,	1,	"DakClockStatus"	},
	{ eHomeHits,	0,	1,	"DakHomeHits"		},
	{ eHomeScore,	2,	2,	"DakHomeScore"		},
	{ eGuestScore,	1,	2,	"DakGuestScore"		},
	{ eInning,		10,	1,	"DakInning"			},
	{ eMinutes,		0,	0,	"DakMin"			},
	{ eSeconds,		0,	0,	"DakSec"			},
	{ eMilli,		0,	0,	"DakTen"			},
	
//The following is the Baseball definition as per the Daktronics feed PDF but it
// does not agree with the reverse engineered feed from our Daktronics AllSport 5000.
//The values below which are not in the current feed are not transmitted via the serial
// port on our reference model. (From PDF)
/*
	{ eHomeScore,			4,	"DakHomeScore"			},
	{ eGuestScore,			4,	"DakGuestScore"			},
	{ eInning,				2,	"DakInning"				},
	{ eInningText,			4,	"DakInningText"			},
	{ eInningDescription,	1,	"DakInningDescription"	},
	{ eHomeAtBat,			1,	"DakHomeAtBat"			},
	{ eGuestAtBat,			1,	"DakGuestAtBat"			},
	{ eHomeHits,			2,	"DakHomeHits"			},
	{ eHomeErrors,			2,	"DakHomeErrors"			},
	{ eHomeLeftOnBase,		2,	"DakHomeLeftOnBase"		},
	{ eGuestHits,			2,	"DakGuestHits"			},
	{ eGuestErrors,			2,	"DakGuestErrors"		},
	{ eGuestLeftOnBase,		2,	"DakGuestLeftOnBase"	},
	{ eBatterNumber,		2,	"DakBatterNumber"		},
	{ eBatterAverage,		5,	"DakBatterAverage"		},
	{ eBall,				1,	"DakBall"				},
	{ eStrike,				1,	"DakStrike"				},
	{ eOut,					1,	"DakOut"				},
	{ eHit,					1,	"DakHit"				},
	{ eError,				1,	"DakError"				},
	{ eHitErrorText,		5,	"DakHitErrorText"		},
	{ eErrorPosition,		2,	"DakErrorPosition"		},
	{ eInningLabel1,		2,	"DakInningLabel1"		},
	{ eInningLabel2,		2,	"DakInningLabel2"		},
	{ eInningLabel3,		2,	"DakInningLabel3"		},
	{ eInningLabel4,		2,	"DakInningLabel4"		},
	{ eInningLabel5,		2,	"DakInningLabel5"		},
	{ eInningLabel6,		2,	"DakInningLabel6"		},
	{ eInningLabel7,		2,	"DakInningLabel7"		},
	{ eInningLabel8,		2,	"DakInningLabel8"		},
	{ eInningLabel9,		2,	"DakInningLabel9"		},
	{ eInningLabel10,		2,	"DakInningLabel10"		},
	{ eInningLabel11,		2,	"DakInningLabel11"		},
	{ eInningLabel12,		2,	"DakInningLabel12"		},
	{ eHomeInningScore1,	2,	"HomeInningScore1"		},
	{ eHomeInningScore2,	2,	"HomeInningScore2"		},
	{ eHomeInningScore3,	2,	"HomeInningScore3"		},
	{ eHomeInningScore4,	2,	"HomeInningScore4"		},
	{ eHomeInningScore5,	2,	"HomeInningScore5"		},
	{ eHomeInningScore6,	2,	"HomeInningScore6"		},
	{ eHomeInningScore7,	2,	"HomeInningScore7"		},
	{ eHomeInningScore8,	2,	"HomeInningScore8"		},
	{ eHomeInningScore9,	2,	"HomeInningScore9"		},
	{ eHomeInningScore10,	2,	"HomeInningScore10"		},
	{ eHomeInningScore11,	2,	"HomeInningScore11"		},
	{ eHomeInningScore12,	2,	"HomeInningScore12"		},
	{ eGuestInningScore1,	2,	"GuestInningScore1"		},
	{ eGuestInningScore2,	2,	"GuestInningScore2"		},
	{ eGuestInningScore3,	2,	"GuestInningScore3"		},
	{ eGuestInningScore4,	2,	"GuestInningScore4"		},
	{ eGuestInningScore5,	2,	"GuestInningScore5"		},
	{ eGuestInningScore6,	2,	"GuestInningScore6"		},
	{ eGuestInningScore7,	2,	"GuestInningScore7"		},
	{ eGuestInningScore8,	2,	"GuestInningScore8"		},
	{ eGuestInningScore9,	2,	"GuestInningScore9"		},
	{ eGuestInningScore10,	2,	"GuestInningScore10"	},
	{ eGuestInningScore11,	2,	"GuestInningScore11"	},
	{ eGuestInningScore12,	2,	"GuestInningScore12"	},
	{ eHomePitcherNum,		2,	"DakHomeScore"			},
	{ eHomePitchesBalls,	3,	"DakHomeScore"			},
	{ eHomePitchesStrikes,	3,	"DakHomeScore"			},
	{ eHomePitchesFoulBall,	3,	"DakHomeScore"			},
	{ eHomePitchesInPlay,	3,	"DakHomeScore"			},
	{ eHomePitchesTotal,	3,	"DakHomeScore"			},
	{ eGuestPitcherNum,		2,	"DakHomeScore"			},
	{ eGuestPitchesBalls,	3,	"DakHomeScore"			},
	{ eGuestPitchesStrikes,	3,	"DakHomeScore"			},
	{ eGuestPitchesFoulBall,3,	"DakHomeScore"			},
	{ eGuestPitchesInPlay,	3,	"DakHomeScore"			},
	{ eGuestPitchesTotal,	3,	"DakHomeScore"			},	//The last four are discerned from the time value.
	{ eHours,				0,	"DakHhr"				},
	{ eMinutes,				0,	"DakMin"				},
	{ eSeconds,				0,	"DakSec"				},
	{ eMilli,				0,	"DakTen"				}
*/
};

const Game_Element c_Baseball_5601[] = {
	{ eClock,		0,	7,	"DakClock"			},
	{ eClockStatus,	0,	1,	"DakClockStatus"	},
	{ eHomeHits,	0,	1,	"DakHomeHits"		},
	{ eHomeScore,	2,	2,	"DakHomeScore"		},
	{ eGuestScore,	1,	2,	"DakGuestScore"		},
	{ eInning,		10,	1,	"DakInning"			},
	{ eMinutes,		0,	0,	"DakMin"			},
	{ eSeconds,		0,	0,	"DakSec"			},
	{ eMilli,		0,	0,	"DakTen"			},
};


const Game_Element c_Baseball_5602[] = {
	{ eClock,		0,	7,	"DakClock"			},
	{ eClockStatus,	0,	1,	"DakClockStatus"	},
	{ eHomeHits,	0,	1,	"DakHomeHits"		},
	{ eHomeScore,	2,	2,	"DakHomeScore"		},
	{ eGuestScore,	1,	2,	"DakGuestScore"		},
	{ eInning,		10,	1,	"DakInning"			},
	{ eMinutes,		0,	0,	"DakMin"			},
	{ eSeconds,		0,	0,	"DakSec"			},
	{ eMilli,		0,	0,	"DakTen"			},
};


}