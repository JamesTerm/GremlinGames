#define min(_x,_y) _x<_y?_x:_y
const float Pi2=PI * 2.0;
//The actual force between two objects are f=(G m1 m2)/ r^2
//For example g = ( G * (Me->5.98E+24)) / (Re->6.38E+6)^2 = 9.8 m/s^2
//G is ideal to compute forces on ships from various planets
//const float G=6.673E-11;

typedef struct
{
  float _v[2];
} Vec2d;

bool PosBNE(float val,float t)
{
	return !(abs(val-t)<.0001);
}

//Use this to check for zero if your value is going to be used as a denominator in division
bool IsZero(float value,float tolerance=.00001)
{
	return abs(value)<tolerance;
}


void Vec2d_Vec2d(Vec2d *this) {  this->_v[0]=0.0;  this->_v[1]=0.0;}
void Vec2d_Vec2d(Vec2d *this,float _x,float _y) { this->_v[0]=_x; this->_v[1]=_y; }

//Divide by scalar
void Vec2d_operator_divide (Vec2d *this,Vec2d &result,float rhs) //const accessor
{
    Vec2d_Vec2d(result,this->_v[0]/rhs, this->_v[1]/rhs);
}

//Multiply by scalar
void Vec2d_operator_multiply (Vec2d *this,Vec2d &result,float rhs) //const accessor
{
    Vec2d_Vec2d(result,this->_v[0]*rhs,this->_v[1]*rhs);
}

// Binary vector subtract.
void Vec2d_operator_subtract (Vec2d *this,Vec2d &result,const Vec2d& rhs) //const accessor
{
    Vec2d_Vec2d(result,this->_v[0]-rhs._v[0], this->_v[1]-rhs._v[1]);
}

// Length of the vector = sqrt( vec . vec )
float Vec2d_length(const Vec2d *this) //const accessor
{
    return sqrt( this->_v[0]*this->_v[0] +this->_v[1]*this->_v[1] );
}

//Length squared of the vector = vec . vec
float Vec2d_length2( Vec2d *this ) //const accessor
{
    return this->_v[0]*this->_v[0] + this->_v[1]*this->_v[1];
}

// Normalize the vector so that it has length unity.  Returns the previous length of the vector.
float Vec2d_normalize( Vec2d *this )
{
    float norm = Vec2d_length(this);
    if (norm>0.0)
    {
        float inv = 1.0/norm;
        this->_v[0] *= inv;
        this->_v[1] *= inv;
    }
    return( norm );
}


void Vec2Multiply (Vec2d &result,const Vec2d &_A,const Vec2d &rhs)
{
    Vec2d_Vec2d(&result,_A._v[0]*rhs._v[0], _A._v[1]*rhs._v[1]);
}

void Vec2_abs (Vec2d &result,const Vec2d &_A)
{
	Vec2d_Vec2d(&result,abs(_A._v[0]),abs( _A._v[1]));
}
void Vec2_min (Vec2d &result,const Vec2d &_A,const Vec2d &B)
{
	Vec2d_Vec2d(&result,min(_A._v[0],B._v[0]),min(_A._v[1],B._v[1]));
}



//Normalize it in the bound of Pi2
void NormalizeToPi2(float &A)
{
	if (A<0)
		A+=Pi2;
	else if (A>Pi2)
		A-=Pi2;
}

void NormalizeToPi2wDirection(float &A)
{
	if (A<-Pi2)
		A+=Pi2;
	else if (A>Pi2)
		A-=Pi2;
}

typedef struct
{
 		float m_EntityMass;
		float m_StaticFriction,m_KineticFriction;

		float m_AngularInertiaCoefficient;
		float m_RadiusOfConcentratedMass; //This is used to compute the moment of inertia for torque (default 1,1,1)

		Vec2d m_LinearVelocity;		///< This must represent global orientation for external forces to work properly
		float m_AngularVelocity;		///< All angle variables are in radians!

		///This variable is factored in but is managed externally
		Vec2d m_SummedExternalForces;
		float m_lastTime_s;
} PhysicsEntity_2D;

///You must set the mass otherwise you will get the default
void PhysicsEntity_2D_SetMass(PhysicsEntity_2D *this,float mass)
{
	this->m_EntityMass=mass;
}
float PhysicsEntity_2D_GetMass(PhysicsEntity_2D *this) //const
{
	return this->m_EntityMass;
}

