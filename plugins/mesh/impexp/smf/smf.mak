DESCRIPTION.smfie = SMF Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make smfie        Make the $(DESCRIPTION.smfie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: smfie smfieclean
plugins meshes all: smfie

smfieclean:
	$(MAKE_CLEAN)
smfie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/smf

ifeq ($(USE_PLUGINS),yes)
  SMFIE = $(OUTDLL)smfie$(DLL)
  LIB.SMFIE = $(foreach d,$(DEP.SMFIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SMFIE)
else
  SMFIE = $(OUT)$(LIB_PREFIX)smfie$(LIB)
  DEP.EXE += $(SMFIE)
  SCF.STATIC += smfie
  TO_INSTALL.STATIC_LIBS += $(SMFIE)
endif

INC.SMFIE = $(wildcard plugins/mesh/impexp/smf/*.h)
SRC.SMFIE = $(wildcard plugins/mesh/impexp/smf/*.cpp)
OBJ.SMFIE = $(addprefix $(OUT),$(notdir $(SRC.SMFIE:.cpp=$O)))
DEP.SMFIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += SMFIE
DSP.SMFIE.NAME = smfie
DSP.SMFIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: smfie smfieclean
smfie: $(OUTDIRS) $(SMFIE)

$(SMFIE): $(OBJ.SMFIE) $(LIB.SMFIE)
	$(DO.PLUGIN)

clean: smfieclean
smfieclean:
	-$(RM) $(SMFIE) $(OBJ.SMFIE)

ifdef DO_DEPEND
dep: $(OUTOS)smfie.dep
$(OUTOS)smfie.dep: $(SRC.SMFIE)
	$(DO.DEP)
else
-include $(OUTOS)smfie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
