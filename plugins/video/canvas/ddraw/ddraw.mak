# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.ddraw = Crystal Space Windows DirectDraw 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make ddraw        Make the $(DESCRIPTION.ddraw)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddraw

all drivers drivers2d: ddraw

ddraw:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

.PHONY: ddraw.lib

ifeq ($(USE_DLL),yes)
  DDRAW=csddraw$(DLL)
  DEP.DDRAW=$(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB) ddraw.lib
else
  DDRAW=$(OUT)$(LIB_PREFIX)csddraw$(LIB)
  DEP.EXE+=$(DDRAW) ddraw.lib
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_DDRAW2D
endif
DESCRIPTION.$(DDRAW)=$(DESCRIPTION.ddraw)
SRC.DDRAW = $(wildcard libs/cs2d/ddraw/*.cpp $(SRC.COMMON.DRV2D)) \
  libs/cssys/win32/directdetection.cpp
OBJ.DDRAW = $(addprefix $(OUT),$(notdir $(SRC.DDRAW:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/ddraw

.PHONY: ddraw ddrawclean ddrawcleanlib

# Chain rules
clean: ddrawclean
cleanlib: ddrawcleanlib

ddraw: $(OUTDIRS) $(DDRAW)

$(DDRAW): $(OBJ.DDRAW) $(DEP.DDRAW)
	$(DO.LIBRARY)

ddrawclean:
	$(RM) $(DDRAW)

ddrawcleanlib:
	$(RM) $(OBJ.DDRAW) $(DDRAW)

ifdef DO_DEPEND
$(OUTOS)ddraw.dep: $(SRC.DDRAW)
	$(DO.DEP)
endif

-include $(OUTOS)ddraw.dep

endif # ifeq ($(MAKESECTION),targets)
