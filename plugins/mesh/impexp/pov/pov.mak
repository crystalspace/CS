DESCRIPTION.povie = POV Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make povie        Make the $(DESCRIPTION.povie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: povie povieclean
plugins meshes all: povie

povieclean:
	$(MAKE_CLEAN)
povie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/pov

ifeq ($(USE_PLUGINS),yes)
  POVIE = $(OUTDLL)povie$(DLL)
  LIB.POVIE = $(foreach d,$(DEP.POVIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(POVIE)
else
  POVIE = $(OUT)$(LIB_PREFIX)povie$(LIB)
  DEP.EXE += $(POVIE)
  SCF.STATIC += povie
  TO_INSTALL.STATIC_LIBS += $(POVIE)
endif

INC.POVIE = $(wildcard plugins/mesh/impexp/pov/*.h)
SRC.POVIE = $(wildcard plugins/mesh/impexp/pov/*.cpp)
OBJ.POVIE = $(addprefix $(OUT),$(notdir $(SRC.POVIE:.cpp=$O)))
DEP.POVIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += POVIE
DSP.POVIE.NAME = povie
DSP.POVIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: povie povieclean
povie: $(OUTDIRS) $(POVIE)

$(POVIE): $(OBJ.POVIE) $(LIB.POVIE)
	$(DO.PLUGIN)

clean: povieclean
povieclean:
	-$(RM) $(POVIE) $(OBJ.POVIE)

ifdef DO_DEPEND
dep: $(OUTOS)povie.dep
$(OUTOS)povie.dep: $(SRC.POVIE)
	$(DO.DEP)
else
-include $(OUTOS)povie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
