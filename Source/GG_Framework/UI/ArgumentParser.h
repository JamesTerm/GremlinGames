// GG_Framework.UI ArgumentParser.h
#pragma once
#include <osg/Notify>
#include <xmlParser.h>

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API ConsoleApp_ArgumentParser : public GG_Framework::Base::IArgumentParser
		{
		public:
			ConsoleApp_ArgumentParser() : GG_Framework::Base::IArgumentParser(), NotifySeverity(osg::FATAL), LogTimer(false) {}
			osg::NotifySeverity NotifySeverity;
			bool LogTimer;

		protected:
			virtual bool ProcessFlag(unsigned& index, unsigned argc, const char** argv);
		};
		//////////////////////////////////////////////////////////////////////////


		class FRAMEWORK_UI_API ViewerApp_ArgumentParser : public ConsoleApp_ArgumentParser
		{
		public:
			ViewerApp_ArgumentParser();
			~ViewerApp_ArgumentParser();

			bool UseAntiAliasing;
			bool MockAudio;
			std::string SoundTrack;
			unsigned RES_WIDTH, RES_HEIGHT;
			double MAX_FPS;

			void AttatchCamera(GG_Framework::UI::MainWindow* mainWindow, GG_Framework::Base::Timer* gameTimer);

		protected:
			virtual bool ProcessFlag(unsigned& index, unsigned argc, const char** argv);

		private:
			// To listen to the Timer Changes
			IEvent::HandlerList ehl;
			void GlobalTimeChangedCallback(double newTime_s);

			// We may want to abort run statistics to print out at the end
			std::string m_statsOutputFN;
			double m_startStats, m_endStats;
			double m_minFrameRate, m_maxFrameRate, m_avgFrameRate;
			double m_actualStart, m_lastTime;
			int m_numFrames;
			GG_Framework::UI::MainWindow* m_mainWindow;
		};

		/* I do not think this is ever even used
		class FRAMEWORK_UI_API XMLUserPrefs
		{
			public:
				// Constructor
				XMLUserPrefs (std::string xmlFileName = "userPrefs.xml") : m_topNode(XMLNode::parseFile(xmlFileName.c_str(), "root"))
				{
					if (m_topNode.isEmpty())
						m_topNode = XMLNode::createXMLTopNode("root");

					if (m_topNode.getChildNode("userPrefs").isEmpty())
						m_topNode.addChild("userPrefs");
					m_userPrefsNode = m_topNode.getChildNode("userPrefs");

					if (m_topNode.getChildNode("keyBindingPrefs").isEmpty())
						m_topNode.addChild("keyBindingPrefs");
					m_keyBindingPrefsNode = m_topNode.getChildNode("keyBindingPrefs");
				}
				// Destructor
				~XMLUserPrefs () { m_topNode.writeToFile("userPrefs.xml"); }

				// Accessors
				XMLNode& getm_keyBindingPrefsNode () { return m_keyBindingPrefsNode; }
				XMLNode& getm_userPrefsNode       () { return       m_userPrefsNode; }

			private:
				XMLNode m_topNode; // reference to the top node of the xml user preference file
				XMLNode m_userPrefsNode;
				XMLNode m_keyBindingPrefsNode;
		};
		*/
	}
}