void PhysicsEntity::SetOrientationHeading(const osg::Quat &qHeading,double DeltaTime_s)
{
	//protect ourself from division by zero
	if (DeltaTime_s==0.0)
	{
		assert(false);
		return;  //we just will not do anything for this session
	}
	//Get the current rotation (in radians)
	//TODO these Quat conversions are not quite correct... need to figure out how to get the correct conversions
#ifdef __UseEulerAngles__
	osg::Vec3 CurrentOrientation=QuatToEulerAngles(m_Parent->GetAtt_quat());
	osg::Vec3 Heading=QuatToEulerAngles(qHeading);
#else
	osg::Vec3 CurrentOrientation;
	osg::Vec3 Heading;

	//obviously the angle-axis is not what we want, but I want to keep it around for a little while
	{





		osg::Vec3 NativeOrientation;
		double Angle;
		qHeading.getRotate(Angle,NativeOrientation);
		Heading[0]=(NativeOrientation[2]*Angle);
		Heading[1]=Pi2-(NativeOrientation[0]*Angle);
		Heading[2]=Pi2-(NativeOrientation[1]*Angle);
		NormalizeVec3ToPi2(Heading);
		//pitch, roll, heading
		m_Parent->GetAtt_quat().getRotate(Angle,NativeOrientation);
		CurrentOrientation[0]=(NativeOrientation[2]*Angle);
		CurrentOrientation[1]=Pi2-(NativeOrientation[0]*Angle);
		CurrentOrientation[2]=Pi2-(NativeOrientation[1]*Angle);
		NormalizeVec3ToPi2(CurrentOrientation);
	}
#endif
	osg::Vec3 Distance;
	//Rick I tried this but it doesn't work :(
#if 1
	{
		//http://www.euclideanspace.com/physics/kinematics/angularvelocity/index.htm
		//W(t) = 2 *d q(t) /dt*conj(q(t))
		//grrr almost...
		osg::Quat qDifference=qHeading-m_Parent->GetAtt_quat();
		osg::Quat qDistance=((qHeading)*2.0)/(m_Parent->GetAtt_quat().conj()*DeltaTime_s);
		if (Heading!=m_LastHeading)
			DebugOutput(" x=%f,y=%f,z=%f,w=%f\t",qDistance[0],qDistance[1],qDistance[2],qDistance[3]);
		//Distance=QuatToEulerAngles(qDistance);
		osg::Vec3 NativeOrientation;
		double Angle;
		qDistance.getRotate(Angle,NativeOrientation);
		Distance[0]=(NativeOrientation[2]*Angle);
		Distance[1]=Pi2-(NativeOrientation[0]*Angle);
		Distance[2]=Pi2-(NativeOrientation[1]*Angle);
		NormalizeVec3ToPi2(Distance);
	}
#else
	{
		for (size_t i=0;i<3;i++)
		{
			double Theta=Heading[i]-CurrentOrientation[i];
			double IntDistance=fabs(Theta);
			double ExtDistance=Pi2-IntDistance;
			if (IntDistance<ExtDistance)
				Distance[i]=Theta;
			else
				Distance[i]=(Theta>0)?-ExtDistance:ExtDistance;
		}
	}
#endif
#ifdef __dumpHeading__
	//if (PosBNE(CurrentOrientation[0],0)||(PosBNE(CurrentOrientation[1],0))||(PosBNE(CurrentOrientation[2],0)))
	if (Heading!=m_LastHeading)
	{
		DebugOutput(" sx=%f,sy=%f,sz=%f\t",Distance[0],Distance[1],Distance[2]);
		//DebugOutput(" x=%f,y=%f,z=%f\t",CurrentOrientation[0],CurrentOrientation[1],CurrentOrientation[2]);
		DebugOutput(" x=%f,y=%f,z=%f\n",Heading[0],Heading[1],Heading[2]);
	}
	m_LastHeading=Heading;
#endif
	//Note: I may wish to apply a speed limit for stability for now its commented out to ensure other equations are working
	//osg::Vec3 vDesiredVelocity=Vec3_min(Distance/DeltaTime_s,osg::Vec3(Pi2,Pi2,Pi2));
	osg::Vec3 vDesiredVelocity=Distance/DeltaTime_s;


#ifndef  __TestHeadingDirect__
	//ApplyTorque(GetTorqueFromVelocity(vDesiredVelocity,DeltaTime_s));
#else
	//This is temporary to see what my heading is
	m_Parent->SetPosAtt(m_Parent->GetPos_m(),qHeading);
#endif

}





