# Plug-in description
DESCRIPTION.sndds3d = Crystal Space DirectSound 3D sound renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make sndds3d      Make the $(DESCRIPTION.sndds3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndds3d sndds3dclean
all plugins drivers snddrivers: sndds3d

sndds3d:
	$(MAKE_TARGET) MAKE_DLL=yes
sndds3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  SNDDS3D = $(OUTDLL)/sndds3d$(DLL)
  LIB.SNDDS3D = $(foreach d,$(DEP.SNDDS3D),$($d.LIB))
  LIB.SNDDS3D.SPECIAL = $(DIRECTX.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(SNDDS3D)
else
  SNDDS3D = $(OUT)/$(LIB_PREFIX)sndds3d$(LIB)
  DEP.EXE += $(SNDDS3D)
  LIB.EXE += $(DIRECTX.LFLAGS)
  SCF.STATIC += sndds3d
  TO_INSTALL.STATIC_LIBS += $(SNDDS3D)
endif

DIR.SNDDS3D = plugins/sound/renderer/ds3d
OUT.SNDDS3D = $(OUT)/$(DIR.SNDDS3D)
INF.SNDDS3D = $(SRCDIR)/$(DIR.SNDDS3D)/sndds3d.csplugin
INC.SNDDS3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SNDDS3D)/*.h \
  plugins/sound/renderer/common/*.h))
SRC.SNDDS3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.SNDDS3D)/*.cpp \
  plugins/sound/renderer/common/*.cpp))
OBJ.SNDDS3D = $(addprefix $(OUT.SNDDS3D)/,$(notdir $(SRC.SNDDS3D:.cpp=$O)))
DEP.SNDDS3D = CSUTIL CSGEOM CSSYS CSUTIL

OUTDIRS += $(OUT.SNDDS3D)

MSVC.DSP += SNDDS3D
DSP.SNDDS3D.NAME = sndds3d
DSP.SNDDS3D.TYPE = plugin
DSP.SNDDS3D.LIBS = dsound

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndds3d sndds3dclean sndds3dcleandep

sndds3d: $(OUTDIRS) $(SNDDS3D)

$(OUT.SNDDS3D)/%$O: $(SRCDIR)/$(DIR.SNDDS3D)/%.cpp
	$(DO.COMPILE.CPP) $(DIRECTX.CFLAGS)

$(OUT.SNDDS3D)/%$O: $(SRCDIR)/plugins/sound/renderer/common/%.cpp
	$(DO.COMPILE.CPP)

$(SNDDS3D): $(OBJ.SNDDS3D) $(LIB.SNDDS3D)
	$(DO.PLUGIN) $(LIB.SNDDS3D.SPECIAL)

clean: sndds3dclean
sndds3dclean:
	-$(RMDIR) $(SNDDS3D) $(OBJ.SNDDS3D) $(OUTDLL)/$(notdir $(INF.SNDDS3D))

cleandep: sndds3dcleandep
sndds3dcleandep:
	-$(RM) $(OUT.SNDDS3D)/sndds3d.dep

ifdef DO_DEPEND
dep: $(OUT.SNDDS3D) $(OUT.SNDDS3D)/sndds3d.dep
$(OUT.SNDDS3D)/sndds3d.dep: $(SRC.SNDDS3D)
	$(DO.DEPEND)
else
-include $(OUT.SNDDS3D)/sndds3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
