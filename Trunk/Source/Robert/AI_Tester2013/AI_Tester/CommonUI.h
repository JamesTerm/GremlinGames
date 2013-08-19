#pragma once

class Side_Wheel_UI
{
public:
	Side_Wheel_UI() : m_UIParent(NULL),m_Rotation(0.0) {}
	typedef osg::Vec2d Vec2D;
	typedef osg::Vec4d Vec4D;

	struct Wheel_Properties
	{
		Vec2D m_Offset;  //Placement of the wheel in reference to the parent object (default 0,0)
		Vec4D m_Color;
		const wchar_t *m_TextDisplay;
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
	void UpdatePosition(double x,double y) {m_props.m_Offset[0]=x,m_props.m_Offset[1]=y;}
	double GetFontSize() const {return m_UIParent?m_UIParent->GetFontSize():10.0;}
private:
	Actor_Text *m_UIParent;
	Wheel_Properties m_props;
	osg::ref_ptr<osgText::Text> m_Wheel; 
	double m_Rotation;

	Vec4D m_Color;
	const wchar_t *m_TextDisplay;
};

class Swivel_Wheel_UI
{
	public:
		Swivel_Wheel_UI() : m_UIParent(NULL) {}
		virtual ~Swivel_Wheel_UI() {}
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
		void SetRotation(double position) {m_Rotation=-position;}
		double GetFontSize() const {return m_UIParent?m_UIParent->GetFontSize():10.0;}
		enum WheelEnum
		{
			eFront, eBack, eTread
		};
		void SetWheelColor(osg::Vec4 Color, WheelEnum Wheel);

		virtual osg::Vec4 GetFrontWheelColor() const {return osg::Vec4(0.0,1.0,0.0,1.0);}
		virtual osg::Vec4 GetBackWheelColor() const {return osg::Vec4(1.0,0.0,0.0,1.0);}
	private:
		Actor_Text *m_UIParent;
		Wheel_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Front,m_Back,m_Tread; //Tread is really a line that helps show speed
		double m_Rotation,m_Swivel;
};
