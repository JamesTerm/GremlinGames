//! GuardianVR.Base	Mech.h
#pragma once

namespace GuardianVR
{
	namespace Base
	{
		// Pre-Declarations
		class Mech_Controller;

		class GUARDIANVR_BASE_API Mech_ChaseVehicle_Imp : public GG_Framework::AppReuse::ChaseVehicle_Imp
		{
		public:
			//! Here is where the metal hits the road
			virtual void GetLookAtVectors(bool usingPOV, const osg::Quat &intendedOrientation, double dTime_s, 
				osg::Vec3d& fixedEye, osg::Vec3d& lookDir, osg::Vec3d& upDir);

			//! Called by ChasePlane_CamManipulator once, may be called again
			virtual void Reset(GG_Framework::Logic::Entity3D& entity);

		protected:
			osg::ref_ptr<osg::Node> m_bbActor;
			osg::ref_ptr<osg::Node> m_camPosOffset;
		};

		//! A Mech is a DestroyableEntity that can also have cannons
		class GUARDIANVR_BASE_API Mech : 
			public GG_Framework::AppReuse::DestroyableEntity, 
			public GG_Framework::AppReuse::ICannonOwner, 
			public Mech_ChaseVehicle_Imp
		{
		public:
			Mech() : m_cannonRange(0.0), m_cannonSpeed(0.0), m_target(NULL), m_controller(NULL), 
				m_currMotionState(NULL) {}
			virtual ~Mech();
			osg::Vec3d GetVelocity_m_s(){return m_Physics.GetLinearVelocity();}
			double GetWeaponsRange(){return m_cannonRange;}
			double GetWeaponsSpeed(){return m_cannonSpeed;}

			// Call this on the different kinds of motion states

			virtual void TimeChange(double dTime_s);
			virtual void TryFireMainWeapon(bool on);


			virtual void OnSpawn(bool on);

			GG_Framework::AppReuse::DestroyableEntity* GetGunTarget(){return m_target;}
			void SetGunTarget(GG_Framework::AppReuse::DestroyableEntity* target){m_target = target;}

			void GetWeaponStatus(GG_Framework::AppReuse::Cannon::TempStatus& tempStatus, double& tempLevel, double& restartLevel) const;

			// ICannonOwner
			virtual GG_Framework::Logic::Entity3D& ICO_GetSelfEntity3D(){return *this;}

			
			Mech_Controller* GetController() {return m_controller;}

			// void SetCurrentLinearAcceleration(const osg::Vec3d &Acceleration) {m_currAccel=Acceleration;}
			void SetCurrentAngularVelocity(const osg::Vec3d &Velocity){m_rotVel_rad_s=Velocity;}

			// Ignore all collisions for now
			virtual bool ShouldIgnoreCollisions(Entity3D* otherEntity){return true;}

			// Watch for being made the controlled ship
			virtual bool IsPlayerControllable(){return true;}
			virtual void SetPlayerControlled(bool isControlled);

			// We will be manipulating our own intended orientation, not just focussing on our actual att quat
			virtual const osg::Quat &GetIntendedOrientation() {return m_IntendedOrientation;}

		private:
			friend Mech_Controller;
			double dHeading, dPitch;
			osg::Vec3d m_rotVel_rad_s;

			//All input for turn pitch and roll apply to this, both the camera and ship need to align to it
			osg::Vec3d m_intendedOrientation_Vec;
			osg::Quat m_IntendedOrientation;
			void UpdateIntendedOrientaton(double dTime_s);

			///Putting force and torque together will make it possible to translate this into actual force with position
			virtual void ApplyLocalTorque(GG_Framework::Logic::Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &Torque,double dTime_s);
			virtual void ApplyLocalForce(GG_Framework::Logic::Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &LocalForce, double dTime_s);

			virtual void Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te);
			virtual void CancelAllControls();

			Mech_Controller* m_controller;
			GG_Framework::AppReuse::DestroyableEntity* m_target;

			std::vector<GG_Framework::AppReuse::Cannon*> m_Cannons;
			double m_cannonRange;
			double m_cannonSpeed;

			// The current motion state, events have already been fired here
			const char* m_currMotionState;

			// Load the rounds for the cannon async
			virtual void OnAsyncLoadComplete();
		};
	}
}