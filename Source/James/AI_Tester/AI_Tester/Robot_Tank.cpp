#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace std;

const double PI=M_PI;
const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*															Robot_Tank																*/
/***********************************************************************************************************************************/

Robot_Tank::Robot_Tank(const char EntityName[]) : Ship_Tester(EntityName), m_LeftLinearVelocity(0.0),m_RightLinearVelocity(0.0)
{
}

void Robot_Tank::ResetPos()
{
	m_LeftLinearVelocity=m_RightLinearVelocity=0.0;
	__super::ResetPos();
}

void Robot_Tank::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	double TorqueRestrained=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);
	double LinearVelocityDelta;

	//First we compute the Y component force to the velocities in the direction that it currently is facing
	//I'm writing this out so I can easily debug
	{
		double AccelerationDelta=LocalForce[1]/Mass;
		LinearVelocityDelta=AccelerationDelta*dTime_s;
	}

	#if 0
	//determine direction
	double ForceHeading;
	{
		osg::Vec2d LocalForce_norm(LocalForce);
		LocalForce_norm.normalize();
		ForceHeading=atan2(LocalForce_norm[0],LocalForce_norm[1]);
		//DOUT2("x=%f y=%f h=%f\n",LocalForce[0],LocalForce[1],RAD_2_DEG(ForceHeading));
	}
	#endif
	double LeftDelta,RightDelta;
	//Now to blend the torque into the velocities
	{
		double Width=GetDimensions()[0];
		//first convert to angular acceleration
		double AccelerationDelta=TorqueRestrained/Mass;
		double AngularVelocityDelta=AccelerationDelta*dTime_s;
		//Convert the angular velocity into linear velocity
		double AngularVelocityDelta_linear=AngularVelocityDelta * Width;
		//I'm keeping this first attempt, I like it because it is simple and reliable however, when going forward in fast speeds the torque will clobber the
		//linear force with abrupt stopping 
		osg::Vec2d CurrentVelocity(m_LeftLinearVelocity,m_RightLinearVelocity);
		{
			//Scale down the amount of torque based on current speed... this helps not slow down the linear force when turning
			double FilterScaler=1.0 - (CurrentVelocity.length() / (ENGAGED_MAX_SPEED*2.0));
			AngularVelocityDelta_linear*=FilterScaler;
		}

		LeftDelta=(AngularVelocityDelta_linear/2)+LinearVelocityDelta;
		RightDelta=(-AngularVelocityDelta_linear/2)+LinearVelocityDelta;

		#if 1
		osg::Vec2d NewDelta(LeftDelta,RightDelta);
		for (size_t i=0;i<2;i++)
		{
			if (CurrentVelocity[i] * AngularVelocityDelta_linear >0.0)
			{
				//The left velocity is about to increase see if the linear velocity is going in the same direction
				if ((CurrentVelocity[i] * LinearVelocityDelta)>0.0)
					NewDelta[i]/=2;  //average out the linear and the angular
			}
			//Now to apply the final force restraint
			double Restraint=(CurrentVelocity[i]>0.0)?MaxAccelForward:MaxAccelReverse;
			if (Restraint>0.0)  //test for -1
			{
				Restraint=min(fabs(NewDelta[i]),Restraint);
				if (NewDelta[i]<0.0)
					Restraint*=-1.0;  //restore the negative sign
				NewDelta[i]=Restraint;
			}
		}
		LeftDelta=NewDelta[0];
		RightDelta=NewDelta[1];

		#endif


	}
	//Now to apply to the velocities with speed control  
	double NewVelocity=m_LeftLinearVelocity+LeftDelta;
	if (fabs(NewVelocity)>ENGAGED_MAX_SPEED)
		NewVelocity=(NewVelocity>0)?ENGAGED_MAX_SPEED:-ENGAGED_MAX_SPEED;
	m_LeftLinearVelocity=NewVelocity;

	NewVelocity=m_RightLinearVelocity+RightDelta;
	if (fabs(NewVelocity)>ENGAGED_MAX_SPEED)
		NewVelocity=(NewVelocity>0)?ENGAGED_MAX_SPEED:-ENGAGED_MAX_SPEED;
	m_RightLinearVelocity=NewVelocity;	

	//DOUT2("left=%f right=%f Ang=%f\n",m_LeftLinearVelocity,m_RightLinearVelocity,RAD_2_DEG(m_Physics.GetAngularVelocity()));
}

