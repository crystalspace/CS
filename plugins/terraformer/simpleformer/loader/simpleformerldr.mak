DESCRIPTION.simpleformerldr = Crystal Space simple terraformer loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make simpleformerldr$" \
  $(NEWLINE)echo $"                    Make the $(DESCRIPTION.simpleformerldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: simpleformerldr simpleformerldrclean
all plugins: simpleformerldr

simpleformerldr:
	$(MAKE_TARGET) MAKE_DLL=yes
simpleformerldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  SIMPLEFORMERLDR = $(OUTDLL)/simpleformerldr$(DLL)
  LIB.SIMPLEFORMERLDR = $(foreach d,$(DEP.SIMPLEFORMERLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SIMPLEFORMERLDR)
else
  SIMPLEFORMERLDR = $(OUT)/$(LIB_PREFIX)simpleformerldr$(LIB)
  DEP.EXE += $(SIMPLEFORMERLDR)
  SCF.STATIC += simpleformerldr
  TO_INSTALL.STATIC_LIBS += $(SIMPLEFORMERLDR)
endif

DIR.SIMPLEFORMERLDR = plugins/terraformer/simpleformer/loader
OUT.SIMPLEFORMERLDR = $(OUT)/$(DIR.SIMPLEFORMERLDR)
INF.SIMPLEFORMERLDR = $(SRCDIR)/$(DIR.SIMPLEFORMERLDR)/simpleformerldr.csplugin
INC.SIMPLEFORMERLDR = $(wildcard $(SRCDIR)/$(DIR.SIMPLEFORMERLDR)/*.h)
SRC.SIMPLEFORMERLDR = $(wildcard $(SRCDIR)/$(DIR.SIMPLEFORMERLDR)/*.cpp)
OBJ.SIMPLEFORMERLDR = \
  $(addprefix $(OUT.SIMPLEFORMERLDR)/,$(notdir $(SRC.SIMPLEFORMERLDR:.cpp=$O)))
DEP.SIMPLEFORMERLDR = CSGEOM CSUTIL

OUTDIRS += $(OUT.SIMPLEFORMERLDR)

MSVC.DSP += SIMPLEFORMERLDR
DSP.SIMPLEFORMERLDR.NAME = simpleformerldr
DSP.SIMPLEFORMERLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: simpleformerldr simpleformerldrclean simpleformerldrcleandep

simpleformerldr: $(OUTDIRS) $(SIMPLEFORMERLDR)

$(OUT.SIMPLEFORMERLDR)/%$O: $(SRCDIR)/$(DIR.SIMPLEFORMERLDR)/%.cpp
	$(DO.COMPILE.CPP)

$(SIMPLEFORMERLDR): $(OBJ.SIMPLEFORMERLDR) $(LIB.SIMPLEFORMERLDR)
	$(DO.PLUGIN)

clean: simpleformerldrclean
simpleformerldrclean:
	-$(RMDIR) $(SIMPLEFORMERLDR) $(OBJ.SIMPLEFORMERLDR) \
	$(OUTDLL)/$(notdir $(INF.SIMPLEFORMERLDR))

cleandep: simpleformerldrcleandep
simpleformerldrcleandep:
	-$(RM) $(OUT.SIMPLEFORMERLDR)/simpleformerldr.dep

ifdef DO_DEPEND
dep: $(OUT.SIMPLEFORMERLDR) $(OUT.SIMPLEFORMERLDR)/simpleformerldr.dep
$(OUT.SIMPLEFORMERLDR)/simpleformerldr.dep: $(SRC.SIMPLEFORMERLDR)
	$(DO.DEPEND)
else
-include $(OUT.SIMPLEFORMERLDR)/simpleformerldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
