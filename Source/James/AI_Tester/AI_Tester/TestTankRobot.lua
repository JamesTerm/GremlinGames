Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787

Wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=22.3125	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=27.5
HighGearSpeed = (427.68 / 60.0) * Pi * Wheel_diameter_in * Inches2Meters  --RPM's from Parker
inv_skid=1.0/math.cos(math.atan2(WheelBase_Width_In,WheelBase_Length_In/2))


TestShip = {

	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = 4, MaxAccelReverse = 4, 
	--MaxAccelForward_High = 10, MaxAccelReverse_High = 10, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = HighGearSpeed, -- Maximum Speed (m/s)
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (deg/sec)
	-- Turn rates with the 0.6477 was 514; however with the real wheel base... this can be smaller to 674
	-- However I don't need to go that fast and it cost more to move and turn so I'll trim it back some
	heading_rad = (HighGearSpeed / (Pi * WheelBase_Width_In * Inches2Meters)) * Pi2 * inv_skid,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (will matter for turning radius!
	
	tank_drive =
	{
		is_closed=1,
		show_pid_dump='no',
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=WheelBase_Length_In/2, width_in=WheelBase_Width_In},	--The length is measure for 4 wheels (so it is half of the wheel base)
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = FRC2012_wheel_diameter_in,
		left_pid=
		{p=200, i=0, d=50},
		right_pid=
		{p=200, i=0, d=50},					--These should always match, but able to be made different
		latency=0.0,
		heading_latency=0.6,
		drive_to_scale=0.80,				--For 4 to 10 50% gives a 5 inch tolerance
		strafe_to_scale=4/20,  --In autonomous we need the max to match the max forward and reverse
		left_max_offset=0.0 , right_max_offset=0.0,   --Ensure both tread top speeds are aligned
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=1.0,			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		curve_voltage=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		reverse_steering='no',
		 left_encoder_reversed='no',
		right_encoder_reversed='no'
	},
	
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

RobotTank = TestShip