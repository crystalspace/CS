# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.ddraw = Crystal Space Windows DirectDraw 2D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make ddraw        Make the $(DESCRIPTION.ddraw)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddraw

all plugins drivers drivers2d: ddraw

ddraw:
	$(MAKE_TARGET) MAKE_DLL=yes
ddrawclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(COMP),GCC)
# COMP_GCC Linker assumes static libs have extension .a
# Mingw/Cygwin both use libddraw.a (static lib) as the
# place to get MS DirectDraw references from
  LIBS.DDRAW+=$(LFLAGS.l)ddraw$
  LIBS.DINPUT+=$(LFLAGS.l)dinput$
else
  LIBS.DDRAW+=$(LFLAGS.l)ddraw$(LIB)
endif

ifeq ($(USE_SHARED_PLUGINS),yes)
  DDRAW=ddraw2d$(DLL)
  DEP.DDRAW=$(CSUTIL.LIB) $(CSSYS.LIB)
  LIBS.LOCAL.DDRAW=$(LIBS.DDRAW)
else
# Generate Static Libs
  DDRAW=$(OUT)$(LIB_PREFIX)ddraw2d$(LIB)
  DEP.EXE+=$(DDRAW)
  ifeq ($(COMP),GCC)
    LIBS.EXE+=$(LIBS.DDRAW) $(LIBS.DINPUT)
  else
    LIBS.EXE+=$(LIBS.DDRAW)
  endif
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_DDRAW2D
endif

DESCRIPTION.$(DDRAW)=$(DESCRIPTION.ddraw)

SRC.DDRAW = $(wildcard libs/cs2d/ddraw/*.cpp $(SRC.COMMON.DRV2D)) \
	libs/cssys/win32/directdetection.cpp
OBJ.DDRAW = $(addprefix $(OUT),$(notdir $(SRC.DDRAW:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/ddraw

.PHONY: ddraw ddrawclean

# Chain rules
clean: ddrawclean

ddraw: $(OUTDIRS) $(DDRAW)

$(DDRAW): $(OBJ.DDRAW) $(DEP.DDRAW)
	$(DO.PLUGIN) $(LIBS.LOCAL.DDRAW)

ddrawclean:
	$(RM) $(DDRAW) $(OBJ.DDRAW)

ifdef DO_DEPEND
depend: $(OUTOS)ddraw.dep
$(OUTOS)ddraw.dep: $(SRC.DDRAW)
	$(DO.DEP)
else
-include $(OUTOS)ddraw.dep
endif

endif # ifeq ($(MAKESECTION),targets)

