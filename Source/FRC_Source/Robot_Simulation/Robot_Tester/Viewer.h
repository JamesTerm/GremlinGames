#pragma once

class Viewer
{
	public:
		Viewer(bool useUserPrefs=true) :m_Callback(NULL),m_UseSyntheticTimeDeltas(false),m_UseUserPrefs(useUserPrefs) {}
		void SetCallbackInterface(Viewer_Callback_Interface *callback) {m_Callback=callback;}
		void Start();
		void SetUseSyntheticTimeDeltas(bool UseSyntheticTimeDeltas) {m_UseSyntheticTimeDeltas=UseSyntheticTimeDeltas;}
		~Viewer();
	private:
		GG_Framework::UI::MainWindow *m_MainWin;
		//Exposing our node to render text
		osg::ref_ptr<osg::Group> m_RootNode;
		osg::ref_ptr<osg::Geode> m_Geode;
		Viewer_Callback_Interface *m_Callback;

		class ViewerCallback  : public osg::NodeCallback
		{
			public:
				ViewerCallback(Viewer *parent) : m_pParent(parent) {}
			protected:
				//From NodeCallback
				void operator()(osg::Node *node,osg::NodeVisitor *nv);
			private:
				Viewer * const m_pParent;
		};
		osg::ref_ptr <ViewerCallback> m_ViewerCallback;
		//This will make all time deltas the same length (ideal for debugging)
		bool m_UseSyntheticTimeDeltas;
		bool m_UseUserPrefs;
};
