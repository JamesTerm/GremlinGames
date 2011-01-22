
In here I've put some tools that I think can be useful.

The new load save is similar to BaseLoadSave that we had, but has been cleaned and conforms to using the wchar_t standard
This code has been used to cache files within filereading and has been clocked for excellent performence for quick file
retrival.  As part of writing a cache file there is an easy to mark a timestamp of when a cache file was created, and for
loading it will ensure the dates are the same as they were.

I've also included some useful macros to easily convert from wchar_t to char and vice versa.  This may be a handy thing we
may need to face especially in regards to porting in other code which uses different standards.

I should add that I put using namespace std... in the FrameWork_Win32.h file and not the stdafx.h.  To be safe I don't
want using namespace to corrupt my precompiled header file.  I also make use of including the compiler settings to disable
warnings that I know I do not care about.

