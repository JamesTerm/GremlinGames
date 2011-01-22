Ramora = {
	-- The OSGV file that represents it visually.  When using '\', you have to have '\\'
	OSGV = "POSER_Federation_Ramora\\Ramora-01.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Cruiser,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = 15,
	
	Mass = 45000000, -- Weight Kg
	RESPAWN_HITPOINTS = 27500,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 5, MaxAccelRight = 5, MaxAccelUp = 5, MaxAccelDown = 20, MaxAccelForward = 32, MaxAccelReverse = 16, 
	MaxTorqueYaw = 0.01, MaxTorquePitch = 0.01, MaxTorqueRoll = 0.01, 

	MAX_SPEED = 400, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 100, -- Engagement Speed
	
	ACCEL = 30,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 15,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 30, dPitch = 20, dRoll = 5,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical.osgv",

	Dimensions =
	{ Length=400, Width=356, Height=120}, --These should be "roughly" correct in meters
	
}

Stripped_Ramora = {
	-- The OSGV file that represents it visually.  When using '\', you have to have '\\'
	OSGV = "Stripped_POSER_Federation_Ramora\\Ramora-01.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Cruiser,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = 15,
	
	Mass = 45000000, -- Weight Kg
	RESPAWN_HITPOINTS = 27500,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 5, MaxAccelRight = 5, MaxAccelUp = 5, MaxAccelDown = 20, MaxAccelForward = 32, MaxAccelReverse = 16, 
	MaxTorqueYaw = 1, MaxTorquePitch = 1, MaxTorqueRoll = 1, 

	MAX_SPEED = 400, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 100, -- Engagement Speed
	
	ACCEL = 30,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 15,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 30, dPitch = 20, dRoll = 5,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical.osgv",

	Dimensions =
	{ Length=400, Width=356, Height=120}, --These should be "roughly" correct in meters
	
}