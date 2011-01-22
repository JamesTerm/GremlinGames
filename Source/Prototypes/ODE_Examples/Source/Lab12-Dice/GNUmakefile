CXXFILES = \
        main.cpp\
        OdeWorld.cpp\
        RigidBody.cpp\
        PBufferTexture2D.cpp\
        Projector.cpp\
       

OBJS = $(CXXFILES:.cpp=.o)
LIBS = -losgDB -losgUtil -losg -losgProducer -lProducer -lode

dice : $(OBJS)
	    $(CXX) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

clean:
	rm -f $(OBJS)

clobber: clean
	rm -f dice
