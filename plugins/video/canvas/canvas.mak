# Common definitions for some 2D drivers
# Place all variables that are used in more than one makefile here

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/common

INC.COMMON.DRV2D = \
  plugins/video/canvas/common/graph2d.h \
  plugins/video/canvas/common/scrshot.h \
  plugins/video/canvas/common/protex2d.h
SRC.COMMON.DRV2D = \
  plugins/video/canvas/common/graph2d.cpp \
  plugins/video/canvas/common/scrshot.cpp \
  plugins/video/canvas/common/protex2d.cpp

INC.COMMON.DRV2D.OPENGL = plugins/video/canvas/openglcommon/*.h
SRC.COMMON.DRV2D.OPENGL = plugins/video/canvas/openglcommon/*.cpp

endif # ifeq ($(MAKESECTION),postdefines)
