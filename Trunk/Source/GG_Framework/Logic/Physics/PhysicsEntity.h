#pragma once

#define __dumpHeading__
#undef __TestInput__

namespace GG_Framework
{
	namespace Logic
	{
		namespace Physics
		{

//Here is a handy helper function to ensure radian measurements remain within 2pi
inline void FRAMEWORK_LOGIC_PHYSICS_API NormalizeVec3ToPi2(osg::Vec3d &A);
inline void FRAMEWORK_LOGIC_PHYSICS_API NormalizeVec3ToPi2wDirection(osg::Vec3d &A);

class PhysicsEntityInteraction;
///This handles local computations and forces of a particular Entity3d object.  Special methods are available for PhysicsEntityInteraction to
///Apply external forces (e.g. gravity, collisions).  Entity3D can apply forces and torque and the results of these (and external forces) will 
///be submitted to the PosATT_UpdateInterface.  Other custom helper functions that deal with physics will be added here like things which help
///determine how and where to apply force and torque.  This class does not interact with other entities and does not implement collision detection.
class FRAMEWORK_LOGIC_PHYSICS_API PhysicsEntity
{
	public:
		PhysicsEntity();
		///You must set the mass otherwise you will get the default
		void SetMass(double mass);
		double GetMass() const;
		///This will zero out all vectors
		virtual void ResetVectors();

		// An overloaded operator for matching the current physics and position
		virtual void CopyFrom(const PhysicsEntity& rhs);

		///This will compute all the displacement. Call this each time slice to obtain the rotation and position displacements.  
		///You must call this to run the physics engine!
		virtual void TimeChangeUpdate(double DeltaTime_s,osg::Vec3d &PositionDisplacement,osg::Vec3d &RotationDisplacement);

		///This simply returns a min operation of speed/time and the maximum speed available to stop within the given distance
		///This is ideal to be used with GetTorqueFromVelocity
		osg::Vec3d GetVelocityFromDistance_Angular(const osg::Vec3d &Distance,const osg::Vec3d &Restraint,double DeltaTime_s);
		osg::Vec3d GetVelocityFromDistance_Angular_v2(const osg::Vec3d &Distance,const osg::Vec3d &Restraint,double DeltaTime_s, const osg::Vec3d& matchVel);

		inline osg::Vec3d GetVelocityFromCollision(osg::Vec3d ThisVelocityToUse,double otherEntityMass,osg::Vec3d otherEntityVelocity);
		///This simply returns a min operation of speed/time and the maximum speed available to stop within the given distance
		///This can either work with local or global orientation that all depends on the orientation of the restraints typically this works in local
		virtual osg::Vec3d GetVelocityFromDistance_Linear(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel);
		osg::Vec3d GetVelocityFromDistance_Linear_v1(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel);
		//TODO fix... this does not work properly, but does work with the RC controller in its current state
		osg::Vec3d GetVelocityFromDistance_Linear_RC_FIXME(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel);
		///These are coefficients to use when dealing with a force of friction typically 0.8 and 0.2 respectively
		void SetFriction(double StaticFriction, ///<The amount of friction to be applied when object is not moving
						 double KineticFriction ///<The amount of friction to be applied when object is moving
						 );

		///This is the direct way to handle torque for various mass distributions
		///Here are some examples:
		/// - Disk rotating around its center				0.5
		/// - Hollow cylinder rotating around its center	1.0
		/// - Hollow sphere									0.66 (2/3)
		/// - Hoop rotating around its center				1.0
		/// - Point mass rotating at radius r				1.0
		/// - Solid cylinder								0.5
		/// - Solid sphere									0.4
		/// \todo Provide helper methods to compute this value
		void SetAngularInertiaCoefficient(double AngularInertiaCoefficient);
		///This sets the radius for yaw axis, pitch axis, and roll axis.  Default = 1,1,1
		void SetRadiusOfConcentratedMass(const osg::Vec3d &RadiusOfConcentratedMass);
		const osg::Vec3d &GetRadiusOfConcentratedMass() const;

		void SetLinearVelocity( const osg::Vec3d &LinearVelocity);
		osg::Vec3d GetLinearVelocity() const;
		//Find the magnitude of a given velocity (i.e. take out the direction factor)
		double GetSpeed(const osg::Vec3d &Velocity);

		void SetAngularVelocity( const osg::Vec3d &AngularVelocity);
		osg::Vec3d GetAngularVelocity() const;

		///These are depreciated... \todo remove
		osg::Vec3d GetLinearAcceleration() const;
		osg::Vec3d GetAngularAcceleration() const;
		//This will give the acceleration delta given the torque which is: torque / AngularInertiaCoefficient * Mass
		inline osg::Vec3d GetAngularAccelerationDelta(const osg::Vec3d &torque,const osg::Vec3d &RadialArmDistance=osg::Vec3d(1.0,1.0,1.0));

