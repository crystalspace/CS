# This is a subinclude file used to define the rules needed
# to build the X-window plugin -- xwin

# Driver description
DESCRIPTION.xwin = Crystal Space X Window Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make xwin         Make the $(DESCRIPTION.xwin)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xwin xwinclean
all softcanvas plugins drivers drivers2d: xwin

xwin:
	$(MAKE_TARGET) MAKE_DLL=yes
xwinclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_XFREE86VM),yes)
  CFLAGS.XWIN += -DXFREE86VM
  # On some platforms -lxf86vm must appear before -lX11.
  LIB.XWIN.SYSTEM += -lXxf86vm
endif

CFLAGS.XWIN += -I$(X11_PATH)/include
LIB.XWIN.SYSTEM += -L$(X11_PATH)/lib -lXext -lX11 $(X11_EXTRA_LIBS)

ifeq ($(USE_PLUGINS),yes)
  XWIN = $(OUTDLL)xwin$(DLL)
  LIB.XWIN = $(foreach d,$(DEP.XWIN),$($d.LIB))
  LIB.XWIN.SPECIAL = $(LIB.XWIN.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(XWIN)
else
  XWIN = $(OUT)$(LIB_PREFIX)xwin$(LIB)
  DEP.EXE += $(XWIN)
  LIBS.EXE += $(LIB.XWIN.SYSTEM)
  SCF.STATIC += xwin
  TO_INSTALL.STATIC_LIBS += $(XWIN)
endif

INC.XWIN = $(wildcard plugins/video/canvas/xwindow/*.h)\
  plugins/video/canvas/common/scancode.h
SRC.XWIN = $(wildcard plugins/video/canvas/xwindow/*.cpp) \
  plugins/video/canvas/common/x11-keys.cpp
OBJ.XWIN = $(addprefix $(OUT),$(notdir $(SRC.XWIN:.cpp=$O)))
DEP.XWIN = CSUTIL CSSYS CSGEOM CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xwin xwinclean

xwin: $(OUTDIRS) $(XWIN)

$(OUT)%$O: plugins/video/canvas/xwindow/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.XWIN)

$(XWIN): $(OBJ.XWIN) $(LIB.XWIN)
	$(DO.PLUGIN) $(LIB.XWIN.SPECIAL)

clean: xwinclean
xwinclean:
	$(RM) $(XWIN) $(OBJ.XWIN)

ifdef DO_DEPEND
dep: $(OUTOS)xwin.dep
$(OUTOS)xwin.dep: $(SRC.XWIN)
	$(DO.DEP1) $(CFLAGS.XWIN) $(DO.DEP2)
else
-include $(OUTOS)xwin.dep
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
