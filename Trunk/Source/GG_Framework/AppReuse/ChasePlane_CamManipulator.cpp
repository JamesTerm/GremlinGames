// GG_Framework.AppReuse ChasePlane_CamManipulator.cpp
#include "stdafx.h"
#include "GG_Framework.AppReuse.h"

using namespace GG_Framework::Base;
using namespace GG_Framework::AppReuse;

const double Mass=10.0; //a very lightweight camera :)

//This is used to test huge deltas to resolve in quats (tests the distance and velocity computations)
#undef __TestHugeQuatDelta__

void ChasePlane_CamManipulator::Reset()
{
	m_Reset=true;
}
//////////////////////////////////////////////////////////////////////////

ChasePlane_CamManipulator::ChasePlane_CamManipulator(GG_Framework::Logic::Entity3D& vehicle) : 
	m_vehicle(vehicle),m_PerformPOVSnapBack(true)
{
	//TODO have some user set up be able to set this
	//m_FixedCameraBlend=0.0;
	m_FixedCameraBlend=0.15;  //set low because the effect is logarithmic
	//m_FixedCameraBlend=1.0;

	m_ChasePlane_Physics.SetMass(Mass); 
	m_POV_Physics.SetMass(Mass);

	m_chaseVehicle = dynamic_cast<IChaseVehicle*>(&m_vehicle);
	ASSERT(m_chaseVehicle);
	
	Reset();
}

//////////////////////////////////////////////////////////////////////////

void ChasePlane_CamManipulator::Reset_PosAtt(GG_Framework::UI::OSG::ICamera* activeCamera)
{
	// Initialize the chase vehicle this one time
	m_chaseVehicle->Reset(m_vehicle);

	// Get the (intended) position of the eye
	osg::Vec3d eye, lookDir, upDir; m_chaseVehicle->GetLookAtVectors(false, GetIntendedOrientation(), 0.0, eye, lookDir, upDir);
	// And we use the intended orientation to set the look direction
	osg::Vec3d look = eye + (m_vehicle.GetIntendedOrientation()*lookDir);
	osg::Vec3d up = m_vehicle.GetIntendedOrientation()*upDir;

	// Build the Matrix
	osg::Matrix camMat;
	camMat.makeLookAt(eye, look, up);

	// Set the matrix a
	activeCamera->SetMatrix(camMat, FindCameraDistortion());
}
//////////////////////////////////////////////////////////////////////////

osg::Vec3d ChasePlane_CamManipulator::ComputeTorque(GG_Framework::UI::OSG::ICamera* activeCamera,const osg::Vec3d &lookDir, double dTime_s,bool LockRoll)
{
	osg::Vec3d TorqueRestraint (Mass*50.0,Mass*50.0,LockRoll?-1:Mass*5.0);
	osg::Vec3d Offset=m_ChasePlane_Physics.ComputeAngularDistance(lookDir,m_vehicle.Get_IO_UpDownDirection());
	//avoid gimbal lock for high offset cases
	if (fabs(Offset[1])>1.0)
		Offset[2]=0.0;

	//DOUT4("%f %f %f",Offset[0],Offset[1],Offset[2]);
	osg::Vec3d angularvelocity(m_ChasePlane_Physics.GetVelocityFromDistance_Angular(-Offset,TorqueRestraint,dTime_s));
	osg::Vec3d TorqueToApply(m_ChasePlane_Physics.GetTorqueFromVelocity(angularvelocity,dTime_s));
	TorqueToApply=m_ChasePlane_Physics.ComputeRestrainedTorque(TorqueToApply,TorqueRestraint,dTime_s);

	return TorqueToApply;
}
//////////////////////////////////////////////////////////////////////////


#define __UsePhysics__
#ifndef __TestHugeQuatDelta__
#define __ApplyTorque__
#endif
//////////////////////////////////////////////////////////////////////////

const osg::Quat ChasePlane_CamManipulator::GetIntendedOrientation()
{
	return m_POV_Physics.GetQuat()*m_vehicle.GetIntendedOrientation();
}
//////////////////////////////////////////////////////////////////////////

