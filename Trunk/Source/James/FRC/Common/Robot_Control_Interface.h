#pragma once

//Note: On most (if not all) of these methods will have a size_t index as the first parameter.  This is enumerated to a specific robot's interpretation
//defined within that robots class

class Robot_Control_Interface
{
public:
	virtual void UpdateVoltage(size_t index,double Voltage) {}
	//Having both Open and Close makes it easier to make the desired call without applying the not operator
	virtual void CloseSolenoid(size_t index,bool Close) {}
	virtual void OpenSolenoid(size_t index,bool Open) {}
	/// \ret true if contact is made 
	virtual bool GetBoolSensorState(size_t index) {return false;}
};

///TODO this one is still tuned to 2011 needs... I'll need to work out a way to make it more generic
class Arm_Control_Interface
{
public:
	virtual void Reset_Arm(size_t index=0)=0; 

	///This is a implemented by reading the potentiometer and converting its value to correspond to the arm's current angle
	///This is in radians of the arm's gear ratio
	///TODO break this apart to reading pure analog values and have the potentiometer conversion happen within the robot
	virtual double GetArmCurrentPosition(size_t index=0)=0;
	virtual void UpdateArmVoltage(size_t index,double Voltage)=0;
	virtual void CloseRist(bool Close)=0;
	virtual void OpenRist(bool Close)=0;
};

class Rotary_Control_Interface
{
public:
	virtual void Reset_Rotary(size_t index=0)=0; 

	/// This is really called get rotary current position or velocity (depending on if we are linear or angular)
	/// current position:  (linear)
	///This is a implemented by reading the potentiometer and converting its value to correspond to the current angle in radians
	/// current velocity (angular)
	/// This is implemented by reading an encoder and converting its value to angular velocity also in radians
	/// \note: Any gearing must be settled within the robot control where the ratio satisfies the rotary system's ratio.  Therefore all conversion
	/// happens at the same place
	virtual double GetRotaryCurrentPorV(size_t index=0)=0;
	virtual void UpdateRotaryVoltage(size_t index,double Voltage)=0;
};
