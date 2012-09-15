Pi=3.14159265358979323846
Pi2=Pi*2
Inches2Meters=0.0254
Feet2Meters=0.3048
Meters2Feet=3.2808399
Meters2Inches=39.3700787
Pounds2Kilograms=0.453592

wheel_diameter_in=6   --This will determine the correct distance try to make accurate too
--TODO where length is in 5 inches in, and width is 3 on each side
WheelBase_Width_In=19.5	  --These determine the turn rate, must be as accurate as possible!
WheelBase_Length_In=27.5
WheelTurningDiameter= ( (WheelBase_Width_In * WheelBase_Width_In) + (WheelBase_Length_In * WheelBase_Length_In) ) ^ 0.5
HighGearSpeed = (492.83 / 60.0) * Pi * wheel_diameter_in * Inches2Meters  --RPM's from Parker
LowGearSpeed  = (184.81 / 60.0) * Pi * wheel_diameter_in * Inches2Meters
KickerSpeed = (307.13 / 60.0) * Pi * wheel_diameter_in * Inches2Meters  --RPM's from Parker

Mass=Pounds2Kilograms * 148
Gravity=9.80665
--Give ability to tweak both sets
Drive_CoF_Omni_Radial=0.85
Kicker_CoF_Omni_Radial=0.85
Kicker_WeightBias=0.33
Drive_WeightBias=1.0-Kicker_WeightBias
Kicker_MaxAccel=Kicker_CoF_Omni_Radial * Kicker_WeightBias * Mass * Gravity / Mass  --does not change
Drive_MaxAccel=  Drive_CoF_Omni_Radial * Drive_WeightBias  * Mass * Gravity / Mass  --does not change
MaxCentripetalTraverse=20 --The maximum amount of centripetal force that can be allowed (may want to be higher for better drive)

TestShip = {

	Mass = 25, -- Weight kg
	MaxAccelLeft = MaxCentripetalTraverse, MaxAccelRight = MaxCentripetalTraverse, 
	--MaxAccelForward = 4, MaxAccelReverse = 4, 
	--MaxAccelForward_High = 10, MaxAccelReverse_High = 10,
	MaxAccelForward = Drive_MaxAccel, MaxAccelReverse = Drive_MaxAccel, 
	MaxTorqueYaw = 25, 
	
	MAX_SPEED = HighGearSpeed,
	ACCEL = 10,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = ACCEL,
	-- Turn Rates (radians/sec) This is always correct do not change
	heading_rad = (HighGearSpeed / (Pi * WheelTurningDiameter * Inches2Meters)) * Pi2,
	
	Dimensions =
	{ Length=0.9525, Width=0.6477 }, --These are 37.5 x 25.5 inches These are ignored
	
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
		{length_in=WheelBase_Length_In, width_in=WheelBase_Width_In},
		
		wheel_diameter_in = 6,
		wheel_pid=
		{p=200, i=0, d=50},
		swivel_pid=
		{p=100, i=0, d=50},
		latency=0.0,
		heading_latency=0.50,
		drive_to_scale=0.50,				--For 4 to 10 50% gives a 5 inch tolerance
		strafe_to_scale=Kicker_MaxAccel/MaxCentripetalTraverse,  --In autonomous we need the max for to be seta kicker max accel
		--This is obtained from stage 2 in the general gear ratios
		encoder_to_wheel_ratio=1.0,			
		voltage_multiply=1.0,				--May be reversed using -1.0
		curve_voltage_wheel=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		--curve_voltage_swivel=
		--{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},
		reverse_steering='no',
	},

	kicker =
	{
		is_closed=1,
		show_pid_dump='no',
		ds_display_row=-1,				--Use this display to determine max speed (try to get a good match)
		pid=
		{p=200, i=0, d=50 },
		latency=0.0,
		encoder_to_wheel_ratio=0.7,     --Just use the gearing ratios here
		voltage_multiply=1.0,
		curve_voltage=
		{t4=3.1199, t3=-4.4664, t2=2.2378, t1=0.1222, c=0},

		length_in=6,					--6 inch diameter (we shouldn't worry about tweaking this just measure it and be done)
		max_speed=KickerSpeed,			--8.04 feet per second From spread sheet
		accel=10.0,						--These are only needed if we bind keys for power in meters per second
		brake=10.0,
		--These are low because of traction
		max_accel_forward=Kicker_MaxAccel,
		max_accel_reverse=Kicker_MaxAccel
	},

	UI =
	{
		Length=5, Width=5,
		TextImage="     \n,   ,\n(   )\n'   '\n     "
	}
}

RobotNona = TestShip