void ChasePlane_CamManipulator::UpdatePOV(double dTime_s)
{	//Now to run the physics on the m_POVOffset
	osg::Vec3d TorqueRestraint (Mass*2.0,Mass*2.0,Mass*2.0);
	osg::Vec3d angularvelocity(m_POV_rotVel_rad_s);
	{
		//Here is the snap back feature

		//This is temporary testing
#if 0
		osg::Vec3d Offset(0.0,0.0,0.0);
		{
			osg::Vec3d lookDir(0,1,0);
			osg::Vec3d UpDir(0,0,1);
			//LookDir Default 0,1,0   +R-L,+F-B,+U-D

			osg::Vec3d CurrentOrientation(m_POV_Physics.GetQuat().conj()*lookDir);
			//printf("\r %f %f %f vehicle                       ",CurrentOrientation[0],CurrentOrientation[1],CurrentOrientation[2]);
			Offset[0]=atan2(CurrentOrientation[0],CurrentOrientation[1]);  //Angle in radians
			if (CurrentOrientation[1]>0.0)
				Offset[1]=atan2(CurrentOrientation[2],CurrentOrientation[1]);  //Angle in radians
			else
				Offset[1]-=atan2(CurrentOrientation[2],-(CurrentOrientation[1]));  //Angle in radians
			
			//printf("\r %f %f radians          ",Offset[0],Offset[1]);
			printf("\r %f %f degrees          ",RAD_2_DEG(Offset[0]),RAD_2_DEG(Offset[1]));
			
			osg::Vec3d RollOrientation(m_POV_Physics.GetQuat().conj()*UpDir);
			//printf("\r %f %f %f vehicle                       ",RollOrientation[0],RollOrientation[1],RollOrientation[2]);
			Offset[2]=atan2(RollOrientation[0],RollOrientation[2]);  //Angle in radians
			//printf("\r %f degrees          ",RAD_2_DEG(RollOffset));
		}
#else
		osg::Vec3d Offset=m_POV_Physics.ComputeAngularDistance(osg::Vec3d(0,1,0),osg::Vec3d(0,0,1));
#endif
		double YawAngleOffset=Offset[0];
		double PitchAngleOffset=Offset[1];
		double RollOffset=Offset[2];

		bool YawSnapLimitExceeded=(fabs(YawAngleOffset)>1.4);
		bool ReversePitchSnapLimitExceeded=(fabs(PitchAngleOffset-M_PI)>1.4);

		bool StopPitch=false;
#if 0
		//Set some hard stops for pitch to avoid gimbal lock
		{
			osg::Vec3d AngularVelocity=m_POV_Physics.GetAngularVelocity();

			#ifndef __TestHugeQuatDelta__
			if (PitchAngleOffset>1.4)
				StopPitch=((AngularVelocity[1]>=0.0)&&(angularvelocity[1]>=0.0));
			else if (PitchAngleOffset<-1.4)
				StopPitch=((AngularVelocity[1]<=0.0)&&(angularvelocity[1]<=0.0));
			#endif 

			if (StopPitch)
			{
				//printf("\r%f %f rad          ",PitchAngleOffset,AngularVelocity[1]);
				angularvelocity[1]=AngularVelocity[1]=0.0;
				m_POV_Physics.SetAngularVelocity(AngularVelocity);
			}
		}
#endif
		if ((angularvelocity[0]!=0.0)||(angularvelocity[1]!=0.0))
		{
			YawAngleOffset=0.0;
			PitchAngleOffset=0.0;
		}
		angularvelocity+=m_POV_Physics.GetVelocityFromDistance_Angular(osg::Vec3d (-YawAngleOffset,-PitchAngleOffset,-RollOffset),TorqueRestraint,dTime_s);

		#ifndef __TestHugeQuatDelta__
		//We can now do a full snap back, but we may prefer to do a half one
		if ((YawSnapLimitExceeded)||(!m_PerformPOVSnapBack))
		{
			angularvelocity[0]=m_POV_rotVel_rad_s[0];
			angularvelocity[1]=m_POV_rotVel_rad_s[1];
		}
		#endif 
		if (StopPitch)
			angularvelocity[1]=0.0;
		//printf("\r%f rad %d         ",angularvelocity[1],StopPitch);

	}
	osg::Vec3d TorqueToApply(m_POV_Physics.GetTorqueFromVelocity(angularvelocity,dTime_s));
	TorqueToApply=m_POV_Physics.ComputeRestrainedTorque(TorqueToApply,TorqueRestraint,dTime_s);
	m_POV_Physics.ApplyFractionalTorque(TorqueToApply,dTime_s);
	osg::Vec3d PositionDisplacement;
	osg::Vec3d RotationDisplacement;
	m_POV_Physics.TimeChangeUpdate(dTime_s,PositionDisplacement,RotationDisplacement);
}
//////////////////////////////////////////////////////////////////////////

