TOPDIR = ..
include $(TOPDIR)/Make/makedefs

CXXFILES = \
	main.cpp\
	glut_teapot.cpp\

LIBS = -lProducer -lGLU
CXXFLAGS += 
LDFLAGS  += 

ALL = example1
ALLSRC = main.cpp glut_teapot.cpp

OBJS = $(CXXFILES:.cpp=.o)

all :: $(ALL)

example1 : $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

include $(TOPDIR)/Make/makerules

