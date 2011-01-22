Q_33 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Fighter_Colonial_Q33/q33_a3.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Fighter,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\Fighterdeath.osgv",
	RESPAWN_TIME = 5,
	
	Mass = 14000, -- Weight kg
	RESPAWN_HITPOINTS = 7,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 40, MaxAccelRight = 40, MaxAccelUp = 70, MaxAccelDown = 40, MaxAccelForward = 87, MaxAccelReverse = 70, 
	MaxTorqueYaw = 2.5, MaxTorquePitch = 3, MaxTorqueRoll = 5.7, 
	
	StructuralDmgGLimit = 10,	-- The limit of the bar, eventually the limit of the ship, default 10
	PilotGLimit = 8,			-- How many G's can the pilot handle before starting to pass out, default 6.5
	PilotTimeToPassOut = 10,	-- How long does it take to pass out at the given limit? default 5
	PilotTimeToRecover = 1,		-- How long does it taks to recover when lower than the limit? default 1
	PilotMaxTimeToRecover = 10,	-- A Maximum amount of time to recover, even when hit with MAJOR G's default 20
	
	MAX_SPEED = 2000, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 400, -- Engagement Speed
	ACCEL = 60,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 107, -- Take this to the limit
	BRAKE = 50,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 270, dPitch = 180, dRoll = 360,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical_Q-33.osgv",
	
	Dimensions =
	{ Length=11, Width=5, Height=5}, --These should be "roughly" correct in meters
	
	Cannons = {
		-- There WILL be four cannons, but lets get this one working
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "gun_01",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
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
			Placement_NULL_Name = "gun_02",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = 0.7,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "gun_03",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = 0.15,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "gun_04",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = 0.22,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
	},
}

Stripped_Q_33 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Stripped_Fighter_Colonial_Q33/q33_a3.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Fighter,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\Fighterdeath.osgv",
	RESPAWN_TIME = 5,
	
	Mass = 14000, -- Weight kg
	RESPAWN_HITPOINTS = 7,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 40, MaxAccelRight = 40, MaxAccelUp = 70, MaxAccelDown = 40, MaxAccelForward = 87, MaxAccelReverse = 70, 
	MaxTorqueYaw = 2.5, MaxTorquePitch = 3, MaxTorqueRoll = 5.7, 
	
	MAX_SPEED = 2000, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 400, -- Engagement Speed
	ACCEL = 60,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 107, -- Take this to the limit
	BRAKE = 50,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 270, dPitch = 180, dRoll = 360,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical_Q-33.osgv",
	
	Dimensions =
	{ Length=11, Width=5, Height=5}, --These should be "roughly" correct in meters
	
	Cannons = {
		-- There WILL be four cannons, but lets get this one working
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "gun_01",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
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
			Placement_NULL_Name = "gun_02",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = 0.7,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "gun_03",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = 0.15,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "gun_04",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.6, -- kg
			Round_Damage = 1,
			
			HEAT_UP_TIME = 3.2,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = 9, -- per second,
			Firing_Offset = 0.22,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
	},
}

