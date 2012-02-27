
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787

FRC2012_wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
--Parker claimed 20.38, but I've measured 22 5/16
WheelBase_Width_In=22.3125	  --The wheel base will determine the turn rate, must be as accurate as possible!
HighGearSpeed = (427.68 / 60.0) * Pi * FRC2012_wheel_diameter_in * Inches2Meters  --RPM's from Parker
LowGearSpeed  = (167.06 / 60.0) * Pi * FRC2012_wheel_diameter_in * Inches2Meters

KeyDistance_in=144;
KeyWidth_in=101;
KeyDepth_in=48;
HalfKeyWidth_in=KeyWidth_in/2.0;

TestShip = {
	Mass = 25, -- Weight kg
	MaxAccelLeft = 10, MaxAccelRight = 10, MaxAccelForward = 10, MaxAccelReverse = 10, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (HighGearSpeed / (Pi * WheelBase_Width_In * Inches2Meters)) * Pi2,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (This is not used except for UI ignore)
	
	tank_drive =
	{
		is_closed=0,						--This should always be false for high gear
		show_pid_dump='no',
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=27.5, width_in=WheelBase_Width_In},	--The length is not used but here for completion
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = FRC2012_wheel_diameter_in,
		left_pid=
		{p=1, i=0, d=0},					--In FRC 2011 pid was 1,1,0 but lets keep i to zero if we can
		right_pid=
		{p=1, i=0, d=0},					--These should always match, but able to be made different
		
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=0.4			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
	},
	
	robot_settings =
	{
		key_1 = { x_in=0				, y_in=KeyDistance_in },
		key_2 = { x_in=-HalfKeyWidth_in	, y_in=KeyDistance_in },
		key_3 = { x_in= HalfKeyWidth_in	, y_in=KeyDistance_in },
		ds_display_row=1,					--This will display the coordinates and heading (may want to leave on)
		ds_target_vars_row=-1,				--Only used during keying the grid
		ds_power_velocity_row=-1,			--Only used during keying the grid
		
		grid_corrections =
		{
			c11={p=1.0, x=1.0}, c12={p=1.0, x=1.0}, c13={p=1.0, x=1.0},
			c21={p=1.0, x=1.0}, c22={p=1.0, x=1.0}, c23={p=1.0, x=1.0},
			c31={p=1.0, x=1.0}, c32={p=1.0, x=1.0}, c33={p=1.0, x=1.0},
		},
		
		turret =
		{
			is_closed='yes',				--It is closed loop when feedback has been properly calibrated
			show_pid_dump='no',				--Only turn on if you want to analyze the PID dump (only one at a time, and it must be closed loop)
			ds_display_row=-1,				--Assign to a row (e.g. 0-4) when trying to calibrate the potentiometer
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
			show_pid_dump='no',
			ds_display_row=-1,
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
			show_pid_dump='no',
			ds_display_row=-1,				--Use this display to determine max speed (try to get a good match)
			pid=
			{p=1, i=0, d=0},
			tolerance=0.1,					--we need decent precision (this will depend on ramp up time too)

			length_in=6,					--6 inch diameter (we shouldn't worry about tweaking this just measure it and be done)
			max_speed=(5000.0/60.0) * Pi2,	--(This is clocked at 5000 rpm) in radians
			accel=200.0,						--These are only needed if we bind keys for power in meters per second
			brake=200.0,
			max_accel_forward=200,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=200			--The wheel may some time to ramp up
		},
		flippers =
		{
			is_closed=1,				--Not sure yet about this
			show_pid_dump='no',
			ds_display_row=-1,
			pid=
			{p=1, i=0, d=0},
			tolerance=0.01,					--should not matter much
			
			max_speed=1.4 * Pi2,			--(Parker gave this one, should be good)
			accel=10.0,						--We may indeed have a two button solution (match with max accel)
			brake=10.0,
			max_accel_forward=10,			--These are in radians, just go with what feels right
			max_accel_reverse=10,
			using_range=1,					--Warning Only use range if we have a potentiometer!
			min_range_deg=-90,				--TODO find out what these are
			max_range_deg= 90
		},
		conveyor =
		{
			--Note: there are no encoders here so is_closed is ignored
			tolerance=0.01,					--we need good precision
			
			max_speed=28,
			accel=112,						--These are needed and should be high enough to grip without slip
			brake=112,
			max_accel_forward=112,
			max_accel_reverse=112
		},
		low_gear = 
		{
			--While it is true we have more torque for low gear, we have to be careful that we do not make this too powerful as it could
			--cause slipping if driver "high sticks" to start or stop quickly.
			MaxAccelLeft = 10, MaxAccelRight = 10, MaxAccelForward = 10 * 2, MaxAccelReverse = 10 * 2, 
			MaxTorqueYaw = 25 * 2, 
			
			MAX_SPEED = LowGearSpeed,
			ACCEL = 10*2,    -- Thruster Acceleration m/s2 (1g = 9.8)
			BRAKE = ACCEL, 
			-- Turn Rates (deg/sec) This is always correct do not change
			heading_rad = (LowGearSpeed / (Pi * WheelBase_Width_In * Inches2Meters)) * Pi2,
			
			tank_drive =
			{
				is_closed=1,						--True should help low gear, but disable if there are problems
				show_pid_dump='no',
				ds_display_row=-1,
				--We must NOT use I or D for low gear, we must keep it very responsive
				--We are always going to use the encoders in low gear to help assist to fight quickly changing gravity shifts
				left_pid=
				{p=1, i=0, d=0},
				right_pid=
				{p=1, i=0, d=0},					--These should always match, but able to be made different
				
				--I'm explicitly keeping this here to show that we have the same ratio (it is conceivable that this would not always be true)
				--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
				encoder_to_wheel_ratio=0.4			--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
			}
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