# Common definitions for some 2D drivers
# Place all variables that are used in more than one makefile here

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cs2d/common

SRC.COMMON.DRV2D=libs/cs2d/common/igraph2d.cpp libs/cs2d/common/graph2d.cpp \
  libs/cs2d/common/fonts.cpp libs/cs2d/common/scrshot.cpp

SRC.COMMON.DRV2D.OPENGL=libs/cs2d/openglcommon/*.cpp

SRC.COMMON.DRV2D.GLIDE = libs/cs2d/glide2common/*.cpp

endif # ifeq ($(MAKESECTION),postdefines)
