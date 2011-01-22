Crusader = {
	-- The OSGV file that represents it visually.  When using '\', you have to have '\\'
	OSGV = "POSER_Crusier_Union_Crusader\\Union-Crusader.lwo",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Cruiser,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = 15,
	
	Mass = 50000000, -- Weight Kg
	RESPAWN_HITPOINTS = 25000,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 10, MaxAccelRight = 10, MaxAccelUp = 10, MaxAccelDown = 10, MaxAccelForward = 16, MaxAccelReverse = 15, 
	MaxTorqueYaw = 0.5, MaxTorquePitch = 0.5, MaxTorqueRoll = 0.5, 

	MAX_SPEED = 400, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 100, -- Engagement Speed
	
	ACCEL = 15,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 7.5,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 30, dPitch = 20, dRoll = 5,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical.osgv",

	Dimensions =
	{ Length=610, Width=90, Height=50}, --These should be "roughly" correct in meters
}