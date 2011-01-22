-- Solo_Q34.lua
-- A simple test script that allows us to focus on a single ship (without being blown up) :)
-- by Rick 10/4/2009
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("../Ships/ShipClasses.lua")
dofile("../Ships/Fighters/Q34.lua")

-- PLAYER SHIP ----------------------------------------------------------------

PlayerShips = {}
AvailableShips = {}

AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Q34 B",
	Ship = Q34,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = 0,
	Heading = 0, Pitch = 0, Roll = 0,
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
EXISTING_SHIPS = {}

-- Here is a nice reusable required function for loading the proper ship for the player
dofile("../Ships/GetNewPlayerShip.lua")

-- This is used to keep a good track of load status, copy this from the "LOAD_TIMES.txt" file after running a script
LOAD_TIMES = {
}

