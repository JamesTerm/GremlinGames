//This code has been depreciated; however I'm still keeping it around for a while in an ifdef 0 state
//This code will eventually be deleted
//  [12/12/2007] James

#if 0
class ThrustShip_Controller;
class ThrustShip : public Fringe::Base::Ship
{
private:
	ThrustShip_Controller* m_controller;

	double MAX_SPEED;
	double ACCEL;
	double BRAKE;
	double dHeading, dPitch, dRoll;

	double m_currAccel;

	osg::Vec3 m_origPos;
	osg::Vec3 m_origAtt;


public:
	ThrustShip() : m_controller(NULL), m_currAccel(0.0)
	{
	}

	virtual ~ThrustShip();

	enum Directions
	{
		Dir_None = 0,
		
		Dir_Up = -1,
		Dir_Down = 1,

		Dir_Left = 1,
		Dir_Right = -1,

		Dir_CW = 1,
		Dir_CCW = -1,
	};

	void Thrust(bool on)
	{
		m_currAccel = on ? ACCEL : 0.0;
	}

	void Brake(bool on)
	{
		m_currAccel = on ? -BRAKE : 0.0;
	}
	
	void Turn(Directions dir){m_rotVel_rad_s[0]=(double)dir*dHeading;}
	void Pitch(Directions dir){m_rotVel_rad_s[1]=(double)dir*dPitch;}
	void Roll(Directions dir){m_rotVel_rad_s[2]=(double)dir*-dRoll;}

	void ResetPos()
	{
		m_currAccel = 0.0;
		m_rotVel_rad_s = osg::Vec3(0,0,0);
		m_velocity_m_s = osg::Vec3(0,0,0);
		SetPosAtt(m_origPos, FromLW_Rot(m_origAtt[0], m_origAtt[1], m_origAtt[2]));
	}

protected:
	virtual void Initialize(GameScene& gs, GG_Framework::Base::EventMap& em, TransmittedEntity& te);
	virtual void TimeChange(double dTime_s)
	{
		if (m_currAccel != 0.0)
		{
			m_velocity_m_s += GetDirection()*m_currAccel;
			osg::Vec3 vel = m_velocity_m_s;
			if (vel.normalize() > MAX_SPEED)
				m_velocity_m_s = vel*MAX_SPEED;
		}

		// This is what updates the position based on these velocities
		Ship::TimeChange(dTime_s);
	}
};
//////////////////////////////////////////////////////////////////////////

class ThrustShip_Controller
{
private:
	ThrustShip& m_ship;
	IEvent::HandlerList ehl;

public:
	ThrustShip_Controller(ThrustShip& ship) : m_ship(ship)
	{
		// Hard code these key bindings at first
		EventMap* em = m_ship.GetEventMap();
		em->GetKeyBindingDnUp("Thrust").Key='w';
		em->GetKeyBindingDnUp("Brake").Key='s';
		em->GetKeyBindingDnUp("Turn_R").Key='d';
		em->GetKeyBindingDnUp("Turn_L").Key='a';
		em->GetKeyBindingDnUp("Roll_CW").Key='e';
		em->GetKeyBindingDnUp("Roll_CCW").Key='q';
		em->GetKeyBindingDnUp("Pitch_Up").Key='f';
		em->GetKeyBindingDnUp("Pitch_Dn").Key='r';
		em->GetKeyBinding("ResetPos").Key=' ';

		em->EventOnOff_Map["Thrust"].Subscribe(ehl, *this, &ThrustShip_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Subscribe(ehl, *this, &ThrustShip_Controller::Brake);
		em->EventOnOff_Map["Turn_R"].Subscribe(ehl, *this, &ThrustShip_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Subscribe(ehl, *this, &ThrustShip_Controller::Turn_L);
		em->EventOnOff_Map["Roll_CW"].Subscribe(ehl, *this, &ThrustShip_Controller::Roll_CW);
		em->EventOnOff_Map["Roll_CCW"].Subscribe(ehl, *this, &ThrustShip_Controller::Roll_CCW);
		em->EventOnOff_Map["Pitch_Up"].Subscribe(ehl, *this, &ThrustShip_Controller::Pitch_Up);
		em->EventOnOff_Map["Pitch_Dn"].Subscribe(ehl, *this, &ThrustShip_Controller::Pitch_Dn);
		em->Event_Map["ResetPos"].Subscribe(ehl, *this, &ThrustShip_Controller::ResetPos);
	}

	void Thrust(bool on){m_ship.Thrust(on);}
	void Brake(bool on){m_ship.Brake(on);}
	void Turn_R(bool on){m_ship.Turn(on?ThrustShip::Dir_Right:ThrustShip::Dir_None);}
	void Turn_L(bool on){m_ship.Turn(on?ThrustShip::Dir_Left:ThrustShip::Dir_None);}
	void Roll_CW(bool on){m_ship.Roll(on?ThrustShip::Dir_CW:ThrustShip::Dir_None);}
	void Roll_CCW(bool on){m_ship.Roll(on?ThrustShip::Dir_CCW:ThrustShip::Dir_None);}
	void Pitch_Up(bool on){m_ship.Pitch(on?ThrustShip::Dir_Up:ThrustShip::Dir_None);}
	void Pitch_Dn(bool on){m_ship.Pitch(on?ThrustShip::Dir_Down:ThrustShip::Dir_None);}
	void ResetPos(){m_ship.ResetPos();}
};
//////////////////////////////////////////////////////////////////////////

void ThrustShip::Initialize(GameScene& gs, GG_Framework::Base::EventMap& em, TransmittedEntity& te)
{
	Ship::Initialize(gs, em, te);
	MAX_SPEED = te.MAX_SPEED;
	ACCEL = te.ACCEL;
	BRAKE = te.BRAKE;
	dHeading = te.dHeading;
	dPitch = te.dPitch;
	dRoll = te.dRoll;

	m_origPos = FromLW_Pos(te.X, te.Y, te.Z);
	m_origAtt = osg::Vec3(te.Heading, te.Pitch, te.Roll);
	
	if (em.IsKB_Controlled())
		m_controller = new ThrustShip_Controller(*this);
}
//////////////////////////////////////////////////////////////////////////

ThrustShip::~ThrustShip()
{
	if (m_controller)
		delete m_controller;
}
//////////////////////////////////////////////////////////////////////////

ClassFactoryT<ThrustShip, Entity3D> ThrustShip_ClassFactory("ThrustShip", Entity3D::ClassFactory);
#endif