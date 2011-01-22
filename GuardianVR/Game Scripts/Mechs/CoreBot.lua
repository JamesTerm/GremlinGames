CoreBot = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "CoreBot/CoreBot-01.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "Mech",
	MECH_CLASS = NormalMech,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = 5,
	
	-- Make the ship light so it will move with bullets
	Mass = 10000, -- Weight kg
	RESPAWN_HITPOINTS = 5,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	
	-- dHeading is used to limit keyboard turn speed, as well as how fast the Mech can turn
	-- the mouse can look faster, but the mech has to "catch up"
	dHeading = 120, 
	
	-- dPitch is only used for looking up and down, and even then only for keyboard
	dPitch = 180,
}
