DESCRIPTION.spr3die = Sprite3D import/export persistance plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make spr3die        Make the $(DESCRIPTION.spr3die)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: spr3die spr3dieclean
plugins meshes all: spr3die

spr3dieclean:
	$(MAKE_CLEAN)
spr3die:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/spr3d/persist/impexp

ifeq ($(USE_PLUGINS),yes)
  SPR3DIE = $(OUTDLL)spr3die$(DLL)
  LIB.SPR3DIE = $(foreach d,$(DEP.SPR3DIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SPR3DIE)
else
  SPR3DIE = $(OUT)$(LIB_PREFIX)spr3die$(LIB)
  DEP.EXE += $(SPR3DIE)
  SCF.STATIC += spr3die
  TO_INSTALL.STATIC_LIBS += $(SPR3DIE)
endif

INC.SPR3DIE = $(wildcard plugins/mesh/spr3d/persist/impexp/*.h)
SRC.SPR3DIE = $(wildcard plugins/mesh/spr3d/persist/impexp/*.cpp)
OBJ.SPR3DIE = $(addprefix $(OUT),$(notdir $(SRC.SPR3DIE:.cpp=$O)))
DEP.SPR3DIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SPR3DIE
DSP.SPR3DIE.NAME = spr3die
DSP.SPR3DIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: spr3die spr3dieclean
spr3die: $(OUTDIRS) $(SPR3DIE)

$(SPR3DIE): $(OBJ.SPR3DIE) $(LIB.SPR3DIE)
	$(DO.PLUGIN)

clean: spr3dieclean
spr3dieclean:
	-$(RM) $(SPR3DIE) $(OBJ.SPR3DIE)

ifdef DO_DEPEND
dep: $(OUTOS)spr3die.dep
$(OUTOS)spr3die.dep: $(SRC.SPR3DIE)
	$(DO.DEP)
else
-include $(OUTOS)spr3die.dep
endif

endif # ifeq ($(MAKESECTION),targets)
