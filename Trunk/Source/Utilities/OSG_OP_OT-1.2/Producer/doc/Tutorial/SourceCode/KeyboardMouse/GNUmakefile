TOPDIR=../
include $(TOPDIR)/Make/makedefs

CXXFLAGS += -g -ggdb -gstabs+
LDFLAGS += -g -ggdb -gstabs+


LIBS 	= -lProducer -lOpenThreads -lGL 
ALL 	= \
	  example1\
	  example2\
	  example3\
	  example4\
	  example5\
	  example6

ALLSRC  = \
	  example1.cpp\
	  example2.cpp\
	  example3.cpp\
	  example4.cpp\
	  example5.cpp\
	  example6.cpp\
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

EX4OBJS = example4.o glut_teapot.o
example4 : $(EX4OBJS)
	$(CXX) $(LDFLAGS) $(EX4OBJS) $(LIBS) -o $@

EX5OBJS = example5.o glut_teapot.o
example5 : $(EX5OBJS)
	$(CXX) $(LDFLAGS) $(EX5OBJS) $(LIBS) -o $@

EX6OBJS = example6.o glut_teapot.o
example6 : $(EX6OBJS)
	$(CXX) $(LDFLAGS) $(EX6OBJS) $(LIBS) -o $@

include $(TOPDIR)/Make/makerules


