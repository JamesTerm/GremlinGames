Calvin_Humanoid1 = {
	-- The OSGV file that represents it visually, you can use backslashes without doubling
	OSGV = "Calvin_Humanoid1/Humanoid Mech.lwo",
	-- The Class that Controls the Ship, must not have spaces, symbols("_" is ok), and must not start with a number
	CPP_CLASS = "Mech",
	MECH_CLASS = NormalMech,
	
	-- The OSGV file that is used for when the ship blows up, Just a common one until Justin gives it to me
	Destruction_OSGV = "Common Scene objects\\CommonDestruction.osgv",
	RESPAWN_TIME = 5,
	
	-- Make the ship light so it will move with bullets
	Mass = 10000, -- Weight kg
	RESPAWN_HITPOINTS = 5,	-- Hitpoints when respawning, 1 hit is about 1 cannon round
	
	MaxThrustLeft = 10, MaxThrustRight = 10, MaxThrustUp = 20, MaxThrustDown = 10, MaxThrustForward = 30, MaxThrustReverse = 10, 
	MaxTorqueYaw = 3, MaxTorquePitch = 3, MaxTorqueRoll = 5.7, 
	
	MAX_SPEED = 1000, -- Maximum Speed (m/s)
	ENGAGED_MAX_SPEED = 200, -- Engagement Speed
	
	ENGAGED_MAX_SPEED = 200, -- Engagement Speed
	ACCEL = 30,    -- Thruster Acceleration m/s2 (1g = 9.8)
	AFTERBURNER_ACCEL = 70, -- Take this to the limit
	BRAKE = 60,     -- Brake Deceleration m/s2 (1g = 9.8)
	-- Turn Rates (deg/sec)
	dHeading = 270, dPitch = 180, dRoll = 360,
	
	INRANGE_LEAD_RET_OSGV = "Common Scene objects/InRangeLeadRetical.osgv",
	LEAD_RET_OSGV = "Common Scene objects/LeadRetical.osgv",
	FWD_RET_OSGV = "Common Scene objects/ForwardRetical.osgv",
	
	Dimensions =
	{ Length=0.5, Width=0.5, Height=3}, --These should be "roughly" correct in meters

	Cannons = {
		{
			-- We WILL connect the cannons to a NULL in the BB scene
			-- But for now we will just fire forward from the center
			Placement_NULL_Name = "Gun01",
			
			-- The OSGV File representing the round being fired, has its own sound effect
			Round_OSGV_Filename = "Polygon FX/bullet_ORANGE.osgv",
			
			-- The name of the event that triggers the cannons to be fired (While)
			Fire_EventName = "Ship.FireWeapon",
			
			-- The name of the (while) event 
			Round_Speed = 1900,  -- m/s
			Round_Life = 1.0, -- s
			Round_Accuracy = 80, -- m
			Round_Mass = 1.0, -- kg
			Round_Damage = 4,  -- Rounds shot in groups of 4
			
			Firing_Rate = 10, -- per second,
			Firing_Offset = 0,		-- amount of time from the time the button is fired to when the first 
									-- round fires.  This is good for chaining multiple cannons or having them
									-- be offset a little from each other
		},
		
		
	},
}
