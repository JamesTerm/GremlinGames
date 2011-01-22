// FileOpenSave.h

namespace GG_Framework
{
	namespace Base
	{
		//! Returns a static string with the filename or NULL if user canceled
		FRAMEWORK_BASE_API std::string OpenFileDialog(
			const char* title,
			const char* startFile);

		//! Returns a static string with the filename or NULL if user canceled
		FRAMEWORK_BASE_API std::string SaveFileDialog(
			const char* title,
			const char* startFile);
	};
};