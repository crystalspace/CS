DESCRIPTION.nullrender3d = Render3D null renderer

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make nullrender3d Make the $(DESCRIPTION.nullrender3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: nullrender3d
all plugins drivers drivers3d: nullrender3d

nullrender3d:
	$(MAKE_TARGET) MAKE_DLL=yes
nullrender3dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  NULLRENDER3D = $(OUTDLL)/nullrender3d$(DLL)
  LIB.NULLRENDER3D = $(foreach d,$(DEP.NULLRENDER3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(NULLRENDER3D)
else
  NULLRENDER3D = $(OUT)/$(LIB_PREFIX)nullrender3d$(LIB)
  DEP.EXE += $(NULLRENDER3D)
  SCF.STATIC += nullrender3d
  TO_INSTALL.STATIC_LIBS += $(NULLRENDER3D)
endif

DIR.NULLRENDER3D = plugins/video/render3d/null
OUT.NULLRENDER3D = $(OUT)/$(DIR.NULLRENDER3D)
INF.NULLRENDER3D = $(SRCDIR)/$(DIR.NULLRENDER3D)/nullrender3d.csplugin
INC.NULLRENDER3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.NULLRENDER3D)/*.h \
  plugins/video/render3d/common/txtmgr.h))
SRC.NULLRENDER3D = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.NULLRENDER3D)/*.cpp \
  plugins/video/render3d/common/txtmgr.cpp))
OBJ.NULLRENDER3D = \
  $(addprefix $(OUT.NULLRENDER3D)/,$(notdir $(SRC.NULLRENDER3D:.cpp=$O)))
DEP.NULLRENDER3D = CSTOOL CSGFX CSGEOM CSUTIL

OUTDIRS += $(OUT.NULLRENDER3D)

MSVC.DSP += NULLRENDER3D
DSP.NULLRENDER3D.NAME = nullrender3d
DSP.NULLRENDER3D.TYPE = plugin
DSP.NULLRENDER3D.RESOURCES = $(wildcard $(SRCDIR)/$(DIR.NULLRENDER3D)/*.inc)

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: nullrender3d nullrender3dclean nullrender3dcleandep

nullrender3d: $(OUTDIRS) $(NULLRENDER3D)

$(OUT.NULLRENDER3D)/%$O: $(SRCDIR)/$(DIR.NULLRENDER3D)/%.cpp
	$(DO.COMPILE.CPP)

$(OUT.NULLRENDER3D)/%$O: $(SRCDIR)/plugins/video/render3d/common/%.cpp
	$(DO.COMPILE.CPP)

$(NULLRENDER3D): $(OBJ.NULLRENDER3D) $(LIB.NULLRENDER3D)
	$(DO.PLUGIN)

clean: nullrender3dclean
nullrender3dclean:
	-$(RMDIR) $(NULLRENDER3D) $(OBJ.NULLRENDER3D) \
	$(OUTDLL)/$(notdir $(INF.NULLRENDER3D))

cleandep: nullrender3dcleandep
nullrender3dcleandep:
	-$(RM) $(OUT.NULLRENDER3D)/nullrender3d.dep

ifdef DO_DEPEND
dep: $(OUT.NULLRENDER3D) $(OUT.NULLRENDER3D)/nullrender3d.dep
$(OUT.NULLRENDER3D)/nullrender3d.dep: $(SRC.NULLRENDER3D)
	$(DO.DEPEND)
else
-include $(OUT.NULLRENDER3D)/nullrender3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
