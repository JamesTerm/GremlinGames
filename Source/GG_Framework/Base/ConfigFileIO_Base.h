// ConfigFileIO_Base.h
#pragma once

#include <vector>

namespace GG_Framework
{
	namespace Base
	{

class FRAMEWORK_BASE_API IConfigFileIO
{
public:
	IConfigFileIO(){}
	virtual ~IConfigFileIO(){}

	bool GetFilesToLoad(const std::vector<std::string>& argv, std::vector<std::string>& fileList, 
		const char* firstFilePrompt, const char* addnFilePrompt);
	std::string GetConfigFileFromExeFile(const std::string exeFilePath);


	//! \brief Attempt to read a list of files from a configuration file
	//! \pre configFilePath is a valid path, but the file may not exist
	//! \pre fileList is an empty vector used to return the full path names written in the config
	//!			file.  The files need no longer exist, but the names should be valid
	//! \post returns true iff fileList has atleast one filename in it
	virtual bool ReadFileList(std::string configFilePath, std::vector<std::string>& fileList) = 0;
	
	//! \brief Write a list of files to a configuration file
	//! \pre configFilePath is a valid path, but the file may not exist
	//! \pre fileList has at least one valid string to write to the file
	//! \post The file at configFilePath is written with files from fileList in such a way that 
	//			ReadFileList could read it
	virtual void WriteFileList(std::string configFilePath, std::vector<std::string>& fileList) = 0;

	//! \brief Allows the user to accept the file startFile, cancel, or choose another file
	//! \pre startFile may or may not be a valid absolute or relative path
	//! \return The file the user wants, or "" for cancel
	virtual std::string OpenFileDialog(std::string title, std::string startFile) = 0;

	//! Called when the user cancels the first call to OpenFileDialog.
	//! Prompt the user that not selecting at least one file would abort the program.
	virtual bool NoFilesLoaded_OK_Cancel() = 0;
};

	}
}
