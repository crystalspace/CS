DESCRIPTION.spr2d = 2D Sprite mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make spr2d        Make the $(DESCRIPTION.spr2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: spr2d spr2dclean
plugins all: spr2d

spr2dclean:
	$(MAKE_CLEAN)
spr2d:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/spr2d

ifeq ($(USE_PLUGINS),yes)
  SPR2D = $(OUTDLL)spr2d$(DLL)
  LIB.SPR2D = $(foreach d,$(DEP.SPR2D),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPR2D)
else
  SPR2D = $(OUT)$(LIB_PREFIX)spr2d$(LIB)
  DEP.EXE += $(SPR2D)
  SCF.STATIC += spr2d
  TO_INSTALL.STATIC_LIBS += $(SPR2D)
endif

INC.SPR2D = $(wildcard plugins/mesh/object/spr2d/*.h)
SRC.SPR2D = $(wildcard plugins/mesh/object/spr2d/*.cpp)
OBJ.SPR2D = $(addprefix $(OUT),$(notdir $(SRC.SPR2D:.cpp=$O)))
DEP.SPR2D = CSGEOM CSUTIL CSSYS

MSVC.DSP += SPR2D
DSP.SPR2D.NAME = spr2d
DSP.SPR2D.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: spr2d spr2dclean
spr2d: $(OUTDIRS) $(SPR2D)

$(SPR2D): $(OBJ.SPR2D) $(LIB.SPR2D)
	$(DO.PLUGIN)

clean: spr2dclean
spr2dclean:
	-$(RM) $(SPR2D) $(OBJ.SPR2D) $(OUTOS)spr2d.dep

ifdef DO_DEPEND
dep: $(OUTOS)spr2d.dep
$(OUTOS)spr2d.dep: $(SRC.SPR2D)
	$(DO.DEP)
else
-include $(OUTOS)spr2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
