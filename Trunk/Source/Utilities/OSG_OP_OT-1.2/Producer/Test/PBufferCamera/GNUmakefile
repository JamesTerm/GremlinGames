TOPDIR = ../..
include $(TOPDIR)/Make/makedefs
include $(TOPDIR)/Test/Make/localdefs

CXXFILES = \
	main.cpp \
	Cube.cpp

LIBS     = \
	-losgProducer -losgDB -losgGA -losgText -losgUtil -losg \
	-lProducer -lOpenThreads

EXEC     = \
	pbufviewer

include $(TOPDIR)/Make/makerules
