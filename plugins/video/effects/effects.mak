# This is a subinclude file used to define the rules needed
# to build the effects plug-in.

# Driver description
DESCRIPTION.effects = Crystal Space Effects plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make effects      Make the $(DESCRIPTION.effects)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: effects effectsclean
all plugins: effects

effects:
	$(MAKE_TARGET) MAKE_DLL=yes
effectsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/video/effects

ifeq ($(USE_PLUGINS),yes)
  EFFECTS = $(OUTDLL)/effects$(DLL)
  LIB.EFFECTS = $(foreach d,$(DEP.EFFECTS),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(EFFECTS)
else
  EFFECTS = $(OUT)/$(LIB_PREFIX)effects$(LIB)
  DEP.EXE += $(EFFECTS)
  SCF.STATIC += effects
  TO_INSTALL.STATIC_LIBS += $(EFFECTS)
endif

INC.EFFECTS = $(wildcard plugins/video/effects/*.h)
SRC.EFFECTS = $(wildcard plugins/video/effects/*.cpp)
OBJ.EFFECTS = $(addprefix $(OUT)/,$(notdir $(SRC.EFFECTS:.cpp=$O)))
DEP.EFFECTS = CSTOOL CSGEOM CSUTIL CSSYS CSUTIL

TO_INSTALL.CONFIG += $(CFG.EFFECTS)

MSVC.DSP += EFFECTS
DSP.EFFECTS.NAME = effects
DSP.EFFECTS.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: effects effectsclean

effects: $(OUTDIRS) $(EFFECTS)

$(EFFECTS): $(OBJ.EFFECTS) $(LIB.EFFECTS)
	$(DO.PLUGIN)

clean: effectsclean
effectsclean:
	$(RM) $(EFFECTS) $(OBJ.EFFECTS)

ifdef DO_DEPEND
dep: $(OUTOS)/effects.dep
$(OUTOS)/effects.dep: $(SRC.EFFECTS)
	$(DO.DEP)
else
-include $(OUTOS)/effects.dep
endif

endif # ifeq ($(MAKESECTION),targets)
