Q_33 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Ships/Colonial-fighter-Q33/q33_a3/q33_a3.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CLASS = "ThrustShip2",
	
	CONTROLLER = "SimpleThrustController",
	BoundingSphereRadius = 5.55,
	Mass = 15000, -- Weight kg
	MaxThrustLeft = 40, MaxThrustRight = 40, MaxThrustUp = 40, MaxThrustDown = 53, MaxThrustForward = 67, MaxThrustReverse = 60, 
	MaxTorqueYaw = 2, MaxTorquePitch = 2, MaxTorqueRoll = 6.7, 
	
	MAX_SPEED = 500, -- Maximum Speed (m/s)
	ACCEL = 60,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 50,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 270, dPitch = 180, dRoll = 360
}