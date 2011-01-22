// GG_Framework.UI SceneRunner.h
#include <osg/Timer>

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API ISceneRunner
		{
		public:
			GG_Framework::Base::KeyboardMouseEvents kbEvents;
			GG_Framework::Base::EventMap globalEventMap;

			virtual GG_Framework::Base::Timer& GetTimerEvent() = 0;
			Event1<bool&> Quit;
			bool IsQuitting(){return m_quitting;}

		public:
			ISceneRunner();
			void RunScene(osg::Node* rootNode, osg::Node* mainGroup,
				OSG::ICamera& cam, OSG::IKeyboardMouse& kbm);

		protected:
			// Return true to continue the loop
			virtual bool LoopUpdate(OSG::ICamera& cam);
			virtual void FireStart();

		private:
			IEvent::HandlerList ehl;
			virtual void OnEscape();
			bool m_quitting;
		};
		//////////////////////////////////////////////////////////////////////////

		class FRAMEWORK_UI_API OSG_Timer_SceneRunner : public ISceneRunner
		{
		public:
			virtual GG_Framework::Base::Timer& GetTimerEvent(){return m_timer;}

		protected:
			//! This versions sets the window title to be the Viewer's VPR
			virtual bool LoopUpdate(OSG::ICamera& cam);

			virtual void UpdateViewer(Viewer& viewer)
			{
				ISceneRunner::UpdateViewer(viewer);
				// Use a fixed Axis ritational mode for OSGV 
				viewer.GetTrackBall().setRotationalMode(Producer::Trackball::FixedAxis);
				viewer.GetKBMCB()->_arrowsChangeVPR = true;
			}

		private:
			OSG_Timer m_timer;
		};
		//////////////////////////////////////////////////////////////////////////
	}
}
