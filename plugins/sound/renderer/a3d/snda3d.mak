# Plug-in description
DESCRIPTION.snda3d = Crystal Space A3D sound renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make snda3d       Make the $(DESCRIPTION.snda3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: snda3d snda3dclean
all plugins drivers snddrivers: snda3d

snda3d:
	$(MAKE_TARGET) MAKE_DLL=yes
snda3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/renderer/a3d

ifeq ($(USE_SHARED_PLUGINS),yes)
  SNDA3D = $(OUTDLL)snda3d$(DLL)
  LIB.SNDA3D = $(foreach d,$(DEP.SNDA3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNDA3D)
else
  SNDA3D = $(OUT)$(LIB_PREFIX)snda3d$(LIB)
  DEP.EXE += $(SNDA3D)
  CFLAGS.STATIC_SCF += $(CFLAGS.D)SCL_SNDA3D
  TO_INSTALL.STATIC_LIBS += $(SNDA3D)
endif

INC.SNDA3D = $(wildcard plugins/sound/renderer/a3d/*.h)
SRC.SNDA3D = $(wildcard plugins/sound/renderer/a3d/*.cpp)
OBJ.SNDA3D = $(addprefix $(OUT),$(notdir $(SRC.SNDA3D:.cpp=$O)))
DEP.SNDA3D = CSUTIL CSGEOM CSSYS

MSVC.DSP += SNDA3D
DSP.SNDA3D.NAME = sndrdra3d
DSP.SNDA3D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: snda3d snda3dclean

snda3d: $(OUTDIRS) $(SNDA3D)

$(SNDA3D): $(OBJ.SNDA3D) $(LIB.SNDA3D)
	$(DO.PLUGIN)

clean: snda3dclean
snda3dclean:
	$(RM) $(SNDA3D) $(OBJ.SNDA3D) $(OUTOS)snda3d.dep

ifdef DO_DEPEND
dep: $(OUTOS)snda3d.dep
$(OUTOS)snda3d.dep: $(SRC.SNDA3D)
	$(DO.DEP)
else
-include $(OUTOS)snda3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
