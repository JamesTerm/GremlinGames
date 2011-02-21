#ifndef INOUT_INTERFACE_H_
#define INOUT_INTERFACE_H_

/**
* Standard hobby style servo.
* 
* The range parameters default to the appropriate values for the Hitec HS-322HD servo provided
* in the FIRST Kit of Parts in 2008.
*/
class Servo
{
public:
	explicit Servo(UINT32 channel) {}
	Servo(UINT32 slot, UINT32 channel) {}
	virtual ~Servo() {}
	void Set(float value) {}
	void SetOffline() {}
	float Get() {return 0.0;}
	void SetAngle(float angle);
	float GetAngle() {return 0.0;}
	static float GetMaxAngle() { return 170.0; };
	static float GetMinAngle() { return 0.0; };
};

class Compressor
{
private:
	bool m_enabled;
public:
	Compressor(UINT32 pressureSwitchChannel, UINT32 compressorRelayChannel) {}
	Compressor(UINT32 pressureSwitchSlot, UINT32 pressureSwitchChannel,UINT32 compresssorRelaySlot, UINT32 compressorRelayChannel) {}
	~Compressor() {}
	void Start() {m_enabled=true;}
	void Stop() {m_enabled=false;}
	bool Enabled() {return m_enabled;}
	UINT32 GetPressureSwitchValue() {}
	//void SetRelayValue(Relay::Value relayValue) {}
};

class Robot_Control : public Robot_Control_Interface
{
	RobotDrive m_RobotDrive;
	RobotDrive m_ArmMotor;
	Compressor m_Compress;
	Solenoid m_OnClaw,m_OffClaw;
	Encoder m_LeftEncoder,m_RightEncoder;
	Servo m_DeployDoor;

	double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
	double m_ArmMaxSpeed;
	public:
		Robot_Control(bool UseSafety) : m_RobotDrive(1,2,3,4),m_ArmMotor(5,6),m_Compress(5,2),m_OnClaw(2),m_OffClaw(1),
			m_LeftEncoder(4,3,4,4),m_RightEncoder(4,1,4,2),m_DeployDoor(1)
		{
			m_Compress.Start();
			if (UseSafety)
			{
				//I'm giving a whole second before the timeout kicks in... I do not want false positives!
				//m_RobotDrive.SetExpiration(1.0);
				//m_RobotDrive.SetSafetyEnabled(true);
			}
			//else
				//m_RobotDrive.SetSafetyEnabled(false);
			m_LeftEncoder.Start(),m_RightEncoder.Start();
		}
		virtual ~Robot_Control() 
		{
			m_LeftEncoder.Stop(),m_RightEncoder.Stop();  //TODO Move for autonomous mode only
			//m_RobotDrive.SetSafetyEnabled(false);
			m_Compress.Stop();
		}
		virtual void Initialize(const Entity_Properties *props);
	protected: //from Robot_Control_Interface
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity);
		virtual void UpdateArmVelocity(double Velocity);
		virtual double GetArmCurrentPosition();
		virtual void CloseClaw(bool Close) {m_OnClaw.Set(Close),m_OffClaw.Set(!Close);}
		virtual void OpenDeploymentDoor(bool Open) {m_DeployDoor.SetAngle(Open?Servo::GetMaxAngle():Servo::GetMinAngle());}
};

class Driver_Station_Joystick : public Framework::Base::IJoystick
{	
	public:
		//Note: this current configuration requires the two joysticks reside in adjacent ports (e.g. 2,3)
		Driver_Station_Joystick(int NoJoysticks,int StartingPort);
		virtual ~Driver_Station_Joystick();
	protected:  //from IJoystick
		virtual size_t GetNoJoysticksFound();
		virtual bool read_joystick (size_t nr, JoyState &Info);
		
		virtual const JoystickInfo &GetJoyInfo(size_t nr) const {return m_JoyInfo[nr];}
	private:
		std::vector<JoystickInfo> m_JoyInfo;
		DriverStation *m_ds;
		int m_NoJoysticks,m_StartingPort;
};

#endif /*INOUT_INTERFACE_H_*/
