// ConfigFileIO.h
#pragma once

namespace GG_Framework
{
	namespace Base
	{
		class FRAMEWORK_BASE_API ConfigFileIO : public IConfigFileIO
		{
		public:
			virtual bool ReadFileList(std::string configFilePath, std::vector<std::string>& fileList);
			virtual void WriteFileList(std::string configFilePath, std::vector<std::string>& fileList);
			virtual std::string OpenFileDialog(std::string title, std::string startFile);
			virtual bool NoFilesLoaded_OK_Cancel();
		};
	}
}