# This is a subinclude file used to define the rules needed
# to build the X-windows Video Mode driver -- xext86vm

# Driver description
DESCRIPTION.xext86vm = Crystal Space X-Extension Video Mode Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make xext86vm     Make the $(DESCRIPTION.xext86vm)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: xext86vm xext86vmclean
all softcanvas plugins drivers drivers2d: xext86vm

xext86vm:
	$(MAKE_TARGET) MAKE_DLL=yes
xext86vmclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)


CFLAGS.XEXT86VM += -I$(X11_PATH)/include
LIB.XEXT86VM.SYSTEM += -L$(X11_PATH)/lib  \
  -lXxf86vm -lXext -lX11 $(X11_EXTRA_LIBS)

ifeq ($(USE_PLUGINS),yes)
  XEXT86VM = $(OUTDLL)xext86vm$(DLL)
  LIB.XEXT86VM = $(foreach d,$(DEP.XEXT86VM),$($d.LIB))
  LIB.XEXT86VM.SPECIAL = $(LIB.XEXT86VM.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(XEXT86VM)
else
  XEXT86VM = $(OUT)$(LIB_PREFIX)xext86vm$(LIB)
  DEP.EXE += $(XEXT86VM)
  LIBS.EXE += $(LIB.XEXT86VM.SYSTEM)
  SCF.STATIC += xext86vm
  TO_INSTALL.STATIC_LIBS += $(XEXT86VM)
endif

INC.XEXT86VM = $(wildcard plugins/video/canvas/xextf86vm/*.h) 
SRC.XEXT86VM = $(wildcard plugins/video/canvas/xextf86vm/*.cpp)
OBJ.XEXT86VM = $(addprefix $(OUT),$(notdir $(SRC.XEXT86VM:.cpp=$O)))
DEP.XEXT86VM = CSUTIL CSSYS CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: xext86vm xext86vmclean

xext86vm: $(OUTDIRS) $(XEXT86VM)

$(OUT)%$O: plugins/video/canvas/xextf86vm/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.XEXT86VM)
 
$(XEXT86VM): $(OBJ.XEXT86VM) $(LIB.XEXT86VM)
	$(DO.PLUGIN) $(LIB.XEXT86VM.SPECIAL)

clean: xext86vmclean
xext86vmclean:
	$(RM) $(XEXT86VM) $(OBJ.XEXT86VM)

ifdef DO_DEPEND
dep: $(OUTOS)xext86vm.dep
$(OUTOS)xext86vm.dep: $(SRC.XEXT86VM)
	$(DO.DEP1) $(CFLAGS.XEXT86VM) $(DO.DEP2)
else
-include $(OUTOS)xext86vm.dep
endif

endif # ifeq ($(MAKESECTION),targets)
