DESCRIPTION.ivie = IV Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ivie        Make the $(DESCRIPTION.ivie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ivie ivieclean
plugins meshes all: ivie

ivieclean:
	$(MAKE_CLEAN)
ivie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/iv

ifeq ($(USE_PLUGINS),yes)
  IVIE = $(OUTDLL)ivie$(DLL)
  LIB.IVIE = $(foreach d,$(DEP.IVIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(IVIE)
else
  IVIE = $(OUT)$(LIB_PREFIX)ivie$(LIB)
  DEP.EXE += $(IVIE)
  SCF.STATIC += ivie
  TO_INSTALL.STATIC_LIBS += $(IVIE)
endif

INC.IVIE = $(wildcard plugins/mesh/impexp/iv/*.h)
SRC.IVIE = $(wildcard plugins/mesh/impexp/iv/*.cpp)
OBJ.IVIE = $(addprefix $(OUT),$(notdir $(SRC.IVIE:.cpp=$O)))
DEP.IVIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += IVIE
DSP.IVIE.NAME = ivie
DSP.IVIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ivie ivieclean
ivie: $(OUTDIRS) $(IVIE)

$(IVIE): $(OBJ.IVIE) $(LIB.IVIE)
	$(DO.PLUGIN)

clean: ivieclean
ivieclean:
	-$(RM) $(IVIE) $(OBJ.IVIE)

ifdef DO_DEPEND
dep: $(OUTOS)ivie.dep
$(OUTOS)ivie.dep: $(SRC.IVIE)
	$(DO.DEP)
else
-include $(OUTOS)ivie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
