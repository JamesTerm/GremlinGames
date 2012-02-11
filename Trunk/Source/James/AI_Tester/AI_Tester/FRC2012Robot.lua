
Pi=3.14159265358979323846
Pi2=Pi*2

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
	
	robot_settings =
	{
		turret =
		{
			is_closed='yes',
			pid=
			{p=1, i=0, d=0},
			tolerance=0.001,				--we need high precision
			
			max_speed=10,
			accel=1.0,						--These are only needed if we bind keys for turret
			brake=1.0,
			max_accel_forward=10,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=10,
			min_range_deg=-180,				--These are probably good to go, but may need to be smaller
			max_range_deg= 180
		},
		pitch =
		{
			is_closed='yes',
			pid=
			{p=1, i=0, d=0},
			tolerance=0.001,				--we need high precision

			max_speed=10,
			max_accel_forward=10,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=10,
			min_range_deg=45-3,				--These should be good to go
			max_range_deg=70+3
		},
		power =
		{
			is_closed='yes',
			pid=
			{p=1, i=0, d=0},
			tolerance=0.1,					--we need decent precision (this will depend on ramp up time too)

			length_in=6,					--6 inch diameter (we shouldn't worry about tweaking this just measure it and be done)
			max_speed=(5000.0/60.0) * Pi2,	--(This is clocked at 5000 rpm) in radians
			accel=60.0,						--These are only needed if we bind keys for power in meters per second
			brake=60.0,
			max_accel_forward=60,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=60			--The wheel may some time to ramp up
		},
		conveyor =
		{
			--Note: there are no encoders here so is_closed is ignored
			pid=
			{p=1, i=0, d=0},
			tolerance=0.01,					--we need good precision
			
			max_speed=28,
			accel=112,						--These are needed and should be high enough to grip without slip
			brake=112,
			max_accel_forward=112,
			max_accel_reverse=112
		}
	},
	--This is only used in the AI tester, can be ignored
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(   )\n|   |\n(   )"
	}
}

Robot2012 = TestShip
