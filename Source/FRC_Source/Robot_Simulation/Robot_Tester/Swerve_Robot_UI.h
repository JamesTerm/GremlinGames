#pragma once

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Swerve_Robot_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		Swerve_Robot_UI(Swerve_Robot *SwerveRobot);
		~Swerve_Robot_UI();
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
		virtual Swivel_Wheel_UI *Create_WheelUI() {return new Swivel_Wheel_UI;}
		virtual void Destroy_WheelUI(Swivel_Wheel_UI *wheel_ui) {delete wheel_ui;}
		//Allow subclasses to change wheels look
		Swivel_Wheel_UI *m_Wheel[4];
	private:
		Swerve_Robot * const m_SwerveRobot;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Swerve_Robot_UI_Control : public Swerve_Robot, public Swerve_Robot_Control
{
	public:
		Swerve_Robot_UI_Control(const char EntityName[]) : Swerve_Robot(EntityName,this),Swerve_Robot_Control(),
			m_SwerveUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_SwerveUI.TimeChange(dTime_s);
			SetDisplayVoltage(m_controller->GetUIController()?true:false);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_SwerveUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_SwerveUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_SwerveUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_SwerveUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_SwerveUI.UpdateScene(geode,AddOrRemove);}

	private:
		Swerve_Robot_UI m_SwerveUI;
};
