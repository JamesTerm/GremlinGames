struct Ship_Props;
struct Ship_1D_Props
{
	void SetFromShip_Properties(const Ship_Props & NewValue);

	//Note there may be a difference between MAX_SPEED and MaxSpeed_Forward/MaxSpeed_Reverse, where MAX_SPEED represents the fastest speed something is capable of traveling, while
	// MaxSpeed_Forward/MaxSpeed_Reverse is the fastest desired speed the controller will want to manage this becomes more important in robotics where the gearing has some rotary
	// systems have a much faster max speed than what the desired speed would want to be.  These are also handy for button controlled operations to operate at a certain desired
	// max speed when held down
	double MAX_SPEED;
	double MaxSpeed_Forward,MaxSpeed_Reverse;
	double ACCEL, BRAKE;

	double MaxAccelForward,MaxAccelReverse;
	double MinRange,MaxRange;
	//This is used to avoid overshoot when trying to rotate to a heading
	double DistanceDegradeScalar;

	//TODO these are somewhat specific, we may want to move subclass them or have more generic meaning
	enum Ship_Type
	{
		eDefault,
		eRobotArm,
		eSimpleMotor,
		eSwivel,
	};
	Ship_Type ShipType;
	bool UsingRange;
};


class Ship_1D;
class Ship_1D_Properties : public Entity1D_Properties
{
	private:
		//typedef Entity1D_Properties __super;
		Ship_1D_Props m_Ship_1D_Props;
	public:
		typedef Ship_1D_Props::Ship_Type Ship_Type;

		Ship_1D_Properties();
		//Allow to construct props in constructor instead of using script
		Ship_1D_Properties(const char EntityName[], double Mass,double Dimension,
			double MAX_SPEED,double ACCEL,double BRAKE,double MaxAccelForward, double MaxAccelReverse,	
			Ship_Type ShipType=Ship_1D_Props::eDefault, bool UsingRange=false, double MinRange=0.0, double MaxRange=0.0,
			bool IsAngular=false);

		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		//This is depreciated (may need to review game use-case)
		//void Initialize(Ship_1D *NewShip) const;
		void UpdateShip1DProperties(const Ship_1D_Props &props);  //explicitly allow updating of ship props here
		Ship_Type GetShipType() const {return m_Ship_1D_Props.ShipType;}
		double GetMaxSpeed() const {return m_Ship_1D_Props.MAX_SPEED;}
		const Ship_1D_Props &GetShip_1D_Props() const {return m_Ship_1D_Props;}
	public:
		//These are for testing purposes only (do not use)
		void SetMinRange(double MinRange) {m_Ship_1D_Props.MinRange=MinRange;}
		void SetMaxRange(double MaxRange) {m_Ship_1D_Props.MaxRange=MaxRange;}
		void SetUsingRange(bool UsingRange) {m_Ship_1D_Props.UsingRange=UsingRange;}
		//copy constructor that can interpret the other type
		void SetFromShip_Properties(const Ship_Props & NewValue) {m_Ship_1D_Props.SetFromShip_Properties(NewValue);}
};

///This is really stripped down from the Ship_2D.  Not only is their one dimension (given), but there is also no controller.  This class is intended
///To be controlled directly from a parent class which has controller support.
class Ship_1D : public Entity1D
{
	public:
		Ship_1D(const char EntityName[]);
		void UpdateShip1DProperties(const Ship_1D_Props &props);
		virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual ~Ship_1D();

		///This implicitly will place back in auto mode with a speed of zero
		void Stop(){SetRequestedVelocity(0.0);}
		void SetRequestedVelocity(double Velocity);
		//This will scale the velocity by max speed and also handle flood control
		void SetRequestedVelocity_FromNormalized(double Normalized_Velocity);
		double GetRequestedVelocity() const {return m_RequestedVelocity;}

		/// \param LockShipHeadingToPosition for this given time slice if this is true the intended orientation is restrained
		/// to the ships restraints and the ship is locked to the orientation (Joy/Key mode).  If false (Mouse/AI) the intended orientation
		/// is not restrained and the ship applies its restraints to catch up to the orientation. \note this defaults to true since this is 
		/// most likely never going to be used with a mouse or AI
		void SetCurrentLinearAcceleration(double Acceleration,bool LockShipToPosition=true) 
		{ m_LockShipToPosition=LockShipToPosition,m_currAccel=Acceleration;
		}
		/// This is like setting a way point since there is one dimension there is only one setting to use here
		void SetIntendedPosition(double Position) 
		{ m_LockShipToPosition=false,m_IntendedPosition=Position;
		}

