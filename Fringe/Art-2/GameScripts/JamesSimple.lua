-- This Test Script Will walk through every aspect of the use case
-- And also provide good examples for the various features and 
-- Usages with lots of comments.
-------------------------------------------------------------

-- COMMENTS, all comments start with "--"

-- Load the parameters from the ships we want in this game
dofile("Ships/Q_33.lua")

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
--	"Test Looping Scene 001-2min.lws",
--	"../Art/JamesEarthAndMoon.osgv"
}

-- List each of the ships already in the scene.  This list will be added to	
EXISTING_SHIPS = {}

-- List the initial AI, keep the number here (do we need this counter?)
NUM_EXISTING_SHIPS = 0

AvailableShips = {}
AvailableShips[1] = {	
	-- Unique Identifier
	ID = "Q33_2",
	Ship = Q_33,
	
	-- Starting Position and Attitude
	X = -85.3, Y = 69.7, Z = -228.3,
	Heading = 0, Pitch = 0, Roll = 0
}


-- This will start at index 2 (1 is for the AI)
function GetNewPlayerShip(index)
	if (index == 2) then
		return AvailableShips[1]
	else		
		return nil
	end
end
