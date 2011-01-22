Ramora = {
	-- The OSGV file that represents it visually.  When using '\', you have to have '\\'
	OSGV = "Ships\\Federation-Crusier-ramora\\Ramora-01.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CLASS = "ThrustShip2",
	
	CONTROLLER = "SimpleThrustController",
	BoundingSphereRadius = 74,
	Mass = 5000000, -- Weight Kg
	MaxThrustLeft = 1, MaxThrustRight = 1, MaxThrustUp = 1, MaxThrustDown = 1, MaxThrustForward = 7, MaxThrustReverse = 2, 
	MaxTorqueYaw = 1, MaxTorquePitch = 1, MaxTorqueRoll = 1, 

	MAX_SPEED = 30, -- Maximum Speed (m/s)
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 5,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 10, dPitch = 8, dRoll = 2
}