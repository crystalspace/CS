# Common definitions for 3D drivers

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/renderer/common

TO_INSTALL.CONFIG += data/config/video.cfg

endif # ifeq ($(MAKESECTION),postdefines)
