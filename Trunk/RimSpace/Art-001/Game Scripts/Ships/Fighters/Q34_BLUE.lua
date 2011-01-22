Q34_BLUE = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Ships/Fighters/Q34-Blue/Q34.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Fighter,
	
	-- The OSGV file that is used for when the ship blows up
	Destruction_OSGV = "Ships/Fighters/Q34-X/DEATH01.osgv",
	RESPAWN_TIME = 5,

	-- GeForce Meter
	StructuralDmgGLimit = 25,	-- The limit of the bar, eventually the limit of the ship, default 10
	PilotGLimit = 18,			-- How many G's can the pilot handle before starting to pass out, default 6.5
	PilotTimeToPassOut = 14,	-- How long does it take to pass out at the given limit? default 5
	PilotTimeToRecover = 2,		-- How long does it taks to recover when lower than the limit? default 1
	PilotMaxTimeToRecover = 6,	-- A Maximum amount of time to recover, even when hit with MAJOR G's default 20
	
	Mass = 6000, -- Weight Kg
	RESPAWN_HITPOINTS = 16,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	
	-- Turn Rates (deg/sec)	
	MAX_SPEED = 1200, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 250, -- Engagement Speed
	
	-- These are used by the keyboard
	ACCEL = 125,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 100,     -- Brake Deceleration m/s2 (1g = 9.8)
	STRAFE = 50,	-- STRAFE Defaults to Brake
	AFTERBURNER_ACCEL = 400,
	AFTERBURNER_BRAKE = 300,
	
	-- Use these default numbers for RAmping up and Down of the engines
	-- Make the engines super snappy by providing a small number.  0 will work
	RAMP_UP_DUR = 0.5,
	RAMP_DOWN_DUR = 0.5,
	
	-- These are all calculated by default, but you can have more precision here if you want
	-- EngineDeceleration=10000.0,
	-- EngineRampStrafe=10000.0,
	-- EngineRampForward=10000.0,
	-- EngineRampReverse=10000.0,
	-- EngineRampAfterBurner=10000.0,
	
	
	
	-- All of the MaxAccel* values default to the ACCEL, BRAKE, and STRAFE values
	-- I've adjusted the MaxTorqueYaw to be equal of that in the q33 see if you like it -James
	NormalFlight_Characteristics = {
		MaxAccelLeft = 50, MaxAccelRight = 50, MaxAccelUp = 50, MaxAccelDown = 50, MaxAccelForward = 100, MaxAccelReverse = 100, 
		MaxTorqueYaw = 2.5, MaxTorquePitch = 2, MaxTorqueRoll = 5, 
		G_Dampener = 1.0, -- 1.0,
		Camera_Restraint = 1.0, -- 1.0
		},
		
	-- All of the Afterburner_Characteristics defaults to the NormalFlight_Characteristics if not here, except AFTERBURNER_ACCEL and AFTERBURNER_BRAKE
	Afterburner_Characteristics = {
		-- All of the MaxAccel* values default to the AFTERBURNER_ACCEL, AFTERBURNER_BRAKE, and STRAFE values
		MaxAccelLeft = 50, MaxAccelRight = 50, MaxAccelUp = 50, MaxAccelDown = 50, MaxAccelForward = 250, MaxAccelReverse = 250,
		MaxTorqueYaw = 0.1, MaxTorquePitch = 0.1, MaxTorqueRoll = 0.1, 
		G_Dampener = 0.5, -- 1.0,
		Camera_Restraint = 0.1, -- 1.0
		},
		
	-- All of the Afterburner_Characteristics defaults to the NormalFlight_Characteristics if not here
	Braking_Characteristics = {
		-- All of the MaxAccel* values default to the ACCEL, BRAKE, and STRAFE values
		MaxAccelLeft = 50, MaxAccelRight = 50, MaxAccelUp = 50, MaxAccelDown = 50, MaxAccelForward = 100, MaxAccelReverse = 100,
		MaxTorqueYaw = 2, MaxTorquePitch = 4, MaxTorqueRoll = 10, 
		G_Dampener = 1.0, -- 1.0,
		Camera_Restraint = 1.0, -- 1.0
		},
	
	-- These are the rates used by the keyboard, and will be modified by Camera_Restraint
	-- These need to be high enough (especially dHeading) for the mouse match velocity to work!   James
	dHeading = 270, dPitch = 300, dRoll = 550,

	INRANGE_LEAD_RET_OSGV = "Common Objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Objects/ForwardRetical.osgv",	
	
	Dimensions = 
	{ Length=20, Width=15, Height=14}, --These should be "roughly" correct in meters


	Cannons = {
		-- There WILL be four cannons, but lets get this one working
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "Gun01",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = .6, -- s
			Round_Accuracy = 50, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 5, -- per second,
			Firing_Offset = 0,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "Gun02",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = .6, -- s
			Round_Accuracy = 50, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 5, -- per second,
			Firing_Offset = .1,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "Gun03",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = .6, -- s
			Round_Accuracy = 50, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 5, -- per second,
			Firing_Offset = .2,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "Gun04",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = .6, -- s
			Round_Accuracy = 50, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 5, -- per second,
			Firing_Offset = .3,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
	},



}