# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.ddraw61 = Crystal Space Windows DirectDraw/DirectX 6.1 2D driver

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

ifeq ($(USE_SHARED_PLUGINS),yes)
  DDRAW61=ddraw61$(DLL)
  DEP.DDRAW61=$(CSUTIL.LIB) $(CSSYS.LIB)
  LIBS.LOCAL.DDRAW61=$(LFLAGS.l)ddraw
else
  DDRAW61=$(OUT)$(LIB_PREFIX)ddraw61$(LIB)
  DEP.EXE+=$(DDRAW61)
  LIBS.EXE+=$(LFLAGS.l)ddraw61
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_DDRAW61
endif

DESCRIPTION.$(DDRAW61)=$(DESCRIPTION.ddraw61)

SRC.DDRAW61 = $(wildcard plugins/video/canvas/ddraw61/*.cpp $(SRC.COMMON.DRV2D)) \
	libs/cssys/win32/directdetection.cpp
OBJ.DDRAW61 = $(addprefix $(OUT),$(notdir $(SRC.DDRAW61:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/ddraw61 plugins/video/canvas/common

.PHONY: ddraw61 ddraw61clean

# Chain rules
clean: ddraw61clean

ddraw61: $(OUTDIRS) $(DDRAW61)

$(DDRAW61): $(OBJ.DDRAW61) $(DEP.DDRAW61)
	$(DO.PLUGIN) $(LIBS.LOCAL.DDRAW61)

ddraw61clean:
	$(RM) $(DDRAW61) $(OBJ.DDRAW61) $(OUTOS)ddraw61.dep

ifdef DO_DEPEND
depend: $(OUTOS)ddraw61.dep
$(OUTOS)ddraw61.dep: $(SRC.DDRAW61)
	$(DO.DEP)
else
-include $(OUTOS)ddraw61.dep
endif

endif # ifeq ($(MAKESECTION),targets)
