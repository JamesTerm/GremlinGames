// GG_Framework.CenteredTrackball_CamManipulator.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

		class FRAMEWORK_UI_API CenteredTrackball_CamManipulator : 
			public GG_Framework::UI::OSG::TrackBall, public GG_Framework::UI::OSG::ICameraManipulator
		{
		public:
			CenteredTrackball_CamManipulator();

			void OnMouseMove(float mx, float my);
			void OnMouseBtnPress(float mx, float my, unsigned int button );
			void OnMouseBtnRelease(float mx, float my, unsigned int button );

			virtual void UpdateCamera(GG_Framework::UI::OSG::ICamera* activeCamera, double dTime_s);
			void SetCenteredNode(osg::Node* node);
			void ResetTB(){ResetFixed(false);}

			// Use this handler to tie events to this manipulator
			IEvent::HandlerList ehl;

		private:
			osg::ref_ptr<osg::Node> m_node;
			unsigned int _button_state;
			float _mx, _my;
		};

		class FRAMEWORK_UI_API TrackballDist_PDCB : public Text_PDCB
		{
		public:
			TrackballDist_PDCB(GG_Framework::UI::OSG::Producer_Trackball& trackball) : Text_PDCB(), m_trackball(trackball) {}

			// Use this handler to tie events to this callback to toggle
			IEvent::HandlerList ehl;

			virtual std::string GetText() const {return GG_Framework::Base::BuildString("%3.2f m", m_trackball.getDistance());}
			virtual osg::Vec2 GetPosition(double winWidth, double winHeight) const {return osg::Vec2(winWidth - 100.0, 20.0);}

		private:
			bool m_enabled;
			GG_Framework::UI::OSG::Producer_Trackball& m_trackball;
			GG_Framework::UI::OSG::Text m_text;
		};
	}
}
