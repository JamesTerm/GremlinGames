Ring_01 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Space_Ring01/ring_01.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "DestroyableEntity",
	SHIP_CLASS = MedSupportVessel,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = -1, -- less than 0 means it will never respawn
	
	Mass = 150000, -- Weight kg
	RESPAWN_HITPOINTS = 75,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	
	Dimensions =
	{ Length=1, Width=34, Height=34}, --These should be "roughly" correct in meters
}

Stripped_Ring_01 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Stripped_Space_Ring01/ring_01.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "DestroyableEntity",
	SHIP_CLASS = MedSupportVessel,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = -1, -- less than 0 means it will never respawn
	
	Mass = 150000, -- Weight kg
	RESPAWN_HITPOINTS = 75,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	
	Dimensions =
	{ Length=1, Width=34, Height=34}, --These should be "roughly" correct in meters
}