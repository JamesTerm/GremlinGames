// ConfigFileIO.cpp
#include "stdafx.h"
#include "GG_Framework.Base.h"

using namespace std;

namespace GG_Framework
{
	namespace Base
	{

bool ConfigFileIO::ReadFileList(string configFilePath, vector<string>& fileList)
{
	ifstream inFile(configFilePath.c_str());
	if (inFile)
	{
		const unsigned MAX_PATH_SIZE = 1024;
		char buff[MAX_PATH_SIZE];

		while(inFile.getline(buff, MAX_PATH_SIZE))
			fileList.push_back(buff);
	}
	return (fileList.size() > 0);
}
//////////////////////////////////////////////////////////////////////////

void ConfigFileIO::WriteFileList(string configFilePath, vector<string>& fileList)
{
	ofstream outFile(configFilePath.c_str());
	if (outFile)
	{
		for (unsigned i = 0; i < fileList.size(); ++i)
			outFile << fileList[i] << endl;
	}
}
//////////////////////////////////////////////////////////////////////////

string ConfigFileIO::OpenFileDialog(string title, string startFile)
{
	return GG_Framework::Base::OpenFileDialog(title.c_str(), startFile.c_str());
}
//////////////////////////////////////////////////////////////////////////

bool ConfigFileIO::NoFilesLoaded_OK_Cancel()
{
	return (MessageBox(NULL, 
		"You must select at least one file.  Press OK to select a file or CANCEL to quit",
		"Select File or Quit?",
		MB_OKCANCEL) == IDOK);
}
//////////////////////////////////////////////////////////////////////////
	}
}