		/// \todo Add ApplyForce from a point
		//void ApplyForce( const osg::Vec3d &force, const osg::Vec3d &point );
		///These will auto sum for each call made, the forces last for one second during each timer update, so you have to call them repeatedly to 
		///continue to apply force.  If you want to apply a force for a specific amount of time, this can be achieved by calling this
		///during each time slice.  
		/// \note This should offer enough precision in time, but you can get more precise by computing a fraction force if necessary
		void ApplyForce( const osg::Vec3d &force);
		void ApplyTorque( const osg::Vec3d &torque);

		///These work like the above except that the force applied only happens for a fraction of a second.  For accurate results FrameDuration
		///should be <= 1/framerate.  Ideally these should be used for high precision movements like moving a ship, where the FrameDuration is
		///typically the TimeDelta value
		void ApplyFractionalForce( const osg::Vec3d &force,double FrameDuration);
		void ApplyFractionalTorque( const osg::Vec3d &torque,double FrameDuration,const osg::Vec3d &RadialArmDistance=osg::Vec3d(1.0,1.0,1.0));

		///This one is ideal to use for collision detection.  It will basically evaluate the point and determine the amount of force and torque to
		///apply.  It will implicitly call ApplyFractionalForce and ApplyFractionalTorque.
		void ApplyFractionalForce( const osg::Vec3d &force, const osg::Vec3d &point,double FrameDuration );

		///You may prefer to set a desired speed instead of computing the forces.  These values returned are intended to be used with 
		///ApplyFractionalForce, and ApplyFractionalTorque respectively.
		/// For the force, this works with the current linear velocity; therefore the desired velocity and return must work in global orientation
		virtual osg::Vec3d GetForceFromVelocity( 
			const osg::Vec3d &vDesiredVelocity,	///< How fast do you want to go (in a specific direction)
			double DeltaTime_s					///< How quickly do you want to get there
			);
		virtual osg::Vec3d GetTorqueFromVelocity( 
			const osg::Vec3d &vDesiredVelocity,	///< How fast do you want to go (in a specific direction)
			double DeltaTime_s					///< How quickly do you want to get there (usually in time slices)
			);

		///This is a clean way to compute how much torque that can be applied given the maximum amount available (e.g. thruster capacity)
		///It should be noted that this treats roll as a separate factor, which is best suited for avionic type of context
		/// \Note all restraint parameters are positive (i.e. ForceRestraintNegative)
		osg::Vec3d ComputeRestrainedTorque(const osg::Vec3d &Torque,const osg::Vec3d &TorqueRestraint,double dTime_s);
		osg::Vec3d ComputeRestrainedForce(const osg::Vec3d &LocalForce,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double dTime_s);

	protected:
		double m_EntityMass;
		double m_StaticFriction,m_KineticFriction;

		double m_AngularInertiaCoefficient;
		osg::Vec3d m_RadiusOfConcentratedMass; //This is used to compute the moment of inertia for torque (default 1,1,1)

		osg::Vec3d m_LinearVelocity;		///< This must represent global orientation for external forces to work properly
		osg::Vec3d m_AngularVelocity;		///< All angle variables are in radians!

		/// \note These are only used for full scale newtons
		osg::Vec3d m_LinearAcceleration;
		osg::Vec3d m_AngularAcceleration;
		struct AccStamp
		{
			osg::Vec3d Acceleration;
			double Time;
		};
		//We need to keep a queue of all times applied to shut them off after a second
		std::queue<AccStamp> m_LinearAccQueue;
		std::queue<AccStamp> m_TorqueAccQueue;
		double m_TrackAccTime;  ///< This is exclusively used to measure the time of queued accelerations

		///This variable is factored in but is managed externally 
		osg::Vec3d m_SummedExternalForces;
		double m_lastTime_s;

		#ifdef __dumpHeading__
		//This is temporary only for testing purposes
		osg::Vec3d m_LastHeading;
		#endif
		#ifdef __TestInput__
		double m_TestInput;
		#endif
};

///This class is a expands on some common tasks that deal more specifically with flight.  This attempts to pull common tasks needed from physics in a way
///Where it is easy to use for ships and other objects that deal with orientation and position
class FRAMEWORK_LOGIC_PHYSICS_API FlightDynamics : public PhysicsEntity
{
	public:
		//provide common area to initialize members
		void init();
		FlightDynamics();
		FlightDynamics(const osg::Quat &quatToUse);

		virtual void ResetVectors();

