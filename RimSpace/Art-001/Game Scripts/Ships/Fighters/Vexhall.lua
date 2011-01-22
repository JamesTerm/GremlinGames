Vexhall = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Ships/Fighters/Vexhall/vexhall.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Fighter,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Objects/Fighterdeath.osgv",
	RESPAWN_TIME = 5,

	-- GeForce Meter
	StructuralDmgGLimit = 10,	-- The limit of the bar, eventually the limit of the ship, default 10
	PilotGLimit = 10,			-- How many G's can the pilot handle before starting to pass out, default 6.5
	PilotTimeToPassOut = 10,	-- How long does it take to pass out at the given limit? default 5
	PilotTimeToRecover = 2,		-- How long does it taks to recover when lower than the limit? default 1
	PilotMaxTimeToRecover = 10,	-- A Maximum amount of time to recover, even when hit with MAJOR G's default 20
	
	
	Mass = 18500, -- Weight Kg
	RESPAWN_HITPOINTS = 42,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 250, MaxAccelRight = 250, MaxAccelUp = 250, MaxAccelDown = 250, MaxAccelForward = 500, MaxAccelReverse = 450, 
	MaxTorqueYaw = 1, MaxTorquePitch = 2, MaxTorqueRoll = 1, 

	MAX_SPEED = 900, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 150, -- Engagement Speed
	
	ACCEL = 200,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 180,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)	
	dHeading = 50, dPitch = 200, dRoll = 250,

	INRANGE_LEAD_RET_OSGV = "Common Objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Objects/ForwardRetical.osgv",	
	
	Dimensions = 
	{ Length=40, Width=30, Height=28}, --These should be "roughly" correct in meters


	Cannons = {
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
			Placement_NULL_Name = "MedGun02",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_BLUE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 2000,  -- m/s
			Round_Life = .6, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 2, -- kg
			Round_Damage = 35,
			
			HEAT_UP_TIME = 5,
			COOL_DOWN_TIME = 10.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 1, -- per second,
			Firing_Offset = 0,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "MedGun01",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_BLUE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 2000,  -- m/s
			Round_Life = .6, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 2, -- kg
			Round_Damage = 35,
			
			HEAT_UP_TIME = 5,
			COOL_DOWN_TIME = 10.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 1, -- per second,
			Firing_Offset = .5,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		

	},



}