DESCRIPTION.simpleformer = Crystal Space simple terraformer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make simpleformer Make the $(DESCRIPTION.simpleformer)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simpleformer simpleformerclean
all plugins: simpleformer

simpleformer:
	$(MAKE_TARGET) MAKE_DLL=yes
simpleformerclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  SIMPLEFORMER = $(OUTDLL)/simpleformer$(DLL)
  LIB.SIMPLEFORMER = $(foreach d,$(DEP.SIMPLEFORMER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SIMPLEFORMER)
else
  SIMPLEFORMER = $(OUT)/$(LIB_PREFIX)simpleformer$(LIB)
  DEP.EXE += $(SIMPLEFORMER)
  SCF.STATIC += simpleformer
  TO_INSTALL.STATIC_LIBS += $(SIMPLEFORMER)
endif

DIR.SIMPLEFORMER = plugins/terraformer/simpleformer
OUT.SIMPLEFORMER = $(OUT)/$(DIR.SIMPLEFORMER)
INF.SIMPLEFORMER = $(SRCDIR)/$(DIR.SIMPLEFORMER)/simpleformer.csplugin
INC.SIMPLEFORMER = $(wildcard $(SRCDIR)/$(DIR.SIMPLEFORMER)/*.h)
SRC.SIMPLEFORMER = $(wildcard $(SRCDIR)/$(DIR.SIMPLEFORMER)/*.cpp)
OBJ.SIMPLEFORMER = \
  $(addprefix $(OUT.SIMPLEFORMER)/,$(notdir $(SRC.SIMPLEFORMER:.cpp=$O)))
DEP.SIMPLEFORMER = CSGEOM CSUTIL

OUTDIRS += $(OUT.SIMPLEFORMER)

MSVC.DSP += SIMPLEFORMER
DSP.SIMPLEFORMER.NAME = simpleformer
DSP.SIMPLEFORMER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: simpleformer simpleformerclean simpleformercleandep

simpleformer: $(OUTDIRS) $(SIMPLEFORMER)

$(OUT.SIMPLEFORMER)/%$O: $(SRCDIR)/$(DIR.SIMPLEFORMER)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPLEFORMER): $(OBJ.SIMPLEFORMER) $(LIB.SIMPLEFORMER)
	$(DO.PLUGIN)

clean: simpleformerclean
simpleformerclean:
	-$(RMDIR) $(SIMPLEFORMER) $(OBJ.SIMPLEFORMER) $(OUTDLL)/$(notdir $(INF.SIMPLEFORMER))

cleandep: simpleformercleandep
simpleformercleandep:
	-$(RM) $(OUT.SIMPLEFORMER)/simpleformer.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPLEFORMER) $(OUT.SIMPLEFORMER)/simpleformer.dep
$(OUT.SIMPLEFORMER)/simpleformer.dep: $(SRC.SIMPLEFORMER)
	$(DO.DEPEND)
else
-include $(OUT.SIMPLEFORMER)/simpleformer.dep
endif

endif # ifeq ($(MAKESECTION),targets)
