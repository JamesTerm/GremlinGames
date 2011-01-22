Crusader_RED = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Ships/Vessels/Crusader/Crusader.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Cruiser,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Objects/Fighterdeath.osgv",
	RESPAWN_TIME = 20,

	-- GeForce Meter
	StructuralDmgGLimit = 25,	-- The limit of the bar, eventually the limit of the ship, default 10
	PilotGLimit = 20,			-- How many G's can the pilot handle before starting to pass out, default 6.5
	PilotTimeToPassOut = 14,	-- How long does it take to pass out at the given limit? default 5
	PilotTimeToRecover = 2,		-- How long does it taks to recover when lower than the limit? default 1
	PilotMaxTimeToRecover = 6,	-- A Maximum amount of time to recover, even when hit with MAJOR G's default 20
	
	Mass = 3187500, -- Weight Kg
	RESPAWN_HITPOINTS = 3187500,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	
	-- Turn Rates (deg/sec)	
	MAX_SPEED = 1200, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 50, -- Engagement Speed
	
	-- These are used by the keyboard
	ACCEL = 25,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 10,     -- Brake Deceleration m/s2 (1g = 9.8)
	STRAFE = 5,	-- STRAFE Defaults to Brake
	AFTERBURNER_ACCEL = 100,
	AFTERBURNER_BRAKE = 100,
	
	-- Use these default numbers for RAmping up and Down of the engines
	-- Make the engines super snappy by providing a small number.  0 will work
	RAMP_UP_DIR = 0.3,
	RAMP_DOWN_DUR = 0.05,
	
	-- These are all calculated by default, but you can have more precision here if you want
	-- EngineDeceleration=10000.0,
	-- EngineRampStrafe=10000.0,
	-- EngineRampForward=10000.0,
	-- EngineRampReverse=10000.0,
	-- EngineRampAfterBurner=10000.0,
	
	
	
	-- All of the MaxAccel* values default to the ACCEL, BRAKE, and STRAFE values
	NormalFlight_Characteristics = {
		-- MaxAccelLeft = 250, MaxAccelRight = 250, MaxAccelUp = 250, MaxAccelDown = 250, MaxAccelForward = 500, MaxAccelReverse = 500, 
		MaxTorqueYaw = 1, MaxTorquePitch = 2, MaxTorqueRoll = 1, 
		G_Dampener = 1.0, -- 1.0,
		Camera_Restraint = 1.0, -- 1.0
		},
		
	-- All of the Afterburner_Characteristics defaults to the NormalFlight_Characteristics if not here, except AFTERBURNER_ACCEL and AFTERBURNER_BRAKE
	Afterburner_Characteristics = {
		-- All of the MaxAccel* values default to the AFTERBURNER_ACCEL, AFTERBURNER_BRAKE, and STRAFE values
		MaxAccelLeft = 25, MaxAccelRight = 25, MaxAccelUp = 25, MaxAccelDown = 25, MaxAccelForward = 750, MaxAccelReverse = 750, 
		MaxTorqueYaw = 0.1, MaxTorquePitch = 0.1, MaxTorqueRoll = 0.1, 
		G_Dampener = 0.5, -- 1.0,
		Camera_Restraint = 0.1, -- 1.0
		},
		
	-- All of the Afterburner_Characteristics defaults to the NormalFlight_Characteristics if not here
	Braking_Characteristics = {
		-- All of the MaxAccel* values default to the ACCEL, BRAKE, and STRAFE values
		-- MaxAccelLeft = 250, MaxAccelRight = 250, MaxAccelUp = 250, MaxAccelDown = 250, MaxAccelForward = 500, MaxAccelReverse = 500, 
		MaxTorqueYaw = 2, MaxTorquePitch = 4, MaxTorqueRoll = 2, 
		G_Dampener = 1.0, -- 1.0,
		Camera_Restraint = 1.0, -- 1.0
		},
	
	-- These are the rates used by the keyboard, and will be modified by Camera_Restraint
	dHeading = 80, dPitch = 300, dRoll = 550,

	INRANGE_LEAD_RET_OSGV = "Common Objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Objects/ForwardRetical.osgv",	
	
	Dimensions = 
	{ Length=611, Width=70, Height=80}, --These should be "roughly" correct in meters


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
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
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
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = .2,		-- amount of time from the time the button is fired to when the first 
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
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = .35,		-- amount of time from the time the button is fired to when the first 
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
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = .45,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
	},



}