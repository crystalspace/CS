DESCRIPTION.fire = Fire mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make fire         Make the $(DESCRIPTION.fire)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: fire fireclean
plugins all: fire

fireclean:
	$(MAKE_CLEAN)
fire:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/meshobj/fire plugins/meshobj/partgen

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

INC.FIRE = $(wildcard plugins/meshobj/fire/*.h plugins/meshobj/partgen/*.h)
SRC.FIRE = $(wildcard plugins/meshobj/fire/*.cpp plugins/meshobj/partgen/*.cpp)
OBJ.FIRE = $(addprefix $(OUT),$(notdir $(SRC.FIRE:.cpp=$O)))
DEP.FIRE = CSGEOM CSUTIL CSSYS

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
	-$(RM) $(FIRE) $(OBJ.FIRE) $(OUTOS)fire.dep

ifdef DO_DEPEND
dep: $(OUTOS)fire.dep
$(OUTOS)fire.dep: $(SRC.FIRE)
	$(DO.DEP)
else
-include $(OUTOS)fire.dep
endif

endif # ifeq ($(MAKESECTION),targets)
