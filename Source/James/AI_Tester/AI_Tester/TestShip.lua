TestShip = {
	Mass = 14000, -- Weight kg
	MaxAccelLeft = 8, MaxAccelRight = 8, MaxAccelUp = 14, MaxAccelDown = 8, MaxAccelForward = 16, MaxAccelReverse = 14, 
	MaxTorqueYaw = 2.5, 
	
	MAX_SPEED = 200, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 40, -- Engagement Speed
	ACCEL = 6,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 12, -- Take this to the limit
	BRAKE = 5,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 270,
	
	Dimensions =
	{ Length=1, Width=1 }, --These should be "roughly" correct in meters
	
	UI =
	{
		Length=2, Width=7,
		TextImage="/^\\\n-||X||-"
	}
}

Ship = TestShip
