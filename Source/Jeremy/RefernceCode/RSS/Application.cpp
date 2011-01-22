#include "StdAfx.hpp"
#include "Application.hpp"
#include "RssReaderFrame.hpp"

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>
#include <wx/fs_inet.h>

namespace RssReader
{
	Application::Application() :
		wxApp(),
		_curlGlobal(),
		_xmlGlobal()
	{
	}

	bool Application::OnInit()
	{
		if (!wxApp::OnInit())
			return false; 

		wxInitAllImageHandlers();
		#if wxUSE_FS_INET && wxUSE_STREAMS && wxUSE_SOCKETS
			wxFileSystem::AddHandler(new wxInternetFSHandler);
		#endif

		wxXmlResource::Get()->InitAllHandlers();
		wxXmlResource::Get()->Load(wxT("RssReaderLayout.xrc"));

		RssReaderFrame* const frame = new RssReaderFrame();
		frame->Show();
		SetTopWindow(frame);

		return true;
	}
}
