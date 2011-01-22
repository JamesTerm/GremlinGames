Karackuss = {
	-- The OSGV file that represents it visually.  When using '\', you have to have '\\'
	OSGV = "POSER_Frigate_Union_Karackuss\\poser_karackuss.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Frigate,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\Fighterdeath.osgv",
	RESPAWN_TIME = 15,
	
	Mass = 34000000, -- Weight Kg
	RESPAWN_HITPOINTS = 17000,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 5, MaxAccelRight = 5, MaxAccelUp = 5, MaxAccelDown = 5, MaxAccelForward = 20, MaxAccelReverse = 15, 
	MaxTorqueYaw = 0.2, MaxTorquePitch = 0.2, MaxTorqueRoll = 0.2, 

	MAX_SPEED = 300, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 80, -- Engagement Speed
	
	ACCEL = 35,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 20,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 35, dPitch = 25, dRoll = 10,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical.osgv",

	Dimensions =
	{ Length=118, Width=23, Height=23}, --These should be "roughly" correct in meters

}