void PhysicsEntity_2D_CopyFrom(PhysicsEntity_2D *this,const PhysicsEntity_2D& rhs)
{
	this->m_LinearVelocity= rhs.m_LinearVelocity;
	this->m_AngularVelocity= rhs.m_AngularVelocity;
}

void PhysicsEntity_2D_ResetVectors(PhysicsEntity_2D *this)
{
	Vec2d_Vec2d(this->m_LinearVelocity,0,0);
	this->m_AngularVelocity= 0.0;
}

void PhysicsEntity_2D_PhysicsEntity_2D(PhysicsEntity_2D *this)
{
	//Plug in some good defaults
	this->m_EntityMass=500; //about 5000 pounds
	//m_EntityMass=200; //about 2000 pounds
	this->m_StaticFriction=0.8;
	this->m_KineticFriction=0.2;
	this->m_AngularInertiaCoefficient=1.0;
	this->m_RadiusOfConcentratedMass=1.0;
	Vec2d_Vec2d(this->m_SummedExternalForces,0,0);
	this->m_lastTime_s=0.0;

	PhysicsEntity_2D_ResetVectors(this);
}

void PhysicsEntity_2D_SetFriction(PhysicsEntity_2D *this,float StaticFriction,float KineticFriction)
{
	this->m_StaticFriction=StaticFriction;
	this->m_KineticFriction=KineticFriction;
}

void PhysicsEntity_2D_SetAngularInertiaCoefficient(PhysicsEntity_2D *this,float AngularInertiaCoefficient)
{
	this->m_AngularInertiaCoefficient=AngularInertiaCoefficient;
}

void PhysicsEntity_2D_SetRadiusOfConcentratedMass(PhysicsEntity_2D *this,float RadiusOfConcentratedMass)
{
	this->m_RadiusOfConcentratedMass=RadiusOfConcentratedMass;
}

float PhysicsEntity_2D_GetRadiusOfConcentratedMass(PhysicsEntity_2D *this) //const  accessor
{
	return this->m_RadiusOfConcentratedMass;
}

void PhysicsEntity_2D_SetLinearVelocity(PhysicsEntity_2D *this, const Vec2d &LinearVelocity)
{
	this->m_LinearVelocity=LinearVelocity;
}
void PhysicsEntity_2D_GetLinearVelocity(PhysicsEntity_2D *this,Vec2d &result) //const accessor
{
	result= this->m_LinearVelocity;
}

void PhysicsEntity_2D_SetAngularVelocity(PhysicsEntity_2D *this, float AngularVelocity)
{
	this->m_AngularVelocity=AngularVelocity;
}

float PhysicsEntity_2D_GetAngularVelocity(PhysicsEntity_2D *this) //const accessor
{
	return this->m_AngularVelocity;
}

void PhysicsEntity_2D_ApplyFractionalForce(PhysicsEntity_2D *this, const Vec2d &force,float FrameDuration)
{
	//I'm writing this out so I can easily debug
	//Vec2d AccelerationDelta=force/this->m_EntityMass;
  Vec2d AccelerationDelta;
  Vec2d_operator_divide(force,AccelerationDelta,this->m_EntityMass);

	//Vec2d VelocityDelta=AccelerationDelta*FrameDuration;
  Vec2d VelocityDelta;
  Vec2d_operator_multiply(AccelerationDelta,VelocityDelta,FrameDuration);

	this->m_LinearVelocity+=VelocityDelta;  //TODO we may need += operator (test if this works)

	//if (AccelerationDelta[1]!=0)
	//	DebugOutput("Acc%f Vel%f\n",AccelerationDelta[1],m_LinearVelocity[1]);
}

float PhysicsEntity_2D_GetAngularAccelerationDelta(PhysicsEntity_2D *this,float torque,float RadialArmDistance)
{
	/* We want a cross product here, and divide by the mass and angular inertia
	return (RadialArmDistance^torque) / (m_EntityMass*m_AngularInertiaCoefficient);

	// [Rick Notes], Why divide by the arm distance?  Shouldn't we be multiplying?  Why square that, and along just the component?
	// We divide by I to solve for a... see formula below
	*/

	// t=Ia
	//I=sum(m*r^2) or sum(AngularCoef*m*r^2)

	float ret;
	{
		//Avoid division by zero... no radial arm distance no acceleration!
		if (RadialArmDistance==0)
		{
			ret=0;
			return ret;
		}
		//Doing it this way keeps the value of torque down to a reasonable level
		// [Rick Notes]  What does a "Reasonable Level" mean?  Perhaps we should see the equation somewhere
		// I forgot what the equation was and I get a bit lost.
		float RadiusRatio=(this->m_RadiusOfConcentratedMass*this->m_RadiusOfConcentratedMass/RadialArmDistance);
		ASSERT(RadiusRatio!=0);  //no-one should be using a zero sized radius!
		ret=(torque/(this->m_AngularInertiaCoefficient*this->m_EntityMass*RadiusRatio));
	}
	return ret;
}