bool ChasePlane_CamManipulator::UsingPOV()
{
	//osg::Quat quat=m_POV_Physics.GetQuat();
	//DOUT4("%f %f %f %f %d",quat[0],quat[1],quat[2],quat[3],GG_Framework::UI::OSG::zeroRotation(quat));
	return !(GG_Framework::UI::OSG::zeroRotation(m_POV_Physics.GetQuat()));
}
//////////////////////////////////////////////////////////////////////////

void ChasePlane_CamManipulator::UpdateCamera(GG_Framework::UI::OSG::ICamera* activeCamera, double dTime_s)
{

#ifndef __UsePhysics__
	Reset_PosAtt(activeCamera);
#else
	// Watch for a reset, where we set everything back to defaults and return
	if (m_Reset)
	{
		m_ChasePlane_Physics.ResetVectors();
		m_POV_Physics.ResetVectors();
		Reset_PosAtt(activeCamera);
		m_Reset=false;
		return;
	}

	// Update the motion for the POV
	UpdatePOV(dTime_s);

	// The intended orientation for the camera includes the intended orientation of the vehicle AND the POV
	bool usingPOV= UsingPOV();

	// Get the current camera matrix so we can use physics to determine how to make it move smoothly
	osg::Matrix CameraMatrix=activeCamera->GetCameraMatrix();
	osg::Vec3d camPos, camCenter, camUp;
	CameraMatrix.getLookAt(camPos, camCenter, camUp);
	
	// Get the (intended) position of the eye
	osg::Vec3d FixedEye, lookDir, upDir; m_chaseVehicle->GetLookAtVectors(usingPOV, GetIntendedOrientation(), dTime_s, FixedEye, lookDir, upDir);

#ifdef __ApplyTorque__
	m_ChasePlane_Physics.ApplyFractionalTorque(ComputeTorque(activeCamera,GetIntendedOrientation()*lookDir,dTime_s,!usingPOV),dTime_s);
#endif

	//Compute the distance delta
	osg::Vec3d LinearDeltaDistance=(FixedEye-camPos);
	//This works better, but should not be used since other things can manipulate the camera
	double Distance=LinearDeltaDistance.length();  //find the magnitude

	//DOUT2("%f %f %f",LinearDeltaDistance[0],LinearDeltaDistance[1],LinearDeltaDistance[2]);
	//DOUT2("%f %f %f",camPos[0],camPos[1],camPos[2]);
	osg::Vec3d Restraint((Distance+2.0)*Mass,(Distance+2.0)*Mass,(Distance+2.0)*Mass);
	//osg::Vec3d Restraint(-1,-1,-1);
	osg::Vec3d linearvelocity(m_ChasePlane_Physics.GetVelocityFromDistance_Linear(LinearDeltaDistance,Restraint,Restraint,dTime_s, m_vehicle.GetPhysics().GetLinearVelocity()));
	//This is more to test V1, and the flight dynamic version is preferred  (Note: this does look interesting though!)
	//osg::Vec3d linearvelocity(m_ChasePlane_Physics.GetVelocityFromDistance_Linear_v1(LinearDeltaDistance,Restraint,Restraint,dTime_s, m_vehicle.GetPhysics().GetLinearVelocity()));

	//This was the first attempt... it is good to keep for testing against match vel
	#if 0	
	osg::Vec3d linearvelocity(m_ChasePlane_Physics.GetVelocityFromDistance_Linear(LinearDeltaDistance,Restraint,Restraint,dTime_s,osg::Vec3d(0,0,0)));

	double VehiclesSpeed=m_vehicle.GetPhysics().GetLinearVelocity().length();
	//If the vehicle is very slow then we need not blend aggressive velocity (it will bounce around)
	if (false) // (VehiclesSpeed>0.1)
	{
		//We blend in the aggressive speed depending on the vehicles current acceleration
		double Diff;
		//We add in the aggressive speed depending on how close the velocities match each other
		Diff=m_ChasePlane_Physics.GetSpeed(m_vehicle.GetPhysics().GetLinearVelocity()-m_ChasePlane_Physics.GetLinearVelocity());

		//This may be computed with the vehicle speed, but distance works very well too
		double ratio=Diff<25.0+Distance?Diff/(25.0+Distance):1.0;
		//The faster the vehicle speed... the greater to spread out the distance ratio
		double DistRatio=Distance<VehiclesSpeed?Distance/VehiclesSpeed:1.0;

		//printf("\rSpeed %f %f %f %f                ",VehiclesSpeed,ChaseSpeed,Diff,ratio);
		//printf("\rSpeed %f %f %f                ",VehiclesSpeed,Diff,ratio);
		//DebugOutput("Speed %f %f %f %f\n",VehiclesSpeed,ChaseSpeed,Diff,ratio);
		osg::Vec3d AggressiveVelocity(LinearDeltaDistance/dTime_s);
		//First we blend the aggressive speed against the safe speed depending on how close the vehicle and chase speeds match up
		linearvelocity= (AggressiveVelocity*(1.0-ratio)) + (linearvelocity*ratio);
		//next we blend the current computed speed with the vehicles speed depending on how close the vehicle is to its target place
		//Doing this prevents the yo-yo effect
		linearvelocity= (m_vehicle.GetPhysics().GetLinearVelocity()*(1.0-DistRatio)) + (linearvelocity*DistRatio);
	}
	#endif

	osg::Vec3d ForceToApply(m_ChasePlane_Physics.GetForceFromVelocity(linearvelocity,dTime_s));
	//To avoid over compensation we boost the restraints even more!
	ForceToApply=m_ChasePlane_Physics.ComputeRestrainedForce(ForceToApply,Restraint*10.0,Restraint*10.0,dTime_s);
	//ForceToApply=m_ChasePlane_Physics.ComputeRestrainedForce(ForceToApply,Restraint,Restraint,dTime_s);
	//DOUT2("%f %f %f",ForceToApply[0],ForceToApply[1],ForceToApply[2]);
	m_ChasePlane_Physics.ApplyFractionalForce(ForceToApply,dTime_s);

	{
		//Run physics update for displacement
		osg::Vec3d PositionDisplacement;
		osg::Vec3d RotationDisplacement;
		m_ChasePlane_Physics.TimeChangeUpdate(dTime_s,PositionDisplacement,RotationDisplacement);
		//DOUT2("%f %f %f %f",dTime_s,PositionDisplacement[0],PositionDisplacement[1],PositionDisplacement[2]);

		osg::Vec3d eye=(camPos+PositionDisplacement);

		#if 1
		//Here is where we apply the fixed eye blend to the physics displacement eye
		if (m_FixedCameraBlend>0.0)
		{
			eye = ((eye *(1.0-m_FixedCameraBlend)) + (FixedEye * m_FixedCameraBlend) );
		}
		#endif
		//DOUT2("%f %f %f",LinearDeltaDistance[1],PositionDisplacement[1],FixedEye[1]-eye[1]);

		// And we use the intended orientation to set the look direction
		#ifdef __ApplyTorque__
		osg::Vec3d look = eye + (m_ChasePlane_Physics.GetQuat()*lookDir);
		osg::Vec3d up = m_ChasePlane_Physics.GetQuat()*upDir;
		#else
		osg::Vec3d look = eye + (GetIntendedOrientation()*lookDir);
		osg::Vec3d up = GetIntendedOrientation()*upDir;
		#endif


		// Build the Matrix
		osg::Matrix camMat;
		camMat.makeLookAt(eye, look, up);

		// Set the matrix and distortion
		activeCamera->SetMatrix(camMat, FindCameraDistortion());
	}
#endif
}
//////////////////////////////////////////////////////////////////////////

