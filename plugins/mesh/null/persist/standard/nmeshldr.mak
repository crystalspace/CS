DESCRIPTION.nullmeshldr = Null mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make nullmeshldr  Make the $(DESCRIPTION.mullmeshldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: nullmeshldr nullmeshldrclean
plugins meshes all: nullmeshldr

nullmeshldrclean:
	$(MAKE_CLEAN)
nullmeshldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/null/persist/standard

ifeq ($(USE_PLUGINS),yes)
  NULLMESHLDR = $(OUTDLL)/nullmeshldr$(DLL)
  LIB.NULLMESHLDR = $(foreach d,$(DEP.NULLMESHLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(NULLMESHLDR)
else
  NULLMESHLDR = $(OUT)/$(LIB_PREFIX)nullmeshldr$(LIB)
  DEP.EXE += $(NULLMESHLDR)
  SCF.STATIC += nullmeshldr
  TO_INSTALL.STATIC_LIBS += $(NULLMESHLDR)
endif

INF.NULLMESHLDR = $(SRCDIR)/plugins/mesh/null/persist/standard/nullmeshldr.csplugin
INC.NULLMESHLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/null/persist/standard/*.h))
SRC.NULLMESHLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/null/persist/standard/*.cpp))
OBJ.NULLMESHLDR = $(addprefix $(OUT)/,$(notdir $(SRC.NULLMESHLDR:.cpp=$O)))
DEP.NULLMESHLDR = CSGEOM CSUTIL CSUTIL

MSVC.DSP += NULLMESHLDR
DSP.NULLMESHLDR.NAME = nullmeshldr
DSP.NULLMESHLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: nullmeshldr nullmeshldrclean
nullmeshldr: $(OUTDIRS) $(NULLMESHLDR)

$(NULLMESHLDR): $(OBJ.NULLMESHLDR) $(LIB.NULLMESHLDR)
	$(DO.PLUGIN)

clean: nullmeshldrclean
nullmeshldrclean:
	-$(RMDIR) $(NULLMESHLDR) $(OBJ.NULLMESHLDR) $(OUTDLL)/$(notdir $(INF.NULLMESHLDR))

ifdef DO_DEPEND
dep: $(OUTOS)/nullmeshldr.dep
$(OUTOS)/nullmeshldr.dep: $(SRC.NULLMESHLDR)
	$(DO.DEP)
else
-include $(OUTOS)/nullmeshldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
