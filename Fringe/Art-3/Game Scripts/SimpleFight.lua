-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Q_33.lua")
dofile("Ships/Karackuss.lua")
dofile("Ships/Ring_01.lua")
dofile("Ships/Vexhall.lua")

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"Space/Planet System Scripts/Baked Earth Moon Sun Neb_B.osgv",
}

AvailableShips = {

	 {
		-- Unique Identifier
		ID = "Vexhall",
		Ship = Vexhall,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 0,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
		
		TARGET_SHIP = "Q33",
	}
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {
	{
		-- Unique Identifier
		ID = "Karackuss",
		Ship = Karackuss,
		
		-- Starting Position and Attitude
		X = 500, Y = 0, Z = 1500,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = false,	-- the default is true
		
		WayPointLoop = {
			{X =  800, Y = 0, Z = 7500, Power = 0.3, TurnSpeedScaler = 0.5}, -- power through the straight away
			{X =  800, Y = 0, Z = 9000, Power = 0.3, TurnSpeedScaler = 0.5}, -- slow down, ready for the turn
			{X = -800, Y = 0, Z = 9000, Power = 0.3, TurnSpeedScaler = 0.5}, -- still keep it slow to make the turn
			{X = -800, Y = 0, Z = 1500, Power = 0.3, TurnSpeedScaler = 0.5}, -- power through the straight away
			{X = -800, Y = 0, Z =    0, Power = 0.3, TurnSpeedScaler = 0.5}, -- slow down, ready for the turn
			{X =  800, Y = 0, Z =    0, Power = 0.3, TurnSpeedScaler = 0.5}  -- still keep it slow to make the turn
		}
	},
	
	{	
		-- Unique Identifier
		ID = "Q33",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = 300, Y = 150, Z = 1500,
		Heading = 0, Pitch = 0, Roll = 0,
		
		TARGET_SHIP = "Vexhall",
		
	},

	{
		-- Unique Identifier
		ID = "Jack1",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 200,
		Heading = 180, Pitch = 0, Roll = 20
	},

	{
		-- Unique Identifier
		ID = "Jack2",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 500.0,
		Heading = 180, Pitch = 0, Roll = 30
	},

	{
		-- Unique Identifier
		ID = "Jack3",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 1000.0,
		Heading = 180, Pitch = 0, Roll = 40
	},

	{
		-- Unique Identifier
		ID = "Jack4",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 3000.0,
		Heading = 180, Pitch = 0, Roll = 50
	},

	{
		-- Unique Identifier
		ID = "Jack5",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 6000.0,
		Heading = 180, Pitch = 0, Roll = 60
	},

	{
		-- Unique Identifier
		ID = "Jack6",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 9000.0,
		Heading = 180, Pitch = 0, Roll = 70
	},

} -- End the Existing Ships Structure


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")

LOAD_TIMES = {
	2.621073,
	2.627084,
	2.633512,
	2.634161,
	2.634766,
	2.636700,
	2.638643,
	2.648639,
	2.649504,
	2.668422,
	3.475748,
	3.502689,
	3.503774,
	3.513871,
	3.533433,
	3.552531,
	3.570577,
	3.588891,
	3.607165,
	3.642804,
	3.677765,
	3.688178,
	3.689027,
	3.689787,
	3.718688,
	3.741778,
	3.750158,
	3.751342,
	3.751663,
	3.752371,
	3.752661,
	3.753174,
	3.778116,
	3.985700,
	4.016310,
	4.046675,
	4.063433,
	4.094254,
	4.113725,
	4.132747,
	4.152091,
	4.157141,
	4.162511,
	4.182575,
	4.237931,
	4.257919,
	4.268673,
	4.272946,
	4.283772,
	4.288019,
	4.298929,
	4.310070,
	4.330620,
	4.349966,
	4.368919,
	4.379018,
	4.390502,
	4.391398,
	4.392588,
	4.393151,
	4.402599,
	4.403148,
	4.410529,
	4.410863,
	4.411355,
	4.411717,
	4.412531,
	4.461655,
	4.481100,
	4.500545,
	4.513241,
	4.523343,
	4.534301,
	4.555367,
	4.566138,
	4.585099,
	4.593489,
	4.612373,
	4.621426,
	4.640088,
	4.641171,
	4.656245,
	4.660828,
	4.661811,
	4.662133,
	4.662861,
	4.663149,
	4.663674,
	4.686438,
	4.719326,
	4.723184,
	4.726915,
	4.730615,
	4.734252,
	4.737914,
	4.741657,
	4.745328,
	4.749085,
	4.752723,
	4.756428,
	4.760111,
	4.763761,
	4.767365,
	4.770952,
	4.774639,
	4.778261,
	4.782033,
	4.785656,
	4.789319,
	4.792917,
	4.796506,
	4.800230,
	4.803870,
	4.807642,
	4.811821,
	4.816616,
	4.820763,
	4.824471,
	4.828063,
	4.831725,
	4.835416,
	4.839077,
	4.842673,
	4.846281,
	4.849953,
	4.853552,
	4.857189,
	4.860827,
	4.864450,
	4.868164,
	4.871943,
	4.875699,
	4.879384,
	4.882995,
	4.886767,
	4.890397,
	4.894018,
	4.897612,
	4.901477,
	4.905139,
	4.908809,
	4.912435,
	4.916892,
	4.921055,
	4.924969,
	4.928752,
	4.932473,
	4.936285,
	4.940021,
	4.943616,
	4.947444,
	4.951420,
	4.955062,
	4.958869,
	4.962661,
	4.966378,
	4.970018,
	4.973722,
	4.977406,
	4.981197,
	4.984828,
	4.988562,
	4.992271,
	4.995896,
	4.999490,
	5.209524,
	5.219713,
	5.230211,
	5.241035,
	5.245345,
	5.249337,
	5.253199,
	5.257211,
	5.261227,
	5.265536,
	5.269631,
	5.273767,
	5.277740,
	5.281721,
	5.285692,
	5.289579,
	5.293478,
	5.297511,
	5.301570,
	5.306172,
	5.310483,
	5.315705,
	5.316717,
	5.317117,
	5.318078,
	5.350723,
	5.362033,
	5.362357,
	5.363228,
	5.363547,
	5.479279,
	5.489507,
	5.500210,
	5.510657,
	5.514841,
	5.518681,
	5.522600,
	5.527062,
	5.531464,
	5.535491,
	5.539419,
	5.543498,
	5.547403,
	5.551334,
	5.555184,
	5.559105,
	5.563123,
	5.566979,
	5.570923,
	5.574846,
	5.578953,
	5.582892,
	5.583697,
	5.584023,
	5.584619,
	5.610462,
	5.622708,
	5.623058,
	5.624264,
	5.624630,
	5.742958,
	5.754605,
	5.765987,
	5.777318,
	5.782132,
	5.786325,
	5.790293,
	5.794173,
	5.798180,
	5.802193,
	5.806925,
	5.812056,
	5.816995,
	5.821029,
	5.825081,
	5.828979,
	5.832928,
	5.836907,
	5.840950,
	5.844986,
	5.848922,
	5.852885,
	5.853693,
	5.853990,
	5.854647,
	5.879677,
	5.892820,
	5.893161,
	5.893893,
	5.894181,
	6.015245,
	6.026529,
	6.039065,
	6.050251,
	6.054549,
	6.058550,
	6.062591,
	6.066541,
	6.070372,
	6.074436,
	6.078448,
	6.082354,
	6.086228,
	6.090190,
	6.094238,
	6.098361,
	6.102418,
	6.106441,
	6.110580,
	6.114425,
	6.118800,
	6.122811,
	6.123583,
	6.123891,
	6.124557,
	6.150722,
	6.162317,
	6.162649,
	6.163451,
	6.163742,
	6.280620,
	6.290711,
	6.301452,
	6.313032,
	6.318552,
	6.322580,
	6.326498,
	6.330472,
	6.334336,
	6.338157,
	6.342090,
	6.346119,
	6.350011,
	6.354036,
	6.357959,
	6.361869,
	6.365841,
	6.370059,
	6.373991,
	6.378428,
	6.383000,
	6.387009,
	6.387818,
	6.388129,
	6.389212,
	6.415408,
	6.428692,
	6.429013,
	6.429727,
	6.430041,
	6.545514,
	6.555574,
	6.565764,
	6.576025,
	6.579996,
	6.583899,
	6.587846,
	6.592261,
	6.596358,
	6.600347,
	6.604386,
	6.608395,
	6.612263,
	6.616088,
	6.620697,
	6.624777,
	6.628637,
	6.632795,
	6.636835,
	6.641056,
	6.645039,
	6.649038,
	6.649797,
	6.650095,
	6.650741,
	6.675942,
	6.687887,
	6.688235,
	6.688951,
	6.689236,
	7.245439,
	7.254346,
	7.285137,
	7.304399,
	7.325422,
	7.344517,
	7.347974,
	7.353582,
	7.372175,
	7.391764,
	7.410872,
	7.428721,
	7.448420,
	7.468012,
	7.478246,
	7.488520,
	7.494877,
	7.500859,
	7.506869,
	7.512975,
	7.522309,
	7.532987,
	7.535128,
	7.536445,
	7.537126,
	7.544165,
	7.544482,
	7.551509,
	7.552043,
	7.552361,
	7.553134,
	7.695105,
	7.705048,
	7.713333,
	7.716634,
	7.717216,
	7.717782,
	7.718472,
	7.729784,
	7.748684,
	7.763040,
	7.767529,
	7.768509,
	7.768819,
	7.769557,
	7.769842,
	7.770534,
	7.792725,
	7.826100,
	7.830019,
	7.833802,
	7.837607,
	7.841368,
	7.845271,
	7.849010,
	7.852766,
	7.856449,
	7.860232,
	7.864045,
	7.867862,
	7.871570,
	7.875411,
	7.879114,
	7.882838,
	7.886526,
	7.890203,
	7.893977,
	7.898187,
	7.902259,
	7.906132,
	7.909963,
	7.913672,
	7.918486,
	7.922338,
	7.926056,
	7.929813,
	7.933688,
	7.937459,
	7.941243,
	7.945199,
	7.949029,
	7.952736,
	7.956485,
	7.960171,
	7.963984,
	7.967744,
	8.275336,
	15.919095,
}


