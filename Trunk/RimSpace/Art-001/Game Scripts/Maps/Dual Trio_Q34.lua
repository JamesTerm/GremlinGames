-- Project "Test Ships"
-- this script is to test the new "RimSpace" content
-- by justin 9/16/09
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("../Ships/ShipClasses.lua")
dofile("../Ships/Fighters/Q34.lua")
dofile("../Ships/Vessels/Crusader.lua")

-- PLAYER SHIP ----------------------------------------------------------------

AvailableShips = {
	
	{
		-- Unique Identifier
		ID = "Gold 1",
		Ship = Q34,
	
		-- Starting Position and Attitude
		X = 0, Y = 200, Z = 5000,
		Heading = 180, Pitch = 0, Roll = 0,
	
		TARGET_SHIP = "Gold 2",
	},
	
	{
		-- Unique Identifier
		ID = "Gold 2",
		Ship = Q34,
		
		-- Starting Position and Attitude
		X = 100, Y = 2000, Z = 3000,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
		
		TARGET_SHIP = "Gold 3",
	},
}

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 2

-- Environment ------------------------------------------------------------------------------

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in here (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"Space/SpaceBox/SpaceBoxB/EarthMoonSun.osgv",
}

-- List each of the ships already in the scene.  This list will be added to 

-- Objects ------------------------------------------------------------------


EXISTING_SHIPS = {
	
	{
		-- Unique Identifier
		ID = "Crusader",
		Ship = Crusader_RED,
		
		-- Starting Position and Attitude
		X = -1000, Y = 2000, Z = 5000,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = false,
	},
	
	{
		-- Unique Identifier
		ID = "Gold 3",
		Ship = Q34,
		
		-- Starting Position and Attitude
		X = 1000, Y = 2000, Z = 3000,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
		
		TARGET_SHIP = "Gold 2",
	},



} -- End of ExistingShips

-- ------------------------------------------------------------------------------


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("../Ships/GetNewPlayerShip.lua")

-- This is used to keep a good track of load status, copy this from the "LOAD_TIMES.txt" file after running a script
LOAD_TIMES = {
}