float ChasePlane_CamManipulator::FindCameraDistortion()
{
	// Base distortion on the size of the vehicle and the velocity, in a pretty abrupt curve
	float sizeFactor = 0.3;
	float speedFactor = 1.0 - 0.3;

	// Scale the size high for a fighter, nothing for a larger vehicle
	float sizeEffect = sizeFactor * sqrt(10.0f / m_vehicle.GetBoundRadius());

	// Find the amount of distortion (for now based on velocity)
	float speed = m_vehicle.GetLinearVelocity().length();
	float min = 500.0f;
	float band = 400.0f;
	float speedEffect(0.0);
	
	if (speed > (min+band))
		speedEffect = 1.0;
	else if (speed > min)
	{
		speedEffect = (speed - min)  / band;
		
		// Make a nicer ease in and ease out
		if (speedEffect < 0.5f)
		{
			speedEffect *= 2.0f;
			speedEffect *= speedEffect;
			speedEffect *= 0.5f;
		}
		else
		{
			speedEffect = 1.0f - speedEffect;
			speedEffect *= 2.0f;
			speedEffect *= speedEffect;
			speedEffect *= 0.5f;
			speedEffect = 1.0f - speedEffect;
		}
	}

	// Apply the speed factor
	speedEffect *= speedFactor;
	
	// We want the sum of the two
	return sizeEffect + speedEffect;
}
//////////////////////////////////////////////////////////////////////////

