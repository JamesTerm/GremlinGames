#pragma once

class Robot_Control_Interface
{
public:
	//See FRC_2011_Robot for enumerations

	/// \param The index is ordinal enumerated to specific robot's interpretation
	/// \see subclass for enumeration specifics
	virtual void UpdateVoltage(size_t index,double Voltage)=0;
	/// \param The index is ordinal enumerated to specific robot's interpretation
	/// \see subclass for enumeration specifics
	virtual void CloseSolenoid(size_t index,bool Close)=0;
	virtual void OpenSolenoid(size_t index,bool Close)=0;
};

///TODO this one is still tunes to 2011 needs... I'll need to work out a way to make it more generic
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
