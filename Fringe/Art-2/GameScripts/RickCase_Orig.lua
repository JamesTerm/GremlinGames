-- This Script is the one that Rick Likes.  It has some pretty high-res features
------------------------------------------------------------- 

-- Load the parameters from the ships we want in this game
dofile("Ships/Q_33.lua")
dofile("Ships/Ramora.lua")

-- MAX_Players is a global variable that Fringe.Server expects.
MAX_Players = 1

-- Tell all of the UI clients what kinds of backgrounds we are using
-- Usually we just want a single System Script.osgv file, but you can have more
-- that one OSGV file in her (e.g. we can break up the planets, stars, sun, etc.)
-- All of the files are relative to the Content Directory
ENVIRONMENT = {
	"Space/Planet System Scripts/EarthAndMoon.osgv",
	"ObstacleCourse.osgv"
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {}

-- List the initial AI, keep the number here (do we need this counter?)
NUM_EXISTING_SHIPS = 1
EXISTING_SHIPS[1] = {
	-- Unique Identifier
	ID = "Ramora_1",
	Ship = Ramora,
	
	-- Starting Position and Attitude
	X = 0, Y = 0, Z = 0,
	Heading = 0, Pitch = 0, Roll = 0
}

PlayerShips = {}
AvailableShips = {}
AvailableShips[2] = {	
	-- Unique Identifier
	ID = "Q33_2",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = -85.3, Y = 69.7, Z = -228.3,
	Heading = 0, Pitch = 0, Roll = 0
}


-- This will start at index 2 (1 is for the AI)
function GetNewPlayerShip(index)
	if (index > 2) then
		return nil
	else 
		if (PlayerShips[index]==nil) then
			PlayerShips[index] = AvailableShips[index]
			EXISTING_SHIPS[NUM_EXISTING_SHIPS+index-1] = PlayerShips[index]
		end
		return PlayerShips[index]
	end
end

