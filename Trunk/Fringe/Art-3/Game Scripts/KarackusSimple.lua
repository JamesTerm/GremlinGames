-- This Test Script Will walk through every aspect of the use case
-- And also provide good examples for the various features and 
-- Usages with lots of comments.
-------------------------------------------------------------

-- COMMENTS, all comments start with "--"

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Karackuss.lua")

-- We will need to know the number of potantial players.
-- Note that this is not used for the Sinlg Player version
-- This is how you declare a global variable.  

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {}

AvailableShips = {}
AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Karackuss",
	Ship = Karackuss,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = 0,
	Heading = 0, Pitch = 0, Roll = 0
}


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")
