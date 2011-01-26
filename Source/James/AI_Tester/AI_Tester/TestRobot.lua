TestShip = {
	Mass = 5, -- Weight kg
	MaxAccelLeft = 50, MaxAccelRight = 50, MaxAccelUp = 5, MaxAccelDown = 5, MaxAccelForward = 20, MaxAccelReverse = 20, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = 11, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 11, -- Engagement Speed
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 2, -- Take this to the limit
	BRAKE = 10,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 1200,
	
	Dimensions =
	{ Length=1, Width=1 }, --These should be "roughly" correct in meters
	
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

RobotTank = TestShip
