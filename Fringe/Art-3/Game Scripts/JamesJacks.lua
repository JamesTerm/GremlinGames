-- This Test Script Will walk through every aspect of the use case
-- And also provide good examples for the various features and 
-- Usages with lots of comments.
-------------------------------------------------------------

-- COMMENTS, all comments start with "--"
-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Q_33.lua")
dofile("Ships/Ramora.lua")

-- We will need to know the number of potantial players.
-- Note that this is not used for the Sinlg Player version
-- This is how you declare a global variable.  

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 3

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"../../Art/JamesEarthAndMoon.osgv"
	-- "Space/Planet System Scripts/EarthAndMoon.osgv",
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {
	{
		-- Unique Identifier
		ID = "Ramora_1",
		Ship = Ramora,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 0,
		Heading = 0, Pitch = 0, Roll = 0
	},
} -- End the Existing Ships Structure


AvailableShips = {}
AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Q33_2",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = -85.3, Y = 69.7, Z = -228.3,
	Heading = 0, Pitch = 0, Roll = 0,
	
	WayPointLoop = {
		{X =  -85.3, Y = 69.7, Z =    -228.3, Power = 1.0}  -- still keep it slow to make the turn
	}
}

AvailableShips[2] = {	
	-- Unique Identifier
	ID = "Q33_3",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = -97, Y = 81, Z = -270,
	Heading = 0, Pitch = 0, Roll = 0
}

AvailableShips[3] = {	
	-- Unique Identifier
	ID = "Q33_4",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = -96.8, Y = 66.7, Z = -214.1,
	Heading = 0, Pitch = 0, Roll = 0
}

-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")

