# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
ifeq ($(COMP),BC)
DESCRIPTION.ddraw5 = Crystal Space Windows DirectDraw 2D driver
else
DESCRIPTION.ddraw = Crystal Space Windows DirectDraw 2D driver
endif

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
ifeq ($(COMP),BC)
DRIVERHELP += $(NEWLINE)echo $"  make ddraw        Make the $(DESCRIPTION.ddraw5)$"
else
DRIVERHELP += $(NEWLINE)echo $"  make ddraw        Make the $(DESCRIPTION.ddraw)$"
endif

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

ifeq ($(COMP),BC)

.PHONY: ddraw5

all plugins drivers drivers2d: ddraw5

ddraw5:
	$(MAKE_TARGET) MAKE_DLL=yes
ddraw5clean:
	$(MAKE_CLEAN)

else

.PHONY: ddraw

all plugins drivers drivers2d: ddraw

ddraw:
	$(MAKE_TARGET) MAKE_DLL=yes
ddrawclean:
	$(MAKE_CLEAN)

endif

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(COMP),BC)
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

else
#		COMP_GCC/COMP_VC
ifeq ($(COMP),VC)
LIBS.DDRAW+=ddraw.lib
else
LIBS.DDRAW+=-lddraw
endif

ifeq ($(USE_SHARED_PLUGINS),yes)
  DDRAW=ddraw2d$(DLL)
  DEP.DDRAW=$(CSUTIL.LIB) $(CSSYS.LIB)
  LIBS.LOCAL.DDRAW=$(LIBS.DDRAW)
else
  DDRAW=$(OUT)$(LIB_PREFIX)ddraw2d$(LIB)
  DEP.EXE+=$(DDRAW)
  LIBS.EXE+=$(LIBS.DDRAW)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_DDRAW2D
endif

DESCRIPTION.$(DDRAW)=$(DESCRIPTION.ddraw)

endif

SRC.DDRAW = $(wildcard libs/cs2d/ddraw/*.cpp $(SRC.COMMON.DRV2d)) \
	libs/cssys/win32/directdetection.cpp
OBJ.DDRAW = $(addprefix $(OUT),$(notdir $(SRC.DDRAW:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/ddraw

ifeq ($(COMP),BC)
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

else
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

endif
   
endif # ifeq ($(MAKESECTION),targets)
