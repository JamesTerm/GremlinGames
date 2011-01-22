Orion = {
	-- The OSGV file that represents it visually.  When using '\', you have to have '\\'
	OSGV = "POSER_Union_Orion\\poser_Orion.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Carrier,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = 35,
	
	Mass = 134000000, -- Weight Kg
	RESPAWN_HITPOINTS = 67000,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 25, MaxAccelRight = 25, MaxAccelUp = 25, MaxAccelDown = 25, MaxAccelForward = 40, MaxAccelReverse = 35, 
	MaxTorqueYaw = 0.005, MaxTorquePitch = 0.005, MaxTorqueRoll = 0.005, 

	MAX_SPEED = 200, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 50, -- Engagement Speed
	
	ACCEL = 15,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 20,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 35, dPitch = 25, dRoll = 10,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical.osgv",

	Dimensions =
	{ Length=2250, Width=600, Height=360}, --These should be "roughly" correct in meters
}