# This is a subinclude file used to define the rules needed
# to build the odedynam plug-in.

# Driver description
DESCRIPTION.odedynam = Crystal Space ODE dynamics plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make odedynam     Make the $(DESCRIPTION.odedynam)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: odedynam odedynamclean
all plugins: odedynam

odedynam:
	$(MAKE_TARGET) MAKE_DLL=yes
odedynamclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  ODEDYNAM = $(OUTDLL)/odedynam$(DLL)
  LIB.ODEDYNAM = $(foreach d,$(DEP.ODEDYNAM),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ODEDYNAM)
else
  ODEDYNAM = $(OUT)/$(LIB_PREFIX)odedynam$(LIB)
  DEP.EXE += $(ODEDYNAM)
  SCF.STATIC += odedynam
  TO_INSTALL.STATIC_LIBS += $(ODEDYNAM)
endif

INF.ODEDYNAM = $(SRCDIR)/plugins/physics/odedynam/odedynam.csplugin
INC.ODEDYNAM = $(wildcard $(addprefix $(SRCDIR)/,plugins/physics/odedynam/*.h))
SRC.ODEDYNAM = $(wildcard $(addprefix $(SRCDIR)/,plugins/physics/odedynam/*.cpp))
OBJ.ODEDYNAM = $(addprefix $(OUT)/,$(notdir $(SRC.ODEDYNAM:.cpp=$O)))
DEP.ODEDYNAM = CSGEOM CSUTIL

MSVC.DSP += ODEDYNAM
DSP.ODEDYNAM.NAME = odedynam
DSP.ODEDYNAM.TYPE = plugin
DSP.ODEDYNAM.LIBS = ode

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: odedynam odedynamclean
odedynam: $(OUTDIRS) $(ODEDYNAM)

$(OUT)/%$O: $(SRCDIR)/plugins/physics/odedynam/%.cpp
	$(DO.COMPILE.CPP) $(ODE.CFLAGS)

$(ODEDYNAM): $(OBJ.ODEDYNAM) $(LIB.ODEDYNAM)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(ODE.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: odedynamclean
odedynamclean:
	-$(RMDIR) $(ODEDYNAM) $(OBJ.ODEDYNAM) $(OUTDLL)/$(notdir $(INF.ODEDYNAM))

ifdef DO_DEPEND
dep: $(OUTOS)/odedynam.dep
$(OUTOS)/odedynam.dep: $(SRC.ODEDYNAM)
	$(DO.DEP1) \
	$(ODE.CFLAGS) \
	$(DO.DEP2)
else
-include $(OUTOS)/odedynam.dep
endif

endif # ifeq ($(MAKESECTION),targets)
