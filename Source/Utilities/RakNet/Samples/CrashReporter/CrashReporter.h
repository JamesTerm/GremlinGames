#ifndef __CRASH_REPORTER_H
#define __CRASH_REPORTER_H

// This is a crash reporter that will send a minidump by email on unhandled exceptions
// This normally only runs if you are not currently debugging.
// To send reports while debugging (mostly to test this class), define _DEBUG_CRASH_REPORTER
// and put your code in a try/except block such as
//
// extern void DumpMiniDump(PEXCEPTION_POINTERS excpInfo);
//
// void main(void)
//{
//__try 
//{
//	RunGame();
//}
//__except(DumpMiniDump(GetExceptionInformation()),EXCEPTION_EXECUTE_HANDLER) 
//{
//}
//}

// Possible actions to take on a crash.  If you want to restart the app as well, see the CrashRelauncher sample.
enum CrashReportAction
{
	// Send an email (mutually exclusive with AOC_EMAIL_WITH_ATTACHMENT)
	AOC_EMAIL_NO_ATTACHMENT=1,

	// Send an email and attach the minidump (mutually exclusive with AOC_EMAIL_NO_ATTACHMENT)
	AOC_EMAIL_WITH_ATTACHMENT=2,

	// Write the minidump to disk in a specified directory
	AOC_WRITE_TO_DISK=4,

	// In silent mode there are no prompts.  This is useful for an unmonitored application.
	AOC_SILENT_MODE=8
};

/// Holds all the parameters to CrashReporter::Start
struct CrashReportControls
{
	// Bitwise OR of CrashReportAction values to determine what to do on a crash.
	int actionToTake;

	// Used to generate the dump filename.  Required with AOC_EMAIL_WITH_ATTACHMENT or AOC_WRITE_TO_DISK
	char appName[128];
	char appVersion[128];

	// Used with AOC_WRITE_TO_DISK .  Path to write to.  Not the filename, just the path. Empty string means the current directory.
	char pathToMinidump[260];

	// Required with AOC_EMAIL_* & AOC_SILENT_MODE . The SMTP server to send emails from.
	char SMTPServer[128];

	// Required with AOC_EMAIL_* & AOC_SILENT_MODE . The account name to send emails with (probably your email address).
	char SMTPAccountName[64];

	// Required with AOC_EMAIL_* & AOC_SILENT_MODE . What to put in the sender field of the email.
	char emailSender[64];

	// Required with AOC_EMAIL_* .  What to put in the subject of the email.
	char emailSubjectPrefix[128];

	// Required with AOC_EMAIL_* as long as you are NOT in AOC_SILENT_MODE . What to put in the body of the email.
	char emailBody[1024];

	// Required with AOC_EMAIL_* . Who to send the email to.
	char emailRecipient[64];

	// How much memory to write. MiniDumpNormal is the least but doesn't seem to give correct globals. MiniDumpWithDataSegs gives more.
	int minidumpType;
};

/// \brief On an unhandled exception, will save a minidump and email it.
/// A minidump can be opened in visual studio to give the callstack and local variables at the time of the crash.
/// It has the same amount of information as if you crashed while debugging in the relevant mode.  So Debug tends to give
/// accurate stacks and info while Release does not.
///
/// Minidumps are only accurate for the code as it was compiled at the date of the release.  So you should label releases in source control
/// and put that label number in the 'appVersion' field.
class CrashReporter
{
public:
	static void Start(CrashReportControls *input);
	static CrashReportControls controls;
};

#endif
