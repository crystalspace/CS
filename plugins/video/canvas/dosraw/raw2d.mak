# This is a subinclude file used to define the rules needed
# to build DOS raw framebuffer access 2D driver raw2d

# Driver description
DESCRIPTION.raw2d = Crystal Space raw DOS SVGA driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make raw2d        Make the $(DESCRIPTION.raw2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: raw2d

ifeq ($(USE_DLL),yes)
all drivers drivers2d: raw2d
endif

raw2d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The raw 2D DOS SVGA driver
RAW2D=$(OUT)$(LIB_PREFIX)raw2d$(LIB)
DEP.EXE+=$(RAW2D)
DESCRIPTION.$(RAW2D) = $(DESCRIPTION.raw2d)
SRC.RAW2D=$(wildcard libs/cs2d/dosraw/*.cpp $(SRC.COMMON.DRV2D))
OBJ.RAW2D = $(addprefix $(OUT),$(notdir $(SRC.RAW2D:.cpp=$O)))
CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_RAW2D

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp libs/cs2d/dosraw

.PHONY: raw2dclean raw2dcleanlib raw2d

# Chain rules
clean: raw2dclean
cleanlib: raw2dcleanlib
raw2d: $(OUTDIRS) $(RAW2D)

$(RAW2D): $(OBJ.RAW2D)
	$(DO.LIBRARY)

raw2dclean:
	$(RM) $(RAW2D)

raw2dcleanlib:
	$(RM) $(OBJ.RAW2D) $(RAW2D)

ifdef DO_DEPEND
$(OUTOS)raw2d.dep: $(SRC.RAW2D)
	$(DO.DEP) $(OUTOS)raw2d.dep
endif

-include $(OUTOS)raw2d.dep

endif # ifeq ($(MAKESECTION),targets)