void PhysicsEntity_2D_ApplyFractionalTorque(PhysicsEntity_2D *this, float torque,float FrameDuration,float RadialArmDistance)
{
	float AccelerationDelta=PhysicsEntity_2D_GetAngularAccelerationDelta(this,torque,RadialArmDistance);
	float VelocityDelta=AccelerationDelta*FrameDuration;
	this->m_AngularVelocity+=VelocityDelta;
}

float atan2(float _Y, float _X)
{
  float ret;
  if (_X!=0.0)
  {
    ret=atan(_Y/_X);
    if (_X<0)
      ret+=(_Y<0)?-PI:PI;
  }
  else
  {
    ret= (_Y>0)? PI/2 : -PI/2;
  }
  return ret;
}

void PhysicsEntity_2D_ApplyFractionalForce(PhysicsEntity_2D *this, const Vec2d &force, const Vec2d &point,float FrameDuration )
{
	//Use this as a "get by" if the code doesn't work properly
#if 0
	ApplyFractionalForce(force,FrameDuration);
	return;
#endif
	//Here is a rough draft to solve in 2 dimensions
	//A=atan2(py,px)   point
	//M=pi/2 - A
	//L=atan2(fy,fx)  force
	//N=L+M
	//Y=sin(N)*f.length = contribution for force
	//X=cos(N)*f.length = contribution for torque

	float TorqueToApply;
	Vec2d ForceToApply;
	float RadialArmDistance;

	{
		float A=atan2(point._v[1],point._v[0]);
		float M=(PI/2) - A;
		float L=atan2(-force._v[1],-force._v[0]);
		float N=L+M;

		float ForceLength= sqrt((force._v[1]*force._v[1])+(force._v[0]*force._v[0]));
		RadialArmDistance= sqrt((point._v[1]*point._v[1])+(point._v[0]*point._v[0]));
		//I've reserved a special case for ships which haven't specified  their radius size, in which case we simply factor out the radial arm too
		if ((this->m_RadiusOfConcentratedMass==1.0)&&(RadialArmDistance>1.0)) RadialArmDistance=1.0;

		//Fr = t   ... We should multiply force by the radial arm distance to get the torque
		//but instead,  we pass it off to physics where the multiply gets applied directly against the Radius of Concentrated Mass
		//We could multiply here but doing it the other way keeps the torque value low, which also makes it easier to debug
		TorqueToApply=(cos(N)*ForceLength);
	}

	//Vec2d vecToCenter = -point;
	Vec2d vecToCenter;
	Vec2d_Vec2d(vecToCenter,-point._v[0],-point._v[1]);
	//Note we should be able to support a point set at 0,0,0 in which case we use the force itself as the direction... otherwise a zero'd point
	//results in a zero'd vector which would omit applying the force
	if (Vec2d_length2(vecToCenter)==0.0)
		vecToCenter=-force;

	Vec2d_normalize(vecToCenter);

	//TODO in AI Tester, this should be using the vec2 multiply... need to support external collision test to fix
	//ForceToApply = vecToCenter * (force * vecToCenter);
  Vec2Multiply(ForceToApply,force,vecToCenter);
  Vec2Multiply(ForceToApply,vecToCenter,ForceToApply);

	PhysicsEntity_2D_ApplyFractionalForce(this,ForceToApply,FrameDuration);
	PhysicsEntity_2D_ApplyFractionalTorque(this,TorqueToApply,FrameDuration,RadialArmDistance);
}


