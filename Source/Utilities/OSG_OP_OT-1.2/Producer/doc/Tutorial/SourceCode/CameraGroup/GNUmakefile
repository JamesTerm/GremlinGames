TOPDIR=../
include $(TOPDIR)/Make/makedefs


LIBS = -lProducer -lOpenThreads -lGL -lGLU
ALL    =\
	example1\
	example2\
	example3

ALLSRC = \
	example1.cpp\
	example2.cpp\
	example3.cpp\
	glut_teapot.cpp

all :: $(ALL)

EX1OBJS = example1.o glut_teapot.o
example1 : $(EX1OBJS)
	$(CXX) $(LDFLAGS) $(EX1OBJS) $(LIBS) -o $@

EX2OBJS = example2.o glut_teapot.o
example2 : $(EX2OBJS)
	$(CXX) $(LDFLAGS) $(EX2OBJS) $(LIBS) -o $@

EX3OBJS = example3.o glut_teapot.o
example3 : $(EX3OBJS)
	$(CXX) $(LDFLAGS) $(EX3OBJS) $(LIBS) -o $@

include $(TOPDIR)/Make/makerules


