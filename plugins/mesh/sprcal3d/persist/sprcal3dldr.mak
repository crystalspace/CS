DESCRIPTION.sprcal3dldr = Cal3D Sprite Mesh Object Loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make sprcal3dldr     Make the $(DESCRIPTION.sprcal3dldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sprcal3dldr sprcal3dldrclean
plugins meshes all: sprcal3dldr

sprcal3dldrclean:
	$(MAKE_CLEAN)
sprcal3dldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/sprcal3d/persist

ifeq ($(USE_PLUGINS),yes)
  SPRCAL3DLDR = $(OUTDLL)/sprcal3dldr$(DLL)
  LIB.SPRCAL3DLDR = $(foreach d,$(DEP.SPRCAL3DLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPRCAL3DLDR)
else
  SPRCAL3DLDR = $(OUT)/$(LIB_PREFIX)sprcal3dldr$(LIB)
  DEP.EXE += $(SPRCAL3DLDR)
  SCF.STATIC += sprcal3dldr
  TO_INSTALL.STATIC_LIBS += $(SPRCAL3DLDR)
endif

INF.SPRCAL3DLDR = $(SRCDIR)/plugins/mesh/sprcal3d/persist/sprcal3dldr.csplugin
INC.SPRCAL3DLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/sprcal3d/persist/*.h))
SRC.SPRCAL3DLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/sprcal3d/persist/*.cpp))
OBJ.SPRCAL3DLDR = $(addprefix $(OUT)/,$(notdir $(SRC.SPRCAL3DLDR:.cpp=$O)))
DEP.SPRCAL3DLDR = CSGEOM CSUTIL CSUTIL

MSVC.DSP += SPRCAL3DLDR
DSP.SPRCAL3DLDR.NAME = sprcal3dldr
DSP.SPRCAL3DLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sprcal3dldr sprcal3dldrclean
sprcal3dldr: $(OUTDIRS) $(SPRCAL3DLDR)

$(SPRCAL3DLDR): $(OBJ.SPRCAL3DLDR) $(LIB.SPRCAL3DLDR)
	$(DO.PLUGIN)

clean: sprcal3dldrclean
sprcal3dldrclean:
	-$(RMDIR) $(SPRCAL3DLDR) $(OBJ.SPRCAL3DLDR) $(OUTDLL)/$(notdir $(INF.SPRCAL3DLDR))

ifdef DO_DEPEND
dep: $(OUTOS)/sprcal3dldr.dep
$(OUTOS)/sprcal3dldr.dep: $(SRC.SPRCAL3DLDR)
	$(DO.DEP)
else
-include $(OUTOS)/sprcal3dldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
