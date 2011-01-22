Asteroid002 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Space\\Asteroids\\Asteroid002.osgv",
	-- OSGV = "MiramarStation\\MiramarStation.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = MedSupportVessel,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	-- Destruction_OSGV = "LOCATE HERE",
	RESPAWN_TIME = 5,
	
	Mass = 1340000000, -- Weight Kg
	RESPAWN_HITPOINTS = 67000,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 25, MaxAccelRight = 25, MaxAccelUp = 25, MaxAccelDown = 25, MaxAccelForward = 40, MaxAccelReverse = 35, 
	MaxTorqueYaw = 1.2, MaxTorquePitch = 1.2, MaxTorqueRoll = 1.2, 

	MAX_SPEED = 100, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 100, -- Engagement Speed
	
	ACCEL = 5,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = .2,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)	
	dHeading = 27, dPitch = 18, dRoll = 36,

	INRANGE_LEAD_RET_OSGV = "Common Objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Objects/ForwardRetical.osgv",	
	
	Dimensions = 
	{ Length=120, Width=120, Height=140}, --These should be "roughly" correct in meters

}