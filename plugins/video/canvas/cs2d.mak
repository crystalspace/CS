# Common definitions for some 2D drivers
# Place all variables that are used in more than one makefile here

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cs2d/common

SRC.COMMON.DRV2D=libs/cs2d/common/igraph2d.cpp libs/cs2d/common/graph2d.cpp \
  libs/cs2d/common/fonts.cpp libs/cs2d/common/scrshot.cpp

SRC.COMMON.DRV2D.OPENGL=libs/cs2d/openglcommon/*.cpp

SRC.COMMON.DRV2D.GLIDE = libs/cs2d/glide2common/*.cpp \
  libs/cs3d/glide2/glcache.cpp libs/cs3d/glide2/hicache.cpp \
  libs/cs3d/glide2/hicache2.cpp libs/cs3d/glide2/gl_txtmgr.cpp \
  libs/cs3d/glide2/itexture.cpp libs/cs3d/common/txtmgr.cpp \
  libs/cs3d/common/memheap.cpp libs/cs3d/common/inv_cmap.cpp \
  libs/cs3d/common/imgtools.cpp

endif # ifeq ($(MAKESECTION),postdefines)
