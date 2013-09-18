#pragma once

class Omni_Wheel_UI : public Swivel_Wheel_UI
{
public:
	virtual osg::Vec4 GetFrontWheelColor() const {return osg::Vec4(0.3,0.3,1.0,1.0);}
	virtual osg::Vec4 GetBackWheelColor() const {return osg::Vec4(0.1,0.1,1.0,1.0);}
};

class Butterfly_Robot_UI : public Swerve_Robot_UI
{
	public:
		typedef Butterfly_Robot::DriveMode DriveMode;
		Butterfly_Robot_UI(Swerve_Robot *SwerveRobot) : Swerve_Robot_UI(SwerveRobot) {}
		void DriveModeManager_SetMode_Callback(DriveMode Mode);
	protected:
		virtual Swivel_Wheel_UI *Create_WheelUI() {return new Omni_Wheel_UI;}
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Butterfly_Robot_UI_Control : public Butterfly_Robot, public Butterfly_Robot_Control
{
	public:
		Butterfly_Robot_UI_Control(const char EntityName[]) : Butterfly_Robot(EntityName,this),Butterfly_Robot_Control(),
			m_ButterflyUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_ButterflyUI.TimeChange(dTime_s);
			SetDisplayVoltage(m_controller->GetUIController()?true:false);
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
		virtual void DriveModeManager_SetMode_Callback(DriveMode Mode) 
		{	__super::DriveModeManager_SetMode_Callback(Mode);
			m_ButterflyUI.DriveModeManager_SetMode_Callback(Mode);
		}

	private:
		Butterfly_Robot_UI m_ButterflyUI;
};


//----------------------------------------------------------Nona-----------------------------------------------------------


//class Omni_Wheel_UI_Nona : public Wheel_UI
//{
//	public:
//		virtual osg::Vec4 GetFrontWheelColor() const {return osg::Vec4(0.3,0.3,1.0,1.0);}
//		virtual osg::Vec4 GetBackWheelColor() const {return osg::Vec4(0.1,0.1,1.0,1.0);}
//};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Nona_Robot_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		Nona_Robot_UI(Nona_Robot *NonaRobot) : m_NonaRobot(NonaRobot) {}
	public:
		typedef Butterfly_Robot::DriveMode DriveMode;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void TimeChange(double dTime_s);

		void DriveModeManager_SetMode_Callback(DriveMode Mode);
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
			SetDisplayVoltage(m_controller->GetUIController()?true:false);
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

		virtual void DriveModeManager_SetMode_Callback(DriveMode Mode) 
		{	__super::DriveModeManager_SetMode_Callback(Mode);
			m_NonaUI.DriveModeManager_SetMode_Callback(Mode);
		}
	private:
		Nona_Robot_UI m_NonaUI;
};
