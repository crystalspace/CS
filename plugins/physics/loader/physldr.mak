DESCRIPTION.physldr = Crystal Space Physics loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make physldr      Make the $(DESCRIPTION.physldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: physldr physldrclean
all plugins: physldr

physldr:
	$(MAKE_TARGET) MAKE_DLL=yes
physldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  PHYSLDR = $(OUTDLL)/physldr$(DLL)
  LIB.PHYSLDR = $(foreach d,$(DEP.PHYSLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(PHYSLDR)
else
  PHYSLDR = $(OUT)/$(LIB_PREFIX)physldr$(LIB)
  DEP.EXE += $(PHYSLDR)
  SCF.STATIC += physldr
  TO_INSTALL.STATIC_LIBS += $(PHYSLDR)
endif

DIR.PHYSLDR = plugins/physics/loader
OUT.PHYSLDR = $(OUT)/$(DIR.PHYSLDR)
INF.PHYSLDR = $(SRCDIR)/$(DIR.PHYSLDR)/physldr.csplugin
INC.PHYSLDR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PHYSLDR)/*.h))
SRC.PHYSLDR = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.PHYSLDR)/*.cpp))
OBJ.PHYSLDR = $(addprefix $(OUT.PHYSLDR)/,$(notdir $(SRC.PHYSLDR:.cpp=$O)))
DEP.PHYSLDR = CSGEOM CSUTIL

OUTDIRS += $(OUT.PHYSLDR)

MSVC.DSP += PHYSLDR
DSP.PHYSLDR.NAME = physldr
DSP.PHYSLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: physldr physldrclean physldrcleandep

physldr: $(OUTDIRS) $(PHYSLDR)

$(OUT.PHYSLDR)/%$O: $(SRCDIR)/$(DIR.PHYSLDR)/%.cpp
	$(DO.COMPILE.CPP)

$(PHYSLDR): $(OBJ.PHYSLDR) $(LIB.PHYSLDR)
	$(DO.PLUGIN)

clean: physldrclean
physldrclean:
	-$(RMDIR) $(PHYSLDR) $(OBJ.PHYSLDR) $(OUTDLL)/$(notdir $(INF.PHYSLDR))

cleandep: physldrcleandep
physldrcleandep:
	-$(RM) $(OUT.PHYSLDR)/physldr.dep

ifdef DO_DEPEND
dep: $(OUT.PHYSLDR) $(OUT.PHYSLDR)/physldr.dep
$(OUT.PHYSLDR)/physldr.dep: $(SRC.PHYSLDR)
	$(DO.DEPEND)
else
-include $(OUT.PHYSLDR)/physldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
