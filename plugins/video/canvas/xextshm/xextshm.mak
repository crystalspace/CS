# This is a subinclude file used to define the rules needed
# to build the X-windows 2D driver -- xextshm

# Driver description
DESCRIPTION.xextshm = Crystal Space X-Extension Shared Memory Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make xextshm      Make the $(DESCRIPTION.xextshm)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xextshm xextshmclean
all softcanvas plugins drivers drivers2d: xextshm

xextshm:
	$(MAKE_TARGET) MAKE_DLL=yes
xextshmclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CFLAGS.XEXTSHM += -I$(X11_PATH)/include
LIB.XEXTSHM.SYSTEM += -L$(X11_PATH)/lib -lXext -lX11

ifeq ($(USE_PLUGINS),yes)
  XEXTSHM = $(OUTDLL)xextshm$(DLL)
  LIB.XEXTSHM = $(foreach d,$(DEP.XEXTSHM),$($d.LIB))
  LIB.XEXTSHM.SPECIAL = $(LIB.XEXTSHM.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(XEXTSHM)
else
  XEXTSHM = $(OUT)$(LIB_PREFIX)xextshm$(LIB)
  DEP.EXE += $(XEXTSHM)
  LIBS.EXE += $(LIB.XEXTSHM.SYSTEM)
  SCF.STATIC += xextshm
  TO_INSTALL.STATIC_LIBS += $(XEXTSHM)
endif

INC.XEXTSHM = $(wildcard plugins/video/canvas/xextshm/*.h)
SRC.XEXTSHM = $(wildcard plugins/video/canvas/xextshm/*.cpp)
OBJ.XEXTSHM = $(addprefix $(OUT),$(notdir $(SRC.XEXTSHM:.cpp=$O)))
DEP.XEXTSHM = CSUTIL CSSYS CSGEOM CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xextshm xextshmclean

xextshm: $(OUTDIRS) $(XEXTSHM)

$(OUT)%$O: plugins/video/canvas/xextshm/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.XEXTSHM)

$(XEXTSHM): $(OBJ.XEXTSHM) $(LIB.XEXTSHM)
	$(DO.PLUGIN) $(LIB.XEXTSHM.SPECIAL)

clean: xextshmclean
xextshmclean:
	$(RM) $(XEXTSHM) $(OBJ.XEXTSHM)

ifdef DO_DEPEND
dep: $(OUTOS)xextshm.dep
$(OUTOS)xextshm.dep: $(SRC.XEXTSHM)
	$(DO.DEP1) $(CFLAGS.XEXTSHM) $(DO.DEP2)
else
-include $(OUTOS)xextshm.dep
endif

endif # ifeq ($(MAKESECTION),targets)

#------------------------------------------------------------------ config ---#
ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)

# Default value for DO_SHM
ifndef DO_SHM
  DO_SHM = yes
endif

ifeq ($(DO_SHM)$(findstring DO_SHM,$(MAKE_VOLATILE_H)),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_SHM$">>volatile.tmp
endif

endif # ifeq ($(ROOTCONFIG)/$(MAKESECTION),volatile/rootdefines)
