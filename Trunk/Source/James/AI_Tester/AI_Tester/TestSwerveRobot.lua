TestShip = {
	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, MaxAccelForward = 5, MaxAccelReverse = 5, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = 2.916, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 2.916, -- Engagement Speed
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 2, -- Take this to the limit
	BRAKE = 10,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 514,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (will matter for turning radius!
	
	UI =
	{
		Length=5, Width=5,
		TextImage="     \n,   ,\n(-+-)\n'   '\n     "
	}
}

RobotSwerve = TestShip