void ChasePlane_CamManipulator::POV_Turn(double dir,bool ByPassPhysics) 
{
	if (!ByPassPhysics)
		m_POV_rotVel_rad_s[0]=dir;
	else
	{
		//I shouldn't be cheating here, but I think this is a rare exception, if not then I can re-evaluate the interface
		osg::Quat &Cheating=const_cast<osg::Quat &>(m_POV_Physics.GetQuat());
		Cheating=FromLW_Rot_Radians(dir,0.0,0.0)*m_POV_Physics.GetQuat();

	}
}
//////////////////////////////////////////////////////////////////////////

void ChasePlane_CamManipulator::POV_Pitch(double dir,bool ByPassPhysics) 
{
	if (!ByPassPhysics)
		m_POV_rotVel_rad_s[1]=dir;
	else
	{
		//I shouldn't be cheating here, but I think this is a rare exception, if not then I can re-evaluate the interface
		osg::Quat &Cheating=const_cast<osg::Quat &>(m_POV_Physics.GetQuat());
		Cheating=FromLW_Rot_Radians(0.0,dir,0.0)*m_POV_Physics.GetQuat();
	}
}
//////////////////////////////////////////////////////////////////////////

osg::Vec3d ChaseVehicle_Imp::GetCockpitCtr()
{
	if (m_cockpitCtr.valid())
		return GG_Framework::UI::OSG::GetNodePosition(m_cockpitCtr.get(), NULL, m_bbActor.get());
	else return osg::Vec3d(0.0, 0.0, 0.0);
}
//////////////////////////////////////////////////////////////////////////

