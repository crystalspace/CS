#------------------------------------------------------------------------------
# Bugplug subemakefile
#------------------------------------------------------------------------------

DESCRIPTION.bugplug = Crystal Space debugging plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make bugplug      Make the $(DESCRIPTION.bugplug)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bugplug bugplugclean
all plugins: bugplug

bugplug:
	$(MAKE_TARGET) MAKE_DLL=yes
bugplugclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  BUGPLUG = $(OUTDLL)/bugplug$(DLL)
  LIB.BUGPLUG = $(foreach d,$(DEP.BUGPLUG),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BUGPLUG)
else
  BUGPLUG = $(OUT)/$(LIB_PREFIX)bugplug$(LIB)
  DEP.EXE += $(BUGPLUG)
  SCF.STATIC += bugplug
  TO_INSTALL.STATIC_LIBS += $(BUGPLUG)
endif

DIR.BUGPLUG = plugins/bugplug
OUT.BUGPLUG = $(OUT)/$(DIR.BUGPLUG)
INF.BUGPLUG = $(SRCDIR)/$(DIR.BUGPLUG)/bugplug.csplugin
INC.BUGPLUG = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.BUGPLUG)/*.h))
SRC.BUGPLUG = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.BUGPLUG)/*.cpp))
OBJ.BUGPLUG = $(addprefix $(OUT.BUGPLUG)/,$(notdir $(SRC.BUGPLUG:.cpp=$O)))
DEP.BUGPLUG = CSTOOL CSGEOM CSUTIL CSUTIL
CFG.BUGPLUG = $(SRCDIR)/data/config/bugplug.cfg

OUTDIRS += $(OUT.BUGPLUG)

TO_INSTALL.CONFIG += $(CFG.BUGPLUG)

MSVC.DSP += BUGPLUG
DSP.BUGPLUG.NAME = bugplug
DSP.BUGPLUG.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bugplug bugplugclean bugplugcleandep

bugplug: $(OUTDIRS) $(BUGPLUG)

$(OUT.BUGPLUG)/%$O: $(SRCDIR)/$(DIR.BUGPLUG)/%.cpp
	$(DO.COMPILE.CPP)

$(BUGPLUG): $(OBJ.BUGPLUG) $(LIB.BUGPLUG)
	$(DO.PLUGIN)

clean: bugplugclean
bugplugclean:
	-$(RMDIR) $(BUGPLUG) $(OBJ.BUGPLUG) $(OUTDLL)/$(notdir $(INF.BUGPLUG))

cleandep: bugplugcleandep
bugplugcleandep:
	-$(RM) $(OUT.BUGPLUG)/bugplug.dep

ifdef DO_DEPEND
dep: $(OUT.BUGPLUG) $(OUT.BUGPLUG)/bugplug.dep
$(OUT.BUGPLUG)/bugplug.dep: $(SRC.BUGPLUG)
	$(DO.DEPEND)
else
-include $(OUT.BUGPLUG)/bugplug.dep
endif

endif # ifeq ($(MAKESECTION),targets)
