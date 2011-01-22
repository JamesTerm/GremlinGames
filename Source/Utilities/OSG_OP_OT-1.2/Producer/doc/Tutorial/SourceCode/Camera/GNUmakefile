TOPDIR=../
include $(TOPDIR)/Make/makedefs

LIBS = -lProducer -lGL -lGLU -lpthread
ALL = example1 
ALLSRC = example1.cpp glut_teapot.cpp


CXXFLAGS += -I$(HOME)/include 
LDFLAGS  += -L$(HOME)/lib32

all :: $(ALL)

EX1OBJS = example1.o glut_teapot.o
example1 : $(EX1OBJS)
	$(CXX) $(LDFLAGS) $(EX1OBJS) $(LIBS) -o $@

include $(TOPDIR)/Make/makerules