void ChaseVehicle_Imp::Reset(GG_Framework::Logic::Entity3D& entity)
{
	__super::Reset(entity);
	if (!m_bbActor)
	{
		m_bbActor = Self_Entity->Get_BB_Actor();
		ASSERT(m_bbActor);
		m_cockpitCtr = GG_Framework::UI::OSG::FindChildNode(m_bbActor.get(), "COCKPIT_CTR");

		// Base the offset positions on the bounding sphere radius
		// This can be overridden by IChaseVehicle
		double boundingSphereRadius = Self_Entity->GetBoundRadius();

		// There is no X offset (side-to-side)
		m_CamerOffset_UpClose[0] = m_CamerOffset_FarAway[0] = m_CamerOffset_JustRight[0] = 0.0;

		// The Z offset (up-down) is consistent between them all too.
		m_CamerOffset_UpClose[2] = m_CamerOffset_FarAway[2] = m_CamerOffset_JustRight[2] =boundingSphereRadius*0.600;

		// The distance behind changes between the three
		m_CamerOffset_JustRight[1] =	boundingSphereRadius*-3.0;
		m_CamerOffset_FarAway[1] =		boundingSphereRadius*-5.0;
		m_CamerOffset_UpClose[1] =		boundingSphereRadius*-2.0;

		// The camera offset starts in the Just right position
		m_CameraOffset = m_CamerOffset_JustRight;
	}
	m_LastVehicleVelocity=Self_Entity->GetPhysics().GetLinearVelocity();
	m_acceleratorBlend->Reset();
}
//////////////////////////////////////////////////////////////////////////

ChaseVehicle_Imp::ChaseVehicle_Imp() : m_acceleratorBlend(new Blend_Averager<double>(0.2))
{
}

ChaseVehicle_Imp::~ChaseVehicle_Imp()
{
	delete m_acceleratorBlend;
}


// Finds the fixed camera position, using the vehicle's position, the intended orientation, camera offset, and the cockpit position
void ChaseVehicle_Imp::GetLookAtVectors(bool usingPOV, const osg::Quat &intendedOrientation, double dTime_s, 
										osg::Vec3d& fixedEye, osg::Vec3d& lookDir, osg::Vec3d& upDir)
{
	osg::Vec3d CurrentVelocity=Self_Entity->GetPhysics().GetLinearVelocity();
	if (!usingPOV)
	{   
		//compute our camera offset; it is simple as applying a ratio to our max thrusts

		// Find the amount of acceleration / deceleration
		osg::Vec3d Acceleration;
		if (dTime_s>0.0) 
			Acceleration = (CurrentVelocity-m_LastVehicleVelocity)/dTime_s;

		double LocalAcceleration=m_acceleratorBlend->GetAverage(Acceleration*Self_Entity->GetDirection());

		// Use that in proportion to the maximum acceleration
		double mtf = GetMaxAccelForward();
		double mtr = GetMaxAccelReverse();
		
		// Find out how much we are pulling away or moving back based
		osg::Vec3d offsetVector;
		if ((LocalAcceleration>0) && (mtf > 0.0))
			offsetVector=(m_CamerOffset_FarAway-m_CamerOffset_JustRight)*LocalAcceleration/mtf;
		else if ((LocalAcceleration<0) && (mtr > 0.0))
			offsetVector=(m_CamerOffset_JustRight-m_CamerOffset_UpClose)*LocalAcceleration/mtr;

		m_CameraOffset=m_CamerOffset_JustRight+offsetVector;
	}
	else
	{
		m_CameraOffset=m_CamerOffset_JustRight;
	}

	//Keep this updated so that when we come back it doesn't freak out on the acceleration computation
	m_LastVehicleVelocity=CurrentVelocity;

	// The vectors we will return
	fixedEye = Self_Entity->GetPos_m() + (intendedOrientation*m_CameraOffset) + (Self_Entity->GetAtt_quat() * GetCockpitCtr());
	lookDir = osg::Vec3d(0.0,1.0,0.0);
	upDir = osg::Vec3d(0.0,0.0,1.0);
}
//////////////////////////////////////////////////////////////////////////
