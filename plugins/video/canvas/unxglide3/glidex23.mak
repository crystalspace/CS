# This is a subinclude file used to define the rules needed
# to build the GlideX 2D V3 driver -- glidex23

# Driver description
DESCRIPTION.glidex23 = Crystal Space Glide/X 2D driver Version 3

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make glidex23        Make the $(DESCRIPTION.glidex23)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: glidex23

all plugins drivers drivers23: glidex23

glidex23:
	$(MAKE_TARGET) MAKE_DLL=yes
glidex23clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

#vpath %.cpp libs/cs2d/glide2common

# local CFLAGS
CFLAGS.GLIDEX23=-I$(X11_PATH)/include -I/usr/local/glide/include -I/usr/include/glide -DGLIDE3

LIBS._GLIDEX23+=-L$(X11_PATH)/lib -lXext -lX11 -lglide3x  

# The 2D GlideX driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  GLIDEX23=glidex23$(DLL)
  LIBS.GLIDEX23=$(LIBS._GLIDEX23)
else
  GLIDEX23=$(OUT)$(LIB_PREFIX)glidex23$(LIB)
  DEP.EXE+=$(GLIDEX23)
  LIBS.EXE+=$(LIBS._GLIDEX23)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_GLIDEX23
endif
DESCRIPTION.$(GLIDEX23) = $(DESCRIPTION.glidex23)
SRC.GLIDEX23 = $(wildcard libs/cs2d/unxglide3/*.cpp $(SRC.COMMON.DRV2D) $(SRC.COMMON.DRV2D.GLIDE))
OBJ.GLIDEX23 = $(addprefix $(OUT),$(notdir $(SRC.GLIDEX23:.cpp=$O)))


endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: glidex23 glidex23clean

# Chain rules
clean: glidex23clean

glidex23: $(OUTDIRS) $(GLIDEX23)

$(OUT)%$O: libs/cs2d/unxglide3/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDEX23)
 
$(OUT)%$O: libs/cs2d/glide2common/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GLIDEX23)
 
$(GLIDEX23): $(OBJ.GLIDEX23) $(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
	$(DO.PLUGIN) $(LIBS.GLIDEX23)

glidex23clean:
	$(RM) $(GLIDEX23) $(OBJ.GLIDEX23)

ifdef DO_DEPEND
depend: $(OUTOS)glidex23.dep
$(OUTOS)glidex23.dep: $(SRC.GLIDEX23)
	$(DO.DEP1) $(CFLAGS.GLIDEX23) $(DO.DEP2)
else
-include $(OUTOS)glidex23.dep
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
