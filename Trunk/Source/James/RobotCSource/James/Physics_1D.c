#define min(_x,_y) _x<_y?_x:_y

typedef struct
{
 		float m_EntityMass;
		float m_StaticFriction,m_KineticFriction;


		float m_LinearVelocity;		///< This must represent global orientation for external forces to work properly

		///This variable is factored in but is managed externally
		float m_SummedExternalForces;
		float m_lastTime_s;
} PhysicsEntity_1D;



bool PosBNE(float val,float t)
{
	return !(abs(val-t)<.0001);
}

//Use this to check for zero if your value is going to be used as a denominator in division
bool IsZero(float value,float tolerance=.00001)
{
	return abs(value)<tolerance;
}

void PhysicsEntity_1D_SetMass(PhysicsEntity_1D *this,float mass)
{
	this->m_EntityMass=mass;
}
float PhysicsEntity_1D_GetMass(PhysicsEntity_1D *this) //const accessor
{
	return this->m_EntityMass;
}

void PhysicsEntity_1D_CopyFrom(PhysicsEntity_1D *this,const PhysicsEntity_1D& rhs)
{
	this->m_LinearVelocity= rhs.m_LinearVelocity;
}

void PhysicsEntity_1D_ResetVectors(PhysicsEntity_1D *this)
{
	this->m_LinearVelocity= 0.0;
}

void PhysicsEntity_1D_PhysicsEntity_1D(PhysicsEntity_1D *this)
{
	//Plug in some good defaults
	this->m_EntityMass=5;  //about 11 pounds in weight
	this->m_StaticFriction=0.8;
	this->m_KineticFriction=0.2;
	this->m_SummedExternalForces=0.0;
	this->m_lastTime_s=0.0;

	PhysicsEntity_1D_ResetVectors(this);
}

void PhysicsEntity_1D_SetFriction(PhysicsEntity_1D *this,float StaticFriction,float KineticFriction)
{
	this->m_StaticFriction=StaticFriction;
	this->m_KineticFriction=KineticFriction;
}

void PhysicsEntity_1D_SetLinearVelocity(PhysicsEntity_1D *this,float LinearVelocity)
{
	this->m_LinearVelocity=LinearVelocity;
}
float PhysicsEntity_1D_GetLinearVelocity(PhysicsEntity_1D *this) //const accessor
{
	return this->m_LinearVelocity;
}

void PhysicsEntity_1D_ApplyFractionalForce(PhysicsEntity_1D *this,float force,float FrameDuration)
{
	//I'm writing this out so I can easily debug
	float AccelerationDelta=force/this->m_EntityMass;
	float VelocityDelta=AccelerationDelta*FrameDuration;
	this->m_LinearVelocity+=VelocityDelta;

	//if (AccelerationDelta[1]!=0)
	//	DebugOutput("Acc%f Vel%f\n",AccelerationDelta[1],this->m_LinearVelocity[1]);
}

float PhysicsEntity_1D_GetForceFromVelocity(PhysicsEntity_1D *this,float vDesiredVelocity,float DeltaTime_s)
{
	float DeltaVelocity=(vDesiredVelocity-this->m_LinearVelocity);
	//A=Delta V / Delta T
	float Acceleration=DeltaVelocity/DeltaTime_s;  //This may be pretty quick (apply Force restrictions later)


	//if (Acceleration!=osg::Vec2d(0,0,0))
	//	printf(" x=%f,y=%f,z=%f\n",Acceleration[0],Acceleration[1],Acceleration[2]);
	//Now that we know what the acceleration needs to be figure out how much force to get it there
	float Force= Acceleration * this->m_EntityMass;
	//if (PosBNE(Force[0],0)||(PosBNE(Force[1],0))||(PosBNE(Force[2],0)))
	//	printf("tx=%f,ty=%f,tz=%f\n",Force[0],Force[1],Force[2]);

	//if (PosBNE(Heading[2],0.0))
	//	DebugOutput(" s=%f,a=%f,w=%f,h=%f,z=%f,t=%f\n",Distance[2],this->m_AngularAcceleration[2],this->m_AngularVelocity[2],Heading[2],CurrentOrientation[2],Force[2]);

	return Force;
}

