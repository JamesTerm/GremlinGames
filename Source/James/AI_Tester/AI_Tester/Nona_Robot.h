#pragma once

class Butterfly_Robot : public Swerve_Robot
{
	public:
			Butterfly_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
			~Butterfly_Robot();
	protected:
		virtual Swerve_Drive *CreateDrive() {return new Butterfly_Drive(this);}
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Butterfly_Robot_UI_Control : public Butterfly_Robot, public Swerve_Robot_Control
{
	public:
		Butterfly_Robot_UI_Control(const char EntityName[]) : Butterfly_Robot(EntityName,this),Swerve_Robot_Control(),
			m_ButterflyUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_ButterflyUI.TimeChange(dTime_s);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_ButterflyUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_ButterflyUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_ButterflyUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_ButterflyUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_ButterflyUI.UpdateScene(geode,AddOrRemove);}

	private:
		Swerve_Robot_UI m_ButterflyUI;
};


//----------------------------------------------------------Nona-----------------------------------------------------------


class Nona_Robot : public Butterfly_Robot
{
	public:
		enum Nona_Robot_SpeedControllerDevices
		{
			eWheel_Kicker = Swerve_Robot::eNoSwerveRobotSpeedControllerDevices,
			eNoNonaRobotSpeedControllerDevices
		};

		Nona_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		~Nona_Robot();
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
	protected:
		virtual Swerve_Drive *CreateDrive();
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
	private:
		Rotary_Angular m_KickerWheel;  //apply control to kicker wheel
		Nona_Drive * const m_NonaDrive; //cache, avoid needing to dynamic cast each iteration
};

class Nona_Robot_Properties : public Swerve_Robot_Properties
{
	public:
		Nona_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);

		const Rotary_Properties &GetKickerWheelProps() const {return m_KickerWheelProps;}
	private:
		//Note the kicker wheel properties is a measurement of linear movement (not angular velocity)
		Rotary_Properties m_KickerWheelProps;
};

class Nona_Robot_Control : public Swerve_Robot_Control
{
	public:
		Nona_Robot_Control();
	protected: //from Rotary_Control_Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage);

		virtual void Swerve_Drive_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		virtual void Reset_Encoders();
	private:
		Encoder_Simulator2 m_KickerWheelEncoder;
		double m_KickerWheelVoltage;
};

class Omni_Wheel_UI
{
	public:
		Omni_Wheel_UI() : m_UIParent(NULL) {}
		typedef osg::Vec2d Vec2D;

		struct Wheel_Properties
		{
			Vec2D m_Offset;  //Placement of the wheel in reference to the parent object (default 0,0)
			double m_Wheel_Diameter; //in meters default 0.1524  (6 inches)
		};

		void UI_Init(Actor_Text *parent);

		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		//Keep virtual for special kind of wheels
		virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);
		//Where 0 is up and 1.57 is right and -1.57 is left
		void SetSwivel(double SwivelAngle){m_Swivel=-SwivelAngle;}
		//This will add to the existing rotation and normalize
		void AddRotation(double RadiansToAdd);
		double GetFontSize() const {return m_UIParent?m_UIParent->GetFontSize():10.0;}
	private:
		Actor_Text *m_UIParent;
		Wheel_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Front,m_Back,m_Tread; //Tread is really a line that helps show speed
		double m_Rotation,m_Swivel;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Nona_Robot_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		Nona_Robot_UI(Nona_Robot *NonaRobot) : m_NonaRobot(NonaRobot) {}
	public:
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void TimeChange(double dTime_s);

	protected:
		virtual void UpdateVoltage(size_t index,double Voltage) {}
		virtual void CloseSolenoid(size_t index,bool Close) {}

	private:
		Nona_Robot * const m_NonaRobot;
		Omni_Wheel_UI m_Wheel[5];
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Nona_Robot_UI_Control : public Nona_Robot, public Nona_Robot_Control
{
	public:
		Nona_Robot_UI_Control(const char EntityName[]) : Nona_Robot(EntityName,this),Nona_Robot_Control(),
			m_NonaUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_NonaUI.TimeChange(dTime_s);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_NonaUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_NonaUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_NonaUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_NonaUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_NonaUI.UpdateScene(geode,AddOrRemove);}

	private:
		Nona_Robot_UI m_NonaUI;
};
