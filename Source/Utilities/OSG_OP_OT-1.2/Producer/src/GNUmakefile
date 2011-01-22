TOPDIR=../
include $(TOPDIR)/Make/makedefs

CXXFILES = \
	InputArea.cpp\
	VisualChooser.cpp \
	RenderSurface.cpp\
	KeyboardMouse.cpp\
	Keyboard.cpp\
	Trackball.cpp\
	RenderSurface_X11.cpp\
	RenderSurface_Win32.cpp\
	RenderSurface_OSX_CGL.cpp\
	RenderSurface_OSX_AGL.cpp\
	Window3D.cpp\
	Camera.cpp\
	CameraGroup.cpp\
	CameraConfig.cpp\
	Timer.cpp\
	Version.cpp\
	ConfigLexer.cpp\
	ConfigParser.cpp\
	WGLExtensions.cpp\
    System.cpp\
    Utils.cpp\
    PipeTimer.cpp\



INC  += -I$(TOPDIR)/include -I$(THISDIR)
LIBS  =   $(GL_LIBS) $(X_LIBS) $(OPEN_THREAD_LIB)
TARGET_BASENAME   = Producer

## required for the way that Cygwin/Minw build is currently configured.
LIB = $(LIB_PREFIX)$(TARGET_BASENAME)

include $(TOPDIR)/Make/makerules


ifeq ("$(PRODUCER_REBUILD_PARSER)","1")
##  This are only needed for internal development
##
ConfigLexer.cpp: $(THISDIR)/ConfigLexer.l
	flex -+ -o$@ $<

ConfigParser.cpp: $(THISDIR)/ConfigParser.y
	bison -l -p ConfigParser_ --defines=$(THISDIR)/ConfigParser.h $< -o $@
endif


