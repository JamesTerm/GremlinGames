Q34 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "TestObjects\\Q34.osgv",
	-- OSGV = "MiramarStation\\MiramarStation.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Fighter,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	-- Destruction_OSGV = "LOCATE HERE",
	RESPAWN_TIME = 5,
	
	Mass = 7000, -- Weight Kg
	RESPAWN_HITPOINTS = 670,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 200, MaxAccelRight = 500, MaxAccelUp = 500, MaxAccelDown = 500, MaxAccelForward = 200, MaxAccelReverse = 200, 
	MaxTorqueYaw = 2, MaxTorquePitch = 2, MaxTorqueRoll = 2, 

	MAX_SPEED = 600, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 350, -- Engagement Speed
	
	ACCEL = 120,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 120,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)	
	dHeading = 300, dPitch = 300, dRoll = 360,

	INRANGE_LEAD_RET_OSGV = "Common Objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Objects/ForwardRetical.osgv",	
	
	Dimensions = 
	{ Length=20, Width=15, Height=14}, --These should be "roughly" correct in meters

}