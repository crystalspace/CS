# This is a subinclude file used to define the rules needed
# to build the X-windows 2D driver -- linex2d

# Driver description
DESCRIPTION.linex2d = Crystal Space XLib 2D driver for Line3D

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make linex2d      Make the $(DESCRIPTION.linex2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: linex2d linex2dclean
all plugins drivers drivers2d: linex2d

linex2d:
	$(MAKE_TARGET) MAKE_DLL=yes
linex2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_XFREE86VM),yes)
  CFLAGS.LINEX2D += -DXFREE86VM
  LIB.LINEX2D.SYSTEM += -lXxf86vm
endif
 
# We need also the X libs
CFLAGS.LINEX2D += -I$(X11_PATH)/include
LIB.LINEX2D.SYSTEM += -L$(X11_PATH)/lib -lXext -lX11 $(X11_EXTRA_LIBS)

# The 2D Xlib driver
ifeq ($(USE_PLUGINS),yes)
  LINEX2D = $(OUTDLL)linex2d$(DLL)
  LIB.LINEX2D = $(foreach d,$(DEP.LINEX2D),$($d.LIB))
  LIB.LINEX2D.SPECIAL = $(LIB.LINEX2D.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(LINEX2D)
else
  LINEX2D = $(OUT)$(LIB_PREFIX)linex2d$(LIB)
  DEP.EXE += $(LINEX2D)
  LIBS.EXE += $(LIB.LINEX2D.SYSTEM)
  SCF.STATIC += linex2d
  TO_INSTALL.STATIC_LIBS += $(LINEX2D)
endif

INC.LINEX2D = $(wildcard plugins/video/canvas/linex/*.h   $(INC.COMMON.DRV2D))
SRC.LINEX2D = $(wildcard plugins/video/canvas/linex/*.cpp $(SRC.COMMON.DRV2D))\
  plugins/video/canvas/common/x11-keys.cpp
OBJ.LINEX2D = $(addprefix $(OUT),$(notdir $(SRC.LINEX2D:.cpp=$O)))
DEP.LINEX2D = CSUTIL CSSYS CSUTIL CSGEOM

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: linex2d linex2dclean

linex2d: $(OUTDIRS) $(LINEX2D)

$(OUT)%$O: plugins/video/canvas/linex/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.LINEX2D)
 
$(LINEX2D): $(OBJ.LINEX2D) $(LIB.LINEX2D)
	$(DO.PLUGIN) $(LIB.LINEX2D.SPECIAL)

clean: linex2dclean
linex2dclean:
	$(RM) $(LINEX2D) $(OBJ.LINEX2D)

ifdef DO_DEPEND
dep: $(OUTOS)linex2d.dep
$(OUTOS)linex2d.dep: $(SRC.LINEX2D)
	$(DO.DEP1) $(CFLAGS.LINEX2D) $(DO.DEP2)
else
-include $(OUTOS)linex2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
