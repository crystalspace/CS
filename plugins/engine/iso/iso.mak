#------------------------------------------------------------------------------
# Isometric plugin submakefile
#------------------------------------------------------------------------------
DESCRIPTION.iso = Crystal Space isometric engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make iso          Make the $(DESCRIPTION.iso)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: iso isoclean
all plugins: iso
iso:
	$(MAKE_TARGET) MAKE_DLL=yes
isoclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  ISO = $(OUTDLL)/iso$(DLL)
  LIB.ISO = $(foreach d,$(DEP.ISO),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(ISO)
else
  ISO = $(OUT)/$(LIB_PREFIX)iso$(LIB)
  DEP.EXE += $(ISO)
  SCF.STATIC += iso
  TO_INSTALL.STATIC_LIBS += $(ISO)
endif

DIR.ISO = plugins/engine/iso
OUT.ISO = $(OUT)/$(DIR.ISO)
INF.ISO = $(SRCDIR)/$(DIR.ISO)/iso.csplugin
INC.ISO = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.ISO)/*.h))
SRC.ISO = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.ISO)/*.cpp))
OBJ.ISO = $(addprefix $(OUT.ISO)/,$(notdir $(SRC.ISO:.cpp=$O)))
DEP.ISO = CSUTIL CSGEOM CSGFX CSUTIL

OUTDIRS += $(OUT.ISO)

MSVC.DSP += ISO
DSP.ISO.NAME = iso
DSP.ISO.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: iso isoclean isocleandep

iso: $(OUTDIRS) $(ISO)

$(OUT.ISO)/%$O: $(SRCDIR)/$(DIR.ISO)/%.cpp
	$(DO.COMPILE.CPP)

$(ISO): $(OBJ.ISO) $(LIB.ISO)
	$(DO.PLUGIN)

clean: isoclean
isoclean:
	-$(RMDIR) $(ISO) $(OBJ.ISO) $(OUTDLL)/$(notdir $(INF.ISO))

cleandep: isocleandep
isocleandep:
	-$(RM) $(OUT.ISO)/iso.dep

ifdef DO_DEPEND
dep: $(OUT.ISO) $(OUT.ISO)/iso.dep
$(OUT.ISO)/iso.dep: $(SRC.ISO)
	$(DO.DEPEND)
else
-include $(OUT.ISO)/iso.dep
endif

endif # ifeq ($(MAKESECTION),targets)
