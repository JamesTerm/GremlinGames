#pragma once
#undef TEST_THRUST_EVENTS
#undef __DisableSpeedControl__  //This one is great for test purposes

namespace Fringe
{
	namespace Base
	{
		enum EntityCollisionTypes
		{
			CannonRound = 1,
			Missile = 5,
			SmallSupportVessel = 10,
			Fighter = 15,
			MedSupportVessel = 20,
			Freighter = 25,
			Frigate = 30,
			Destroyer = 35,
			Cruiser = 40,
			Battleship = 45,
			Carrier = 50,
			SpaceStation = 55,
		};
		
class UI_Controller;
class AI_Base_Controller;
class ThrustShip2_RC_Controller;
/// \note It is understood that ThrusterShip2 and Ship may be merged, since all ships should be ThrusterShips.  There is no hurry for the merge.
class FRINGE_BASE_API ThrustShip2 : 
	public Fringe::Base::Ship, 
	public GG_Framework::AppReuse::ChaseVehicle_Imp
{
	public:
		ThrustShip2();
		virtual ~ThrustShip2();

		///This implicitly will place back in auto mode with a speed of zero
		void Stop(){SetRequestedSpeed(0.0);}
		void SetRequestedSpeed(double Speed);
		double GetRequestedSpeed(){return m_RequestedSpeed;}
		void FireAfterburner() {SetRequestedSpeed(GetMaxSpeed());}
		void SetCurrentLinearAcceleration(const osg::Vec3d &Acceleration) {m_currAccel=Acceleration;}
		void SetCurrentAngularVelocity(const osg::Vec3d &Velocity) {m_rotVel_rad_s=Velocity;}

		// This is where both the vehicle entity and camera need to align to
		virtual const osg::Quat &GetIntendedOrientation() {return m_IntendedOrientation;}

		// virtual void ResetPos();
		void SetStabilizeRotation(bool StabilizeRotation) { m_StabilizeRotation=StabilizeRotation;	}
		void SetSimFlightMode(bool SimFlightMode);
		void SetEnableAutoLevel(bool EnableAutoLevel);
		bool GetStabilizeRotation() const { return m_StabilizeRotation;}
		bool GetAlterTrajectory() const { return m_SimFlightMode;}
		bool GetEnableAutoLevel() const { return m_EnableAutoLevel;}
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

		// ChasePlane_CamManipulator::IChaseVehicle
		virtual double GetMaxAccelForward(){return GetFlightCharacteristics().MaxAccelForward;}
		virtual double GetMaxAccelReverse(){return GetFlightCharacteristics().MaxAccelReverse;}

		enum eThrustState { TS_AfterBurner_Brake=0, TS_Brake, TS_Coast, TS_Thrust, TS_AfterBurner, TS_NotVisible };
		eThrustState GetThrustState(){ return m_thrustState; }

		const FlightCharacteristics& GetFlightCharacteristics();

		// We have our own RC_Controller
		virtual GG_Framework::Logic::RC_Controller& GetRC_Controller();

		// This function fires the various thruster events and updates the ThrsutState
		// Called from my own timer update when locally controlled, or from my RC Controller when remote controlled
		virtual void UpdateThrustState(const osg::Vec3d& localThrust, const osg::Vec3d& localTorque);

		/// Called as we are losing control of this entity
		virtual void SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner);

		/// Someone is getting new control.  All clients get this message to assign the new controller and get last update
		virtual void RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner);

