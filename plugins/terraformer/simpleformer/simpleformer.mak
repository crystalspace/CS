# This is a subinclude file used to define the rules needed
# to build the simpleformer plug-in.

# Driver description
DESCRIPTION.simpleformer = Crystal Space Terraformer plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make simpleformer     Make the $(DESCRIPTION.simpleformer)$"

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

INF.SIMPLEFORMER = $(SRCDIR)/plugins/physics/simpleformer/simpleformer.csplugin
INC.SIMPLEFORMER = $(wildcard $(addprefix $(SRCDIR)/,plugins/physics/simpleformer/*.h))
SRC.SIMPLEFORMER = $(wildcard $(addprefix $(SRCDIR)/,plugins/physics/simpleformer/*.cpp))
OBJ.SIMPLEFORMER = $(addprefix $(OUT)/,$(notdir $(SRC.SIMPLEFORMER:.cpp=$O)))
DEP.SIMPLEFORMER = CSGEOM CSUTIL

MSVC.DSP += SIMPLEFORMER
DSP.SIMPLEFORMER.NAME = simpleformer
DSP.SIMPLEFORMER.TYPE = plugin
endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: simpleformer simpleformerclean
simpleformer: $(OUTDIRS) $(SIMPLEFORMER)

$(OUT)/%$O: $(SRCDIR)/plugins/physics/simpleformer/%.cpp
	$(DO.COMPILE.CPP) $(ODE.CFLAGS)

$(SIMPLEFORMER): $(OBJ.SIMPLEFORMER) $(LIB.SIMPLEFORMER)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(ODE.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: simpleformerclean
simpleformerclean:
	-$(RMDIR) $(SIMPLEFORMER) $(OBJ.SIMPLEFORMER) $(OUTDLL)/$(notdir $(INF.SIMPLEFORMER))

ifdef DO_DEPEND
dep: $(OUTOS)/simpleformer.dep
$(OUTOS)/simpleformer.dep: $(SRC.SIMPLEFORMER)
	$(DO.DEP1) \
	$(ODE.CFLAGS) \
	$(DO.DEP2)
else
-include $(OUTOS)/simpleformer.dep
endif

endif # ifeq ($(MAKESECTION),targets)
