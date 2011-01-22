-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Q_33.lua")
dofile("Ships/Ramora.lua")
dofile("Ships/Vexhall.lua")

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
	ID = "Q33",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = 400, Y = 75, Z = 1250,
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
			{X =  500, Y = 0, Z = 1750, Power = 0.1 }, -- Start off slow (~10 mps)
			{X =  500, Y = 0, Z = 75000, Power = 1.0 }, -- Go Faster (~100 mps)
			{X =  500, Y = 0, Z = 0, Power = 1.0 }, -- Turn Around
		}
	},

	{
		-- Unique Identifier
		ID = "Vexhall",
		Ship = Vexhall,
		
		-- Starting Position and Attitude
		X = 390, Y = 82, Z = 1275,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
		
		FollowShip = {
			ShipID = "Q33", X = -10, Y = 7, Z = 25
			},
	},

} -- End the Existing Ships Structure


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")


