#pragma once


#ifdef Robot_TesterCode

//This is a simplified version of the wheel UI without the swivel or the graphics to show direction (only the tread)
class Tank_Wheel_UI
{
	public:
		Tank_Wheel_UI() : m_UIParent(NULL) {}
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
		//This will add to the existing rotation and normalize
		void AddRotation(double RadiansToAdd);
		double GetFontSize() const {return m_UIParent?m_UIParent->GetFontSize():10.0;}
	private:
		Actor_Text *m_UIParent;
		Wheel_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Tread; //Tread is really a line that helps show speed
		double m_Rotation;
};

class Tank_Robot_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		Tank_Robot_UI(Tank_Robot *tank_robot) : m_TankRobot(tank_robot) {}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void TimeChange(double dTime_s);
	private:
		Tank_Robot * const m_TankRobot;
		Tank_Wheel_UI m_Wheel[6];
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Tank_Robot_UI_Control : public Tank_Robot, public Tank_Robot_Control
{
	public:
		Tank_Robot_UI_Control(const char EntityName[]) : Tank_Robot(EntityName,this),Tank_Robot_Control(),
		m_TankUI(this) {}
	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_TankUI.TimeChange(dTime_s);
			SetDisplayVoltage(m_controller->GetUIController()?true:false);
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

#endif
