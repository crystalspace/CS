# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.ddraw5 = Crystal Space Windows DirectDraw 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make ddraw        Make the $(DESCRIPTION.ddraw5)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddraw5

all plugins drivers drivers2d: ddraw5

ddraw5:
	$(MAKE_TARGET) MAKE_DLL=yes
ddraw5clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

LIBS.DDRAW5+=ddraw.lib

ifeq ($(USE_SHARED_PLUGINS),yes)
  DDRAW5=ddraw5$(DLL)
  DEP.DDRAW5=$(CSUTIL.LIB) $(CSSYS.LIB)
  LIBS.LOCAL.DDRAW5=$(LIBS.DDRAW5)
else
  DDRAW5=$(OUT)$(LIB_PREFIX)ddraw5$(LIB)
  DEP.EXE+=$(DDRAW5)
  LIBS.EXE+=$(LIBS.DDRAW5)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_DDRAW2D
endif
DESCRIPTION.$(DDRAW5)=$(DESCRIPTION.ddraw5)
SRC.DDRAW5 = $(wildcard libs/cs2d/ddraw/*.cpp $(SRC.COMMON.DRV2D)) \
  libs/cssys/win32/directdetection.cpp
OBJ.DDRAW5 = $(addprefix $(OUT),$(notdir $(SRC.DDRAW5:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/ddraw

.PHONY: ddraw5 ddraw5clean

# Chain rules
clean: ddraw5clean

ddraw5: $(OUTDIRS) $(DDRAW5)

$(DDRAW5): $(OBJ.DDRAW5) $(DEP.DDRAW5)
	$(DO.PLUGIN) $(LIBS.LOCAL.DDRAW5)

ddraw5clean:
	$(RM) $(DDRAW5) $(OBJ.DDRAW5)

ifdef DO_DEPEND
depend: $(OUTOS)ddraw5.dep
$(OUTOS)ddraw5.dep: $(SRC.DDRAW5)
	$(DO.DEP)
else
-include $(OUTOS)ddraw5.dep
endif

endif # ifeq ($(MAKESECTION),targets)
