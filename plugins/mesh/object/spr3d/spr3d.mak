DESCRIPTION.spr3d = 3D Sprite mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make spr3d        Make the $(DESCRIPTION.spr3d)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: spr3d spr3dclean
plugins meshes all: spr3d

spr3dclean:
	$(MAKE_CLEAN)
spr3d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/spr3d

ifeq ($(USE_PLUGINS),yes)
  SPR3D = $(OUTDLL)spr3d$(DLL)
  LIB.SPR3D = $(foreach d,$(DEP.SPR3D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPR3D)
else
  SPR3D = $(OUT)$(LIB_PREFIX)spr3d$(LIB)
  DEP.EXE += $(SPR3D)
  SCF.STATIC += spr3d
  TO_INSTALL.STATIC_LIBS += $(SPR3D)
endif

INC.SPR3D = $(wildcard plugins/mesh/object/spr3d/*.h)
SRC.SPR3D = $(wildcard plugins/mesh/object/spr3d/*.cpp)
OBJ.SPR3D = $(addprefix $(OUT),$(notdir $(SRC.SPR3D:.cpp=$O)))
DEP.SPR3D = CSGEOM CSUTIL CSSYS

MSVC.DSP += SPR3D
DSP.SPR3D.NAME = spr3d
DSP.SPR3D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: spr3d spr3dclean
spr3d: $(OUTDIRS) $(SPR3D)

$(SPR3D): $(OBJ.SPR3D) $(LIB.SPR3D)
	$(DO.PLUGIN)

clean: spr3dclean
spr3dclean:
	-$(RM) $(SPR3D) $(OBJ.SPR3D)

ifdef DO_DEPEND
dep: $(OUTOS)spr3d.dep
$(OUTOS)spr3d.dep: $(SRC.SPR3D)
	$(DO.DEP)
else
-include $(OUTOS)spr3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