void PhysicsEntity_2D_GetForceFromVelocity(PhysicsEntity_2D *this,Vec2d &result,const Vec2d &vDesiredVelocity,float DeltaTime_s)
{
	//Vec2d DeltaVelocity=(vDesiredVelocity-GetLinearVelocity());
  Vec2d DeltaVelocity;
  Vec2d_operator_subtract(DeltaVelocity,vDesiredVelocity,this->m_LinearVelocity);

	//A=Delta V / Delta T
	//Vec2d Acceleration=DeltaVelocity/DeltaTime_s;  //This may be pretty quick (apply Force restrictions later)
	Vec2d Acceleration;
	Vec2d_operator_divide(Acceleration,DeltaVelocity,DeltaTime_s);


	//if (Acceleration!=Vec2d(0,0,0))
	//	printf(" x=%f,y=%f,z=%f\n",Acceleration[0],Acceleration[1],Acceleration[2]);
	//Now that we know what the acceleration needs to be figure out how much force to get it there
	//Vec2d Force= Acceleration * m_EntityMass;
	Vec2d_operator_multiply(result,Acceleration,this->m_EntityMass);

	//if (PosBNE(Force[0],0)||(PosBNE(Force[1],0))||(PosBNE(Force[2],0)))
	//	printf("tx=%f,ty=%f,tz=%f\n",Force[0],Force[1],Force[2]);

	//if (PosBNE(Heading[2],0.0))
	//	DebugOutput(" s=%f,a=%f,w=%f,h=%f,z=%f,t=%f\n",Distance[2],m_AngularAcceleration[2],m_AngularVelocity[2],Heading[2],CurrentOrientation[2],Force[2]);

}

float PhysicsEntity_2D_GetTorqueFromVelocity(PhysicsEntity_2D *this, float vDesiredVelocity,float DeltaTime_s)
{

	//TODO input torque restraints from script (this would be due to the capabilities of the engines)
	//Note: desired speed is a separated variable controlled from the ship's speed script, which we fine tune given the torque restraints
	//And also by minimizing the amount of G's pulled at the outer most edge of the ship... so for large ships that rotate this could be
	//significant, and you wouldn't want people slamming into the walls.
	//Note: if the speed is too high and the torque restraint is too low the ship will "wobble" because it trying to to go a faster speed that it
	//can "brake" for... ideally a little wobble is reasonable and this is controlled by a good balance between desired speed and torque restraints


	float DeltaVelocity=(vDesiredVelocity-this->m_AngularVelocity);
	//A=Delta V / Delta T
	float Acceleration=DeltaVelocity/DeltaTime_s;  //This may be pretty quick (apply torque restrictions later)

	//if (Acceleration!=Vec2d(0,0,0))
	//	printf(" x=%f,y=%f,z=%f\n",Acceleration[0],Acceleration[1],Acceleration[2]);
	//Now that we know what the acceleration needs to be figure out how much force to get it there
	float Torque= Acceleration * this->m_EntityMass;
	//if (PosBNE(Torque[0],0)||(PosBNE(Torque[1],0))||(PosBNE(Torque[2],0)))
	//	printf("tx=%f,ty=%f,tz=%f\n",Torque[0],Torque[1],Torque[2]);

	//if (PosBNE(Heading[2],0.0))
	//	DebugOutput(" s=%f,a=%f,w=%f,h=%f,z=%f,t=%f\n",Distance[2],m_AngularAcceleration[2],m_AngularVelocity[2],Heading[2],CurrentOrientation[2],Torque[2]);

	return Torque;
}

#if 0
float PhysicsEntity_2D_GetVelocityFromDistance_Angular(float Distance,float Restraint,float DeltaTime_s)
{
	float ret;
	//This is how many radians the ship is capable to turn for this given time frame
	float Acceleration=(Restraint/m_EntityMass); //obtain acceleration

	{
		//first compute which direction to go
		float DistanceDirection=Distance;
		if (IsZero(DistanceDirection))
		{
			ret=0.0;
			continue;
		}

		if (DistanceDirection>PI)
			DistanceDirection-=Pi2;
		else if (DistanceDirection<-PI)
			DistanceDirection+=Pi2;
		float DistanceLength=fabs(DistanceDirection);

		float IdealSpeed=fabs(DistanceDirection/DeltaTime_s);

		if (Restraint!=-1)
		{
			//Given the distance compute the time needed
			//Time = sqrt( Distance / Acceleration  )
			//float Time=sqrt(fabs(DistanceDirection/Acceleration[i]));
			float Time=sqrt(2.0*(DistanceLength/Acceleration[i]));

			//Now compute maximum speed for this time
			float MaxSpeed=Acceleration*Time;
			ret=min(IdealSpeed,MaxSpeed);
		}
		else
			ret=IdealSpeed;  //i.e. god speed

		if (DistanceDirection<0)
			ret=-ret;
	}
	return ret;
}
#endif

