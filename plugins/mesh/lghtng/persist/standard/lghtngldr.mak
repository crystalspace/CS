#------------------------------------------------------------------------------
# Lghtngldr subemakefile
#------------------------------------------------------------------------------

DESCRIPTION.lghtngldr = Crystal Space lightning loader plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make lghtngldr    Make the $(DESCRIPTION.lghtngldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: lghtngldr lghtngldrclean
all meshes plugins: lghtngldr

lghtngldr:
	$(MAKE_TARGET) MAKE_DLL=yes
lghtngldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  LGHTNGLDR = $(OUTDLL)/lghtngldr$(DLL)
  LIB.LGHTNGLDR = $(foreach d,$(DEP.LGHTNGLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(LGHTNGLDR)
else
  LGHTNGLDR = $(OUT)/$(LIB_PREFIX)lghtngldr$(LIB)
  DEP.EXE += $(LGHTNGLDR)
  SCF.STATIC += lghtngldr
  TO_INSTALL.STATIC_LIBS += $(LGHTNGLDR)
endif

DIR.LGHTNGLDR = plugins/mesh/lghtng/persist/standard
OUT.LGHTNGLDR = $(OUT)/$(DIR.LGHTNGLDR)
INF.LGHTNGLDR = $(SRCDIR)/$(DIR.LGHTNGLDR)/lghtngldr.csplugin
INC.LGHTNGLDR = $(wildcard $(SRCDIR)/$(DIR.LGHTNGLDR)/*.h)
SRC.LGHTNGLDR = $(wildcard $(SRCDIR)/$(DIR.LGHTNGLDR)/*.cpp)
OBJ.LGHTNGLDR = \
  $(addprefix $(OUT.LGHTNGLDR)/,$(notdir $(SRC.LGHTNGLDR:.cpp=$O)))
DEP.LGHTNGLDR = CSGEOM CSUTIL

OUTDIRS += $(OUT.LGHTNGLDR)

MSVC.DSP += LGHTNGLDR
DSP.LGHTNGLDR.NAME = lghtngldr
DSP.LGHTNGLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: lghtngldr lghtngldrclean lghtngldrcleandep

lghtngldr: $(OUTDIRS) $(LGHTNGLDR)

$(OUT.LGHTNGLDR)/%$O: $(SRCDIR)/$(DIR.LGHTNGLDR)/%.cpp
	$(DO.COMPILE.CPP)

$(LGHTNGLDR): $(OBJ.LGHTNGLDR) $(LIB.LGHTNGLDR)
	$(DO.PLUGIN)

clean: lghtngldrclean
lghtngldrclean:
	-$(RMDIR) $(LGHTNGLDR) $(OBJ.LGHTNGLDR) \
	$(OUTDLL)/$(notdir $(INF.LGHTNGLDR))

cleandep: lghtngldrcleandep
lghtngldrcleandep:
	-$(RM) $(OUT.LGHTNGLDR)/lghtngldr.dep

ifdef DO_DEPEND
dep: $(OUT.LGHTNGLDR) $(OUT.LGHTNGLDR)/lghtngldr.dep
$(OUT.LGHTNGLDR)/lghtngldr.dep: $(SRC.LGHTNGLDR)
	$(DO.DEPEND)
else
-include $(OUT.LGHTNGLDR)/lghtngldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
