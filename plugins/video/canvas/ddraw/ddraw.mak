# This is a subinclude file used to define the rules needed
# to build the Windows DirectDraw 2D driver

# Driver description
DESCRIPTION.ddraw = Crystal Space Windows DirectDraw 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ddraw        Make the $(DESCRIPTION.ddraw)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddraw ddrawclean

all plugins drivers drivers2d: ddraw

ddraw:
	$(MAKE_TARGET) MAKE_DLL=yes
ddrawclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_SHARED_PLUGINS),yes)
  DDRAW=csddraw$(DLL)
  DEP.DDRAW=$(CSUTIL.LIB) $(CSSYS.LIB)
  LIBS.LOCAL.DDRAW=$(LFLAGS.l)ddraw
  TO_INSTALL.DYNAMIC_LIBS+=$(DDRAW)
else
  DDRAW=$(OUT)$(LIB_PREFIX)ddraw$(LIB)
  DEP.EXE += $(DDRAW)
  LIBS.EXE += $(LFLAGS.l)ddraw
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_DDRAW
  TO_INSTALL.STATIC_LIBS += $(DDRAW)
endif

TO_INSTALL.CONFIG += data/config/direct3ddx5.cfg data/config/direct3ddx6.cfg
DESCRIPTION.$(DDRAW) = $(DESCRIPTION.ddraw)

SRC.DDRAW = $(wildcard plugins/video/canvas/ddraw/*.cpp $(SRC.COMMON.DRV2D)) \
	$(wildcard plugins/video/canvas/common/*.cpp) \
	libs/cssys/win32/directdetection.cpp
OBJ.DDRAW = $(addprefix $(OUT),$(notdir $(SRC.DDRAW:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/ddraw plugins/video/canvas/common

.PHONY: ddraw ddrawclean

# Chain rules
clean: ddrawclean

ddraw: $(OUTDIRS) $(DDRAW)

$(DDRAW): $(OBJ.DDRAW) $(DEP.DDRAW)
	$(DO.PLUGIN) $(LIBS.LOCAL.DDRAW)

ddrawclean:
	$(RM) $(DDRAW) $(OBJ.DDRAW) $(OUTOS)ddraw.dep

ifdef DO_DEPEND
depend: $(OUTOS)ddraw.dep
$(OUTOS)ddraw.dep: $(SRC.DDRAW)
	$(DO.DEP)
else
-include $(OUTOS)ddraw.dep
endif

endif # ifeq ($(MAKESECTION),targets)
