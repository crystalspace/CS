DESCRIPTION.spr3dldr = Sprite 3D mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make spr3dldr     Make the $(DESCRIPTION.spr3dldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: spr3dldr spr3dldrclean
plugins meshes all: spr3dldr

spr3dldrclean:
	$(MAKE_CLEAN)
spr3dldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/spr3d/persist/classic

ifeq ($(USE_PLUGINS),yes)
  SPR3DLDR = $(OUTDLL)spr3dldr$(DLL)
  LIB.SPR3DLDR = $(foreach d,$(DEP.SPR3DLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPR3DLDR)
else
  SPR3DLDR = $(OUT)$(LIB_PREFIX)spr3dldr$(LIB)
  DEP.EXE += $(SPR3DLDR)
  SCF.STATIC += spr3dldr
  TO_INSTALL.STATIC_LIBS += $(SPR3DLDR)
endif

INC.SPR3DLDR = $(wildcard plugins/mesh/spr3d/persist/classic/*.h)
SRC.SPR3DLDR = $(wildcard plugins/mesh/spr3d/persist/classic/*.cpp)
OBJ.SPR3DLDR = $(addprefix $(OUT),$(notdir $(SRC.SPR3DLDR:.cpp=$O)))
DEP.SPR3DLDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SPR3DLDR
DSP.SPR3DLDR.NAME = spr3dldr
DSP.SPR3DLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: spr3dldr spr3dldrclean
spr3dldr: $(OUTDIRS) $(SPR3DLDR)

$(SPR3DLDR): $(OBJ.SPR3DLDR) $(LIB.SPR3DLDR)
	$(DO.PLUGIN)

clean: spr3dldrclean
spr3dldrclean:
	-$(RM) $(SPR3DLDR) $(OBJ.SPR3DLDR)

ifdef DO_DEPEND
dep: $(OUTOS)spr3dldr.dep
$(OUTOS)spr3dldr.dep: $(SRC.SPR3DLDR)
	$(DO.DEP)
else
-include $(OUTOS)spr3dldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
