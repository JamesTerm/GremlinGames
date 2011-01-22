TOPDIR = .
include $(TOPDIR)/Make/makedirdefs
include $(TOPDIR)/Make/makedefs


ifeq ($(OS),MINGW)
	SRC_DIR = win32_src
else
    	SRC_DIR = pthread_src
endif

DIRS = $(SRC_DIR) \
	$(NULL)

include $(TOPDIR)/Make/makedirrules
include $(TOPDIR)/Make/versionrules
include $(TOPDIR)/Make/docrules
include $(TOPDIR)/Make/instrules
include $(TOPDIR)/Make/distrules
include $(TOPDIR)/Make/helprules
