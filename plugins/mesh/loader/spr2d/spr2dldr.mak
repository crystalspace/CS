DESCRIPTION.spr2dldr = Sprite 2D mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make spr2dldr     Make the $(DESCRIPTION.spr2dldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: spr2dldr spr2dldrclean
plugins meshes all: spr2dldr

spr2dldrclean:
	$(MAKE_CLEAN)
spr2dldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/loader/spr2d

ifeq ($(USE_PLUGINS),yes)
  SPR2DLDR = $(OUTDLL)spr2dldr$(DLL)
  LIB.SPR2DLDR = $(foreach d,$(DEP.SPR2DLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPR2DLDR)
else
  SPR2DLDR = $(OUT)$(LIB_PREFIX)spr2dldr$(LIB)
  DEP.EXE += $(SPR2DLDR)
  SCF.STATIC += spr2dldr
  TO_INSTALL.STATIC_LIBS += $(SPR2DLDR)
endif

INC.SPR2DLDR = $(wildcard plugins/mesh/loader/spr2d/*.h)
SRC.SPR2DLDR = $(wildcard plugins/mesh/loader/spr2d/*.cpp)
OBJ.SPR2DLDR = $(addprefix $(OUT),$(notdir $(SRC.SPR2DLDR:.cpp=$O)))
DEP.SPR2DLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += SPR2DLDR
DSP.SPR2DLDR.NAME = spr2dldr
DSP.SPR2DLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: spr2dldr spr2dldrclean
spr2dldr: $(OUTDIRS) $(SPR2DLDR)

$(SPR2DLDR): $(OBJ.SPR2DLDR) $(LIB.SPR2DLDR)
	$(DO.PLUGIN)

clean: spr2dldrclean
spr2dldrclean:
	-$(RM) $(SPR2DLDR) $(OBJ.SPR2DLDR)

ifdef DO_DEPEND
dep: $(OUTOS)spr2dldr.dep
$(OUTOS)spr2dldr.dep: $(SRC.SPR2DLDR)
	$(DO.DEP)
else
-include $(OUTOS)spr2dldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
