# Common definitions for 2D drivers

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cs2d/common

SRC.COMMON.DRV2D=libs/cs2d/common/igraph2d.cpp libs/cs2d/common/graph2d.cpp \
  libs/cs2d/common/fonts.cpp

endif # ifeq ($(MAKESECTION),postdefines)
