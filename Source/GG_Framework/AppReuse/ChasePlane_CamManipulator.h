// GG_Framework.AppReuse ChasePlane_CamManipulator.h
#pragma once

namespace GG_Framework
{
	namespace AppReuse
	{
		class FRAMEWORK_APPREUSE_API ChasePlane_CamManipulator :
			public GG_Framework::UI::OSG::ICameraManipulator
		{
		public:
			class FRAMEWORK_APPREUSE_API IChaseVehicle
			{
			public:
				IChaseVehicle() : Self_Entity(NULL) {}

				//! Get the position of the Fixed Eye point, based on the position of the vehicle and its offset
				virtual void GetLookAtVectors(bool usingPOV, const osg::Quat &intendedOrientation, double dTime_s, 
					osg::Vec3d& fixedEye, osg::Vec3d& lookDir, osg::Vec3d& upDir) = 0;

				//! Called by ChasePlane_CamManipulator when position is initialized or reset
				//! This is usually a self reference, but we know the actors are set up when this is called
				//! and the Entity is handy.
				virtual void Reset(GG_Framework::Logic::Entity3D& entity){Self_Entity = &entity;}

			protected:
				GG_Framework::Logic::Entity3D* Self_Entity;
			};

			//! The Vehicle passed in should also inherit from IChaseVehicle
			ChasePlane_CamManipulator(GG_Framework::Logic::Entity3D& vehicle);

			virtual void UpdateCamera(GG_Framework::UI::OSG::ICamera* activeCamera, double dTime_s);
			void Reset();

			/// \param ByPassPhysics typically the mouse will set this to true where it sets the quat delta directly
			void POV_Turn(double dir,bool ByPassPhysics=false);
			void POV_Pitch(double dir,bool ByPassPhysics=false);

			///Specify whether or not to perform the snap back
			void SetPerformPOVSnapBack(bool performSnapBack) {m_PerformPOVSnapBack=performSnapBack;}
		private:
			void Reset_PosAtt(GG_Framework::UI::OSG::ICamera* activeCamera);
			osg::Vec3d ComputeTorque(GG_Framework::UI::OSG::ICamera* activeCamera,const osg::Vec3d &lookDir, double dTime_s,bool LockRoll);
			void ChasePlane_CamManipulator::UpdatePOV(double dTime_s);
			///This will get the vehicles intended orientation + any POV offset as well
			
			const osg::Quat GetIntendedOrientation();
			bool UsingPOV();

			void UpdateCameraOffset();

			float FindCameraDistortion();

			///Here we have the offset to be applied
			osg::Vec3d m_POV_rotVel_rad_s;
			GG_Framework::Logic::Physics::FlightDynamics m_POV_Physics;

			// These should both point back to the same thing
			GG_Framework::Logic::Entity3D& m_vehicle;
			IChaseVehicle* m_chaseVehicle;

			///This one is used to manipulate the orientation of the camera
			GG_Framework::Logic::Physics::FlightDynamics m_ChasePlane_Physics;
			
			double m_FixedCameraBlend;  ///< This is how much of the fixed camera to blend
			
			bool m_Reset; ///< used to align the camera directly when true
			bool m_PerformPOVSnapBack;
		};

		class FRAMEWORK_APPREUSE_API ChaseVehicle_Imp : public ChasePlane_CamManipulator::IChaseVehicle
		{
		public:
			ChaseVehicle_Imp();
			~ChaseVehicle_Imp();

			//! Here is where the metal hits the road
			virtual void GetLookAtVectors(bool usingPOV, const osg::Quat &intendedOrientation, double dTime_s, 
				osg::Vec3d& fixedEye, osg::Vec3d& lookDir, osg::Vec3d& upDir);

			//! Called by ChasePlane_CamManipulator once, may be called again
			virtual void Reset(GG_Framework::Logic::Entity3D& entity);

		protected:
			//! Should always be +
			virtual double GetMaxAccelForward(){return 0.0;}
			virtual double GetMaxAccelReverse(){return 0.0;}

			//! Uses m_cockpitCtr if valid, or 0,0,0
			osg::Vec3d GetCockpitCtr();

			Blend_Averager<double>* m_acceleratorBlend;

		protected:
			osg::ref_ptr<osg::Node> m_cockpitCtr;
			osg::ref_ptr<osg::Node> m_bbActor;

			///Cache the offsets for blending acceleration (cached because the bounding sphere computation introduce erratic behavior)
			osg::Vec3d m_CamerOffset_UpClose,m_CamerOffset_FarAway,m_CamerOffset_JustRight;

			///Establish how far the camera is from the vehicle
			osg::Vec3d m_CameraOffset;

			//obtain the vehicles acceleration by keeping tabs on the velocity
			osg::Vec3d m_LastVehicleVelocity;
		};
	}
}