# This is a subinclude file used to define the rules needed
# to build the sprcal3d plug-in.

# Driver description
DESCRIPTION.sprcal3d = Cal3D Sprite Mesh Object Plugin

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sprcal3d     Make the $(DESCRIPTION.sprcal3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sprcal3d sprcal3dclean
all plugins: sprcal3d

sprcal3d:
	$(MAKE_TARGET) MAKE_DLL=yes
sprcal3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  SPRCAL3D = $(OUTDLL)/sprcal3d$(DLL)
  LIB.SPRCAL3D = $(foreach d,$(DEP.SPRCAL3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPRCAL3D)
else
  SPRCAL3D = $(OUT)/$(LIB_PREFIX)sprcal3d$(LIB)
  DEP.EXE += $(SPRCAL3D)
  SCF.STATIC += sprcal3d
  TO_INSTALL.STATIC_LIBS += $(SPRCAL3D)
endif

INF.SPRCAL3D = $(SRCDIR)/plugins/mesh/sprcal3d/object/sprcal3d.csplugin
INC.SPRCAL3D = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/sprcal3d/object/*.h))
SRC.SPRCAL3D = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/sprcal3d/object/*.cpp))
OBJ.SPRCAL3D = $(addprefix $(OUT)/,$(notdir $(SRC.SPRCAL3D:.cpp=$O)))
DEP.SPRCAL3D = CSGFX CSGEOM CSUTIL

MSVC.DSP += SPRCAL3D
DSP.SPRCAL3D.NAME = sprcal3d
DSP.SPRCAL3D.TYPE = plugin
DSP.SPRCAL3D.LIBS = cal3d

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sprcal3d sprcal3dclean
sprcal3d: $(OUTDIRS) $(SPRCAL3D)

$(OUT)/%$O: $(SRCDIR)/plugins/mesh/sprcal3d/object/%.cpp
	$(DO.COMPILE.CPP) $(CAL3D.CFLAGS)

$(SPRCAL3D): $(OBJ.SPRCAL3D) $(LIB.SPRCAL3D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(CAL3D.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: sprcal3dclean
sprcal3dclean:
	-$(RMDIR) $(SPRCAL3D) $(OBJ.SPRCAL3D) $(OUTDLL)/$(notdir $(INF.SPRCAL3D))

ifdef DO_DEPEND
dep: $(OUTOS)/sprcal3d.dep
$(OUTOS)/sprcal3d.dep: $(SRC.SPRCAL3D)
	$(DO.DEP1) \
	$(CAL3D.CFLAGS) \
	$(DO.DEP2)
else
-include $(OUTOS)/sprcal3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
