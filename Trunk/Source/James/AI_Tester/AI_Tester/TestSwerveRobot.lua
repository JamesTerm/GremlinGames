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
	dHeading = 390,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (will matter for turning radius!
	
	swerve_drive =
	{
		is_closed=1,
		is_closed_swivel=0,
		
		show_pid_dump_wheel=
		{fl=1, fr=0, rl=0, rr=0},
		show_pid_dump_swivel=
		{fl=0, fr=0, rl=0, rr=0},
		
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=27.5, width_in=19.5},	--where length is in 5 inches in, and width is 3 on each side (can only go 390 degrees a second)
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = 6,
		wheel_pid=
		{p=200, i=0, d=50},
		swivel_pid=
		{p=100, i=0, d=50},
		latency=0.0,
		heading_latency=0.0,
		drive_to_scale=0.50,				--For 4 to 10 50% gives a 5 inch tolerance
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		--curve_voltage_wheel=
		--{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		--curve_voltage_swivel=
		--{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		reverse_steering='no',
	},

	UI =
	{
		Length=5, Width=5,
		TextImage="     \n,   ,\n(-+-)\n'   '\n     "
	}
}

RobotSwerve = TestShip
