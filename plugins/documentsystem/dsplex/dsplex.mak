#------------------------------------------------------------------------------
# Document system multiplexer submakefile
#------------------------------------------------------------------------------
DESCRIPTION.dsplex = Crystal Space document system multiplexer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make dsplex       Make the $(DESCRIPTION.dsplex)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: dsplex dsplexclean
all plugins: dsplex

dsplex:
	$(MAKE_TARGET) MAKE_DLL=yes
dsplexclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  DSPLEX = $(OUTDLL)/dsplex$(DLL)
  LIB.DSPLEX = $(foreach d,$(DEP.DSPLEX),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(DSPLEX)
else
  DSPLEX = $(OUT)/$(LIB_PREFIX)dsplex$(LIB)
  DEP.EXE += $(DSPLEX)
  SCF.STATIC += dsplex
  TO_INSTALL.STATIC_LIBS += $(DSPLEX)
endif

DIR.DSPLEX = plugins/documentsystem/dsplex
OUT.DSPLEX = $(OUT)/$(DIR.DSPLEX)
INF.DSPLEX = $(SRCDIR)/$(DIR.DSPLEX)/dsplex.csplugin
INC.DSPLEX = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.DSPLEX)/*.h))
SRC.DSPLEX = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.DSPLEX)/*.cpp))
OBJ.DSPLEX = $(addprefix $(OUT.DSPLEX)/,$(notdir $(SRC.DSPLEX:.cpp=$O)))
DEP.DSPLEX = CSUTIL CSTOOL CSUTIL

OUTDIRS += $(OUT.DSPLEX)

MSVC.DSP += DSPLEX
DSP.DSPLEX.NAME = dsplex
DSP.DSPLEX.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: dsplex dsplexclean dsplexcleandep
dsplex: $(OUTDIRS) $(DSPLEX)

$(OUT.DSPLEX)/%$O: $(SRCDIR)/$(DIR.DSPLEX)/%.cpp
	$(DO.COMPILE.CPP)

$(DSPLEX): $(OBJ.DSPLEX) $(LIB.DSPLEX)
	$(DO.PLUGIN)

clean: dsplexclean
dsplexclean:
	-$(RMDIR) $(DSPLEX) $(OBJ.DSPLEX) $(OUTDLL)/$(notdir $(INF.DSPLEX))

cleandep: dsplexcleandep
dsplexcleandep:
	-$(RM) $(OUT.DSPLEX)/dsplex.dep

ifdef DO_DEPEND
dep: $(OUT.DSPLEX) $(OUT.DSPLEX)/dsplex.dep
$(OUT.DSPLEX)/dsplex.dep: $(SRC.DSPLEX)
	$(DO.DEPEND)
else
-include $(OUT.DSPLEX)/dsplex.dep
endif

endif # ifeq ($(MAKESECTION),targets)
