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
			Nona_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
			~Nona_Robot();
	protected:
		virtual Swerve_Drive *CreateDrive() {return new Nona_Drive(this);}
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Nona_Robot_UI_Control : public Nona_Robot, public Swerve_Robot_Control
{
	public:
		Nona_Robot_UI_Control(const char EntityName[]) : Nona_Robot(EntityName,this),Swerve_Robot_Control(),
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
		Swerve_Robot_UI m_NonaUI;
};
