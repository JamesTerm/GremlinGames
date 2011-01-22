#pragma once

namespace Basketball
{

enum GameType
{
	eBasketball_1101,
	eBasketball_1102,
	eBasketball_1103,
	eBasketball_1104,
	eBasketball_1105,
	eBasketball_1301,
	eBasketball_1401,
	eBasketball_1402,
	eNumGameTypes
};

enum MasterFieldList
{
	eClock,			//[7+1];
	eClockStatus,	//[1+1];
	eShotClock,		//[2+1];
	eHomeScore,		//[3+1];
	eGuestScore,	//[3+1];
	eHomeFouls,		//[2+1];
	eGuestFouls,	//[2+1];
	eHomeTOFull,	//[1+1];
	eHomeTOPart,	//[1+1];
	eHomeTOTotal,	//[1+1];
	eGuestTOFull,	//[1+1];
	eGuestTOPart,	//[1+1];
	eGuestTOTotal,	//[1+1];
	ePeriod,		//[1+1];
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

const Game_Element c_Basketball_1101[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Basketball_1102[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Basketball_1103[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Basketball_1104[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Basketball_1105[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Basketball_1301[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Basketball_1401[] = {
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};

const Game_Element c_Basketball_1402[] = {			
	{ eClock,		1,	7, "DakClock"		},
	{ eClockStatus,	0,	1, "DakClockStatus"	},
	{ eShotClock,	0,	2, "DakShotClock"	},
	{ eHomeScore,	0,	3, "DakHomeScore"	},
	{ eGuestScore,	0,	3, "DakGuestScore"	},
	{ eHomeFouls,	0,	2, "DakHomeFouls"	},
	{ eGuestFouls,	0,	2, "DakGuestFouls"	},
	{ eHomeTOFull,	0,	1, "DakHomeTOFull"	},
	{ eHomeTOPart,	0,	1, "DakHomeTOPart"	},
	{ eHomeTOTotal,	0,	1, "DakHomeTOTotal"	},
	{ eGuestTOFull,	0,	1, "DakGuestTOFull"	},
	{ eGuestTOPart,	0,	1, "DakGuestTOPart"	},
	{ eGuestTOTotal,0,	1, "DakGuestTOTotal"},
	{ ePeriod,		0,	1, "DakPeriod"		},	//The last four are discerned from the time value.
	{ eMinutes,		0,	0, "DakMin"			},
	{ eSeconds,		0,	0, "DakSec"			},
	{ eMilli,		0,	0, "DakTen"			}
};


}