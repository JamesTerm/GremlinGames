#ifndef __FileDropSaveInterface__
#define __FileDropSaveInterface__

/*	Since DirectoryLayout saves LSD versions of files when they are dropped into it, you can use this
	interface to disable that property.
  */

class DirectoryLayoutSaveDisable
{	public:		virtual bool DirectoryLayoutSaveDisable_ShouldISave(void) { return false; }
};

#endif __FileDropSaveInterface__