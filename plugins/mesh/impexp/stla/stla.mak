DESCRIPTION.stlaie = STLA Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make stlaie        Make the $(DESCRIPTION.stlaie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: stlaie stlaieclean
plugins meshes all: stlaie

stlaieclean:
	$(MAKE_CLEAN)
stlaie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/stla

ifeq ($(USE_PLUGINS),yes)
  STLAIE = $(OUTDLL)stlaie$(DLL)
  LIB.STLAIE = $(foreach d,$(DEP.STLAIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(STLAIE)
else
  STLAIE = $(OUT)$(LIB_PREFIX)stlaie$(LIB)
  DEP.EXE += $(STLAIE)
  SCF.STATIC += stlaie
  TO_INSTALL.STATIC_LIBS += $(STLAIE)
endif

INC.STLAIE = $(wildcard plugins/mesh/impexp/stla/*.h)
SRC.STLAIE = $(wildcard plugins/mesh/impexp/stla/*.cpp)
OBJ.STLAIE = $(addprefix $(OUT),$(notdir $(SRC.STLAIE:.cpp=$O)))
DEP.STLAIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += STLAIE
DSP.STLAIE.NAME = stlaie
DSP.STLAIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: stlaie stlaieclean
stlaie: $(OUTDIRS) $(STLAIE)

$(STLAIE): $(OBJ.STLAIE) $(LIB.STLAIE)
	$(DO.PLUGIN)

clean: stlaieclean
stlaieclean:
	-$(RM) $(STLAIE) $(OBJ.STLAIE)

ifdef DO_DEPEND
dep: $(OUTOS)stlaie.dep
$(OUTOS)stlaie.dep: $(SRC.STLAIE)
	$(DO.DEP)
else
-include $(OUTOS)stlaie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
