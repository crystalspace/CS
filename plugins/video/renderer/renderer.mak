# Common definitions for 3D drivers

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/video/renderer/common

TO_INSTALL.CONFIG += $(SRCDIR)/data/config/video.cfg

endif # ifeq ($(MAKESECTION),postdefines)
