-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/ShipClasses.lua")
dofile("Ships/Q_33.lua")
dofile("Ships/Ramora.lua")
dofile("Ships/Ring_01.lua")
dofile("Ships/Vexhall.lua")
dofile("Ships/Crusader.lua")
dofile("Ships/Karackuss.lua")
dofile("Ships/Orion.lua")

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"Space/Planet System Scripts/Baked Earth Moon Sun Neb_B.osgv",
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
			{X =  800, Y = 0, Z = 7500, Power = 0.3}, -- power through the straight away
			{X =  800, Y = 0, Z = 9000, Power = 0.1}, -- slow down, ready for the turn
			{X =    0, Y = 0, Z =10000, Power = 0.1}, -- Around the corner
			{X = -800, Y = 0, Z = 9000, Power = 0.1}, -- still keep it slow to make the turn
			{X = -800, Y = 0, Z = 1500, Power = 0.3}, -- power through the straight away
			{X = -800, Y = 0, Z =    0, Power = 0.1}, -- slow down, ready for the turn
			{X =    0, Y = 0, Z =-1000, Power = 0.1}, -- Around the corner
			{X =  800, Y = 0, Z =    0, Power = 0.1}  -- still keep it slow to make the turn
		}
	},

	{
		-- Unique Identifier
		ID = "Vexhall",
		Ship = Vexhall,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 100,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = true,
		
		FollowShip = {
			ShipID = "Ramora", X =  -150, Y = 50, Z = -150
			}
	},

	{
		-- Unique Identifier
		ID = "Jack1",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 200,
		Heading = 180, Pitch = 0, Roll = 20
	},

	{
		-- Unique Identifier
		ID = "Jack2",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 500.0,
		Heading = 180, Pitch = 0, Roll = 30
	},

	{
		-- Unique Identifier
		ID = "Jack3",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 1000.0,
		Heading = 180, Pitch = 0, Roll = 40
	},

	{
		-- Unique Identifier
		ID = "Jack4",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 3000.0,
		Heading = 180, Pitch = 0, Roll = 50
	},

	{
		-- Unique Identifier
		ID = "Jack5",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 6000.0,
		Heading = 180, Pitch = 0, Roll = 60
	},

	{
		-- Unique Identifier
		ID = "Jack6",
		Ship = Ring_01,
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 9000.0,
		Heading = 180, Pitch = 0, Roll = 70
	},

	{
		-- Unique Identifier
		ID = "Karackuss",
		Ship = Karackuss,

		UserCanPilot = false,	-- the default is true
		
		-- Starting Position and Attitude
		X = 0, Y = 0, Z = 11000.0,
		Heading = 180, Pitch = 0, Roll = 0
	},


	{
		-- Unique Identifier
		ID = "Orion",
		Ship = Orion,
		
		-- Starting Position and Attitude
		X = 0, Y = 100, Z = 1500,
		Heading = 0, Pitch = 0, Roll = 0,
		
		UserCanPilot = false,	-- the default is true
			
	},

} -- End the Existing Ships Structure



-- Player ships load seperate
AvailableShips = {}

AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Q33_2",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = 0,
	Heading = 0, Pitch = 0, Roll = 0,
	
	FollowShip = {
		ShipID = "Ramora", X =  -120, Y = 30, Z = -250
		}

}



-- Here is a nice reusable required function for loading the proper ship for the player
dofile("GetNewPlayerShip.lua")



