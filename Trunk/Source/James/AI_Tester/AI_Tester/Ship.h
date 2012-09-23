#pragma once

class UI_Controller;
class AI_Base_Controller;

inline osg::Vec2d GlobalToLocal(double Heading,const osg::Vec2d &GlobalVector);
inline osg::Vec2d LocalToGlobal(double Heading,const osg::Vec2d &LocalVector);
inline void NormalizeRotation(double &Rotation)
{
	const double Pi2=M_PI*2.0;
	//Normalize the rotation
	if (Rotation>M_PI)
		Rotation-=Pi2;
	else if (Rotation<-M_PI)
		Rotation+=Pi2;
}

inline double NormalizeRotation2(double Rotation)
{
	const double Pi2=M_PI*2.0;
	//Normalize the rotation
	if (Rotation>M_PI)
		Rotation-=Pi2;
	else if (Rotation<-M_PI)
		Rotation+=Pi2;
	return Rotation;
}

inline double SaturateRotation(double Rotation)
{
	const double Pi2=M_PI*2.0;
	//Normalize the rotation
	if (Rotation>M_PI)
		Rotation=M_PI;
	else if (Rotation<-M_PI)
		Rotation=-M_PI;
	return Rotation;
}

struct Ship_Props
{
	//typedef Entity_Properties __super;
	// This is the rate used by the keyboard
	double dHeading;

	//May need these later to simulate pilot error in the AI
	//! G-Force limits
	//double StructuralDmgGLimit, PilotGLimit, PilotTimeToPassOut, PilotTimeToRecover, PilotMaxTimeToRecover;

	//! We can break this up even more if needed
	double EngineRampForward,EngineRampReverse,EngineRampAfterBurner;
	double EngineDeceleration,EngineRampStrafe;

	//! Engaged max speed is basically the fastest speed prior to using after-burner.  For AI and auto pilot it is the trigger speed to
	//! enable the afterburner
	double MAX_SPEED,ENGAGED_MAX_SPEED;
	double ACCEL, BRAKE, STRAFE, AFTERBURNER_ACCEL, AFTERBURNER_BRAKE;

	double MaxAccelLeft,MaxAccelRight,MaxAccelForward,MaxAccelReverse;
	double MaxAccelForward_High,MaxAccelReverse_High;
	double MaxTorqueYaw;
	enum Ship_Type
	{
		eDefault,
		eRobotTank,
		eSwerve_Robot,
		eButterfly_Robot,
		eNona_Robot,
		eFRC2011_Robot,
		eFRC2012_Robot,
	};
	Ship_Type ShipType;

	double GetMaxAccelForward(double Velocity) const;
	double GetMaxAccelReverse(double Velocity) const;
};