float PhysicsEntity_2D_GetVelocityFromDistance_Angular(PhysicsEntity_2D *this,float Distance,float Restraint,float DeltaTime_s,float matchVel)
{
	float ret;

	//This is how many radians the ship is capable to turn for this given time frame
	float Acceleration=(Restraint/this->m_EntityMass); //obtain acceleration

	{
		//first compute which direction to go
		float DistanceDirection=Distance;
		DistanceDirection-=matchVel*DeltaTime_s;
		if (IsZero(DistanceDirection))
		{
			ret=matchVel;
			return ret;
		}

		//Unlike in the 3D physics, we'll need while loops to ensure all of the accumulated turns are normalized, in the 3D physics the
		//Quat is auto normalized to only require one if check here
		while (DistanceDirection>PI)
			DistanceDirection-=Pi2;
		while (DistanceDirection<-PI)
			DistanceDirection+=Pi2;
		float DistanceLength=abs(DistanceDirection);

		//Ideal speed needs to also be normalized
		float IDS=Distance;
		if (IDS>PI)
			IDS-=Pi2;
		else if (IDS<-PI)
			IDS+=Pi2;

		float IdealSpeed=abs(IDS/DeltaTime_s);

		if (Restraint!=-1)
		{
			//Given the distance compute the time needed
			//Place the division first keeps the multiply small
			float Time=sqrt(2.0*(DistanceLength/Acceleration));

			//Now compute maximum speed for this time
			float MaxSpeed=DistanceLength/Time;
			ret=min(IdealSpeed,MaxSpeed);

			if (DistanceDirection<0)
				ret=-ret;
			ret+=matchVel;
		}
		else
		{
			ret=IdealSpeed;  //i.e. god speed
			if (IDS<0)
				ret=-ret;
		}
	}
	return ret;
}

//TODO convert to C if used
#if 0
Vec2d PhysicsEntity_2D_GetVelocityFromCollision(const Vec2d &ThisVelocityToUse,float otherEntityMass,const Vec2d &otherEntityVelocity)
{
	//almost not quite
	//return (m_LinearVelocity*(m_EntityMass-otherEntityMass)) / (m_EntityMass+otherEntityMass);

	/// en.wikipedia.org/wiki/Elastic_collision
	// Here is the equation
	// ((vel1 ( mass1 - mass2 )) + (2 * mass2 * vel2))) / (m1+m2)
	Vec2d ret= (ThisVelocityToUse *(m_EntityMass-otherEntityMass));
	ret+= (otherEntityVelocity*(2 * otherEntityMass));
	ret/= (m_EntityMass+otherEntityMass);
	return ret;
}
#endif

//just like GetVelocityFromDistance_Angular except we do not normalize the DistanceDirection
void PhysicsEntity_2D_GetVelocityFromDistance_Linear(PhysicsEntity_2D *this,Vec2d &result,const Vec2d &Distance,const Vec2d &ForceRestraintPositive,const Vec2d &ForceRestraintNegative,float DeltaTime_s, const Vec2d& matchVel)
{
	//These are initialized as we go
	float Acceleration;
	float Restraint;

	for (int i=0;i<2;i++)
	{
		float DistanceDirection=Distance._v[i];
		DistanceDirection-=matchVel._v[i]*DeltaTime_s;
		if (IsZero(DistanceDirection))
		{
			result._v[i]=matchVel._v[i];
			continue;
		}
		float DistanceLength=abs(DistanceDirection);

		//Compose our restraint and acceleration based on the component direction
		Restraint=(DistanceDirection>0)?ForceRestraintPositive._v[i]:ForceRestraintNegative._v[i];
		Acceleration=(Restraint/this->m_EntityMass);

		float IdealSpeed=abs(Distance._v[i]/DeltaTime_s);

		if (Restraint!=-1)
		{
			//Given the distance compute the time needed
			//Place the division first keeps the multiply small
			float Time=sqrt(2.0*(DistanceLength/Acceleration));

			//Now compute maximum speed for this time
			float MaxSpeed=DistanceLength/Time;
			result._v[i]=min(IdealSpeed,MaxSpeed);

			if (DistanceDirection<0.0)
				result._v[i]=-result._v[i];
			result._v[i]+=matchVel._v[i];
		}
		else
		{
			result._v[i]=IdealSpeed;  //i.e. god speed
			if (Distance._v[i]<0.0)
				result._v[i]=-result._v[i];
		}
	}
}

