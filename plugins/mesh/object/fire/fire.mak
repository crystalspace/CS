DESCRIPTION.fire = Fire mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fire         Make the $(DESCRIPTION.fire)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fire fireclean
plugins meshes all: fire

fireclean:
	$(MAKE_CLEAN)
fire:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/fire plugins/mesh/object/partgen

ifeq ($(USE_PLUGINS),yes)
  FIRE = $(OUTDLL)fire$(DLL)
  LIB.FIRE = $(foreach d,$(DEP.FIRE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(FIRE)
else
  FIRE = $(OUT)$(LIB_PREFIX)fire$(LIB)
  DEP.EXE += $(FIRE)
  SCF.STATIC += fire
  TO_INSTALL.STATIC_LIBS += $(FIRE)
endif

INC.FIRE = $(wildcard plugins/mesh/object/fire/*.h plugins/mesh/object/partgen/*.h)
SRC.FIRE = $(wildcard plugins/mesh/object/fire/*.cpp plugins/mesh/object/partgen/*.cpp)
OBJ.FIRE = $(addprefix $(OUT),$(notdir $(SRC.FIRE:.cpp=$O)))
DEP.FIRE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += FIRE
DSP.FIRE.NAME = fire
DSP.FIRE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: fire fireclean
fire: $(OUTDIRS) $(FIRE)

$(FIRE): $(OBJ.FIRE) $(LIB.FIRE)
	$(DO.PLUGIN)

clean: fireclean
fireclean:
	-$(RM) $(FIRE) $(OBJ.FIRE)

ifdef DO_DEPEND
dep: $(OUTOS)fire.dep
$(OUTOS)fire.dep: $(SRC.FIRE)
	$(DO.DEP)
else
-include $(OUTOS)fire.dep
endif

endif # ifeq ($(MAKESECTION),targets)
