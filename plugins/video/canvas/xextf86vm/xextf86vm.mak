# This is a subinclude file used to define the rules needed
# to build the X-windows Video Mode driver -- xextf86vm

# Driver description
DESCRIPTION.xextf86vm = Crystal Space X-Extension Video Mode Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make xextf86vm    Make the $(DESCRIPTION.xextf86vm)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xextf86vm xextf86vmclean
all softcanvas plugins drivers drivers2d: xextf86vm

xextf86vm:
	$(MAKE_TARGET) MAKE_DLL=yes
xextf86vmclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


CFLAGS.XEXTF86VM += -I$(X11_PATH)/include
LIB.XEXTF86VM.SYSTEM += -L$(X11_PATH)/lib  \
  -lXxf86vm -lXext -lX11 $(X11_EXTRA_LIBS)

ifeq ($(USE_PLUGINS),yes)
  XEXTF86VM = $(OUTDLL)xextf86vm$(DLL)
  LIB.XEXTF86VM = $(foreach d,$(DEP.XEXTF86VM),$($d.LIB))
  LIB.XEXTF86VM.SPECIAL = $(LIB.XEXTF86VM.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(XEXTF86VM)
else
  XEXTF86VM = $(OUT)$(LIB_PREFIX)xextf86vm$(LIB)
  DEP.EXE += $(XEXTF86VM)
  LIBS.EXE += $(LIB.XEXTF86VM.SYSTEM)
  SCF.STATIC += xextf86vm
  TO_INSTALL.STATIC_LIBS += $(XEXTF86VM)
endif

INC.XEXTF86VM = $(wildcard plugins/video/canvas/xextf86vm/*.h) 
SRC.XEXTF86VM = $(wildcard plugins/video/canvas/xextf86vm/*.cpp)
OBJ.XEXTF86VM = $(addprefix $(OUT),$(notdir $(SRC.XEXTF86VM:.cpp=$O)))
DEP.XEXTF86VM = CSUTIL CSSYS CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xextf86vm xextf86vmclean

xextf86vm: $(OUTDIRS) $(XEXTF86VM)

$(OUT)%$O: plugins/video/canvas/xextf86vm/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.XEXTF86VM)
 
$(XEXTF86VM): $(OBJ.XEXTF86VM) $(LIB.XEXTF86VM)
	$(DO.PLUGIN) $(LIB.XEXTF86VM.SPECIAL)

clean: xextf86vmclean
xextf86vmclean:
	$(RM) $(XEXTF86VM) $(OBJ.XEXTF86VM)

ifdef DO_DEPEND
dep: $(OUTOS)xextf86vm.dep
$(OUTOS)xextf86vm.dep: $(SRC.XEXTF86VM)
	$(DO.DEP1) $(CFLAGS.XEXTF86VM) $(DO.DEP2)
else
-include $(OUTOS)xextf86vm.dep
endif

endif # ifeq ($(MAKESECTION),targets)