void Robot_Tank::InterpolateThrusterChanges(osg::Vec2d &LocalForce,double &Torque,double dTime_s)
{
	osg::Vec2d LocalVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity());
	double LeftLinearVelocity=m_LeftLinearVelocity,RightLinearVelocity=m_RightLinearVelocity;
	double LeftMagnitude=fabs(m_LeftLinearVelocity);
	double RightMagnitude=fabs(m_RightLinearVelocity);
	double CommonMagnitude=min(LeftMagnitude,RightMagnitude);
	double RightAngularDelta;
	double LeftAngularDelta;
	//We do not care about x, but we may want to keep an eye for intense x forces
	LocalForce[0]=0.0;
	double Width=GetDimensions()[0];
	double NewVelocityY;
	//See if velocities are going in the same direction
	if (m_LeftLinearVelocity * m_RightLinearVelocity >= 0)
	{
		//First lets simplify the the overall velocity by transferring the common speed to local force
		double CommonVelocity=m_LeftLinearVelocity>0?CommonMagnitude:-CommonMagnitude; //put back in the correct direction
		LeftLinearVelocity-=CommonVelocity;
		RightLinearVelocity-=CommonVelocity;
		NewVelocityY=CommonVelocity;
	}
	else
	{
		//going in different direction
		NewVelocityY=0;  //nothing to do... the common code will cancel them out
	}

	RightAngularDelta=LeftLinearVelocity / (Pi2 * Width);
	LeftAngularDelta=-(RightLinearVelocity / (Pi2 * Width));

	//We also need to add displacement of the left over turn..
	{
		double RAD_Slice=RightAngularDelta *Pi2 * dTime_s;
		double LAD_Slice=LeftAngularDelta * Pi2 * dTime_s;
		double Radius=Width/2.0;
		double Height=(sin(RAD_Slice) * Radius) + (sin(-LAD_Slice) * Radius);
		double Width=((1.0-cos(RAD_Slice))*Radius) + (-(1.0-cos(LAD_Slice))*Radius);
		double LinearAcceleration=Width-LocalVelocity[0];

		LocalForce[0]=(LinearAcceleration*Mass) / dTime_s;
		NewVelocityY+=(Height / dTime_s);
	}
	double LinearAcceleration=NewVelocityY-LocalVelocity[1];
	LocalForce[1]=(LinearAcceleration*Mass) / dTime_s;

	//Now then we'll compute the torque
	double AngularAcceleration=((LeftAngularDelta+RightAngularDelta)*Pi2) - m_Physics.GetAngularVelocity();
	Torque = (AngularAcceleration * Mass) / dTime_s;
}

void Robot_Tank::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	osg::Vec2d NewLocalForce(LocalForce);
	double NewTorque=Torque;
	InterpolateThrusterChanges(NewLocalForce,NewTorque,dTime_s);
	//No torque restraint... restraints are applied during the update of velocities
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,-1,dTime_s);
}

  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

FRC_2011_Robot::Robot_Arm::Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control) : 
	Ship_1D(EntityName),m_RobotControl(robot_control)
{
}
void FRC_2011_Robot::Robot_Arm::TimeChange(double dTime_s)
{
	//TODO add method to read height here
	__super::TimeChange(dTime_s);
	m_RobotControl->UpdateArmVelocity(m_Physics.GetLinearVelocity());
}

  /***********************************************************************************************************************************/
 /*															FRC_2011_Robot															*/
/***********************************************************************************************************************************/
FRC_2011_Robot::FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control) : 
	Robot_Tank(EntityName), m_RobotControl(robot_control), m_Arm(EntityName,robot_control)
{
}

void FRC_2011_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);
	m_Arm.Initialize(em);
}
void FRC_2011_Robot::ResetPos()
{
	__super::ResetPos();
	m_Arm.ResetPos();
}

void FRC_2011_Robot::TimeChange(double dTime_s)
{
	__super::TimeChange(dTime_s);
	Entity1D &arm_entity=m_Arm;  //This gets around keeping time change protected in derived classes
	arm_entity.TimeChange(dTime_s);
}

void FRC_2011_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	m_RobotControl->UpdateLeftRightVelocity(GetLeftVelocity(),GetRightVelocity());
}

  /***********************************************************************************************************************************/
 /*															Robot_Control															*/
/***********************************************************************************************************************************/

void Robot_Control::Initialize(const Entity_Properties *props)
{
	const Ship_Properties *ship_props=dynamic_cast<const Ship_Properties *>(props);
	assert(ship_props);
	m_ENGAGED_MAX_SPEED=ship_props->GetEngagedMaxSpeed();
}

void Robot_Control::UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity)
{
	DOUT2("left=%f right=%f \n",LeftVelocity/m_ENGAGED_MAX_SPEED,RightVelocity/m_ENGAGED_MAX_SPEED);
}
void Robot_Control::UpdateArmVelocity(double Velocity)
{
	DOUT4("Arm=%f",Velocity);
}
