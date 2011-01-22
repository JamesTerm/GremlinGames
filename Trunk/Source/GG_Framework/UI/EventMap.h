// GG_Framework.UI Interfaces.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API EventMap : public GG_Framework::Base::EventMap
		{
		public:
			EventMap(bool listOwned = false) : GG_Framework::Base::EventMap(listOwned) {}

			// Include point based events
			std::map<std::string,Event1<const osg::Vec3d&>,std::greater<std::string> > EventPt_Map;
			std::map<std::string,Event2<const osg::Vec3d&, bool>,std::greater<std::string> > EventPtOnOff_Map;
			std::map<std::string,Event2<const osg::Vec3d&, double>,std::greater<std::string> > EventPtValue_Map;
		};
	}
}