float PhysicsEntity_1D_GetVelocityFromCollision(PhysicsEntity_1D *this,float ThisVelocityToUse,float otherEntityMass,float otherEntityVelocity)
{
	//almost not quite
	//return (this->m_LinearVelocity*(this->m_EntityMass-otherEntityMass)) / (this->m_EntityMass+otherEntityMass);

	/// en.wikipedia.org/wiki/Elastic_collision
	// Here is the equation
	// ((vel1 ( mass1 - mass2 )) + (2 * mass2 * vel2))) / (m1+m2)
	float ret= (ThisVelocityToUse *(this->m_EntityMass-otherEntityMass));
	ret+= (otherEntityVelocity*(2 * otherEntityMass));
	ret/= (this->m_EntityMass+otherEntityMass);
	return ret;
}

float PhysicsEntity_1D_GetVelocityFromDistance_Linear(PhysicsEntity_1D *this,float Distance,float ForceRestraintPositive,float ForceRestraintNegative,float DeltaTime_s, float matchVel)
{
	float ret;
	float DistToUse=Distance; //save the original distance

	DistToUse-=(matchVel*DeltaTime_s);
	float Distance_Length=abs(DistToUse);
	if (IsZero(Distance_Length))
		return matchVel;

	//This is how many meters per second the ship is capable to stop for this given time frame
	//Compute the restraint based off of its current direction
	float Restraint=DistToUse>0?ForceRestraintPositive:ForceRestraintNegative;
	float Acceleration=(Restraint/this->m_EntityMass); //obtain acceleration

	float IdealSpeed=Distance_Length/DeltaTime_s;
	float AccelerationMagnitude=abs(Acceleration);
	float Time=sqrt(2.0*(Distance_Length/AccelerationMagnitude));

	float MaxSpeed=Distance_Length/Time;
	float SpeedToUse=min(IdealSpeed,MaxSpeed);

	//DebugOutput("d=%f i=%f m=%f\n",Distance[1],IdealSpeed,MaxSpeed);
	//Now to give this magnitude a direction based of the velocity
	float scale=SpeedToUse/Distance_Length;
	ret=DistToUse*scale;
	ret+=matchVel;
	return ret;
}

float PhysicsEntity_1D_ComputeRestrainedForce(float LocalForce,float ForceRestraintPositive,float ForceRestraintNegative,float dTime_s)
{
	//Note: This could be simplified; however, (at least for now) I am keeping it intact as it is in 2D to see how the function behaves
	float ForceToApply=LocalForce;
	if (ForceRestraintPositive!=-1)
	{
		float SmallestRatio=1.0;
		//Apply Force restraints; This method computes the smallest ratio needed to scale down the vector.  It should give the maximum amount
		//of magnitude available without sacrificing the intended direction
		{
			float Temp;
			//separate the positive and negative coordinates
			if (LocalForce>0)
			{
				if (LocalForce>ForceRestraintPositive)
				{
					Temp=ForceRestraintPositive/LocalForce;
					//SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
					if (Temp<SmallestRatio)
					  SmallestRatio=Temp;
				}
			}
			else
			{
				float AbsComponent=abs(LocalForce);
				if (AbsComponent>ForceRestraintNegative)
				{
					Temp=ForceRestraintNegative/AbsComponent;
					//SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
					if (Temp<SmallestRatio)
					  SmallestRatio=Temp;
				}
			}
		}
		ForceToApply*=SmallestRatio;
		//Test my restraints
		//printf("\r lr %f fr %f ud %f                ",LocalForce[0],LocalForce[1],LocalForce[2]);
	}
	return ForceToApply;
}

void PhysicsEntity_1D_TimeChangeUpdate(PhysicsEntity_1D *this,float DeltaTime_s,float &PositionDisplacement)
{
	//Transfer the velocity to displacement
	PositionDisplacement = this->m_LinearVelocity * DeltaTime_s;
}
