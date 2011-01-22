-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Ramora.lua")

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
	ID = "Ramora1",
	Ship = Ramora,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = -100,
	Heading = 0, Pitch = 0, Roll = 0
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {
	{
		-- Unique Identifier
		ID = "Ramora2",
		Ship = Ramora,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 200,
		Heading = 180, Pitch = 0, Roll = 0
	}

} -- End the Existing Ships Structure


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")


