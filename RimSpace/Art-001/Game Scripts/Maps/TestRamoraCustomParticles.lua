-- Project "Test Ships"
-- this script is to test the new "RimSpace" content
-- by justin 9/16/09
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("../Ships/ShipClasses.lua")
dofile("../Ships/Fighters/Q34.lua")
dofile("../Ships/Vessels/Ramora.lua")

-- PLAYER SHIP ----------------------------------------------------------------

AvailableShips = {
	
	{
		-- Unique Identifier
		ID = "Q34 B",
		Ship = Q34,
	
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 6000,
		Heading = 180, Pitch = 0, Roll = 0,
	},
}

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- Environment ------------------------------------------------------------------------------

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in here (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"Space/SpaceBox/SpaceBoxC/Scace-C.osgv",
}

-- List each of the ships already in the scene.  This list will be added to 

-- Objects ------------------------------------------------------------------


EXISTING_SHIPS = {

	{
		-- Unique Identifier
		ID = "Ramora",
		Ship = Ramora,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 5000,
		Heading = 180, Pitch = 0, Roll = 0,
		
		UserCanPilot = false,
	},

} -- End of ExistingShips

-- ------------------------------------------------------------------------------


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("../Ships/GetNewPlayerShip.lua")

-- This is used to keep a good track of load status, copy this from the "LOAD_TIMES.txt" file after running a script
LOAD_TIMES = {
}

