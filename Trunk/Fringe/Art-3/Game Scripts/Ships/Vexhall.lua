Vexhall = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Fighter_Union_Vexhall/vexhall.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Fighter,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\Fighterdeath.osgv",
	RESPAWN_TIME = 5,
	
	Mass = 52000, -- Weight kg
	RESPAWN_HITPOINTS = 52,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 28, MaxAccelRight = 28, MaxAccelUp = 30, MaxAccelDown = 60, MaxAccelForward = 51, MaxAccelReverse = 49, 
	MaxTorqueYaw = 1.5, MaxTorquePitch = 1.5, MaxTorqueRoll = 3.0, 
	
	MAX_SPEED = 1500, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 375, -- Engagement Speed
	
	ACCEL = 45,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 40,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 190, dPitch = 125, dRoll = 230,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical_Vexhall.osgv",
	
	Dimensions =
	{ Length=19, Width=14.5, Height=5}, --These should be "roughly" correct in meters

	Cannons = {
		-- There WILL be one gatling gun and two cannons, but lets get this one working
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "Fire",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_BLUE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.35, -- kg
			Round_Damage = 1,
			
			Firing_Rate = 12, -- per second,
			Firing_Offset = 0,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
									
			HEAT_UP_TIME = 4.0,
			COOL_DOWN_TIME = 8.0,
			RESTART_LEVEL = 0.4,
		}
	}
}

Stripped_Vexhall = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Stripped_Fighter_Union_Vexhall/vexhall.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Fighter,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\Fighterdeath.osgv",
	RESPAWN_TIME = 5,
	
	Mass = 52000, -- Weight kg
	RESPAWN_HITPOINTS = 52,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 28, MaxAccelRight = 28, MaxAccelUp = 30, MaxAccelDown = 60, MaxAccelForward = 51, MaxAccelReverse = 49, 
	MaxTorqueYaw = 1.5, MaxTorquePitch = 1.5, MaxTorqueRoll = 3.0, 
	
	MAX_SPEED = 1500, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 375, -- Engagement Speed
	
	ACCEL = 45,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 40,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 190, dPitch = 125, dRoll = 230,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical_Vexhall.osgv",
	
	Dimensions =
	{ Length=19, Width=14.5, Height=5}, --These should be "roughly" correct in meters

	Cannons = {
		-- There WILL be one gatling gun and two cannons, but lets get this one working
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "Fire",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_BLUE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 3000,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 0.35, -- kg
			Round_Damage = 1,
			
			Firing_Rate = 12, -- per second,
			Firing_Offset = 0,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
									
			HEAT_UP_TIME = 4.0,
			COOL_DOWN_TIME = 8.0,
			RESTART_LEVEL = 0.4,
		}
	}
}