void ThrustShip2::TimeChange(double dTime_s)



#else
//add all the various input types to the main rotation velocity
m_rotVel_rad_s=m_Keyboard_rotVel_rad_s+m_JoyMouse_rotVel_rad_s;
//flush the JoyMouse rotation vec3 since it works on an additive nature
m_JoyMouse_rotVel_rad_s=osg::Vec3(0.0,0.0,0.0);

//TODO have keyboard and joystick manipulate camera 
const bool UsingMouse=(m_rotVel_rad_s[0]+m_rotVel_rad_s[1]==0);
#endif


if (!UsingMouse)
//Make our own writable copy of this so that we keep the member variable intact with the keyboard input
rotVel=m_rotVel_rad_s;
else

			//Now to compensate for any auto roll to keep nose level with both x and y axis
			if (!UsingMouse)
			{
				//Adding no force will return the current roll state
				double CurrentRollOffset=ComputeRollOffset(osg::Vec3(0,0,0));
				//We must determine the roll velocity and average out a good angle that will keep the nose in line during this time slice
				//TODO it is not quite right yet but getting closer
				double RollVel=(m_Physics.GetAngularVelocity()[2]);

				//To get the true velocity we *may* need to factor the next acceleration change of it.  
				//We only care about the roll component, so this function should work even though the yaw and pitch are not yet established
				//I'll have to work this out

				//osg::Vec3 TorqueToApply(m_Physics.GetTorqueFromVelocity(rotVel,dTime_s));
				//if (fabs(TorqueToApply[2])>MaxTorqueRoll)
				//	TorqueToApply[2]=TorqueToApply[2]>0? MaxTorqueRoll:-MaxTorqueRoll;

				//osg::Vec3 AccelerationDelta=m_Physics.GetAngularAccelerationDelta(TorqueToApply);
				//RollVel+=AccelerationDelta[2]*dTime_s;
				//RollVel*=dTime_s;
				//CurrentRollOffset+=RollVel;

				//CurrentRollOffset=(CurrentRollOffset + (CurrentRollOffset+RollVel))/2.0;
				CurrentRollOffset+=(RollVel*0.62);

				double Pitch=sin(-CurrentRollOffset)*rotVel[0]+cos(CurrentRollOffset)*rotVel[1];
				double Yaw=cos(-CurrentRollOffset)*rotVel[0]+sin(CurrentRollOffset)*rotVel[1];

				rotVel[0]=Yaw;
				rotVel[1]=Pitch;
				//double pitch=asin(sin(CurrentRollOffset*sin(rotVel[0])));  //grrrr this works with displacement
				#if 0
				if (fabs(RollVel)>0.0001)
				{
					//DebugOutput("before- Yaw=%f, Pitch=%f Roll=%f\n",rotVel[0],rotVel[1],CurrentRollOffset);
					DebugOutput("Yaw=%f, Pitch=%f Roll=%f %f RollVel=%f\n",rotVel[0],rotVel[1],CurrentRollOffset-RollVel,CurrentRollOffset,RollVel);
				}
				#endif
			}



	#else
		osg::Vec3 GlobalForce(0.0,0.0,9.80665*Mass);
		double UpDown=sin(-m_RollOffset)*GlobalForce[0]+cos(m_RollOffset)*GlobalForce[2];
		double LeftRight=cos(-m_RollOffset)*GlobalForce[0]+sin(m_RollOffset)*GlobalForce[2];
		osg::Vec3 ForceToApply(LeftRight,0.0,UpDown);
		return ForceToApply;
	#endif

		if (m_CoordinateTurns)
		{
			{
				double CurrentRollVelocity=m_Physics.GetAngularVelocity()[2];

				m_RollOffset+=(CurrentRollVelocity*dTime_s);
				if (m_RollOffset>Pi)
					m_RollOffset-=Pi2;
				else if (m_RollOffset<-Pi)
					m_RollOffset+=Pi2;

				if ((!m_LockRoll)&&(fabs(CurrentRollVelocity)<0.001))
				{
					m_RollOffset=0.0;
					m_LockRoll=true;
				}

			}

		}

		//while in automatic mode this keeps track of how far an "induced" roll has been applied, which helps to keep track of the original roll
		//orientation.  An induced roll usually occurs for auto coordinated turns when m_AlterTrajectory is being applied
		double m_RollOffset;








