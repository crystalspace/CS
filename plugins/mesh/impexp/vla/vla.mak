DESCRIPTION.vlaie = VLA Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make vlaie        Make the $(DESCRIPTION.vlaie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: vlaie vlaieclean
plugins meshes all: vlaie

vlaieclean:
	$(MAKE_CLEAN)
vlaie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/vla

ifeq ($(USE_PLUGINS),yes)
  VLAIE = $(OUTDLL)vlaie$(DLL)
  LIB.VLAIE = $(foreach d,$(DEP.VLAIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(VLAIE)
else
  VLAIE = $(OUT)$(LIB_PREFIX)vlaie$(LIB)
  DEP.EXE += $(VLAIE)
  SCF.STATIC += vlaie
  TO_INSTALL.STATIC_LIBS += $(VLAIE)
endif

INC.VLAIE = $(wildcard plugins/mesh/impexp/vla/*.h)
SRC.VLAIE = $(wildcard plugins/mesh/impexp/vla/*.cpp)
OBJ.VLAIE = $(addprefix $(OUT),$(notdir $(SRC.VLAIE:.cpp=$O)))
DEP.VLAIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += VLAIE
DSP.VLAIE.NAME = vlaie
DSP.VLAIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: vlaie vlaieclean
vlaie: $(OUTDIRS) $(VLAIE)

$(VLAIE): $(OBJ.VLAIE) $(LIB.VLAIE)
	$(DO.PLUGIN)

clean: vlaieclean
vlaieclean:
	-$(RM) $(VLAIE) $(OBJ.VLAIE)

ifdef DO_DEPEND
dep: $(OUTOS)vlaie.dep
$(OUTOS)vlaie.dep: $(SRC.VLAIE)
	$(DO.DEP)
else
-include $(OUTOS)vlaie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
