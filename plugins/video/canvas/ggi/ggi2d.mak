# This is a subinclude file used to define the rules needed
# to build the GGI 2D driver -- ggi2d

# Driver description
DESCRIPTION.ggi2d = Crystal Space GGI 2D driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make ggi2d        Make the $(DESCRIPTION.ggi2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ggi2d ggi2dclean
all plugins drivers drivers2d: ggi2d

ggi2d:
	$(MAKE_TARGET) MAKE_DLL=yes
ggi2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

LIB.GGI2D.SYSTEM = -lggi

ifeq ($(USE_PLUGINS),yes)
  GGI2D = $(OUTDLL)ggi2d$(DLL)
  LIB.GGI2D = $(foreach d,$(DEP.GGI2D),$($d.LIB))
  LIB.GGI2D.SPECIAL += $(LIB.GGI2D.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(GGI2D)
else
  GGI2D = $(OUT)$(LIB_PREFIX)ggi2d$(LIB)
  DEP.EXE += $(GGI2D)
  LIBS.EXE += $(LIB.GGI2D.SYSTEM)
  SCF.STATIC += ggi2d
  TO_INSTALL.STATIC_LIBS += $(GGI2D)
endif

INC.GGI2D = $(wildcard plugins/video/canvas/ggi/*.h   $(INC.COMMON.DRV2D))
SRC.GGI2D = $(wildcard plugins/video/canvas/ggi/*.cpp $(SRC.COMMON.DRV2D))
OBJ.GGI2D = $(addprefix $(OUT),$(notdir $(SRC.GGI2D:.cpp=$O)))
DEP.GGI2D = CSUTIL CSSYS

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ggi2d ggi2dclean

ggi2d: $(OUTDIRS) $(GGI2D)

$(OUT)%$O: plugins/video/canvas/ggi/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.GGI2D)

$(GGI2D): $(OBJ.GGI2D) $(LIB.GGI2D)
	$(DO.PLUGIN) $(LIB.GGI2D.SPECIAL)

clean: ggi2dclean
ggi2dclean:
	$(RM) $(GGI2D) $(OBJ.GGI2D)

ifdef DO_DEPEND
dep: $(OUTOS)ggi2d.dep
$(OUTOS)ggi2d.dep: $(SRC.GGI2D)
	$(DO.DEP)
else
-include $(OUTOS)ggi2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