void ThrustShip_Controller2::CameraUpdate(const osg::Matrix& camMatrix)
{
	// Get the direction the camera is pointing so we can turn the ship
	osg::Vec3d camPos, camCenter, camUp;
	camMatrix.getLookAt(camPos, camCenter, camUp);
	osg::Vec3d lookDir = camCenter - camPos;


	////////////////////////////////////////////////////////////////////////////////////////////////
	//Default 0,1,0   Yaw,Pitch,Roll
	//Look up negative, down positive
	//Look left negative right positive


	//printf("\r %f %f %f camera                       ",lookDir[0],lookDir[1],lookDir[2]);
	osg::Vec3 CurrentOrientation(m_ship.GetAtt_quat().conj()*lookDir);
	//printf("\r %f %f %f ship                       ",CurrentOrientation[0],CurrentOrientation[1],CurrentOrientation[2]);
	double YawAngleOffset=atan2(CurrentOrientation[0],CurrentOrientation[1]);  //Angle in radians
	double PitchAngleOffset=atan2(CurrentOrientation[2],CurrentOrientation[1]);  //Angle in radians
	#if 0
	{
		printf("\r %f %f radians          ",m_YawAngleOffset,m_PitchAngleOffset);
		//printf("\r %f %f degrees          ",RAD_2_DEG(m_YawAngleOffset),RAD_2_DEG(m_PitchAngleOffset));

	}
	#endif
	m_ship.SetRotationDisplacement(-YawAngleOffset,-PitchAngleOffset);
}



{
	//We blend in the aggressive speed depending on how close the velocities match each other
	double ShipsSpeed=m_ship.m_Physics.GetSpeed(m_ship.m_Physics.GetLinearVelocity());
	double ChaseSpeed=m_ChasePlane_Physics.GetSpeed(m_ChasePlane_Physics.GetLinearVelocity());
	double Diff=fabs(ChaseSpeed-ShipsSpeed);
	double ratio=Diff<25.0?Diff/25.0:1.0;
	double DistRatio=Distance<20?Distance/20.0:1.0;
	ratio=(ratio+DistRatio)*0.5; //average the two ratio's
	//printf("\rSpeed %f                 ",ratio);
	printf("\rSpeed %f %f                 ",ShipsSpeed,ChaseSpeed);
	osg::Vec3d AggressiveVelocity(LinearDeltaDistance/dTime_s);
	linearvelocity= (AggressiveVelocity*(1.0-ratio)) + (linearvelocity*ratio);
}

