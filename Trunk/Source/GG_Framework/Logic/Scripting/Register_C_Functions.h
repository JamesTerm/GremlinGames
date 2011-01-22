// Register_C_Functions.h
#pragma once

// Anyone that registers 

namespace GG_Framework
{
	namespace Logic
	{
		namespace Scripting
		{
			class FRAMEWORK_LOGIC_SCRIPTING_API Register_C_Function
			{
			public:
				Register_C_Function(lua_CFunction func, const char* funcName);
				~Register_C_Function();

				static void RegisterAllFunctions(lua_State* lua_state);
			private:
				static OpenThreads::Mutex s_mutex;
				static std::vector<Register_C_Function*> C_funcs;
				std::string m_funcName;
				unsigned m_index;
				lua_CFunction m_func;
			};
		}
	}
}

#define REGISTER_C_SCRIPT_FUNC(funcName) GG_Framework::Logic::Scripting::Register_C_Function __RC##funcName(funcName, #funcName);
