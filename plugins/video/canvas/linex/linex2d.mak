# This is a subinclude file used to define the rules needed
# to build the X-windows 2D driver -- linex2d

# Driver description
DESCRIPTION.linex2d = Crystal Space XLib 2D driver for Line3D

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make linex2d      Make the $(DESCRIPTION.linex2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: linex2d

all plugins drivers drivers2d: linex2d

linex2d:
	$(MAKE_TARGET) MAKE_DLL=yes
linex2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need also the X libs
CFLAGS.LINEX2D+=-I$(X11_PATH)/include
LIBS.LINEX2D+=-L$(X11_PATH)/lib -lXext -lX11
 
# The 2D Xlib driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  LINEXLIB2D=$(OUTDLL)linex2d$(DLL)
  LIBS.LOCAL.LINEX2D=$(LIBS.LINEX2D)
  DEP.LINEX2D=$(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  LINEXLIB2D=$(OUT)$(LIB_PREFIX)linex2d$(LIB)
  DEP.EXE+=$(LINEXLIB2D)
  LIBS.EXE+=$(LIBS.LINEX2D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_LINEX2D
endif
DESCRIPTION.$(LINEXLIB2D) = $(DESCRIPTION.linex2d)
SRC.LINEXLIB2D = $(wildcard libs/cs2d/linex/*.cpp $(SRC.COMMON.DRV2D))
OBJ.LINEXLIB2D = $(addprefix $(OUT),$(notdir $(SRC.LINEXLIB2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: linex2d linelibxclean

# Chain rules
clean: linelibxclean

linex2d: $(OUTDIRS) $(LINEXLIB2D)

$(OUT)%$O: libs/cs2d/linex/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.LINEX2D)
 
$(LINEXLIB2D): $(OBJ.LINEXLIB2D) $(DEP.LINEX2D)
	$(DO.PLUGIN) $(LIBS.LOCAL.LINEX2D)

linelibxclean:
	$(RM) $(LINEXLIB2D) $(OBJ.LINEXLIB2D)

ifdef DO_DEPEND
depend: $(OUTOS)linex2d.dep
$(OUTOS)linex2d.dep: $(SRC.LINEXLIB2D)
	$(DO.DEP)
else
-include $(OUTOS)linex2d.dep
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
