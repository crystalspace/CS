DESCRIPTION.sprcal3dldr = Cal3D sprite mesh loader plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sprcal3dldr  Make the $(DESCRIPTION.sprcal3dldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sprcal3dldr sprcal3dldrclean
all plugins: sprcal3dldr

sprcal3dldr:
	$(MAKE_TARGET) MAKE_DLL=yes
sprcal3dldrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  SPRCAL3DLDR = $(OUTDLL)/sprcal3dldr$(DLL)
  LIB.SPRCAL3DLDR = $(foreach d,$(DEP.SPRCAL3DLDR),$($d.LIB))
  LIB.SPRCAL3DLDR.LFLAGS = $(CAL3D.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(SPRCAL3DLDR)
else
  SPRCAL3DLDR = $(OUT)/$(LIB_PREFIX)sprcal3dldr$(LIB)
  DEP.EXE += $(SPRCAL3DLDR)
  LIBS.EXE += $(CAL3D.LFLAGS)
  SCF.STATIC += sprcal3dldr
  TO_INSTALL.STATIC_LIBS += $(SPRCAL3DLDR)
endif

DIR.SPRCAL3DLDR = plugins/mesh/sprcal3d/persist
OUT.SPRCAL3DLDR = $(OUT)/$(DIR.SPRCAL3DLDR)
INF.SPRCAL3DLDR = $(SRCDIR)/$(DIR.SPRCAL3DLDR)/sprcal3dldr.csplugin
INC.SPRCAL3DLDR = $(wildcard $(SRCDIR)/$(DIR.SPRCAL3DLDR)/*.h)
SRC.SPRCAL3DLDR = $(wildcard $(SRCDIR)/$(DIR.SPRCAL3DLDR)/*.cpp)
OBJ.SPRCAL3DLDR = \
  $(addprefix $(OUT.SPRCAL3DLDR)/,$(notdir $(SRC.SPRCAL3DLDR:.cpp=$O)))
DEP.SPRCAL3DLDR = CSGEOM CSUTIL

OUTDIRS += $(OUT.SPRCAL3DLDR)

MSVC.DSP += SPRCAL3DLDR
DSP.SPRCAL3DLDR.NAME = sprcal3dldr
DSP.SPRCAL3DLDR.TYPE = plugin
DSP.SPRCAL3DLDR.LIBS = cal3d

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sprcal3dldr sprcal3dldrclean sprcal3dldrcleandep

sprcal3dldr: $(OUTDIRS) $(SPRCAL3DLDR)

$(OUT.SPRCAL3DLDR)/%$O: $(SRCDIR)/$(DIR.SPRCAL3DLDR)/%.cpp
	$(DO.COMPILE.CPP) $(CAL3D.CFLAGS)

$(SPRCAL3DLDR): $(OBJ.SPRCAL3DLDR) $(LIB.SPRCAL3DLDR)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(LIB.SPRCAL3DLDR.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: sprcal3dldrclean
sprcal3dldrclean:
	-$(RMDIR) $(SPRCAL3DLDR) $(OBJ.SPRCAL3DLDR) \
	$(OUTDLL)/$(notdir $(INF.SPRCAL3DLDR))

cleandep: sprcal3dldrcleandep
sprcal3dldrcleandep:
	-$(RM) $(OUT.SPRCAL3DLDR)/sprcal3dldr.dep

ifdef DO_DEPEND
dep: $(OUT.SPRCAL3DLDR) $(OUT.SPRCAL3DLDR)/sprcal3dldr.dep
$(OUT.SPRCAL3DLDR)/sprcal3dldr.dep: $(SRC.SPRCAL3DLDR)
	$(DO.DEPEND1) \
	$(CAL3D.CFLAGS) \
	$(DO.DEPEND2)
else
-include $(OUT.SPRCAL3DLDR)/sprcal3dldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
