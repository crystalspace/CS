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

vpath %.cpp plugins/sound/renderer/ds3d

ifeq ($(USE_PLUGINS),yes)
  SNDDS3D = $(OUTDLL)sndds3d$(DLL)
  LIB.SNDDS3D = $(foreach d,$(DEP.SNDDS3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDDS3D)
else
  SNDDS3D = $(OUT)$(LIB_PREFIX)sndds3d$(LIB)
  DEP.EXE += $(SNDDS3D)
  SCF.STATIC += sndds3d
  TO_INSTALL.STATIC_LIBS += $(SNDDS3D)
endif

INC.SNDDS3D = $(wildcard plugins/sound/renderer/ds3d/*.h)
SRC.SNDDS3D = $(wildcard plugins/sound/renderer/ds3d/*.cpp)
OBJ.SNDDS3D = $(addprefix $(OUT),$(notdir $(SRC.SNDDS3D:.cpp=$O)))
DEP.SNDDS3D = CSUTIL CSGEOM CSSYS

MSVC.DSP += SNDDS3D
DSP.SNDDS3D.NAME = sndrdrds3d
DSP.SNDDS3D.TYPE = plugin
DSP.SNDDS3D.LIBS = dsound

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndds3d sndds3dclean

sndds3d: $(OUTDIRS) $(SNDDS3D)

$(SNDDS3D): $(OBJ.SNDDS3D) $(LIB.SNDDS3D)
	$(DO.PLUGIN)

clean: sndds3dclean
sndds3dclean:
	$(RM) $(SNDDS3D) $(OBJ.SNDDS3D)

ifdef DO_DEPEND
dep: $(OUTOS)sndds3d.dep
$(OUTOS)sndds3d.dep: $(SRC.SNDDS3D)
	$(DO.DEP)
else
-include $(OUTOS)sndds3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
