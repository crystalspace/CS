# This is a subinclude file used to define the rules needed
# to build the metaball plug-in.

# Plug-in description
DESCRIPTION.metaball = MetaBalls mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make metaball     Make the $(DESCRIPTION.metaball)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: metaball metaballclean
meshes all plugins: metaball

metaball:
	$(MAKE_TARGET) MAKE_DLL=yes
metaballclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/metaball/object

ifeq ($(USE_PLUGINS),yes)
  METABALL = $(OUTDLL)metaball$(DLL)
  LIB.METABALL = $(foreach d,$(DEP.METABALL),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(METABALL)
else
  METABALL = $(OUT)$(LIB_PREFIX)metaball$(LIB)
  DEP.EXE += $(METABALL)
  SCF.STATIC += metaball
  TO_INSTALL.STATIC_LIBS += $(METABALL)
endif

INC.METABALL = $(wildcard plugins/mesh/metaball/object/*.h)
SRC.METABALL = $(wildcard plugins/mesh/metaball/object/*.cpp)
OBJ.METABALL = $(addprefix $(OUT),$(notdir $(SRC.METABALL:.cpp=$O)))
DEP.METABALL = CSGEOM CSUTIL CSSYS

MSVC.DSP += METABALL
DSP.METABALL.NAME = metaball
DSP.METABALL.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: metaball metaballclean

# Chain rules
net: metaball
clean: metaballclean

metaball: $(OUTDIRS) $(METABALL)

$(METABALL): $(OBJ.METABALL) $(LIB.METABALL)
	$(DO.PLUGIN)

metaballclean:
	$(RM) $(METABALL) $(OBJ.METABALL)

ifdef DO_DEPEND
dep: $(OUTOS)metaball.dep
$(OUTOS)metaball.dep: $(SRC.METABALL)
	$(DO.DEP)
else
-include $(OUTOS)metaball.dep
endif

endif # ifeq ($(MAKESECTION),targets)
