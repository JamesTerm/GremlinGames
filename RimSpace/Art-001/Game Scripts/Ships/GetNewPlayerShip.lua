-- This file contains the reusable function for Getting the Player's Ships.
-- The function depends on MAX_Players, AvailableShips[], EXISTING_SHIPS[]

-- Holds the list of ships the players already have
PlayerShips = {}

-- Maintain the original number of existing ships
NUM_ORIG_EXISTING_SHIPS = #EXISTING_SHIPS

-- This will start at index 1
function GetNewPlayerShip(index)
	if (index > (MAX_Players)) then
		return nil
	else 
		if (PlayerShips[index]==nil) then
			PlayerShips[index] = AvailableShips[index]
			EXISTING_SHIPS[NUM_ORIG_EXISTING_SHIPS+index] = PlayerShips[index]
		end
		return PlayerShips[index]
	end
end