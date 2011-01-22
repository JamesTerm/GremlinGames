TOPDIR = ../../
include $(TOPDIR)/Make/makedefs
include $(TOPDIR)/Test/Make/localdefs

all: pbufferTest pbufferTest2 pbufferSnapshot

LIBS = -losgDB -losgUtil -losg -losgProducer -lProducer

OBJS1 = main.o Cube.o 
pbufferTest : $(OBJS1)
	$(CXX) $(LDFLAGS) $(OBJS1) $(LIBS) -o $@

OBJS2 = main2.o Cube.o
pbufferTest2: $(OBJS2)
	$(CXX) $(LDFLAGS) $(OBJS2) $(LIBS) -o $@

OBJS3 = snapshot.o 
pbufferSnapshot : $(OBJS3)
	$(CXX) $(LDFLAGS) $(OBJS3) $(LIBS) -o $@


OBJS = $(OBJS1) $(OBJS2) $(OBJS3)

ALL_EXECS = pbufferTest pbufferTest2 pbufferSnapshot

clean : 
	rm -f $(OBJS)

clobber : clean
	rm -f $(ALL_EXECS)

