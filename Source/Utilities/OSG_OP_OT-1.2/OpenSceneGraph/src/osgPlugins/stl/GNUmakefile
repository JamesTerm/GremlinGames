#
# $Id: GNUmakefile,v 1.1 2004/11/12 14:49:22 robert Exp $
#

TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
        ReaderWriterSTL.cpp

LIBS   += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = stl
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules
