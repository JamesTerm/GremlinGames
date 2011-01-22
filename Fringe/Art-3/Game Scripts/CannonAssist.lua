-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Ramora.lua")
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
		ID = "Vexhall",
		Ship = Vexhall,
		
		-- Starting Position and Attitude
		X = 300, Y = 150, Z = 1500,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
		
		-- TARGET_SHIP = "Ramora",
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
} -- End the Existing Ships Structure


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")



