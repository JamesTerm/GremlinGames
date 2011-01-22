TOPDIR=../
include $(TOPDIR)/Make/makedefs

LIBS   = -losgText -losgGA -losgDB -losgUtil -losg -losgProducer -lProducer -lOpenThreads -lGLw -lGL -lGLU -lXm -lXt -lX11
LDFLAGS += -L/usr/X11R6/lib -L$(HOME)/lib
CXXFLAGS += -I$(HOME)/include -g -ggdb -gstabs+
ALL    = example1 example2
ALLSRC = example1.cpp example2.cpp gui.cpp

all :: $(ALL)

EX1OBJS = example1.o gui.o
example1 : $(EX1OBJS)
	$(CXX) $(LDFLAGS) $(EX1OBJS) $(LIBS) -o $@

EX2OBJS = example2.o gui.o
example2 : $(EX2OBJS)
	$(CXX) $(LDFLAGS) $(EX2OBJS) $(LIBS) -o $@

include $(TOPDIR)/Make/makerules


