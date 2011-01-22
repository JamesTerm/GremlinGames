-- This file contains the reusable function for Getting the Player's Mechs.
-- The function depends on MAX_Players, AvailableMechs[], EXISTING_Mechs[]

-- Holds the list of Mechs the players already have
PlayerMechs = {}

-- Maintain the original number of existing Mechs
NUM_ORIG_EXISTING_Mechs = #EXISTING_Mechs

-- This will start at index 1
function GetNewPlayerMech(index)
	if (index > (MAX_Players)) then
		return nil
	else 
		if (PlayerMechs[index]==nil) then
			PlayerMechs[index] = AvailableMechs[index]
			EXISTING_Mechs[NUM_ORIG_EXISTING_Mechs+index] = PlayerMechs[index]
		end
		return PlayerMechs[index]
	end
end