# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.ddraw61 = Crystal Space DirectDraw/DX 6.1 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ddraw61      Make the $(DESCRIPTION.ddraw61)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddraw61 ddraw61clean
all plugins drivers drivers2d: ddraw61

ddraw61:
	$(MAKE_TARGET) MAKE_DLL=yes
ddraw61clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/ddraw61 plugins/video/canvas/common

ifeq ($(USE_PLUGINS),yes)
  DDRAW61 = $(OUTDLL)ddraw61$(DLL)
  LIB.DDRAW61 = $(foreach d,$(DEP.DDRAW61),$($d.LIB))
  LIB.DDRAW61.SPECIAL = $(LFLAGS.l)ddraw $(LFLAGS.l)dxguid
  TO_INSTALL.DYNAMIC_LIBS += $(DDRAW61)
else
  DDRAW61 = $(OUT)$(LIB_PREFIX)ddraw61$(LIB)
  DEP.EXE += $(DDRAW61)
  LIBS.EXE += $(LFLAGS.l)ddraw61
  SCF.STATIC += ddraw61
  TO_INSTALL.STATIC_LIBS += $(DDRAW61)
endif

INC.DDRAW61 = $(wildcard plugins/video/canvas/ddraw61/*.h \
  $(INC.COMMON.DRV2D)) libs/cssys/win32/directdetection.h
SRC.DDRAW61 = $(wildcard plugins/video/canvas/ddraw61/*.cpp \
  $(SRC.COMMON.DRV2D)) libs/cssys/win32/directdetection.cpp
OBJ.DDRAW61 = $(addprefix $(OUT),$(notdir $(SRC.DDRAW61:.cpp=$O)))
DEP.DDRAW61 = CSUTIL CSSYS CSUTIL

MSVC.DSP += DDRAW61
DSP.DDRAW61.NAME = ddraw61
DSP.DDRAW61.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ddraw61 ddraw61clean

ddraw61: $(OUTDIRS) $(DDRAW61)

$(DDRAW61): $(OBJ.DDRAW61) $(LIB.DDRAW61)
	$(DO.PLUGIN) $(LIB.DDRAW61.SPECIAL)

clean: ddraw61clean
ddraw61clean:
	$(RM) $(DDRAW61) $(OBJ.DDRAW61)

ifdef DO_DEPEND
depend: $(OUTOS)ddraw61.dep
$(OUTOS)ddraw61.dep: $(SRC.DDRAW61)
	$(DO.DEP)
else
-include $(OUTOS)ddraw61.dep
endif

endif # ifeq ($(MAKESECTION),targets)