{
	osg::Vec3 LocalForce;
	//osg::Vec3 GlobalForce(m_ChasePlane_Physics.GetForceFromVelocity(m_ship.GetLinearVelocity(),dTime_s));
	osg::Vec3 GlobalForce(m_ChasePlane_Physics.GetForceFromVelocity(m_ship.GetLinearVelocity()+linearvelocity,dTime_s));
	//so we'll need to convert to local
	LocalForce=m_ship.GetForwardReverseDirectionConj()*GlobalForce[1];
	LocalForce+=m_ship.GetUpDownDirectionConj()*GlobalForce[2];
	LocalForce+=m_ship.GetLeftRightDirectionConj()*GlobalForce[0];

	osg::Vec3 ForceRestraintPositive((m_ship.MaxThrustRight/m_ship.Mass)*Mass,(m_ship.MaxThrustForward/m_ship.Mass)*Mass,(m_ship.MaxThrustUp/m_ship.Mass)*Mass);
	osg::Vec3 ForceRestraintNegative((m_ship.MaxThrustLeft/m_ship.Mass)*Mass,(m_ship.MaxThrustReverse/m_ship.Mass)*Mass,(m_ship.MaxThrustDown/m_ship.Mass)*Mass);
	LocalForce=m_ChasePlane_Physics.ComputeRestrainedForce(LocalForce,ForceRestraintPositive,ForceRestraintNegative,dTime_s);

	osg::Vec3 ForceToApply=m_ship.GetDirection()*LocalForce[1];
	ForceToApply+=m_ship.GetUpDownDirection()*LocalForce[2];
	ForceToApply+=m_ship.GetLeftRightDirection()*LocalForce[0];

	m_ChasePlane_Physics.ApplyFractionalForce(ForceToApply,dTime_s);
}

//Here I attempted to use aggressive speed for a way to keep the ship more in line with its intended orientation
#if 1
if (m_UseHeadingSpeed || m_UsePitchSpeed)
{
	//We add in the aggressive speed depending on how close the velocities match each other
	osg::Vec3d Diff=m_Physics.GetAngularVelocity()-m_IntendedOrientationPhysics.GetAngularVelocity();

	for (size_t i=0;i<2;i++)
	{
		//This may be computed with the ship speed, but distance works very well too
		double ratio=fabs(Diff[i])<0.785?fabs(Diff[i])/0.785:1.0;
		//double IntendedShipsSpeed=m_IntendedOrientationPhysics.GetAngularVelocity()[i];
		//The faster the ship speed... the greater to spread out the distance ratio
		//double DistRatio=fabs(m_rotDisplacement_rad[i])<fabs(IntendedShipsSpeed)?fabs(m_rotDisplacement_rad[i])/fabs(IntendedShipsSpeed):1.0;

		//if (i==0)
		//	printf("\rSpeed %f %f %f                ",IntendedShipsSpeed,Diff,ratio);
		double AggressiveVelocity=m_rotDisplacement_rad[i]/dTime_s;
		//First we blend the aggressive speed against the safe speed depending on how close the ship and chase speeds match up
		rotVel[i]= (AggressiveVelocity*(1.0-ratio)) + (rotVel[i]*ratio);
		//next we blend the current computed speed with the ships speed depending on how close the ship is to its target place
		//Doing this prevents the yo-yo effect
		//rotVel[i]= (IntendedShipsSpeed*(1.0-DistRatio)) + (rotVel[i]*DistRatio);
	}
}
#endif 

