DESCRIPTION.csconout = Crystal Space standard output console

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make csconout     Make the $(DESCRIPTION.csconout)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconout csconoutclean
all plugins: csconout

csconout:
	$(MAKE_TARGET) MAKE_DLL=yes
csconoutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  CSCONOUT = $(OUTDLL)/csconout$(DLL)
  LIB.CSCONOUT = $(foreach d,$(DEP.CSCONOUT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSCONOUT)
else
  CSCONOUT = $(OUT)/$(LIB_PREFIX)csconout$(LIB)
  DEP.EXE += $(CSCONOUT)
  SCF.STATIC += csconout
  TO_INSTALL.STATIC_LIBS += $(CSCONOUT)
endif

DIR.CSCONOUT = plugins/console/output/standard
OUT.CSCONOUT = $(OUT)/$(DIR.CSCONOUT)
INF.CSCONOUT = $(SRCDIR)/$(DIR.CSCONOUT)/csconout.csplugin
INC.CSCONOUT = $(wildcard $(SRCDIR)/$(DIR.CSCONOUT)/*.h)
SRC.CSCONOUT = $(wildcard $(SRCDIR)/$(DIR.CSCONOUT)/*.cpp)
OBJ.CSCONOUT = $(addprefix $(OUT.CSCONOUT)/,$(notdir $(SRC.CSCONOUT:.cpp=$O)))
DEP.CSCONOUT = CSGEOM CSUTIL
CFG.CSCONOUT = $(SRCDIR)/data/config/standardcon.cfg

OUTDIRS += $(OUT.CSCONOUT)

TO_INSTALL.CONFIG += $(CFG.CSCONOUT)

MSVC.DSP += CSCONOUT
DSP.CSCONOUT.NAME = csconout
DSP.CSCONOUT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconout csconoutclean csconoutcleandep

csconout: $(OUTDIRS) $(CSCONOUT)

$(OUT.CSCONOUT)/%$O: $(SRCDIR)/$(DIR.CSCONOUT)/%.cpp
	$(DO.COMPILE.CPP)

$(CSCONOUT): $(OBJ.CSCONOUT) $(LIB.CSCONOUT)
	$(DO.PLUGIN)

clean: csconoutclean
csconoutclean:
	-$(RMDIR) $(CSCONOUT) $(OBJ.CSCONOUT) \
	$(OUTDLL)/$(notdir $(INF.CSCONOUT))

cleandep: csconoutcleandep
csconoutcleandep:
	-$(RM) $(OUT.CSCONOUT)/csconout.dep

ifdef DO_DEPEND
dep: $(OUT.CSCONOUT) $(OUT.CSCONOUT)/csconout.dep
$(OUT.CSCONOUT)/csconout.dep: $(SRC.CSCONOUT)
	$(DO.DEPEND)
else
-include $(OUT.CSCONOUT)/csconout.dep
endif

endif # ifeq ($(MAKESECTION),targets)
