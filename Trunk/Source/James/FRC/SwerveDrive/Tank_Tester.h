//This Tank_Robot.h version is stripped down to allow tank robot control to build
#pragma once
#define __SwerveDriveEnvironment__

#undef __Tank_UseScalerPID__
#undef __Tank_UseInducedLatency__
#undef __Tank_ShowEncoderPrediction__

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Tank_Drive_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void Tank_Drive_Control_TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Encoders()=0;

		//Encoders populate this with current velocity of motors
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)=0;  ///< in meters per second
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)=0;
};

struct Tank_Robot_Props
{
	typedef Framework::Base::Vec2d Vec2D;
	//typedef osg::Vec2d Vec2D;

	//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
	//the width is a measurement of the the center of the wheel width to the other wheel
	Vec2D WheelDimensions;
	double WheelDiameter;
	double VoltageScalar;		//Used to handle reversed voltage wiring
	double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
	double LeftPID[3]; //p,i,d
	double RightPID[3]; //p,i,d
	double InputLatency;  //Used with PID to help avoid oscillation in the error control (We can make one for each if needed)
	double HeadingLatency; //Should be about 100ms + Input Latency... this will establish intervals to sync up the heading with entity
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	double LeftMaxSpeedOffset;	//These are used to align max speed to what is reported by encoders (Encoder MaxSpeed - Computed MaxSpeed)
	double RightMaxSpeedOffset;
	double TankSteering_Tolerance; //used to help controls drive straight
	Vec2D DriveTo_ForceDegradeScalar;  //Used for way point driving in autonomous in conjunction with max force to get better deceleration precision
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	bool IsOpen;  //This property only applies in teleop
	bool PID_Console_Dump;  //This will dump the console PID info (Only active if __DebugLUA__ is defined)
	bool ReverseSteering;  //This will fix if the wiring on voltage has been reversed (e.g. voltage to right turns left side)
	//Note: I cannot imagine one side ever needing to be different from another (PID can solve if that is true)
	//Currently supporting 4 terms in polynomial equation
	double Polynomial[5];  //Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	//This may be computed from stall torque and then torque at wheel (does not factor in traction) to linear in reciprocal form to avoid division
	//or alternatively solved empirically.  Using zero disables this feature
	double InverseMaxAccel;  //This is used to solve voltage at the acceleration level where the acceleration / max acceleration gets scaled down to voltage
	//Different robots may have the encoders flipped or not which must represent the same direction of both treads
	//for instance the hiking viking has both of these false, while the admiral has the right encoder reversed
	bool LeftEncoderReversed,RightEncoderReversed;
};

class Tank_Robot_Properties : public Ship_Properties
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;

		Tank_Robot_Properties();
		virtual void LoadFromScript(Framework::Scripting::Script& script);
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
	protected:
		Tank_Robot_Props m_TankRobotProps;
	private:
		typedef Ship_Properties __super;
};
