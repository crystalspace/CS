# Common definitions for some 2D drivers
# Place all variables that are used in more than one makefile here

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/common

SRC.COMMON.DRV2D=plugins/video/canvas/common/graph2d.cpp \
  plugins/video/canvas/common/fonts.cpp \
  plugins/video/canvas/common/scrshot.cpp

SRC.COMMON.DRV2D.OPENGL=plugins/video/canvas/openglcommon/*.cpp

SRC.COMMON.DRV2D.GLIDE = plugins/video/canvas/glide2common/*.cpp

endif # ifeq ($(MAKESECTION),postdefines)