class Ship_2D : public Ship
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Ship_2D(const char EntityName[]);
		virtual void Initialize(Entity2D::EventMap& em,const Entity_Properties *props=NULL);
		virtual ~Ship_2D();

		///This implicitly will place back in auto mode with a speed of zero
		void Stop(){SetRequestedVelocity(0.0);}
		void SetRequestedVelocity(double Velocity);
		void SetRequestedVelocity(Vec2D Velocity);
		double GetRequestedVelocity(){return m_RequestedVelocity[1];}
		void FireAfterburner() {SetRequestedVelocity(GetMaxSpeed());}
		void SetCurrentLinearAcceleration(const Vec2D &Acceleration) {m_currAccel=Acceleration;}

		/// \param LockShipHeadingToOrientation for this given time slice if this is true the intended orientation is restrained
		/// to the ships restraints and the ship is locked to the orientation (Joy/Key mode).  If false (Mouse/AI) the intended orientation
		/// is not restrained and the ship applies its restraints to catch up to the orientation
		void SetCurrentAngularAcceleration(double Acceleration,bool LockShipHeadingToOrientation) 
		{	m_LockShipHeadingToOrientation=LockShipHeadingToOrientation,m_rotAccel_rad_s=Acceleration;
		}

		/// This is where both the vehicle entity and camera need to align to
		virtual const double &GetIntendedOrientation() const {return m_IntendedOrientation;}

		// virtual void ResetPos();
		void SetStabilizeRotation(bool StabilizeRotation) { m_StabilizeRotation=StabilizeRotation;	}
		void SetSimFlightMode(bool SimFlightMode);
		void SetEnableAutoLevel(bool EnableAutoLevel);
		bool GetStabilizeRotation() const { return m_StabilizeRotation;}
		bool GetAlterTrajectory() const { return m_SimFlightMode;}
		bool GetCoordinateTurns() const { return m_CoordinateTurns;}

		void SetHeadingSpeedScale(double scale) {m_HeadingSpeedScale=scale;}

		enum eThrustState { TS_AfterBurner_Brake=0, TS_Brake, TS_Coast, TS_Thrust, TS_AfterBurner, TS_NotVisible };
		eThrustState GetThrustState(){ return m_thrustState; }

		/// We use a toggling mechanism to use afterburners since there is some internal functionality that behave differently
		bool GetIsAfterBurnerOn() const { return (m_thrustState==TS_AfterBurner);	}
		bool GetIsAfterBurnerBrakeOn() const { return (m_thrustState==TS_AfterBurner_Brake);	}

		double GetMaxSpeed() const		    {return MAX_SPEED;}
		double GetEngaged_Max_Speed() const {return ENGAGED_MAX_SPEED;}
		double GetStrafeSpeed() const		{return STRAFE;}
		double GetAccelSpeed() const		{return ACCEL;}
		double GetBrakeSpeed() const		{return BRAKE;}
		double GetCameraRestraintScaler() const		{return Camera_Restraint;}
		double GetHeadingSpeed() const		{ return dHeading;}

		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

		// Turn off all thruster controls
		virtual void CancelAllControls();

		AI_Base_Controller *GetController() {return m_controller;}

		//The UI controller will call this when attaching or detaching control.  The Bind parameter will either bind or unbind.  Since these are 
		//specific controls to a specific ship there is currently no method to transfer these specifics from one ship to the next.  Ideally there
		//should be no member variables needed to implement the bindings
		virtual void BindAdditionalEventControls(bool Bind) {}
		//Its possible that each ship may have its own specific controls
		virtual void BindAdditionalUIControls(bool Bind, void *joy) {}
		//callback from UI_Controller for custom controls override if ship has specific controls... all outputs to be written are optional
		//so derived classes can only write to things of interest
		virtual void UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,double dTime_s) {}
		//Override to get sensor/encoder's real velocity
		virtual Vec2D GetLinearVelocity_ToDisplay() {return GlobalToLocal(GetAtt_r(),GetPhysics().GetLinearVelocity());}
		virtual double GetAngularVelocity_ToDisplay() {return GetPhysics().GetAngularVelocity();}
	protected:
		///This presents a downward force vector in MPS which simulates the pull of gravity.  This simple test case would be to work with the global
		///coordinates, but we can also present this in a form which does not have global orientation.
		//virtual Vec2D GetArtificialHorizonComponent() const;

		///This will apply turn pitch and roll to the intended orientation
		void UpdateIntendedOrientaton(double dTime_s);

		virtual void ApplyTorqueThrusters(PhysicsEntity_2D &PhysicsToUse,double Torque,double TorqueRestraint,double dTime_s);
		///Putting force and torque together will make it possible to translate this into actual force with position
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual void TestPosAtt_Delta(const Vec2D pos_m, double att,double dTime_s);

		virtual void TimeChange(double dTime_s);

		// Watch for being made the controlled ship
		virtual bool IsPlayerControllable(){return true;}

		// Called by the OnCollision that finishes the ship off
		//virtual void DestroyEntity(bool shotDown, osg::Vec3d collisionPt);

		eThrustState SetThrustState(eThrustState ts); // Handles the ON/OFF events, only for controlled entities

		//Override with the controller to be used with ship.  Specific ships have specific type of controllers.
		virtual AI_Base_Controller *Create_Controller();

		friend class AI_Base_Controller;
		friend class Ship_Properties;

		///This is to only be used by AI controller (this will have LockShipHeadingToOrientation set to false)
		void SetIntendedOrientation(double IntendedOrientation);

		///This allows subclass to evaluate the requested velocity when it is in use
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s) {}
		//override to manipulate a distance force degrade, which is used to compensate for deceleration inertia
		virtual Vec2D Get_DriveTo_ForceDegradeScalar() const {return Vec2D(1.0,1.0);}

		AI_Base_Controller* m_controller;
		Ship_Props m_ShipProps;
		double MAX_SPEED,ENGAGED_MAX_SPEED;

		// Used in Keyboard acceleration and braking
		double ACCEL, BRAKE, STRAFE, AFTERBURNER_ACCEL, AFTERBURNER_BRAKE;
		double dHeading;

		double m_RadialArmDefault; //cache the radius of concentrated mass square, which will allow us to apply torque in a r = 1 case

		//Stuff needed for physics
		double Mass;
		double MaxAccelLeft,MaxAccelRight;//,MaxAccelForward,MaxAccelReverse;
		double MaxTorqueYaw;
		double Camera_Restraint;
		double G_Dampener;

		//! We can break this up even more if needed
		double EngineRampForward,EngineRampReverse,EngineRampAfterBurner;
		double EngineDeceleration,EngineRampStrafe;
	
		//Use this technique when m_AlterTrajectory is true
		Vec2D m_RequestedVelocity;
		double m_AutoLevelDelay; ///< The potential gimbal lock, and user rolling will trigger a delay for the autolevel (when enabled)
		double m_HeadingSpeedScale; //used by auto pilot control to have slower turn speeds for way points
		double m_rotAccel_rad_s;

		//All input for turn pitch and roll apply to this, both the camera and ship need to align to it
		double m_IntendedOrientation;
		//We need the m_IntendedOrientation quarterion to work with its own physics
		FlightDynamics_2D m_IntendedOrientationPhysics;

		//For slide mode all strafe is applied here
		Vec2D m_currAccel;  //This is the immediate request for thruster levels

		///Typically this contains the distance of the intended direction from the actual direction.  This is the only variable responsible for
		///changing the ship's orientation
		double m_rotDisplacement_rad;

		bool m_SimFlightMode;  ///< If true auto strafing will occur to keep ship in line with its position
		bool m_StabilizeRotation;  ///< If true (should always be true) this will fire reverse thrusters to stabilize rotation when idle
		bool m_CoordinateTurns;   ///< Most of the time this is true, but in some cases (e.g. Joystick w/rudder pedals) it may be false

		Threshold_Averager<eThrustState,5> m_thrustState_Average;
		eThrustState m_thrustState;
		//double m_Last_AccDel;  ///< This monitors a previous AccDec session to determine when to reset the speed
		Vec2D m_Last_RequestedVelocity;  ///< This monitors the last caught requested velocity from a speed delta change

		// When notifying everything about thrusters, we want to keep a bit of an averager
		Averager<osg::Vec3d, 5> m_ThrustReported_Averager;
		Blend_Averager<osg::Vec3d> m_TorqueReported_Averager;
	private:
		//typedef Entity2D __super;
		bool m_LockShipHeadingToOrientation; ///< Locks the ship and intended orientation (Joystick and Keyboard controls use this)

};

