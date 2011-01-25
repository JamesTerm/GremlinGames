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

class Ship_2D : public Ship
{
	public:
		Ship_2D(const char EntityName[]);
		virtual void Initialize(Entity2D::EventMap& em,const Entity_Properties *props=NULL);
		virtual ~Ship_2D();

		///This implicitly will place back in auto mode with a speed of zero
		void Stop(){SetRequestedSpeed(0.0);}
		void SetRequestedSpeed(double Speed);
		double GetRequestedSpeed(){return m_RequestedSpeed;}
		void FireAfterburner() {SetRequestedSpeed(GetMaxSpeed());}
		void SetCurrentLinearAcceleration(const osg::Vec2d &Acceleration) {m_currAccel=Acceleration;}
		void SetCurrentAngularVelocity(double Velocity) {m_rotVel_rad_s=Velocity;}

		// This is where both the vehicle entity and camera need to align to
		virtual const double &GetIntendedOrientation() const {return m_IntendedOrientation;}

		// virtual void ResetPos();
		void SetStabilizeRotation(bool StabilizeRotation) { m_StabilizeRotation=StabilizeRotation;	}
		void SetSimFlightMode(bool SimFlightMode);
		void SetEnableAutoLevel(bool EnableAutoLevel);
		bool GetStabilizeRotation() const { return m_StabilizeRotation;}
		bool GetAlterTrajectory() const { return m_SimFlightMode;}
		bool GetCoordinateTurns() const { return m_CoordinateTurns;}

		void SetHeadingSpeedScale(double scale) {m_HeadingSpeedScale=scale;}
		
		/// We use a toggling mechanism to use afterburners since there is some internal functionality that behave differently
		bool GetIsAfterBurnerOn() const { return (m_thrustState==TS_AfterBurner);	}
		bool GetIsAfterBurnerBrakeOn() const { return (m_thrustState==TS_AfterBurner_Brake);	}

		double GetMaxSpeed() const		    {return MAX_SPEED;}
		double GetEngaged_Max_Speed() const {return ENGAGED_MAX_SPEED;}

		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

		// Turn off all thruster controls
		virtual void CancelAllControls();

		AI_Base_Controller* GetController() {return m_controller;}


		enum eThrustState { TS_AfterBurner_Brake=0, TS_Brake, TS_Coast, TS_Thrust, TS_AfterBurner, TS_NotVisible };
		eThrustState GetThrustState(){ return m_thrustState; }

		// This function fires the various thruster events and updates the ThrsutState
		// Called from my own timer update when locally controlled, or from my RC Controller when remote controlled
		//virtual void UpdateThrustState(const osg::Vec3d& localThrust, const osg::Vec3d& localTorque);

	protected:
		///This presents a downward force vector in MPS which simulates the pull of gravity.  This simple test case would be to work with the global
		///coordinates, but we can also present this in a form which does not have global orientation.
		//virtual osg::Vec2d GetArtificialHorizonComponent() const;

		///This will apply turn pitch and roll to the intended orientation
		void UpdateIntendedOrientaton(double dTime_s);

		virtual void ApplyTorqueThrusters(PhysicsEntity_2D &PhysicsToUse,double Torque,double TorqueRestraint,double dTime_s);
		///Putting force and torque together will make it possible to translate this into actual force with position
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual void TestPosAtt_Delta(const osg::Vec2d pos_m, double att,double dTime_s);

		virtual void TimeChange(double dTime_s);

		// Watch for being made the controlled ship
		virtual bool IsPlayerControllable(){return true;}

		// Called by the OnCollision that finishes the ship off
		//virtual void DestroyEntity(bool shotDown, osg::Vec3d collisionPt);

		eThrustState SetThrustState(eThrustState ts); // Handles the ON/OFF events, only for controlled entities

	private:
		friend AI_Base_Controller;
		friend UI_Controller;
		friend Ship_Properties;

		AI_Base_Controller* m_controller;
		double MAX_SPEED,ENGAGED_MAX_SPEED;

		// Used in Keyboard acceleration and braking
		double ACCEL, BRAKE, STRAFE, AFTERBURNER_ACCEL, AFTERBURNER_BRAKE;
		double dHeading;

		double m_RadialArmDefault; //cache the radius of concentrated mass square, which will allow us to apply torque in a r = 1 case

		//Stuff needed for physics
		double Mass;
		double MaxAccelLeft,MaxAccelRight,MaxAccelForward,MaxAccelReverse;
		double MaxTorqueYaw;
		double Camera_Restraint;
		double G_Dampener;

		//! We can break this up even more if needed
		double EngineRampForward,EngineRampReverse,EngineRampAfterBurner;
		double EngineDeceleration,EngineRampStrafe;
	
		//Use this technique when m_AlterTrajectory is true
		double m_RequestedSpeed;
		double m_AutoLevelDelay; ///< The potential gimbal lock, and user rolling will trigger a delay for the autolevel (when enabled)
		double m_HeadingSpeedScale; //used by auto pilot control to have slower turn speeds for way points
		double m_rotVel_rad_s;

		//All input for turn pitch and roll apply to this, both the camera and ship need to align to it
		double m_IntendedOrientation;
		//We need the m_IntendedOrientation quarterion to work with its own physics
		FlightDynamics_2D m_IntendedOrientationPhysics;

		//For slide mode all strafe is applied here
		osg::Vec2d m_currAccel;  //This is the immediate request for thruster levels

		///Typically this contains the distance of the intended direction from the actual direction.  This is the only variable responsible for
		///changing the ship's orientation
		double m_rotDisplacement_rad;

		bool m_SimFlightMode;  ///< If true auto strafing will occur to keep ship in line with its position
		bool m_StabilizeRotation;  ///< If true (should always be true) this will fire reverse thrusters to stabilize rotation when idle
		bool m_CoordinateTurns;   ///< Most of the time this is true, but in some cases (e.g. Joystick w/rudder pedals) it may be false

		bool m_LockShipHeadingToOrientation; ///< Locks the ship and intended orientation (Joystick and Keyboard controls use this)

		Threshold_Averager<eThrustState,5> m_thrustState_Average;
		eThrustState m_thrustState;
		//double m_Last_AccDel;  ///< This monitors a previous AccDec session to determine when to reset the speed
		double m_Last_RequestedSpeed;  ///< This monitors the last caught requested speed from a speed delta change

		// When notifying everything about thrusters, we want to keep a bit of an averager
		Averager<osg::Vec3d, 5> m_ThrustReported_Averager;
		Blend_Averager<osg::Vec3d> m_TorqueReported_Averager;
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
		void SetAttitude(double radians);
		Goal *ClearGoal();
		void SetGoal(Goal *goal);
};
