# This is a subinclude file used to define the rules needed
# to build DOS raw framebuffer access 2D driver -- raw2d

# Driver description
DESCRIPTION.dosraw = Crystal Space raw DOS SVGA driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make dosraw        Make the $(DESCRIPTION.dosraw)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: dosraw dosrawclean

all softcanvas plugins drivers drivers2d: dosraw
dosraw:
	$(MAKE_TARGET) MAKE_DLL=yes
dosrawclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/canvas/dosraw

ifeq ($(USE_PLUGINS),yes)
  RAW2D = $(OUTDLL)dosraw$(DLL)
  LIB.RAW2D = $(foreach d,$(DEP.RAW2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(RAW2D)
else
  RAW2D = $(OUT)$(LIB_PREFIX)dosraw$(LIB)
  DEP.EXE += $(RAW2D)
  SCF.STATIC += dosraw
  TO_INSTALL.STATIC_LIBS += $(RAW2D)
endif

INC.RAW2D = $(wildcard plugins/video/canvas/dosraw/*.h   $(INC.COMMON.DRV2D))
SRC.RAW2D = $(wildcard plugins/video/canvas/dosraw/*.cpp $(SRC.COMMON.DRV2D))
OBJ.RAW2D = $(addprefix $(OUT),$(notdir $(SRC.RAW2D:.cpp=$O)))
DEP.RAW2D = CSUTIL

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: dosraw dosrawclean

dosraw: $(OUTDIRS) $(RAW2D)

$(RAW2D): $(OBJ.RAW2D) $(LIB.RAW2D)
	$(DO.PLUGIN)

clean: dosrawclean
dosrawclean:
	$(RM) $(RAW2D) $(OBJ.RAW2D)

ifdef DO_DEPEND
dep: $(OUTOS)raw2d.dep
$(OUTOS)raw2d.dep: $(SRC.RAW2D)
	$(DO.DEP)
else
-include $(OUTOS)raw2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