		///This will measure the distance between this quat and the look dir quat.  With the current algorithm the most desired results occur when the
		///delta's are less than 90 degrees for yaw and pitch.  Roll is computed separately.
		/// \param lookDir This is another orientation that you are comparing against
		/// \param UpDir is usually the same quat's orientation * 0,0,1
		osg::Vec3d ComputeAngularDistance(const osg::Vec3d &lookDir,const osg::Vec3d &UpDir);
		/// \param Orientation this will break down the quat into its lookDir and UpDir for you
		osg::Vec3d ComputeAngularDistance(const osg::Quat &Orientation);
		const osg::Quat &GetQuat() {return m_QuatToUse;}

		virtual void TimeChangeUpdate(double DeltaTime_s,osg::Vec3d &PositionDisplacement,osg::Vec3d &RotationDisplacement);

		//Acceleration rate methods:

		///If these methods are being used, this must be set to true
		void SetUsingAccelerationRate(bool UseIt) {m_UsingAccelerationRate=UseIt;} 
		struct LinearAccelerationRates
		{
			osg::Vec3d AccDeltaPos;        //when increasing from a positive position
			osg::Vec3d AccDeltaNeg;        //when -increasing from a negative position
			osg::Vec3d DecDeltaPos;        //when decreasing from a positive position
			osg::Vec3d DecDeltaNeg;        //when -decreasing from a negative position
		};
		///Get and set the linear acceleration rates here.
		///For now this is the parameters for the simple model, which may or may not be applied for other engine models
		///Typically this should be a one time setup, but can be dynamic (e.g. afterburner)
		LinearAccelerationRates &GetLinearAccelerationRates();
		//Set to the desired acceleration level.  This should be called first per time interval, so the other methods can work properly
		//This must use local orientation
		void SetTargetAcceleration(const osg::Vec3d &TargetAcceleration);
		///This can be defined to just about anything, but to keep things simple it is a linear constant depending on direction
		///We could later have different models and have client choose which to use
		osg::Vec3d GetAcceleration_Delta (double dTime_s);
		///With this version you may use to predict the capabilities of max acceleration or deceleration
		/// \param Clipping this chooses whether to extract the full power when the acceleration approaches target acceleration, or
		/// whether to compute the actual force necessary to hit target.  Typically this will be on when manipulating the force
		/// The no clip is useful for things which need know full capability to get to threshold such as in GetForceFromVelocity
		osg::Vec3d GetAcceleration_Delta (double dTime_s,const osg::Vec3d &TargetAcceleration,bool Clipping=true);
		///This applies the current acceleration delta (i.e. calls GetAcceleration_Delta) to acceleration.
		/// \note Special care has been taken to ensure that any reads to GetAcceleration_Delta will be the same value applied here.
		///This must be consistent per time slice so that other clients can properly predict the acceleration.
		void Acceleration_TimeChangeUpdate(double dTime_s);
		///Read-only access of the current acceleration
		const osg::Vec3d &GetCurrentAcceleration() {return m_CurrentAcceleration;}

		/// These are overloaded to optionally factor in the acceleration period
		virtual osg::Vec3d GetForceFromVelocity(const osg::Vec3d &vDesiredVelocity,double DeltaTime_s);
		virtual osg::Vec3d GetVelocityFromDistance_Linear(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel);
		class Pilot
		{
			public:
				Pilot (FlightDynamics *parent);
				const double Get_Gs() const {return m_Gs;}
				const double Get_blackOUTredOUT() const {return m_blackOUTredOUT;}

				double GLimit;	// How many G's can the pilot handle before starting to black out
				double PassOutTime_s; // Time it takes to pass out
				double PassOutRecoveryTime_s; // Time to get OUT of pass out and start recovery
				double MaxRecoveryTime_s;	// So we do not pass out forever
				
			private:
				friend FlightDynamics;
				FlightDynamics * const m_pParent;
				// This is called implicitly frame to determine G forces and current velocities
				void Update(double dTime_s, double G_Dampener);
				//This is also called implicity when parent is called
				void ResetVectors();

				enum {INITIALIZE_Gs = 1000};

				// Tracking G's
				double m_Gs;

				Averager<double, 45> m_G_Averager;
				double m_blackOUTredOUT;	// value between -1(red) and +1(black) most of the time 0.0
		};
		//Read only access to the pilot information
		const Pilot &GetPilotInfo() const {return m_Pilot;}
		Pilot &GetPilotInfo() {return m_Pilot;}

		double StructuralDmgGLimit;
		double G_Dampener;

	private:
		osg::Quat m_DefaultQuat;
		// I'll try to keep this read only, so that client who own their own quat can use this code, without worrying about the quat being changed
		const osg::Quat &m_QuatToUse;
		//This keeps track of the current rate of acceleration.  These are in local orientation.
		osg::Vec3d m_CurrentAcceleration,m_TargetAcceleration;
		LinearAccelerationRates m_LinearAccelerationRates;
		//Some objects may not need to use this (by default they will not)
		bool m_UsingAccelerationRate;
		Pilot m_Pilot;
};

		}
	}
}