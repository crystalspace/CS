DESCRIPTION.sprcal3d = Cal3D sprite mesh plugin

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
  LIBS.EXE += $(CAL3D.LFLAGS)
  SCF.STATIC += sprcal3d
  TO_INSTALL.STATIC_LIBS += $(SPRCAL3D)
endif

DIR.SPRCAL3D = plugins/mesh/sprcal3d/object
OUT.SPRCAL3D = $(OUT)/$(DIR.SPRCAL3D)
INF.SPRCAL3D = $(SRCDIR)/$(DIR.SPRCAL3D)/sprcal3d.csplugin
INC.SPRCAL3D = $(wildcard $(SRCDIR)/$(DIR.SPRCAL3D)/*.h)
SRC.SPRCAL3D = $(wildcard $(SRCDIR)/$(DIR.SPRCAL3D)/*.cpp)
OBJ.SPRCAL3D = \
  $(addprefix $(OUT.SPRCAL3D)/,$(notdir $(SRC.SPRCAL3D:.cpp=$O)))
DEP.SPRCAL3D = CSUTIL

OUTDIRS += $(OUT.SPRCAL3D)

MSVC.DSP += SPRCAL3D
DSP.SPRCAL3D.NAME = sprcal3d
DSP.SPRCAL3D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sprcal3d sprcal3dclean sprcal3dcleandep

sprcal3d: $(OUTDIRS) $(SPRCAL3D)

$(OUT.SPRCAL3D)/%$O: $(SRCDIR)/$(DIR.SPRCAL3D)/%.cpp
	$(DO.COMPILE.CPP) $(CAL3D.CFLAGS)

$(SPRCAL3D): $(OBJ.SPRCAL3D) $(LIB.SPRCAL3D)
	$(DO.PLUGIN.PREAMBLE) \
	$(DO.PLUGIN.CORE) $(CAL3D.LFLAGS) \
	$(DO.PLUGIN.POSTAMBLE)

clean: sprcal3dclean
sprcal3dclean:
	-$(RMDIR) $(SPRCAL3D) $(OBJ.SPRCAL3D) \
	$(OUTDLL)/$(notdir $(INF.SPRCAL3D))

cleandep: sprcal3dcleandep
sprcal3dcleandep:
	-$(RM) $(OUT.SPRCAL3D)/sprcal3d.dep

ifdef DO_DEPEND
dep: $(OUT.SPRCAL3D) $(OUT.SPRCAL3D)/sprcal3d.dep
$(OUT.SPRCAL3D)/sprcal3d.dep: $(SRC.SPRCAL3D)
	$(DO.DEPEND1) \
	$(CAL3D.CFLAGS) \
	$(DO.DEPEND2)
else
-include $(OUT.SPRCAL3D)/sprcal3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
