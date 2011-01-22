AsteroidCity = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Stations\\AsteroidCity.osgv",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	
	CPP_CLASS = "ThrustShip2",
	SHIP_CLASS = Cruiser,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Objects/Fighterdeath.osgv",
	RESPAWN_TIME = 5,

	-- GeForce Meter
	StructuralDmgGLimit = 10,	-- The limit of the bar, eventually the limit of the ship, default 10
	PilotGLimit = 14,			-- How many G's can the pilot handle before starting to pass out, default 6.5
	PilotTimeToPassOut = 10,	-- How long does it take to pass out at the given limit? default 5
	PilotTimeToRecover = 2,		-- How long does it taks to recover when lower than the limit? default 1
	PilotMaxTimeToRecover = 10,	-- A Maximum amount of time to recover, even when hit with MAJOR G's default 20
	
	
	Mass = 6000000000, -- Weight Kg
	RESPAWN_HITPOINTS = 1600,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	MaxAccelLeft = 0, MaxAccelRight = 0, MaxAccelUp = 0, MaxAccelDown = 0, MaxAccelForward = 0, MaxAccelReverse = 0, 
	MaxTorqueYaw = 0, MaxTorquePitch = 0, MaxTorqueRoll = 0, 

	MAX_SPEED = 1200, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 30, -- Engagement Speed
	
	ACCEL = 0,    -- Thruster Acceleration m/s2 (1g = 9.8)
	BRAKE = 0,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)	
	dHeading = 0, dPitch = 0, dRoll = 0,

	INRANGE_LEAD_RET_OSGV = "Common Objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Objects/ForwardRetical.osgv",	
	
	Dimensions = 
	{ Length=8024, Width=10050, Height=6000}, --These should be "roughly" correct in meters


	Cannons = {
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			Placement_NULL_Name = "Gun01",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Space\Asteroids\Asteroid003.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- Properties of the cannon round
			Round_Speed = 1000,  -- m/s
			Round_Life = 5, -- s
			Round_Accuracy = 100, -- m
			Round_Mass = 1000000, -- kg
			Round_Damage = 10000,
			
			HEAT_UP_TIME = 10,
			COOL_DOWN_TIME = 5.0,
			RESTART_LEVEL = 0.2,
			
			Firing_Rate = .5, -- per second,
			Firing_Offset = 2,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
	},

}