//Here I tried to blend in the computed speed of the ship... which was off enough to not work
#if 1
if (m_UseHeadingSpeed || m_UsePitchSpeed)
{
	//Adding no force will return the current roll state
	double CurrentRollOffset=ComputeRollOffset(osg::Vec3(0,0,0));
	//We must determine the roll velocity and average out a good angle that will keep the nose in line during this time slice
	//TODO it is not quite right yet but getting closer
	double RollVel=(m_Physics.GetAngularVelocity()[2]);

	//CurrentRollOffset=(CurrentRollOffset + (CurrentRollOffset+RollVel))/2.0;
	CurrentRollOffset+=(RollVel*0.62);

	double IntendedShipsYaw=m_IntendedOrientationPhysics.GetAngularVelocity()[0];
	double IntendedShipsPitch=m_IntendedOrientationPhysics.GetAngularVelocity()[1];

	double Pitch=sin(-CurrentRollOffset)*IntendedShipsYaw+cos(CurrentRollOffset)*IntendedShipsPitch;
	double Yaw=cos(-CurrentRollOffset)*IntendedShipsYaw+sin(CurrentRollOffset)*IntendedShipsPitch;

	double IntendedShipsSpeed[2];
	IntendedShipsSpeed[0]=Yaw;
	IntendedShipsSpeed[1]=Pitch;

	//We add in the aggressive speed depending on how close the velocities match each other
	osg::Vec3d Diff=m_Physics.GetAngularVelocity()-m_IntendedOrientationPhysics.GetAngularVelocity();

	for (size_t i=0;i<2;i++)
	{
		//This may be computed with the ship speed, but distance works very well too
		double ratio=fabs(Diff[i])<0.785?fabs(Diff[i])/0.785:1.0;
		double IntendedShipsSpeedNoRoll=m_IntendedOrientationPhysics.GetAngularVelocity()[i];
		//The faster the ship speed... the greater to spread out the distance ratio
		double DistRatio=fabs(m_rotDisplacement_rad[i])<fabs(IntendedShipsSpeedNoRoll)?fabs(m_rotDisplacement_rad[i])/fabs(IntendedShipsSpeedNoRoll):1.0;

		//if (i==0)
		//	printf("\rSpeed %f %f %f                ",IntendedShipsSpeed,Diff,ratio);
		double AggressiveVelocity=m_rotDisplacement_rad[i]/dTime_s;
		//First we blend the aggressive speed against the safe speed depending on how close the ship and chase speeds match up
		rotVel[i]= (AggressiveVelocity*(1.0-ratio)) + (rotVel[i]*ratio);
		//next we blend the current computed speed with the ships speed depending on how close the ship is to its target place
		//Doing this prevents the yo-yo effect
		rotVel[i]= (IntendedShipsSpeed[i]*(1.0-DistRatio)) + (rotVel[i]*DistRatio);
	}
}
#endif 


//here I tried to use aggressive speed to overcompensate... it handled sharp turn cases poorly.

