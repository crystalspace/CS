DESCRIPTION.hrcie = HRC Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make hrcie        Make the $(DESCRIPTION.hrcie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: hrcie hrcieclean
plugins meshes all: hrcie

hrcieclean:
	$(MAKE_CLEAN)
hrcie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/hrc

ifeq ($(USE_PLUGINS),yes)
  HRCIE = $(OUTDLL)hrcie$(DLL)
  LIB.HRCIE = $(foreach d,$(DEP.HRCIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(HRCIE)
else
  HRCIE = $(OUT)$(LIB_PREFIX)hrcie$(LIB)
  DEP.EXE += $(HRCIE)
  SCF.STATIC += hrcie
  TO_INSTALL.STATIC_LIBS += $(HRCIE)
endif

INC.HRCIE = $(wildcard plugins/mesh/impexp/hrc/*.h)
SRC.HRCIE = $(wildcard plugins/mesh/impexp/hrc/*.cpp)
OBJ.HRCIE = $(addprefix $(OUT),$(notdir $(SRC.HRCIE:.cpp=$O)))
DEP.HRCIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += HRCIE
DSP.HRCIE.NAME = hrcie
DSP.HRCIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: hrcie hrcieclean
hrcie: $(OUTDIRS) $(HRCIE)

$(HRCIE): $(OBJ.HRCIE) $(LIB.HRCIE)
	$(DO.PLUGIN)

clean: hrcieclean
hrcieclean:
	-$(RM) $(HRCIE) $(OBJ.HRCIE)

ifdef DO_DEPEND
dep: $(OUTOS)hrcie.dep
$(OUTOS)hrcie.dep: $(SRC.HRCIE)
	$(DO.DEP)
else
-include $(OUTOS)hrcie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