	protected:
		virtual void Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te);

		///This presents a downward force vector in MPS which simulates the pull of gravity.  This simple test case would be to work with the global
		///coordinates, but we can also present this in a form which does not have global orientation.
		virtual osg::Vec3d GetArtificialHorizonComponent() const;
		///This will apply turn pitch and roll to the intended orientation
		void UpdateIntendedOrientaton(double dTime_s);

		///This computes the current roll offset state that is applied on each frame.  This default implementation should work for most ships where it tries to
		///Keep the summed forces to pointing downward locally to the ships floor against an imaginary downward force of gravity.
		virtual double ComputeRollOffset(const osg::Vec3d &LocalTrajectoryForceToApply) const;
		double ComputeAutoLevelRollOffset();

		virtual void ApplyTorqueThrusters(GG_Framework::Logic::Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &Torque,const osg::Vec3d &TorqueRestraint,double dTime_s);
		///Putting force and torque together will make it possible to translate this into actual force with position
		virtual void ApplyThrusters(GG_Framework::Logic::Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &LocalForce,const osg::Vec3d &LocalTorque,const osg::Vec3d &TorqueRestraint,double dTime_s);
		virtual void TestPosAtt_Delta(const osg::Vec3d pos_m, const osg::Vec3d att,double dTime_s);

		virtual void TimeChange(double dTime_s);

		// Watch for being made the controlled ship
		virtual bool IsPlayerControllable(){return true;}

		// Called by the OnCollision that finishes the ship off
		virtual void DestroyEntity(bool shotDown, osg::Vec3d collisionPt);

		eThrustState SetThrustState(eThrustState ts); // Handles the ON/OFF events, only for controlled entities

		// Only set my the RC_Controller
		virtual void Show(bool show);

	private:
		friend AI_Base_Controller;
		friend UI_Controller;

		AI_Base_Controller* m_controller;
		double MAX_SPEED,ENGAGED_MAX_SPEED;

		// Used in Keyboard acceleration and braking
		double ACCEL, BRAKE, STRAFE, AFTERBURNER_ACCEL, AFTERBURNER_BRAKE;
		double dHeading, dPitch, dRoll;

		osg::Vec3d m_RadialArmDefault; //cache the radius of concentrated mass square, which will allow us to apply torque in a r = 1 case

		//Stuff needed for physics
		double Mass;

		FlightCharacteristics NormalFlight_Characteristics, Afterburner_Characteristics, Braking_Characteristics;

		//! We can break this up even more if needed
		double EngineRampForward,EngineRampReverse,EngineRampAfterBurner;
		double EngineDeceleration,EngineRampStrafe;
	
		//Use this technique when m_AlterTrajectory is true
		double m_RequestedSpeed;
		double m_AutoLevelDelay; ///< The potential gimbal lock, and user rolling will trigger a delay for the autolevel (when enabled)
		double m_HeadingSpeedScale; //used by auto pilot control to have slower turn speeds for way points
		osg::Vec3d m_rotVel_rad_s;

		//All input for turn pitch and roll apply to this, both the camera and ship need to align to it
		osg::Quat m_IntendedOrientation;
		//We need the m_IntendedOrientation quarterion to work with its own physics
		GG_Framework::Logic::Physics::FlightDynamics m_IntendedOrientationPhysics;

		//For slide mode all strafe is applied here
		osg::Vec3d m_currAccel;  //This is the immediate request for thruster levels

		///Typically this contains the distance of the intended direction from the actual direction.  This is the only variable responsible for
		///changing the ship's orientation
		osg::Vec3d m_rotDisplacement_rad;

		bool m_SimFlightMode;  ///< If true auto strafing will occur to keep ship in line with its position
		bool m_StabilizeRotation;  ///< If true (should always be true) this will fire reverse thrusters to stabilize rotation when idle
		bool m_CoordinateTurns;   ///< Most of the time this is true, but in some cases (e.g. Joystick w/rudder pedals) it may be false
		//This is false from when roll is used until the roll velocity has come to a stop... auto roll is disabled during this time
		bool m_LockRoll;

		bool m_GimbalLockTriggered; ///< This is a valve mechanism to get the timer to count down once the Gimbal is triggered
		bool m_EnableAutoLevel; ///< Toggle whether or not to auto level to the artificial horizon
		bool m_LockShipHeadingToOrientation; ///< Locks the ship and intended orientation (Joystick and Keyboard controls use this)
		bool m_LockShipRollToOrientation; ///< Same as above except it only applies for Roll

		Threshold_Averager<eThrustState,5> m_thrustState_Average;
		eThrustState m_thrustState;
		//double m_Last_AccDel;  ///< This monitors a previous AccDec session to determine when to reset the speed
		double m_Last_RequestedSpeed;  ///< This monitors the last caught requested speed from a speed delta change
		ThrustShip2_RC_Controller* m_TS_RC_Controller;

		// When notifying everything about thrusters, we want to keep a bit of an averager
		Averager<osg::Vec3d, 5> m_ThrustReported_Averager;
		Blend_Averager<osg::Vec3d> m_TorqueReported_Averager;

		// Used to handle all of the Thruster Events
		class ThrustEventHandler
		{
		private:
			// Thrust alone
			Event1<double> *Thrust_F, *Thrust_B, *Thrust_L, *Thrust_R, *Thrust_U, *Thrust_D;

			// Forward, Heading Pitch
			Event1<double> *Thrust_FR, *Thrust_FL, *Thrust_FU, *Thrust_FD, *Thrust_FRU, *Thrust_FRD, *Thrust_FLU, *Thrust_FLD;

			// Backward, Heading Pitch
			Event1<double> *Thrust_BR, *Thrust_BL, *Thrust_BU, *Thrust_BD, *Thrust_BRU, *Thrust_BRD, *Thrust_BLU, *Thrust_BLD;

			// Left Thrust, Heading, Roll
			Event1<double> *Thrust_LR0, *Thrust_LL0, *Thrust_L0L, *Thrust_L0R, *Thrust_LLL, *Thrust_LLR, *Thrust_LRL, *Thrust_LRR; 

			// Right Thrust, Heading, Roll
			Event1<double> *Thrust_RR0, *Thrust_RL0, *Thrust_R0L, *Thrust_R0R, *Thrust_RLL, *Thrust_RLR, *Thrust_RRL, *Thrust_RRR;

			// Up Thrust, Pitch, Roll
			Event1<double> *Thrust_UD, *Thrust_UU, *Thrust_UR, *Thrust_UL, *Thrust_UUR, *Thrust_UUL, *Thrust_UDL, *Thrust_UDR;

			// Down Thrust, Pitch, Roll
			Event1<double> *Thrust_DD, *Thrust_DU, *Thrust_DR, *Thrust_DL, *Thrust_DUR, *Thrust_DUL, *Thrust_DDL, *Thrust_DDR;

			FlightCharacteristics m_flightCharacteristics;

			double m_mass;
			osg::Vec3d m_RadiusOfConcentratedMass;

#ifdef TEST_THRUST_EVENTS
			ThrustShip2* m_ts;
#endif
		public:
			// This function updates All of the Events
			void SetThrusterShip(ThrustShip2* ts, double mass, const osg::Vec3d& RadiusOfConcentratedMass);

			// Use this function to update all of the existing limits
			void ResetFlightCharacteristics();

			// Additional available flight characteristics are added here, maximizing the flight characteristics available
			void MaximizeFlightCharacteristics(const FlightCharacteristics& maxMe);

			void ApplyThrustAndTorque(const osg::Vec3d& LocalForce, const osg::Vec3d& LocalTorque);
		};
		ThrustEventHandler m_thrustEventHandler;
};

class FRINGE_BASE_API ThrustShip2_RC_Controller : public GG_Framework::Logic::RC_Controller
{
public:
	ThrustShip2_RC_Controller(ThrustShip2* parent) : 
	  m_parent(parent), GG_Framework::Logic::RC_Controller(parent) {}

	  virtual void PopulateStandardEntityUpdate(RakNet::BitStream& entityUpdateBS);
	  virtual void ReadStandardEntityUpdate(RakNet::BitStream& entityUpdateBS, double msgTime_s);

	  // For locally controlled ships, these are updated by the ship and written in PopulateStandardEntityUpdate
	  // For Remotely controlled, they are read from ReadStandardEntityUpdate and the ship uses them
	  // The ship then uses these values to fire events
	  osg::Vec3d LocalThrust;
	  osg::Vec3d LocalTorque;

private:
	ThrustShip2* m_parent;
};
//////////////////////////////////////////////////////////////////////////


	}
}