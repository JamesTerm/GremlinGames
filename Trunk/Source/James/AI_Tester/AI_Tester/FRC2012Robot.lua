TestShip = {
	Mass = 25, -- Weight kg
	MaxAccelLeft = 5, MaxAccelRight = 5, MaxAccelForward = 5, MaxAccelReverse = 5, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = 2.916, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 2.916, -- Engagement Speed
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 2, -- Take this to the limit
	BRAKE = 10,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 514,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (This is not used except for UI ignore)
	
	tank_drive =
	{
		wheel_base_dimensions =
		{length_in=27.5, width_in=20.38},	--The wheel base will determine the turn rate, must be as accurate as possible!
		wheel_diameter_in = 6,				--This will determine the correct distance try to make accurate too
		left_pid=
		{p=1, i=0, d=0},					--In FRC 2011 pid was 1,1,0 but lets keep i to zero if we can
		right_pid=
		{p=1, i=0, d=0},					--These should always match, but able to be made different
		
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
	},
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(   )\n|   |\n(   )"
	}
}

Robot2012 = TestShip
