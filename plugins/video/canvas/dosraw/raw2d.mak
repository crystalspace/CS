# This is a subinclude file used to define the rules needed
# to build DOS raw framebuffer access 2D driver raw2d

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

all plugins drivers drivers2d: dosraw

dosraw:
	$(MAKE_TARGET) MAKE_DLL=yes
dosrawclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The raw 2D DOS SVGA driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  RAW2D=$(OUTDLL)dosraw$(DLL)
  DEP.RAW2D=$(CSUTIL.LIB)
# $(CSSYS.LIB)
else
  RAW2D=$(OUT)$(LIB_PREFIX)dosraw$(LIB)
  DEP.EXE+=$(RAW2D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_RAW2D
endif
DESCRIPTION.$(RAW2D) = $(DESCRIPTION.dosraw)
SRC.RAW2D=$(wildcard plugins/video/canvas/dosraw/*.cpp $(SRC.COMMON.DRV2D))
OBJ.RAW2D = $(addprefix $(OUT),$(notdir $(SRC.RAW2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/dosraw

.PHONY: dosraw dosrawclean

# Chain rules
clean: dosrawclean
dosraw: $(OUTDIRS) $(RAW2D)

$(RAW2D): $(OBJ.RAW2D) $(DEP.RAW2D)
	$(DO.PLUGIN)

dosrawclean:
	$(RM) $(RAW2D) $(OBJ.RAW2D) $(OUTOS)raw2d.dep

ifdef DO_DEPEND
dep: $(OUTOS)raw2d.dep
$(OUTOS)raw2d.dep: $(SRC.RAW2D)
	$(DO.DEP)
else
-include $(OUTOS)raw2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
