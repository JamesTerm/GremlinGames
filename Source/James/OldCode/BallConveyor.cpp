		class BallConveyorSystem
		{
			private:
				FRC_2012_Robot * const m_pParent;
				double GetFireDirection() const;
				//typedef Ship_1D __super;
				Rotary_Angular m_MainConveyor,m_FireConveyor;
				bool m_Grip,m_Squirt,m_Fire;
			public:
				BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				bool GetIsFireRequested() const {return m_Fire;}

				IEvent::HandlerList ehl;
				//public access needed for goals
				void Fire(bool on) {m_Fire=on;}
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on) {m_Grip=on;}
				void Squirt(bool on) {m_Squirt=on;}

				void ResetPos() {m_MainConveyor.ResetPos(),m_FireConveyor.ResetPos();}
				//Intercept the time change to send out voltage
				void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				void SetRequestedVelocity_FromNormalized(double Velocity);
		};



/***********************************************************************************************************************************/
 /*												FRC_2012_Robot::BallConveyorSystem													*/
/***********************************************************************************************************************************/

FRC_2012_Robot::BallConveyorSystem::BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),
	m_MainConveyor("MainConveyor",robot_control,eMainConveyor),m_FireConveyor("FireConveyor",robot_control,eFireConveyor),
		m_Grip(false),m_Squirt(false),m_Fire(false)
{
}

void FRC_2012_Robot::BallConveyorSystem::Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props)
{
	//These share the same props and fire is scaled from this level
	m_MainConveyor.Initialize(em,props);
	m_FireConveyor.Initialize(em,props);
}

double FRC_2012_Robot::BallConveyorSystem::GetFireDirection() const
{
	double FireScaler=m_Fire|m_Squirt?-1.0:1.0;  
	return FireScaler;
}

void FRC_2012_Robot::BallConveyorSystem::TimeChange(double dTime_s)
{
	const double PowerWheelSpeedDifference=m_pParent->m_PowerWheels.GetRequestedVelocity_Difference();
	const bool PowerWheelReachedTolerance=fabs(PowerWheelSpeedDifference)<m_pParent->m_PowerWheels.GetRotary_Properties().PrecisionTolerance;
	//Only fire when the wheel has reached its aiming speed
	bool Fire=m_Fire && PowerWheelReachedTolerance;

	double FireScaler=GetFireDirection();
	//This assumes the motors are in the same orientation: 
	double ConveyorVelocity=((m_Grip ^ m_Squirt) | Fire)?((m_Grip|Fire)?m_MainConveyor.GetACCEL():-m_MainConveyor.GetBRAKE()):0.0;
	if (fabs(m_MainConveyor.GetPhysics().GetVelocity())< m_MainConveyor.GetMaxSpeed() * 0.80)
		m_MainConveyor.SetCurrentLinearAcceleration(ConveyorVelocity);
	m_FireConveyor.SetCurrentLinearAcceleration(ConveyorVelocity * FireScaler);
	m_MainConveyor.AsEntity1D().TimeChange(dTime_s);
	m_FireConveyor.AsEntity1D().TimeChange(dTime_s);
}

void FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized(double Velocity)
{
	m_MainConveyor.SetRequestedVelocity_FromNormalized(Velocity);
	m_FireConveyor.SetRequestedVelocity_FromNormalized(Velocity*GetFireDirection());
}

void FRC_2012_Robot::BallConveyorSystem::BindAdditionalEventControls(bool Bind)
{
	GG_Framework::Base::EventMap *em=m_MainConveyor.GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
	else
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
}

  /***************************************************************************************************************/
 /*											FRC_2012_Main_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Main_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(-0.75,-1.25);
	Myprops.m_Color=osg::Vec4(1.0,1.0,0.5,1.0);
	Myprops.m_TextDisplay=L"|";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Main_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eMainConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}

  /***************************************************************************************************************/
 /*											FRC_2012_Fire_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Fire_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.75,-1.35);
	Myprops.m_Color=osg::Vec4(0.0,1.0,0.5,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Fire_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eFireConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}