osg::Vec3d rotVel;
{
	const bool UseAggresiveSpeedBoost=true;
	if (UseAggresiveSpeedBoost)
	{
		//This method does not cheat... it will not keep a safe distance so it will be more aggressive, and over compensate when it does. 
		//Ideally there is a fine balance to allow some over compensation that is rare for extreme movement that should correct after one
		//pass.  The second pass should be half the distance, which should not have any aggression, therefore, no over-compensation

		osg::Vec3d Acceleration=(Ships_TorqueRestraint/Mass); //obtain acceleration

		for (size_t i=0;i<3;i++)
		{
			//first compute which direction to go
			double DistanceDirection=m_rotDisplacement_rad[i];
			if (DistanceDirection>Pi)
				DistanceDirection-=Pi2;

			double IdealSpeed=fabs(DistanceDirection/dTime_s);

			if (Ships_TorqueRestraint[i]!=-1)
			{
				//Given the distance compute the time needed
				//Time = sqrt( Distance / Acceleration  )
				double Time=sqrt(fabs(DistanceDirection/Acceleration[i]));
				//Now compute maximum speed for this time
				double MaxSpeed=Acceleration[i]*Time;

				double ratio=fabs(DistanceDirection/Pi);  //scale down to a 0 - 1 range

				if (ratio<0.5) //avoid jitters if we are close enough don't blend
					rotVel[i]=min(IdealSpeed,MaxSpeed); //This is how GetVelocityFromDistance computes it
				else
					//But now we will blend the aggression with the distance
				{
					ratio*=0.25; //lets scale this down to minimize overcompensation
					rotVel[i]=((1.0-ratio)*MaxSpeed)+(ratio*IdealSpeed);
				}
			}
			else
				rotVel[i]=IdealSpeed;  //i.e. god speed

			if (DistanceDirection<0)
				rotVel[i]=-rotVel[i];
		}
	}
	else
		rotVel=m_Physics.GetVelocityFromDistance(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s);




//Here is my first attempt to ramp up.  I almost had the parabola effect but it kept over-compensating
	GG_Framework::Logic::Physics::PhysicsEntity m_EnginePhysics;  //Seems like overkill, perhaps we could make a scaled down version of the physics
	osg::Vec3d m_EnginesCurrentAcceleration;  //This * mass is what will be submitted as force to apply


	m_EnginePhysics.ResetVectors();

	{ //Ramp up engine effect
		//We restrain the force down even more by using the computed ForceToApply as the influence against the previous existing force that was used
		//The idea here is that it takes a certain amount of time for an engine to power up or down to the influence.  One idea to think of is an 
		//electric motor where once you apply a new amount of voltage (the potential for doing work) it takes time for the current to build and for
		//the electro magnet to produce a stronger magnetic field, and add to this the the resistance of moment of inertia.  All these things add up to
		//to a logarithmic type of acceleration which can be simulated like how we deal with linear and angular motion

		//TODO script this one acceleration to represent the maximum number of mps^2 that the force can ramp up to
		double EngineAcceleration=1.0;
		//osg::Vec3d EngineForceRestraint(500.0*EngineAcceleration,500.0*EngineAcceleration,500.0*EngineAcceleration);
		osg::Vec3d EngineForceRestraint(-1,-1,-1);
		if ((m_controller)&&(strcmp(this->GetID().c_str(),"Q33_2")==0)&&(ForceToApply[0]==0))
			printf("\r %f %f %f              ",ForceToApply[0]/Mass,ForceToApply[1]/Mass,ForceToApply[2]/Mass);
		osg::Vec3d EngineForceToApply(m_EnginePhysics.GetForceFromVelocity(ForceToApply/Mass,dTime_s));
		EngineForceToApply=m_EnginePhysics.ComputeRestrainedForce(EngineForceToApply,EngineForceRestraint,EngineForceRestraint,dTime_s);
		m_EnginePhysics.ApplyFractionalForce(EngineForceToApply,dTime_s);
		osg::Vec3d PositionDisplacement;
		osg::Vec3d RotationDisplacement;
		m_EnginePhysics.TimeChangeUpdate(dTime_s,PositionDisplacement,RotationDisplacement);
		m_EnginesCurrentAcceleration+=PositionDisplacement;
		ForceToApply=m_EnginesCurrentAcceleration*Mass;
	}

	//This works, but the problem is that it takes way too long and cuts down on the engine's potential ramp up
#ifndef __DisableEngineRampUp__
	{
		double DeltaVelocity=(ForceToApply[1]-m_Physics.GetLinearVelocity()[1]);
		//A=Delta V / Delta T
		double IdealAcceleration=fabs(DeltaVelocity/dTime_s);  //This may be pretty quick
		//IdealAcceleration-=m_EnginesCurrentAcceleration[1]; //factor in where acceleration currently is
		double RestraintAcc=EngineForceRestraint[1]/c_EngineRampMass;

		double AccToUse;
		assert (EngineForceRestraint[1]!=-1);
		{
			//Given the distance compute the time needed
			//Time = sqrt( Distance / Acceleration  )
			double Time=sqrt(fabs(DeltaVelocity/RestraintAcc));
			//Now compute maximum speed for this time
			double MaxAcceleration=RestraintAcc*Time;
			AccToUse=min(IdealAcceleration,MaxAcceleration);
		}

		ForceToApply[1]=(ForceToApply[1]>0)?AccToUse:-AccToUse;


		//if (m_CurrentSpeed!=0)
		//	DebugOutput("%f \n",Acceleration[1]);
	}


#else
	{
		ForceToApply[0]=ForceToApply[2]=0;  //no strafes here
		osg::Vec3d GlobalVelocity(m_Physics.GetLinearVelocity());
		//convert to local
		osg::Vec3d LocalVelocity=(GetAtt_quat().conj() * GlobalVelocity);

		double DeltaVelocity=(m_CurrentSpeed-LocalVelocity[1]);
		//A=Delta V / Delta T
		double IdealAcceleration=DeltaVelocity/dTime_s;  //This may be pretty quick
		IdealAcceleration-=m_EnginesCurrentAcceleration[1]; //factor in where acceleration currently is
		bool IsNegative=IdealAcceleration<0;
		double RestraintAcc=EngineForceRestraint[1]/c_EngineRampMass;

		double AccToUse;
		assert (EngineForceRestraint[1]!=-1);
		{
			//Given the distance compute the time needed
			//Time = sqrt( Distance / Acceleration  )
			double Time=sqrt(fabs(DeltaVelocity/RestraintAcc));
			//Now compute maximum speed for this time
			double MaxAcceleration=RestraintAcc*Time;
			AccToUse=min(fabs(IdealAcceleration),MaxAcceleration);
			if (m_CurrentSpeed!=0)
				DebugOutput("%f %f ",IdealAcceleration,MaxAcceleration);

		}

		ForceToApply[1]=(IsNegative)?-AccToUse:AccToUse;
		if (m_CurrentSpeed!=0)
			DebugOutput("%f %f %f\n",ForceToApply[1],LocalVelocity[1],m_CurrentSpeed);
		ForceToApply[1]*=Mass;
	}
#endif

//This may be what I want if I can figure out why it doesn't work
	for (size_t i=0;i<3;i++)
	{
		// Provide other variables, just to make the math look cleaner
		double A = EngineForceRestraint[i]/c_EngineRampMass;
		double T = dTime_s;
		double D = localvelocityDelta[i];
		double Sc = m_EnginesCurrentAcceleration[i];

		// Watch for just needing to decelerate all the way to 0 as fast as we can
		if ((Sc > A*T) && ((A*T*T) > (2*D)))
			ForceToApply[i]=A*T;
		else	// There is some other speed we want
		{
			// Solve the quadratic to get the speed we need
			// ax^2 + bx + c = 0
			// x = (-b +-sqrt(b^2 - 4ac)) / 2a

			// (1/2A)x^2 + Tx - D = 0
			double a = 1.0 / (2.0*A);
			double b = T;
			double c = -D;

			// Solve for both possible results, we only need a positive one (
			double x1 = (-b + sqrt(b*b - 4*a*c)) / 2*a;
			double x2 = (-b - sqrt(b*b - 4*a*c)) / 2*a;
			ForceToApply[i]= ((x1 > 0) ? x1 : x2);
		}
	}


	//This was my first attempt... it could work but I found another way that could be better
#if 0
	//First get the direction
	osg::Vec3d LinearForce=-point;  //This is basically the opposite direction from 0 to point.
	//using a force restraint technique.  Use "LinearForce" variable as the restraint to compute the smallest ratio
	//The ratio should determine how much of the force is applied to linear and angular.
	double SmallestRatio=1.0;
	//Apply Torque restraints; This method computes the smallest ratio needed to scale down the vector.  It should give the maximum amount
	//of magnitude available without sacrificing the intended direction
	for (size_t i=0;i<3;i++)
	{
		//todo check for zero
		double ForceComponent=force[i];
		if (ForceComponent>LinearForce[i])
		{
			double Temp=LinearForce[i]/ForceComponent;
			SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
		}
	}
	//apply magnitude to the computed direction of "LinearForce" this will be the final force to be applied
	//The reciprocal of the ratio is the magnitude to be applied for the torque.  The direction is perpendicular to the final force
	//convert this to yaw pitch and roll components and apply torque
#endif 