//TODO convert to c if used
#if 0
Vec2d PhysicsEntity_2D_GetVelocityFromDistance_Linear_v1(const Vec2d &Distance,const Vec2d &ForceRestraintPositive,const Vec2d &ForceRestraintNegative,float DeltaTime_s, const Vec2d& matchVel)
{
	Vec2d ret;
	Vec2d DistToUse=Distance; //save the original distance as it is const anyhow

	DistToUse-=(matchVel*DeltaTime_s);
	float dDistance=DistToUse.length();
	if (IsZero(dDistance))
		return matchVel;

	//This is how many meters per second the ship is capable to stop for this given time frame
	Vec2d Restraint;
	//Compute the restraint based off of its current direction
	for (int i=0;i<3;i++)
		Restraint[i]=DistToUse[i]>0?ForceRestraintPositive[i]:ForceRestraintNegative[i];

	Vec2d Acceleration=(Restraint/m_EntityMass); //obtain acceleration

	float IdealSpeed=Distance.length()/DeltaTime_s;
	float AccelerationMagnitude=Acceleration.length();
	float Time=sqrt(2.0*(dDistance/AccelerationMagnitude));

	float MaxSpeed=dDistance/Time;
	float SpeedToUse=min(IdealSpeed,MaxSpeed);

	//DebugOutput("d=%f i=%f m=%f\n",Distance[1],IdealSpeed,MaxSpeed);
	//Now to give this magnitude a direction based of the velocity
	float scale=SpeedToUse/dDistance;
	ret=DistToUse*scale;
	ret+=matchVel;
	return ret;
}
#endif

float PhysicsEntity_2D_ComputeRestrainedTorque(float Torque,float TorqueRestraint,float dTime_s)
{
	float TorqueToApply=Torque;

	if (TorqueRestraint!=-1)
	{
		float SmallestRatio=1.0;
		{
			float AbsComponent=abs(TorqueToApply);
			if ((AbsComponent>TorqueRestraint) && (TorqueRestraint > 0.0))
			{
				float Temp=TorqueRestraint/AbsComponent;
				//SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;  //Not sure why this cannot compile
				if (Temp<SmallestRatio)
				  SmallestRatio=Temp;
			}
		}
		TorqueToApply*=SmallestRatio;
	}

	return TorqueToApply;
}


void PhysicsEntity_2D_ComputeRestrainedForce(PhysicsEntity_2D *this,Vec2d &result,const Vec2d &LocalForce,const Vec2d &ForceRestraintPositive,const Vec2d &ForceRestraintNegative,float dTime_s)
{
	result=LocalForce;
	if (ForceRestraintPositive._v[0]!=-1)
	{
		float SmallestRatio=1.0;
		//Apply Force restraints; This method computes the smallest ratio needed to scale down the vector.  It should give the maximum amount
		//of magnitude available without sacrificing the intended direction
		for (int i=0;i<2;i++)
		{
			float Temp;
			//separate the positive and negative coordinates
			if (LocalForce._v[i]>0)
			{
				if (LocalForce._v[i]>ForceRestraintPositive._v[i])
				{
					Temp=ForceRestraintPositive._v[i]/LocalForce._v[i];
					//SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
					if (Temp<SmallestRatio)
					  SmallestRatio=Temp;
				}
			}
			else
			{
				float AbsComponent=abs(LocalForce._v[i]);
				if (AbsComponent>ForceRestraintNegative._v[i])
				{
					Temp=ForceRestraintNegative._v[i]/AbsComponent;
					//SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
					if (Temp<SmallestRatio)
					  SmallestRatio=Temp;
				}
			}
		}
		//ForceToApply*=SmallestRatio;
		Vec2d_operator_multiply(result,result,SmallestRatio);
		//Test my restraints
		//printf("\r lr %f fr %f ud %f                ",LocalForce[0],LocalForce[1],LocalForce[2]);
	}
}

void PhysicsEntity_2D_TimeChangeUpdate(PhysicsEntity_2D *this,float DeltaTime_s,Vec2d &PositionDisplacement,float &RotationDisplacement)
{
	//Transfer the velocity to displacement
	RotationDisplacement = this->m_AngularVelocity * DeltaTime_s;
	//PositionDisplacement = this->m_LinearVelocity * DeltaTime_s;
	Vec2d_operator_multiply(PositionDisplacement,this->m_LinearVelocity,DeltaTime_s);
}
