#------------------------------------------------------------------------------
# Isometric loader plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.isoload = Crystal Space isometric world loader plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make isoload      Make the $(DESCRIPTION.isoload)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: isoload isoloadclean
all plugins: isoload
isoload:
	$(MAKE_TARGET) MAKE_DLL=yes
isoloadclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/isoldr

ifeq ($(USE_PLUGINS),yes)
  ISOLOAD = $(OUTDLL)/isoload$(DLL)
  LIB.ISOLOAD = $(foreach d,$(DEP.ISO),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ISOLOAD)
else
  ISOLOAD = $(OUT)/$(LIB_PREFIX)isoload$(LIB)
  DEP.EXE += $(ISOLOAD)
  SCF.STATIC += isoload
  TO_INSTALL.STATIC_LIBS += $(ISOLOAD)
endif

INF.ISOLOAD = $(SRCDIR)/plugins/isoldr/isoload.csplugin
INC.ISOLOAD = $(wildcard $(addprefix $(SRCDIR)/,plugins/isoldr/*.h))
SRC.ISOLOAD = $(wildcard $(addprefix $(SRCDIR)/,plugins/isoldr/*.cpp))
OBJ.ISOLOAD = $(addprefix $(OUT)/,$(notdir $(SRC.ISOLOAD:.cpp=$O)))
DEP.ISOLOAD = CSUTIL CSGEOM CSGFX CSUTIL

MSVC.DSP += ISOLOAD
DSP.ISOLOAD.NAME = isoload
DSP.ISOLOAD.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: isoload isoloadclean
isoload: $(OUTDIRS) $(ISOLOAD)

$(ISOLOAD): $(OBJ.ISOLOAD) $(LIB.ISOLOAD)
	$(DO.PLUGIN)

clean: isoloadclean
isoloadclean:
	-$(RMDIR) $(ISOLOAD) $(OBJ.ISOLOAD) $(OUTDLL)/$(notdir $(INF.ISOLOAD))

ifdef DO_DEPEND
dep: $(OUTOS)/isoload.dep
$(OUTOS)/isoload.dep: $(SRC.ISOLOAD)
	$(DO.DEP)
else
-include $(OUTOS)/isoload.dep
endif

endif # ifeq ($(MAKESECTION),targets)
