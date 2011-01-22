// GG_Framework.UI Interfaces.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
		class ActorScene;
		class IScriptOptionsLineAcceptor
		{
		public:
			IScriptOptionsLineAcceptor(ActorScene* actorScene) : m_actorScene(actorScene) {}
			//! Return true if you accept this line
			virtual bool AcceptScriptLine(
				GG_Framework::UI::EventMap& eventMap, 
				char indicator,	//!< If there is an indicator character, the caps version is here, or 0 
				const char* lineFromFile
				) = 0;

			ActorScene* GetActorScene(){return m_actorScene;}

		private:
			ActorScene* const m_actorScene;
		};

	}
}

