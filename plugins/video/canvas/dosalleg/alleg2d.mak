# This is a subinclude file used to define the rules needed
# to build DOS Allegro driver alleg2d

# Driver description
DESCRIPTION.alleg2d = Crystal Space Allegro driver

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make alleg2d      Make the $(DESCRIPTION.alleg2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: alleg2d

all plugins drivers drivers2d: alleg2d

alleg2d:
	$(MAKE_TARGET) MAKE_DLL=yes
alleg2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# The Allegro 2D DOS driver
ifeq ($(USE_PLUGINS),yes)
  ALLEG2D=$(OUTDLL)alg2d$(DLL)
  DEP.ALLEG2D=
  TO_INSTALL.DYNAMIC_LIBS+=$(ALLEG2D)
else
  ALLEG2D=$(OUT)$(LIB_PREFIX)alg2d$(LIB)
  DEP.EXE+=$(ALLEG2D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_DOSALLEGRO
  TO_INSTALL.STATIC_LIBS+=$(ALLEG2D)
endif
DESCRIPTION.$(ALLEG2D) = $(DESCRIPTION.alleg2d)
SRC.ALLEG2D=$(wildcard plugins/video/canvas/dosalleg/*.cpp $(SRC.COMMON.DRV2D))
OBJ.ALLEG2D = $(addprefix $(OUT),$(notdir $(SRC.ALLEG2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/dosalleg

.PHONY: alleg2d alleg2dclean

# Chain rules
clean: alleg2dclean
alleg2d: $(OUTDIRS) $(ALLEG2D)

$(ALLEG2D): $(OBJ.ALLEG2D) $(DEP.ALLEG2D)
	$(DO.PLUGIN)

alleg2dclean:
	$(RM) $(ALLEG2D) $(OBJ.ALLEG2D)

ifdef DO_DEPEND
dep: $(OUTOS)alleg2d.dep
$(OUTOS)alleg2d.dep: $(SRC.ALLEG2D)
	$(DO.DEP)
else
-include $(OUTOS)alleg2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
