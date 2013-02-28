
Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787
Inches2Meters=0.0254
OunceInchToNewton=0.00706155183333
Pounds2Kilograms=0.453592

g_wheel_diameter_in=4   --This will determine the correct distance try to make accurate too
WheelBase_Width_In=27.25	  --The wheel base will determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=9.625
WheelTurningDiameter_In= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Length_In * WheelBase_Length_In) ) ^ 0.5
HighGearSpeed = (733.14 / 60.0) * Pi * g_wheel_diameter_in * Inches2Meters  --RPM's from Parker
ClimbGearSpeed  = (724.284 / 60.0) * Pi * g_wheel_diameter_in * Inches2Meters
Drive_MaxAccel=4
skid=math.cos(math.atan2(WheelBase_Length_In,WheelBase_Width_In))

KeyDistance_in=144
--KeyDistance_in=0
KeyWidth_in=101
KeyDepth_in=48
HalfKeyWidth_in=KeyWidth_in/2.0

MainRobot = {
	--Version helps to identify a positive update to lua
	--version = 1;
	
	Mass = 25, -- Weight kg
	MaxAccelLeft = 20, MaxAccelRight = 20, 
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	MaxAccelForward_High = Drive_MaxAccel, MaxAccelReverse_High = Drive_MaxAccel, 
	MaxTorqueYaw =  (2 * Drive_MaxAccel * Meters2Inches / WheelTurningDiameter_In) * skid,
	rotate_to_scale = 1.0, rotate_to_scale_high = 1.0,
	
	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (2 * HighGearSpeed * Meters2Inches / WheelTurningDiameter_In) * skid,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches (This is not used except for UI ignore)
	--{ Length=0.48895, Width=0.69215 }  --this is what it really is, but looks too small
	
	tank_drive =
	{
		is_closed=1,
		show_pid_dump='no',
		ds_display_row=-1,
		wheel_base_dimensions =
		{length_in=WheelBase_Width_In, width_in=WheelBase_Width_In},	--The length is measure for 4 wheels (so it is half of the wheel base)
		
		--This encoders/PID will only be used in autonomous if we decide to go steal balls
		wheel_diameter_in = g_wheel_diameter_in,
		left_pid=
		{p=200, i=0, d=50},
		right_pid=
		{p=200, i=0, d=50},					--These should always match, but able to be made different
		latency=0.0,
		heading_latency=0.0,
		drive_to_scale=0.50,				--For 4 to 10 50% gives a 5 inch tolerance
		left_max_offset=0.0 , right_max_offset=0.0,   --Ensure both tread top speeds are aligned
		--This is obtainer from encoder RPM's of 1069.2 and Wheel RPM's 427.68 (both high and low have same ratio)
		encoder_to_wheel_ratio=12/36,	--example if encoder spins at 1069.2 multiply by this to get 427.68 (for the wheel rpm)
		voltage_multiply=1.0,				--May be reversed using -1.0
		curve_voltage=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		reverse_steering='no',
		 left_encoder_reversed='no',
		right_encoder_reversed='no',
		inv_max_accel = 1/15.0,  --solved empiracally
		forward_deadzone_left  = 0.02,
		forward_deadzone_right = 0.02,
		reverse_deadzone_left  = 0.02,
		reverse_deadzone_right = 0.02
	},
	
	robot_settings =
	{
		ds_display_row=-1,					--This will display the coordinates and heading (may want to leave on)
		ds_target_vars_row=-1,				--Only used during keying the grid
		ds_power1_velocity_row=-1,			--Only used during keying the grid
		ds_power2_velocity_row=-1,
		fire_trigger_delay=0.100,			--Used to wait for a stable rate before engaging the conveyor
		fire_stay_on_time=0.200,			--Used to prevent ball from get stuck during a fire operation (keep small)
		yaw_tolerance=0.001,				--Used for drive yaw targeting (the drive is the turret) to avoid oscillation
		
		grid_corrections =
		{
			c11={p=1.0, y=1.0}, c12={p=1.0, y=1.0}, c13={p=1.0, y=1.0},
			c21={p=1.0, y=1.0}, c22={p=1.0, y=1.0}, c23={p=1.0, y=1.0},
			c31={p=1.0, y=1.0}, c32={p=1.0, y=1.0}, c33={p=1.0, y=1.0},
			c41={p=1.0, y=1.0}, c42={p=1.0, y=1.0}, c43={p=1.0, y=1.0},
			c51={p=1.0, y=1.0}, c52={p=1.0, y=1.0}, c53={p=1.0, y=1.0},
			c61={p=1.0, y=1.0}, c62={p=1.0, y=1.0}, c63={p=1.0, y=1.0},
		},
	
		climb_1 =
		{
			lift_ft=2,
			drop_ft=-2
		},
		climb_2 =
		{
			lift_ft=5,
			drop_ft=-5
		},
		--Note: if we need a climb 3 it will require a code change

		pitch =
		{
			is_closed='yes',
			show_pid_dump='no',
			ds_display_row=-1,
			pid=
			{p=200, i=0, d=50},
			tolerance=0.001,				--we need high precision
			encoder_to_wheel_ratio=0.5,
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},

			max_speed=1.4 * Pi2,			--(Parker gave this one, should be good)
			accel=10.0,						--We may indeed have a two button solution (match with max accel)
			brake=10.0,
			max_accel_forward=10,			--These are in radians, just go with what feels right
			max_accel_reverse=10,

			min_range_deg=0,				--These should be good to go
			max_range_deg=85,
			inv_max_accel = 1.0/36.0
		},
		power =
		{
			is_closed='yes',
			show_pid_dump='no',
			ds_display_row=-1,				--Use this display to determine max speed (try to get a good match)
			pid=
			{p=200, i=0, d=50 },
			tolerance=10.0,					--we need decent precision (this will depend on ramp up time too)
			encoder_to_wheel_ratio=30/40,     --Just use the gearing ratios here
			voltage_multiply=1.0,
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},

			length_in=4,
			max_speed=(3804.55/60.0) * Pi2,	--This is about 63 rps)
			accel=64 * Pi2 * 5,
			brake=64 * Pi2 * 5,
			max_accel_forward=64 * Pi2 * 5,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=64 * Pi2 * 5,			--The wheel may some time to ramp up
			min_range=28 * Pi2				--We borrow the min range to represent the min speed
		},
		power_first_stage =
		{
			is_closed='yes',
			show_pid_dump='no',
			ds_display_row=-1,				--Use this display to determine max speed (try to get a good match)
			pid=
			{p=200, i=0, d=50 },
			tolerance=10.0,					--we need decent precision (this will depend on ramp up time too)
			encoder_to_wheel_ratio=30/44,     --Just use the gearing ratios here
			voltage_multiply=1.0,
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},

			length_in=4,
			max_speed=(3804.55/60.0) * Pi2,	--(This is clocked at 5000 rpm) in radians
			accel=64 * Pi2 * 5,						--These are only needed if we bind keys for power in meters per second
			brake=64 * Pi2 * 5,
			max_accel_forward=64 * Pi2 * 5,			--These are in radians, plan on increasing these as much as possible
			max_accel_reverse=64 * Pi2 * 5,			--The wheel may some time to ramp up
			min_range=28 * Pi2				--We borrow the min range to represent the min speed
		},
		intake_deployment =
		{
			is_closed=1,
			show_pid_dump='n',
			ds_display_row=-1,
			pid=
			{p=1000, i=0, d=250},
			tolerance=0.01,					--should not matter much
			voltage_multiply=1.0,			--May be reversed
			encoder_to_wheel_ratio=20/50,
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
			
			--max_speed=(19300/64/60) * Pi2,	--This is about 5 rps (a little slower than hiking viking drive)
			max_speed=(969.86/360) * Pi2,	--loaded max speed (see sheet) which is 2.69 rps
			accel=10,						--We may indeed have a two button solution (match with max accel)
			brake=10,
			max_accel_forward=10,			--These are in radians, just go with what feels right
			max_accel_reverse=10,
			using_range=1,					--Warning Only use range if we have a potentiometer!
			min_range_deg=0,				--Stowed position where 0 degrees is vertical up
			min_drop_deg=45,				--The minimum amount of intake drop to occur to be able to fire shots
			max_range_deg= 90,				--Dropped position where 90 degrees is horizontal
			use_aggressive_stop = 'yes',
			inv_max_accel = 1.0/15.0,
			inv_max_decel = 1.0/24.0,
			distance_scale = 0.5,
		},
		helix =
		{
			--ds_display_row=-1,
			--these are for simulation only should be disabled to ensure state is set to no encoder
			--is_closed=0,
			--show_pid_dump='y',
			--Note: there are no encoders here so is_closed is ignored and can not show pid dump
			tolerance=0.01,					--should not matter much
			voltage_multiply=1.0,			--May be reversed
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
			max_speed=(11353.86/360.0) * Pi2,  --loaded max speed (see sheet) about 32 rps
			accel=32 * Pi2 * 5,
			brake=32 * Pi2 * 5,
			max_accel_forward=32 * Pi2 * 5,
			max_accel_reverse=32 * Pi2 * 5
		},

		rollers =
		{
			ds_display_row=-1,
			--these are for simulation only should be disabled to ensure state is set to no encoder
			--is_closed=0,
			--show_pid_dump='y',
			--Note: there are no encoders here so is_closed is ignored and can not show pid dump
			tolerance=0.01,					--we need good precision
			voltage_multiply=1.0,			--May be reversed
			--For open loop, and using limit... the curve will help achieve proper velocity
			curve_voltage=
			{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
			max_speed=(14000.0/60.0) * Pi2,  --233 rps!
			accel=233 * Pi2 * 5,						--These are needed and should be high enough to grip without slip
			brake=233 * Pi2 * 5,
			max_accel_forward=233 * Pi2 * 5,
			max_accel_reverse=233 * Pi2 * 5,
			using_range=0,					--Warning Only use range if we have a potentiometer!
			min_range=-10,				--TODO find out what these are
			max_range= 10
		},

		climb_gear_lift = 
		{
			MAX_SPEED = ClimbGearSpeed,
			heading_rad = 0,  --No turning for climbing mode
			
			tank_drive =
			{
				is_closed=1,						--Must be on (there is no passive mode for tank drive), but start open loop for calibration
				left_pid=
				{p=200, i=0, d=50},
				right_pid=
				{p=200, i=0, d=50},					--These should always match, but able to be made different
				tolerance=0.2,
				drive_to_scale=0.8,
				inv_max_accel = 1/16
				--forward_deadzone_left  = 0,
				--forward_deadzone_right = 0,
				--reverse_deadzone_left  = 0,
				--reverse_deadzone_right = 0
			}
		},
		--This get copy of everything set in climb_gear_lift by default... so everything in common does not need to be duplicated
		climb_gear_drop = 
		{
			tank_drive =
			{
				is_closed=1,						--Must be on
				left_pid=
				{p=200, i=0, d=50},
				right_pid=
				{p=200, i=0, d=50},					--These should always match, but able to be made different
				inv_max_accel = 1
			}
		}
	},

	controls =
	{
		Joystick_1 =
		{
			control = "airflo",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			PowerWheels_SetCurrentVelocity_Axis = {type="joystick_analog", key=5, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--PitchRamp_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.01, curve_intensity=1.0},
			--To use this without surprises during calibration __DisableIntakeAutoPosition__ must be enabled
			Intake_Deployment_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.01, curve_intensity=1.0},
			--Robot_SetClimbGearOff = {type="joystick_button", key=11, on_off=false},
			--Robot_SetClimbGear_RightButton = {type="joystick_button", key=10, on_off=true},
			--Robot_SetClimbGear_LeftButton = {type="joystick_button", key=9, on_off=true},
			Robot_SetClimbGearOff = {type="joystick_button", key=9, on_off=false},
			Robot_SetClimbGearOn = {type="joystick_button", key=10, on_off=false},
			Ball_Fire = {type="joystick_button", key=4, on_off=true},
			PowerWheels_IsRunning = {type="joystick_button", key=2, on_off=true},
			Ball_Grip = {type="joystick_button", key=1, on_off=true},
			Ball_Squirt = {type="joystick_button", key=3, on_off=true},
			Intake_Deployment_Retract = {type="joystick_button", key=13, on_off=false},
			Intake_Deployment_Advance = {type="joystick_button", key=12, on_off=false},
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			Turn_180 = {type="joystick_button", key=7, on_off=false}
		},
		
		Joystick_2 =
		{
			control = "logitech dual action",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			PowerWheels_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=false, multiplier=0.5, filter=0.1, curve_intensity=0.0},
			--PitchRamp_SetCurrentVelocity = {type="joystick_analog", key=5, is_flipped=true, multiplier=1.0000, filter=0.0, curve_intensity=1.0},
			--To use this without surprises during calibration __DisableIntakeAutoPosition__ must be enabled
			Intake_Deployment_SetCurrentVelocity = {type="joystick_analog", key=5, is_flipped=false, multiplier=1.0, filter=0.01, curve_intensity=1.0},
			--Robot_SetClimbGearOff = {type="joystick_button", key=11, on_off=false},
			--Robot_SetClimbGear_RightButton = {type="joystick_button", key=10, on_off=true},
			--Robot_SetClimbGear_LeftButton = {type="joystick_button", key=9, on_off=true},
			Robot_SetClimbGearOff = {type="joystick_button", key=9, on_off=false},
			Robot_SetClimbGearOn = {type="joystick_button", key=10, on_off=false},
			Ball_Fire = {type="joystick_button", key=4, on_off=true},
			PowerWheels_IsRunning = {type="joystick_button", key=3, on_off=true},
			Ball_Grip = {type="joystick_button", key=2, on_off=true},
			Ball_Squirt = {type="joystick_button", key=1, on_off=true},
			Intake_Deployment_Retract = {type="joystick_button", key=12, on_off=false},
			Intake_Deployment_Advance = {type="joystick_button", key=11, on_off=false},
			POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			Turn_180 = {type="joystick_button", key=7, on_off=false}
		},
		
		Joystick_3 =
		{	
			control = "ch throttle quadrant",
			PitchRamp_SetIntendedPosition = {type="joystick_analog", key=0, is_flipped=true, multiplier=1.142000, filter=0.0, curve_intensity=0.0},
			PowerWheels_FirstStage_SetCurrentVelocity = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0000, filter=0.0, curve_intensity=0.0},
			PowerWheels_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=true, multiplier=1.0000, filter=0.0, curve_intensity=0.0},
			
			Ball_Grip = {type="joystick_button", key=2, on_off=true},
			Ball_Squirt = {type="joystick_button", key=1, on_off=true},
			Ball_Fire = {type="joystick_button", key=6, on_off=true},
			PowerWheels_IsRunning = {type="joystick_button", key=4, on_off=true},
		},
		
		Joystick_4 =
		{
			control = "logitech attack 3",
			Analog_Turn = {type="joystick_analog", key=0, is_flipped=false, multiplier=1.0, filter=0.3, curve_intensity=1.0},
			Joystick_SetCurrentSpeed_2 = {type="joystick_analog", key=1, is_flipped=true, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
			PowerWheels_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.1, curve_intensity=0.0},
			--PitchRamp_SetCurrentVelocity = {type="joystick_analog", key=2, is_flipped=false, multiplier=1.0, filter=0.01, curve_intensity=1.0},
			Robot_SetClimbGearOff = {type="joystick_button", key=8, on_off=false},
			Robot_SetClimbGearOn = {type="joystick_button", key=9, on_off=false},
			Ball_Fire = {type="joystick_button", key=2, on_off=true},
			PowerWheels_IsRunning = {type="joystick_button", key=1, on_off=true},
			Ball_Grip = {type="joystick_button", key=6, on_off=true},
			Ball_Squirt = {type="joystick_button", key=7, on_off=true},
			Intake_Deployment_Retract = {type="joystick_button", key=10, on_off=false},
			Intake_Deployment_Advance = {type="joystick_button", key=11, on_off=false},
			--POV_Turn =  {type="joystick_analog", key=8, is_flipped=false, multiplier=1.0, filter=0.0, curve_intensity=0.0},
			--Turn_180 = {type="joystick_button", key=7, on_off=false}
		}
	},
	
	--This is only used in the AI tester, can be ignored
	UI =
	{
		Length=5, Width=5,
		TextImage="(   )\n|   |\n(-+-)\n|   |\n(   )"
	}
}

Robot2013 = MainRobot