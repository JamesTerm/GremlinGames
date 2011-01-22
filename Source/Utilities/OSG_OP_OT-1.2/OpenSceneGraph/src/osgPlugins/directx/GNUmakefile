#
# $Id: GNUmakefile,v 1.2 2006/07/04 09:13:15 robert Exp $
#

TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
        ReaderWriterDirectX.cpp\
	directx.cpp\
        mesh.cpp\
        types.cpp

LIBS   += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = x
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules
