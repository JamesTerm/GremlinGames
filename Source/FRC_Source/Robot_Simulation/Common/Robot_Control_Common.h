#pragma once

//This parses out the LUA into two table for each control element... its population properties and LUT
class COMMON_API Control_Assignment_Properties
{
	public:
		struct Control_Element_1C
		{
			std::string name;
			size_t Channel;
			size_t Module;
		};
		struct Control_Element_2C
		{
			std::string name;
			size_t ForwardChannel,ReverseChannel;
			size_t Module;
		};
		typedef std::vector<Control_Element_1C> Controls_1C;
		typedef std::vector<Control_Element_2C> Controls_2C;

		virtual void LoadFromScript(Scripting::Script& script);

		const Controls_1C &GetVictors() const {return m_Victors;}
		const Controls_1C &GetDigitalInputs() const {return m_Digital_Inputs;}
		const Controls_2C &GetDoubleSolenoids() const {return m_Double_Solenoids;}
	private:
		Controls_1C m_Victors,m_Digital_Inputs;
		Controls_2C m_Double_Solenoids;
};
