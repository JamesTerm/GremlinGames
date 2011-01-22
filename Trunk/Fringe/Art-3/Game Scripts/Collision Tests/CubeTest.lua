-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("../Ships/ShipClasses.lua")
dofile("../Ships/Cube.lua")
dofile("../Ships/Q_33.lua")

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"Space/Planet System Scripts/Baked Earth Moon Sun Neb_B.osgv",
}

EXISTING_SHIPS = {

	{
		-- Unique Identifier
		ID = "Cube",
		Ship = Cube,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 0,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
	},
	
	{
		-- Unique Identifier
		ID = "Q_33_N",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 100,
		Heading = 180, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
	},
	
	{
		-- Unique Identifier
		ID = "Q_33_W",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = -100, Y = 0, Z = 0,
		Heading = -90, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
	},
	
	{
		-- Unique Identifier
		ID = "Q_33_E",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = 100, Y = 0, Z = 0,
		Heading = 90, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
	},
	
	{
		-- Unique Identifier
		ID = "Q_33_U",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = 0, Y = 100, Z = 0,
		Heading = 0, Pitch = 90, Roll = 0,
		
		UserCanPilot = true,
	},
	
	{
		-- Unique Identifier
		ID = "Q_33_D",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = 0, Y = -100, Z = 0,
		Heading = 0, Pitch = -90, Roll = 0,
		
		UserCanPilot = true,
	},
}

AvailableShips = {}

AvailableShips[1] = {
	-- Unique Identifier
	ID = "Q_33_S",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = -100,
	Heading = 0, Pitch = 0, Roll = 0,
	
	UserCanPilot = true,
}

-- Here is a nice reusable required function for loading the proper ship for the player
dofile("../GetNewPlayerShip.lua")



