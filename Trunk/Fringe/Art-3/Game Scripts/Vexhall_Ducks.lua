-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Vexhall.lua")
dofile("Ships/Q_33.lua")

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
}

AvailableShips = {}

AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Vexhall",
	Ship = Vexhall,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = -100,
	Heading = 0, Pitch = 0, Roll = 0
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {
	{
		-- Unique Identifier
		ID = "Q_33_MoveRt",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = 100, Y = 0, Z = 0,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = false,	-- the default is true
		
		WayPointLoop = {
			{X =  100, Y = 0, Z = 75000, Power = 0.1 }, -- Start off slow (~10 mps)
		}
	},

	{
		-- Unique Identifier
		ID = "Q_33_StillCtr",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 0,
		Heading = 0, Pitch = 85, Roll = 0,
		
		UserCanPilot = true,
	},
	
	{
		-- Unique Identifier
		ID = "Q_33_StillUL",
		Ship = Q_33,
		
		-- Starting Position and Attitude
		X = -20, Y = 20, Z = 0,
		Heading = 180, Pitch = 60, Roll = 0,
		
		UserCanPilot = true,
	},

} -- End the Existing Ships Structure


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")


