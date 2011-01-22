// ConfigFileIO.cpp
#include "stdafx.h"
#include "GG_Framework.Base.h"

using namespace std;

namespace GG_Framework
{
	namespace Base
	{

string IConfigFileIO::GetConfigFileFromExeFile(const string exeFilePath)
{
	string configFilePath = exeFilePath.substr(exeFilePath.rfind('\\'));
	string::size_type idx = configFilePath.rfind('.');
	configFilePath.replace(idx, string::npos, ".config");

	// We want the full path of the current PWD
	char contentDIR[512];
	_getcwd(contentDIR, 512);
	configFilePath = std::string(contentDIR) + configFilePath;

	return configFilePath;
}
//////////////////////////////////////////////////////////////////////////

bool IConfigFileIO::GetFilesToLoad
	(const std::vector<std::string>& argv, std::vector<std::string>& fileList, 
		const char* firstFilePrompt, const char* addnFilePrompt)
{
	unsigned argc = (unsigned)argv.size();
	std::vector<std::string> fileListFromConfig;
	std::string config = GetConfigFileFromExeFile(argv[0]);
	ReadFileList(config, fileListFromConfig);
	
	unsigned passNum = 0;
	std::string file = "init";
	do
	{
		if (file == "")
		{
			if (!NoFilesLoaded_OK_Cancel())
				return false;
		}
		std::string fromConfig = argc > (passNum+1) ? argv[passNum+1] : 
			(fileListFromConfig.size() > passNum ? fileListFromConfig[passNum] : "");
		if (argc > (passNum+1))
		{
			// We were provided the argument, no need to ask the user again
			// We DO have to set the CWD though.
			char folderName[512];
			strcpy(folderName, fromConfig.c_str());
			char* lastSlash = GetLastSlash(folderName, NULL);
			if (lastSlash) *lastSlash = 0;
			chdir(folderName);
			file = fromConfig;
		}
		else
			file = OpenFileDialog(firstFilePrompt, fromConfig);
	}
	while (file == "");

	if (addnFilePrompt != NULL)
	{
		while (file != "")
		{
			fileList.push_back(file);
			passNum++;
			std::string fromConfig = argc > (passNum+1) ? argv[passNum+1] : 
				(fileListFromConfig.size() > passNum ? fileListFromConfig[passNum] : "");
			if (argc > (passNum+1))
				file = fromConfig;
			else
				file = OpenFileDialog(addnFilePrompt, fromConfig);
		}
	}
	else
		fileList.push_back(file);

	if (fileList.size() > 0)
		WriteFileList(config, fileList);

	return (fileList.size() > 0);
}
//////////////////////////////////////////////////////////////////////////

	}
}