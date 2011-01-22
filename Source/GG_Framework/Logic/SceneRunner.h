// GG_Framework.Logic SceneRunner.h

namespace GG_Framework
{
	namespace Logic
	{
		class FRAMEWORK_LOGIC_API GameSceneRunner : public GG_Framework::UI::ISceneRunner
		{
		public:
			GameSceneRunner() : m_gameScene(NULL) {}

			// Only call this after the Thread exits all loaded
			void MyRunScene(UI_GameScene& gameScene,
				GG_Framework::UI::OSG::ICamera& cam, GG_Framework::UI::OSG::IKeyboardMouse& kbm);

			virtual GG_Framework::Base::Timer& GetTimerEvent(){return m_timer;}
			GG_Framework::Logic::Network::SynchronizedTimer& GetSyncTimer(){return m_timer;}

		protected:
			virtual void FireStart();

		private:
			UI_GameScene* m_gameScene;
			GG_Framework::Logic::Network::SynchronizedTimer m_timer;
		};
		//////////////////////////////////////////////////////////////////////////

		class FRAMEWORK_LOGIC_API LogoSceneRunner : public GG_Framework::UI::OSG_Timer_SceneRunner
		{
		public:
			LogoSceneRunner(std::string gameName, ThreadedClientGameLoader& threadConn, ServerManager* threadAI=NULL);
			void MyRunScene(const char* fn, GG_Framework::UI::OSG::ICamera& cam, GG_Framework::UI::OSG::IKeyboardMouse& kbm);

		protected:
			virtual bool LoopUpdate(GG_Framework::UI::OSG::ICamera& cam);
			virtual void FireStart();

		private:
			ThreadedClientGameLoader& m_threadConn;
			ServerManager* const m_threadAI;
			std::string m_gameName;
		};
		//////////////////////////////////////////////////////////////////////////
	}
}
