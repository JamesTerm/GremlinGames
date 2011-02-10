
///This is really stripped down from the Ship_2D.  Not only is their one dimension (given), but there is also no controller.  This class is intended
///To be controlled directly from a parent class which has controller support.
class Ship_1D : public Entity1D
{
	public:
		Ship_1D(const char EntityName[]);
		virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
		virtual ~Ship_1D();

		///This implicitly will place back in auto mode with a speed of zero
		void Stop(){SetRequestedVelocity(0.0);}
		void SetRequestedVelocity(double Velocity);
		double GetRequestedVelocity(){return m_RequestedVelocity;}
		void SetCurrentLinearAcceleration(double Acceleration) {m_currAccel=Acceleration;}

		// This is where both the vehicle entity and camera need to align to
		virtual const double &GetIntendedOrientation() const {return m_IntendedPosition;}
		void SetSimFlightMode(bool SimFlightMode);

		// virtual void ResetPos();
		bool GetAlterTrajectory() const { return m_SimFlightMode;}
		double GetMaxSpeed() const		    {return MAX_SPEED;}

		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

	protected:
		///This will apply turn pitch and roll to the intended orientation
		void UpdateIntendedPosition(double dTime_s);

		virtual void TimeChange(double dTime_s);

		// Watch for being made the controlled ship
		virtual bool IsPlayerControllable(){return true;}

		friend Ship_1D_Properties;

		double MAX_SPEED;

		// Used in Keyboard acceleration and braking
		double ACCEL, BRAKE;

		//Stuff needed for physics
		double Mass;
		double MaxAccelForward,MaxAccelReverse;
		double Camera_Restraint;

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

		bool m_SimFlightMode;  ///< If true auto strafing will occur to keep ship in line with its position
		bool m_LockShipToPosition; ///< Locks the ship to intended position (Joystick and Keyboard controls use this)

		double m_Last_RequestedVelocity;  ///< This monitors the last caught requested velocity  from a speed delta change
};
