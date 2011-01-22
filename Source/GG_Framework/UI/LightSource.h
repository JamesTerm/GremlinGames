// GG_Framework.UI LightSource.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

		class FRAMEWORK_UI_API LightSource : 
			public osg::LightSource, public IScriptOptionsLineAcceptor
		{
			public:
				// Pass in only the parameters, (E.G. fileLine+2)
				LightSource(ActorScene* actorScene, const char* params);

				virtual bool AcceptScriptLine(
					GG_Framework::UI::EventMap& eventMap, 
					char indicator,	//!< If there is an indicator character, the caps version is here, or 0 
					const char* lineFromFile);
		};


	}
}