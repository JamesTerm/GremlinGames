#pragma once


class FRC_2012_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
};

class FRC_2012_Robot : public Tank_Robot
{
	public:
		enum SpeedControllerDevices
		{
			eTurret,
			ePitchRamp,
			ePowerWheels
		};

		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		//TODO change UseEncoders to be passive
		FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:
		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		virtual void BindAdditionalEventControls(bool Bind);
	private:
		//typedef  Tank_Drive __super;
		FRC_2012_Control_Interface * const m_RobotControl;
		bool m_VoltageOverride;  //when true will kill voltage
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class FRC_2012_Robot_Control : public FRC_2012_Control_Interface
{
	public:
		FRC_2012_Robot_Control();
		//This is only needed for simulation
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		//Solenoid not used
		virtual void CloseSolenoid(size_t index,bool Close) {}
		virtual void OpenSolenoid(size_t index,bool Close) {}
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPosition(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {}

	protected: //from FRC_2011_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);

	protected:
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Potentiometer_Tester m_Potentiometer; //simulate a real potentiometer for calibration testing
		KalmanFilter m_KalFilter_Arm;
		//cache voltage values for display
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2012_Robot_UI : public FRC_2012_Robot, public FRC_2012_Robot_Control
{
	public:
		FRC_2012_Robot_UI(const char EntityName[]) : FRC_2012_Robot(EntityName,this),FRC_2012_Robot_Control(),
			m_TankUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_TankUI.TimeChange(dTime_s);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_TankUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_TankUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_TankUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_TankUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_TankUI.UpdateScene(geode,AddOrRemove);}

	private:
		Tank_Robot_UI m_TankUI;
};

class FRC_2012_Robot_Properties : public Tank_Robot_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		FRC_2012_Robot_Properties();
	private:
};
