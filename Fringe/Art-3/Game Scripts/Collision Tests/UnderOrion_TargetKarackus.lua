-- Project "TWILIGHTZONE"
-- I am working on this script for the VIP demo, Rick will most likely tear this thing apart... LOSER!!
-- new edit by justin 5/7/08
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("../Ships/ShipClasses.lua")
dofile("../Ships/Q_33.lua")
dofile("../Ships/Ramora.lua")
dofile("../Ships/Ring_01.lua")
dofile("../Ships/Vexhall.lua")
dofile("../Ships/Karackuss.lua")
dofile("../Ships/Orion.lua")


-- PLAYER SHIP ----------------------------------------------------------------

PlayerShips = {}
AvailableShips = {}

AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Red_A",
	Ship = Vexhall,
	
	-- Starting Position and Attitude
	X = 0, Y = -100, Z = 7500,
	Heading = 180, Pitch = 0, Roll = 0,
	
	TARGET_SHIP = "Red Guard",
}

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- ------------------------------------------------------------------------------

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"Space/Planet System Scripts/Baked Earth Moon Sun Neb_B.osgv",
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {}


-- RED TEAM ------------------------------------------------------------------

-- Large Ships

EXISTING_SHIPS = {
{
	-- Unique Identifier
	ID = "Red Command",
	Ship = Orion,
	
	UserCanPilot = false,	-- the default is true

	-- Starting Position and Attitude
	X = 0, Y = 0, Z = 8000.0,
	Heading = 180, Pitch = 0, Roll = 0,

	WayPointLoop = {
		{X = 0, Y = 0, Z = 8000.0, Power = 0.25, TurnSpeedScaler = 1},
	}
},


{
	-- Unique Identifier
	ID = "Red Guard",
	Ship = Karackuss,
	
	-- Starting Position and Attitude
	X = -400, Y = 150, Z = 8500,
	Heading = 180, Pitch = 0, Roll = -0,

	UserCanPilot = false,	-- the default is true

	WayPointLoop = {
		{X = -400, Y = 150, Z = 8500, Power = 0.25, TurnSpeedScaler = 1},
	}

},

} -- End of ExistingShips

-- ------------------------------------------------------------------------------


-- Here is a nice reusable required function for loading the proper ship for the player
dofile("../GetNewPlayerShip.lua")
