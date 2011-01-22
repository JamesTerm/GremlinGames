-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Q_33.lua")
dofile("Ships/Ramora.lua")
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

AvailableShips = {}

AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Q33_Duck",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = 0,
	Heading = 0, Pitch = 0, Roll = 0,
	
	FollowShip = {
			ShipID = "Ramora", X = -100, Y = 75, Z = -250
			},
	
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {
	{
		-- Unique Identifier
		ID = "Ramora",
		Ship = Ramora,
		
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
		ID = "Vexhall",
		Ship = Vexhall,
		
		-- Starting Position and Attitude
		X = 300, Y = 150, Z = 1500,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
		
		TARGET_SHIP = "Q33_Duck",
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
	8.540445,
	8.543309,
	8.547199,
	8.547591,
	8.547934,
	8.549784,
	8.551689,
	8.617698,
	8.618239,
	8.636817,
	9.370326,
	9.398725,
	9.408678,
	9.418486,
	9.428437,
	9.438445,
	9.439399,
	9.439846,
	9.458251,
	9.476751,
	9.486815,
	9.496769,
	9.506679,
	9.516637,
	9.526690,
	9.536542,
	9.546574,
	9.556529,
	9.566424,
	9.567255,
	9.567701,
	9.586003,
	9.604572,
	9.614734,
	9.624719,
	9.634756,
	9.644846,
	9.667830,
	9.689325,
	9.711335,
	9.733057,
	9.760859,
	9.765051,
	9.765388,
	9.766051,
	9.854147,
	9.863046,
	9.863400,
	9.863767,
	9.864133,
	9.866412,
	9.866685,
	9.867359,
	9.867602,
	10.448868,
	10.457264,
	10.479753,
	10.498640,
	10.516978,
	10.536029,
	10.539206,
	10.544590,
	10.562892,
	10.581457,
	10.599854,
	10.617763,
	10.636367,
	10.655334,
	10.665163,
	10.692730,
	10.699230,
	10.705307,
	10.711289,
	10.717222,
	10.726288,
	10.776563,
	10.778477,
	10.779678,
	10.780270,
	10.788843,
	10.789148,
	10.795877,
	10.796350,
	10.796728,
	10.797463,
	10.939010,
	10.946166,
	10.953438,
	10.956390,
	10.956769,
	10.957135,
	10.957502,
	10.966728,
	10.985039,
	11.005140,
	11.013587,
	11.014569,
	11.014849,
	11.015536,
	11.015792,
	11.016283,
	11.038394,
	11.069792,
	11.073327,
	11.076887,
	11.080297,
	11.083636,
	11.086969,
	11.090319,
	11.093739,
	11.097147,
	11.100477,
	11.103837,
	11.107162,
	11.110660,
	11.114035,
	11.117393,
	11.120797,
	11.124143,
	11.127478,
	11.130816,
	11.134161,
	11.137602,
	11.141021,
	11.144384,
	11.147726,
	11.151051,
	11.154374,
	11.157752,
	11.161087,
	11.164473,
	11.167862,
	11.171203,
	11.174539,
	11.177881,
	11.181211,
	11.184579,
	11.188521,
	11.191941,
	11.195277,
	11.405066,
	11.415202,
	11.425210,
	11.435169,
	11.439207,
	11.443040,
	11.446920,
	11.450852,
	11.454764,
	11.458606,
	11.462557,
	11.466413,
	11.470272,
	11.474113,
	11.477939,
	11.481767,
	11.485662,
	11.489660,
	11.493543,
	11.497461,
	11.501365,
	11.505243,
	11.505861,
	11.506206,
	11.506867,
	11.536383,
	11.547247,
	11.547505,
	11.548167,
	11.548413,
	11.663103,
	11.673012,
	11.683005,
	11.692899,
	11.696814,
	11.700718,
	11.704597,
	11.708471,
	11.712332,
	11.716180,
	11.720119,
	11.723969,
	11.727927,
	11.731913,
	11.735778,
	11.739686,
	11.743526,
	11.747426,
	11.751296,
	11.755150,
	11.758998,
	11.762950,
	11.763546,
	11.763795,
	11.764359,
	11.788804,
	11.799685,
	11.799948,
	11.800681,
	11.800925,
	11.916739,
	11.926848,
	11.937151,
	11.947351,
	11.951648,
	11.955936,
	11.959837,
	11.963726,
	11.967669,
	11.971560,
	11.975467,
	11.979338,
	11.983306,
	11.987247,
	11.991085,
	11.994947,
	11.998893,
	12.002719,
	12.006580,
	12.010592,
	12.014521,
	12.018452,
	12.019059,
	12.019327,
	12.019952,
	12.044278,
	12.055839,
	12.056413,
	12.057323,
	12.057574,
	12.173950,
	12.184463,
	12.194996,
	12.205746,
	12.210771,
	12.214740,
	12.218738,
	12.222598,
	12.226553,
	12.230489,
	12.235444,
	12.239331,
	12.243279,
	12.247410,
	12.251574,
	12.255693,
	12.259594,
	12.263507,
	12.267367,
	12.271309,
	12.275506,
	12.279578,
	12.280173,
	12.280426,
	12.280984,
	12.305605,
	12.317316,
	12.317572,
	12.318262,
	12.318512,
	12.437785,
	12.449355,
	12.461085,
	12.473215,
	12.479119,
	12.483153,
	12.487205,
	12.491230,
	12.495284,
	12.499611,
	12.503734,
	12.507724,
	12.511934,
	12.516138,
	12.520127,
	12.524167,
	12.528220,
	12.532209,
	12.536235,
	12.540200,
	12.544507,
	12.548542,
	12.549151,
	12.549408,
	12.550080,
	12.575779,
	12.586810,
	12.587065,
	12.587831,
	12.588071,
	12.703617,
	12.713633,
	12.724060,
	12.734377,
	12.738672,
	12.742713,
	12.746861,
	12.750818,
	12.754656,
	12.758636,
	12.762667,
	12.766655,
	12.771056,
	12.774907,
	12.778793,
	12.782739,
	12.786552,
	12.790462,
	12.794821,
	12.798725,
	12.802654,
	12.806909,
	12.807527,
	12.807775,
	12.808334,
	12.833265,
	12.844984,
	12.845251,
	12.846255,
	12.846533,
	13.062380,
	13.084571,
	13.106968,
	13.116261,
	13.138696,
	13.157638,
	13.175876,
	13.194123,
	13.198905,
	13.204072,
	13.218847,
	13.229135,
	13.243933,
	13.254167,
	13.258310,
	13.269026,
	13.273333,
	13.284155,
	13.292348,
	13.310287,
	13.329168,
	13.347551,
	13.357623,
	13.367723,
	13.368366,
	13.369647,
	13.370295,
	13.378069,
	13.378606,
	13.388959,
	13.389298,
	13.389748,
	13.390135,
	13.390831,
	13.438943,
	13.455485,
	13.471944,
	13.481907,
	13.491605,
	13.499708,
	13.518119,
	13.526320,
	13.544574,
	13.552515,
	13.571886,
	13.580492,
	13.600296,
	13.601162,
	13.615821,
	13.620472,
	13.621325,
	13.621604,
	13.622410,
	13.622669,
	13.623188,
	13.646451,
	13.679368,
	13.682945,
	13.686323,
	13.689944,
	13.693387,
	13.696775,
	13.700172,
	13.704066,
	13.707441,
	13.710920,
	13.714532,
	13.718074,
	13.721395,
	13.724914,
	13.728257,
	13.731626,
	13.735166,
	13.738538,
	13.741964,
	13.745261,
	13.748556,
	13.751795,
	13.755033,
	13.758256,
	13.761478,
	13.764936,
	13.768284,
	13.771620,
	13.774953,
	13.778204,
	13.781444,
	13.784742,
	13.787975,
	13.791408,
	13.794811,
	13.798088,
	13.801467,
	13.804821,
	13.808212,
	13.811566,
	13.815110,
	13.818431,
	13.821757,
	13.825067,
	13.828536,
	13.831828,
	13.835092,
	13.838336,
	13.841575,
	13.844834,
	13.848081,
	13.851313,
	13.854573,
	13.858133,
	13.861424,
	13.864705,
	13.867968,
	13.871965,
	13.875731,
	13.879034,
	13.882266,
	13.885515,
	13.888805,
	13.892158,
	13.895489,
	13.898811,
	13.902566,
	13.905893,
	13.909140,
	13.912454,
	13.915696,
	13.918938,
	13.922666,
	13.926239,
	13.929532,
	13.932853,
	14.234706,
	16.376408,
}


