DESCRIPTION.terrainldr = Terrain object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make terrainldr   Make the $(DESCRIPTION.terrainldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: terrainldr terrainldrclean
plugins meshes all: terrainldr

terrainldrclean:
	$(MAKE_CLEAN)
terrainldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/terrain/persist/standard

ifeq ($(USE_PLUGINS),yes)
  TERRAINLDR = $(OUTDLL)/terrainldr$(DLL)
  LIB.TERRAINLDR = $(foreach d,$(DEP.TERRAINLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(TERRAINLDR)
else
  TERRAINLDR = $(OUT)/$(LIB_PREFIX)terrainldr$(LIB)
  DEP.EXE += $(TERRAINLDR)
  SCF.STATIC += terrainldr
  TO_INSTALL.STATIC_LIBS += $(TERRAINLDR)
endif

INF.TERRAINLDR = $(SRCDIR)/plugins/mesh/terrain/persist/standard/terrainldr.csplugin
INC.TERRAINLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/terrain/persist/standard/*.h))
SRC.TERRAINLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/terrain/persist/standard/*.cpp))
OBJ.TERRAINLDR = $(addprefix $(OUT)/,$(notdir $(SRC.TERRAINLDR:.cpp=$O)))
DEP.TERRAINLDR = CSGEOM CSUTIL CSUTIL

MSVC.DSP += TERRAINLDR
DSP.TERRAINLDR.NAME = terrainldr
DSP.TERRAINLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: terrainldr terrainldrclean
terrainldr: $(OUTDIRS) $(TERRAINLDR)

$(TERRAINLDR): $(OBJ.TERRAINLDR) $(LIB.TERRAINLDR)
	$(DO.PLUGIN)

clean: terrainldrclean
terrainldrclean:
	-$(RMDIR) $(TERRAINLDR) $(OBJ.TERRAINLDR) $(OUTDLL)/$(notdir $(INF.TERRAINLDR))

ifdef DO_DEPEND
dep: $(OUTOS)/terrainldr.dep
$(OUTOS)/terrainldr.dep: $(SRC.TERRAINLDR)
	$(DO.DEP)
else
-include $(OUTOS)/terrainldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
