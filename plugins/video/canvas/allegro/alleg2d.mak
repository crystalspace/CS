# This is a subinclude file used to define the rules needed
# to build Allegro driver -- alleg2d

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

.PHONY: alleg2d alleg2dclean
all plugins drivers drivers2d: alleg2d

alleg2d:
	$(MAKE_TARGET) MAKE_DLL=yes
alleg2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# Link with Allegro libraries.
LIB.ALLEG2D.SYSTEM+=`allegro-config --libs release`

# The Allegro 2D driver
ifeq ($(USE_PLUGINS),yes)
  ALLEG2D = $(OUTDLL)alleg2d$(DLL)
  LIB.ALLEG2D = $(foreach d,$(DEP.ALLEG2D),$($d.LIB))
  LIB.ALLEG2D.SPECIAL += $(LIB.ALLEG2D.SYSTEM)
  TO_INSTALL.DYNAMIC_LIBS += $(ALLEG2D)
else
  ALLEG2D = $(OUT)$(LIB_PREFIX)alleg2d$(LIB)
  DEP.EXE += $(ALLEG2D)
  LIBS.EXE += $(LIB.ALLEG2D.SYSTEM)
  SCF.STATIC += alleg2d
  TO_INSTALL.STATIC_LIBS += $(ALLEG2D)
endif

INC.ALLEG2D = \
  $(wildcard plugins/video/canvas/allegro/*.h $(INC.COMMON.DRV2D))
SRC.ALLEG2D = \
  $(wildcard plugins/video/canvas/allegro/*.cpp $(SRC.COMMON.DRV2D))
OBJ.ALLEG2D = $(addprefix $(OUT),$(notdir $(SRC.ALLEG2D:.cpp=$O)))
DEP.ALLEG2D = CSUTIL CSSYS CSUTIL

#MSVC.DSP += ALLEG2D
#DSP.ALLEG2D.NAME = alleg2d
#DSP.ALLEG2D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

vpath %.cpp plugins/video/canvas/allegro

.PHONY: alleg2d alleg2dclean

# Chain rules
clean: alleg2dclean
alleg2d: $(OUTDIRS) $(ALLEG2D)

$(OUT)%$O: plugins/video/canvas/allegro/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.ALLEG2D)

$(ALLEG2D): $(OBJ.ALLEG2D) $(LIB.ALLEG2D)
	$(DO.PLUGIN) $(LIB.ALLEG2D.SPECIAL)

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