		///This allows subclass to evaluate the requested velocity when it is in use
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s) {}

		// This is where both the vehicle entity and camera need to align to
		virtual const double &GetIntendedPosition() const {return m_IntendedPosition;}
		void SetSimFlightMode(bool SimFlightMode);

		// virtual void ResetPos();
		bool GetAlterTrajectory() const { return m_SimFlightMode;}
		//Note:  This returns the maximum speed possible and not the desired max speed
		double GetMaxSpeed() const		{return m_MaxSpeed;}
		double GetACCEL() const			{return m_Accel;}
		double GetBRAKE() const			{return m_Brake;}

		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

		//The UI controller will call this when attaching or detaching control.  The Bind parameter will either bind or unbind.  Since these are 
		//specific controls to a specific ship there is currently no method to transfer these specifics from one ship to the next.  Ideally there
		//should be no member variables needed to implement the bindings
		virtual void BindAdditionalEventControls(bool Bind) {}
		bool GetLockShipToPosition() const;
		double GetMinRange() const {return m_MinRange;}
		double GetMaxRange() const {return m_MaxRange;}
		bool GetUsingRange() const {return m_UsingRange;}

		Entity1D &AsEntity1D() {return *this;}

	protected:
		///override if the intended position has a known velocity to match (this is great for locking)
		virtual double GetMatchVelocity() const {return 0.0;}
		///This will apply turn pitch and roll to the intended orientation
		void UpdateIntendedPosition(double dTime_s);

		///Client code can use this to evaluate the distance to intervene as necessary
		///For example this could be used to allow more tolerance for position by killing voltage in the tolerance zone
		virtual void PosDisplacementCallback(double posDisplacement_m) {}

		virtual void TimeChange(double dTime_s);

		// Watch for being made the controlled ship
		virtual bool IsPlayerControllable(){return true;}

		friend class Ship_1D_Properties;

		double m_MaxSpeed;
		// Used in Keyboard acceleration and braking
		double m_Accel, m_Brake;

		//Stuff needed for physics
		double m_Mass;
		double m_MaxAccelForward,m_MaxAccelReverse;
		double m_MaxSpeed_Forward,m_MaxSpeed_Reverse;

		//I don't think we would need this for the game, but it is possible, (certainly not for the robot arm)
		//! We can break this up even more if needed
		//double EngineRampForward,EngineRampReverse,EngineRampAfterBurner;
		//double EngineDeceleration,EngineRampStrafe;
	
		//Use this technique when m_AlterTrajectory is true
		double m_RequestedVelocity;
		//All input for turn pitch and roll apply to this, both the camera and ship need to align to it
		double m_IntendedPosition;
		//We need the m_IntendedPosition to work with its own physics
		PhysicsEntity_1D m_IntendedPositionPhysics;

		//For slide mode all strafe is applied here
		double m_currAccel;  //This is the immediate request for thruster levels
		double m_Last_RequestedVelocity;  ///< This monitors the last caught requested velocity  from a speed delta change
		double m_MinRange,m_MaxRange;
		double m_DistanceDegradeScalar;
		bool m_SimFlightMode;  ///< If true auto strafing will occur to keep ship in line with its position
		bool m_UsingRange; 
	private:
		//Only used with SetRequestedVelocity_FromNormalized()
		//this is managed direct from being set to avoid need for precision tolerance
		double m_LastNormalizedVelocity;  
		//typedef Entity1D __super;
		bool m_LockShipToPosition; ///< Locks the ship to intended position (Joystick and Keyboard controls use this)
};


//This is similar to Traverse_Edge in book (not to be confused with its MoveToPosition)
class Goal_Ship1D_MoveToPosition : public AtomicGoal
{
	public:
		Goal_Ship1D_MoveToPosition(Ship_1D &ship,double position,double tolerance=0.10);
		~Goal_Ship1D_MoveToPosition();
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate() {m_Terminate=true;}
	private:
		Ship_1D &m_ship;
		double m_Position,m_Tolerance;
		bool m_Terminate;
};
