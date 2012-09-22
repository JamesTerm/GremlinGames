Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787

wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=19.5	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=27.5
WheelTurningDiameter= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Length_In * WheelBase_Length_In) ) ^ 0.5

HighGearSpeed = (427.68 / 60.0) * Pi * wheel_diameter_in * Inches2Meters  --RPM's from Parker
LowGearSpeed  = (167.06 / 60.0) * Pi * wheel_diameter_in * Inches2Meters

TestShip = {

	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = 4, MaxAccelReverse = 4, 
	MaxAccelForward_High = 10, MaxAccelReverse_High = 10, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	--heading_rad = (HighGearSpeed / (Pi * WheelTurningDiameter * Inches2Meters)) * Pi2,
	heading_rad = 19.19,  --TODO figure out why this is not adding up
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (will matter for turning radius!
	
	swerve_drive =
	{
		is_closed=1,
		is_closed_swivel=0,
		
		show_pid_dump_wheel=
		{fl=0, fr=0, rl=0, rr=0},
		show_pid_dump_swivel=
		{fl=0, fr=0, rl=0, rr=0},
		
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=WheelBase_Length_In, width_in=WheelBase_Width_In},	--where length is in 5 inches in, and width is 3 on each side (can only go 390 degrees a second)
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = 6,
		wheel_pid=
		{p=200, i=0, d=50},
		swivel_pid=
		{p=100, i=0, d=50},
		latency=0.0,
		heading_latency=0.50,
		drive_to_scale=0.50,				--For 4 to 10 50% gives a 5 inch tolerance
		strafe_to_scale=4/20,  --In autonomous we need the max to match the max forward and reverse
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		curve_voltage_wheel=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		--curve_voltage_swivel=
		--{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		reverse_steering='no',
		inv_max_force = 1/15.0  --solved empiracally
	},

	UI =
	{
		Length=5, Width=5,
		TextImage="     \n,   ,\n(-+-)\n'   '\n     "
	}
}

RobotSwerve = TestShip
