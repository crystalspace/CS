#------------------------------------------------------------------------------
# Bugplug subemakefile
#------------------------------------------------------------------------------

DESCRIPTION.cscursor = Crystal Space custom cursor plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make cscursor     Make the $(DESCRIPTION.cscursor)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cscursor cscursorclean
all plugins: cscursor

cscursor:
	$(MAKE_TARGET) MAKE_DLL=yes
cscursorclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  CSCURSOR = $(OUTDLL)/cscursor$(DLL)
  LIB.CSCURSOR = $(foreach d,$(DEP.CSCURSOR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSCURSOR)
else
  CSCURSOR = $(OUT)/$(LIB_PREFIX)cscursor$(LIB)
  DEP.EXE += $(CSCURSOR)
  SCF.STATIC += cscursor
  TO_INSTALL.STATIC_LIBS += $(CSCURSOR)
endif

DIR.CSCURSOR = plugins/video/cursor
OUT.CSCURSOR = $(OUT)/$(DIR.CSCURSOR)
INF.CSCURSOR = $(SRCDIR)/$(DIR.CSCURSOR)/cscursor.csplugin
INC.CSCURSOR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSCURSOR)/*.h))
SRC.CSCURSOR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.CSCURSOR)/*.cpp))
OBJ.CSCURSOR = $(addprefix $(OUT.CSCURSOR)/,$(notdir $(SRC.CSCURSOR:.cpp=$O)))
DEP.CSCURSOR = CSTOOL CSUTIL
#CFG.CSCURSOR = $(SRCDIR)/data/config/cscursor.cfg

OUTDIRS += $(OUT.CSCURSOR)

#TO_INSTALL.CONFIG += $(CFG.CSCURSOR)

MSVC.DSP += CSCURSOR
DSP.CSCURSOR.NAME = cscursor
DSP.CSCURSOR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cscursor cscursorclean cscursorcleandep

cscursor: $(OUTDIRS) $(CSCURSOR)

$(OUT.CSCURSOR)/%$O: $(SRCDIR)/$(DIR.CSCURSOR)/%.cpp
	$(DO.COMPILE.CPP)

$(CSCURSOR): $(OBJ.CSCURSOR) $(LIB.CSCURSOR)
	$(DO.PLUGIN)

clean: cscursorclean
cscursorclean:
	-$(RMDIR) $(CSCURSOR) $(OBJ.CSCURSOR) \
	$(OUTDLL)/$(notdir $(INF.CSCURSOR))

cleandep: cscursorcleandep
cscursorcleandep:
	-$(RM) $(OUT.CSCURSOR)/cscursor.dep

ifdef DO_DEPEND
dep: $(OUT.CSCURSOR) $(OUT.CSCURSOR)/cscursor.dep
$(OUT.CSCURSOR)/cscursor.dep: $(SRC.CSCURSOR)
	$(DO.DEPEND)
else
-include $(OUT.CSCURSOR)/cscursor.dep
endif

endif # ifeq ($(MAKESECTION),targets)
