DESCRIPTION.spr3dbin = Sprite 3D mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make spr3dbin     Make the $(DESCRIPTION.spr3dbin)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: spr3dbin spr3dbinclean
plugins meshes all: spr3dbin

spr3dbinclean:
	$(MAKE_CLEAN)
spr3dbin:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/spr3d/persist/binary

ifeq ($(USE_PLUGINS),yes)
  SPR3DBIN = $(OUTDLL)spr3dbin$(DLL)
  LIB.SPR3DBIN = $(foreach d,$(DEP.SPR3DBIN),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPR3DBIN)
else
  SPR3DBIN = $(OUT)$(LIB_PREFIX)spr3dbin$(LIB)
  DEP.EXE += $(SPR3DBIN)
  SCF.STATIC += spr3dbin
  TO_INSTALL.STATIC_LIBS += $(SPR3DBIN)
endif

INC.SPR3DBIN = $(wildcard plugins/mesh/spr3d/persist/binary/*.h)
SRC.SPR3DBIN = $(wildcard plugins/mesh/spr3d/persist/binary/*.cpp)
OBJ.SPR3DBIN = $(addprefix $(OUT),$(notdir $(SRC.SPR3DBIN:.cpp=$O)))
DEP.SPR3DBIN = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SPR3DBIN
DSP.SPR3DBIN.NAME = spr3dbin
DSP.SPR3DBIN.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: spr3dbin spr3dbinclean
spr3dbin: $(OUTDIRS) $(SPR3DBIN)

$(SPR3DBIN): $(OBJ.SPR3DBIN) $(LIB.SPR3DLDR)
	$(DO.PLUGIN)

clean: spr3dbinclean
spr3dbinclean:
	-$(RM) $(SPR3DBIN) $(OBJ.SPR3DBIN)

ifdef DO_DEPEND
dep: $(OUTOS)spr3dbin.dep
$(OUTOS)spr3dbin.dep: $(SRC.SPR3DBIN)
	$(DO.DEP)
else
-include $(OUTOS)spr3dbin.dep
endif

endif # ifeq ($(MAKESECTION),targets)