class Physics_Tester : public Ship
{
	public:
		Physics_Tester(const char EntityName[]) : Ship(EntityName) {}
};

class Ship_Tester : public Ship_2D
{
	public:
		Ship_Tester(const char EntityName[]) : Ship_2D(EntityName) {}
		~Ship_Tester();
		void SetPosition(double x,double y);
		virtual void SetAttitude(double radians);
		Goal *ClearGoal();
		void SetGoal(Goal *goal);
};

class Ship_Properties : public Entity_Properties
{
	public:
		Ship_Properties();
		virtual ~Ship_Properties() {}
		const char *SetUpGlobalTable(GG_Framework::Logic::Scripting::Script& script);
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		void Initialize(Ship_2D *NewShip) const;
		Ship_Props::Ship_Type GetShipType() const {return m_ShipProps.ShipType;}
		double GetEngagedMaxSpeed() const {return m_ShipProps.ENGAGED_MAX_SPEED;}
		//These methods are really more for the simulation... so using the high yields a better reading for testing
		double GetMaxAccelForward() const {return m_ShipProps.MaxAccelForward_High;}
		double GetMaxAccelReverse() const {return m_ShipProps.MaxAccelReverse_High;}

		const Ship_Props &GetShipProps() const {return m_ShipProps;}
	private:
		Ship_Props m_ShipProps;
};


class UI_Ship_Properties : public Ship_Properties
{
	public:
		UI_Ship_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		void Initialize(const char **TextImage,osg::Vec2d &Dimension) const;
	private:
		std::string m_TextImage;
		osg::Vec2d m_UI_Dimensions;
};