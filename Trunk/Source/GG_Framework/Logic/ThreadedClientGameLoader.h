// GG_Framework.Logic ThreadedClientGameLoader.h
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>

namespace GG_Framework
{
	namespace Logic
	{
		class FRAMEWORK_LOGIC_API ThreadedClientGameLoader : public GG_Framework::Base::ThreadedClass
		{
		public:
			ThreadedClientGameLoader(
				GG_Framework::Logic::UI_GameClient& gameClient);

			bool IsLoadComplete();
			std::string GetStatusUpdate();

		protected:
			// This is the part that is really run, surrounded by try/catch blocks
			void tryRun();
		private:
			IEvent::HandlerList ehl;
			void UpdateStatus(const char* status);
			volatile bool m_loadComplete;
			const char* m_status;
			OpenThreads::Mutex m_mutex;
			GG_Framework::Logic::UI_GameClient& m_gameClient;
		};
	}
}