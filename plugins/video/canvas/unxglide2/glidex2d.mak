# This is a subinclude file used to define the rules needed
# to build the GlideX 2D driver -- glidex2d

# Driver description
DESCRIPTION.glidex2d = Crystal Space Glide/X 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make glidex2d        Make the $(DESCRIPTION.glidex2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glidex2d

all plugins drivers drivers2d: glidex2d

glidex2d:
	$(MAKE_TARGET) MAKE_DLL=yes
glidex2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cs2d/glide2common

# local CFLAGS
CFLAGS.GLIDEX2D+=-L$(X11_PATH)/include -I/usr/local/glide/include -I/usr/include/glide

#add for proper compiling in glide2common
CFLAGS+=$(CFLAGS.GLIDEX2D)

LIBS._GLIDEX2D+=-L$(X11_PATH)/lib -lXext -lX11 -lglide2x  

# The 2D GlideX driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLIDEX2D=glidex2d$(DLL)
  LIBS.GLIDEX2D=$(LIBS._GLIDEX2D)
else
  GLIDEX2D=$(OUT)$(LIB_PREFIX)glidex2d$(LIB)
  DEP.EXE+=$(GLIDEX2D)
  LIBS.EXE+=$(LIBS._GLIDEX2D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_GLIDEX2D
endif
DESCRIPTION.$(GLIDEX2D) = $(DESCRIPTION.glidex2d)
#SRC.GLIDEX2D = $(wildcard libs/cs2d/unxglide2/*.cpp libs/cs2d/glide2common/*.cpp )
SRC.GLIDEX2D = $(wildcard libs/cs2d/unxglide2/*.cpp $(SRC.COMMON.DRV2D) $(SRC.COMMON.DRV2D.GLIDE))
OBJ.GLIDEX2D = $(addprefix $(OUT),$(notdir $(SRC.GLIDEX2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glidex2d glidexclean

# Chain rules
clean: glidexclean

glidex2d: $(OUTDIRS) $(GLIDEX2D)

$(OUT)%$O: libs/cs2d/unxglide2/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDEX2D)
 
$(GLIDEX2D): $(OBJ.GLIDEX2D) $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
	$(DO.PLUGIN) $(LIBS.GLIDEX2D)

glidexclean:
	$(RM) $(GLIDEX2D) $(OBJ.GLIDEX2D)

ifdef DO_DEPEND
depend: $(OUTOS)glidex2d.dep
$(OUTOS)glidex2d.dep: $(SRC.GLIDEX2D)
	$(DO.DEP)
else
-include $(OUTOS)glidex2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)

#------------------------------------------------------------------- config ---#
ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)

# Default value for DO_SHM
ifndef DO_SHM
  DO_SHM = yes
endif

ifeq ($(DO_SHM)$(findstring DO_SHM,$(MAKE_VOLATILE_H)),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_SHM$">>volatile.tmp
endif

endif # ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)
