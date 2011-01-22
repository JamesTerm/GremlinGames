OBJS1 = example1.o
OBJS2 = example2.o
LIBS = -losgDB -losg -losgProducer -lProducer

all : example1 example2

example1 : $(OBJS1)
	$(CXX) $(LDFLAGS) $(OBJS1) $(LIBS) -o $@

example2 : $(OBJS2)
	$(CXX) $(LDFLAGS) $(OBJS2) $(LIBS) -o $@
