# This is a subinclude file used to define the rules needed
# to build the GL 2D driver common functionality

# Driver description
DESCRIPTION.glcommon2d = Crystal Space GL 2D driver basic functions

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)


endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)


endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Local CFLAGS and libraries
CFLAGS.GLCOMMON2D+=-I$(X11_PATH)/include
LIBS._GLCOMMON2D+=-L$(X11_PATH)/lib -lXext -lX11

# The 2D modules
DESCRIPTION.$(GLCOMMON2D) = $(DESCRIPTION.glcommon2d)
SRC.GLCOMMON2D = $(wildcard libs/cs2d/openglcommon/*.cpp)
OBJ.GLCOMMON2D = $(addprefix $(OUT),$(notdir $(SRC.GLCOMMON2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

$(OUT)%$O: libs/cs2d/openglcommon/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLX2D)
 
ifdef DO_DEPEND
depend: $(OUTOS)glcommon2d.dep
$(OUTOS)glcommon2d.dep: $(SRC.GLCOMMON2D)
	$(DO.DEP)
else
-include $(OUTOS)glcommon2d.dep
endif

endif
