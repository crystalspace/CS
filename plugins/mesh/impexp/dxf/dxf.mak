DESCRIPTION.dxfie = DXF Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make dxfie        Make the $(DESCRIPTION.dxfie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: dxfie dxfieclean
plugins meshes all: dxfie

dxfieclean:
	$(MAKE_CLEAN)
dxfie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/impexp/dxf

ifeq ($(USE_PLUGINS),yes)
  DXFIE = $(OUTDLL)dxfie$(DLL)
  LIB.DXFIE = $(foreach d,$(DEP.DXFIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(DXFIE)
else
  DXFIE = $(OUT)$(LIB_PREFIX)dxfie$(LIB)
  DEP.EXE += $(DXFIE)
  SCF.STATIC += dxfie
  TO_INSTALL.STATIC_LIBS += $(DXFIE)
endif

INC.DXFIE = $(wildcard plugins/mesh/impexp/dxf/*.h)
SRC.DXFIE = $(wildcard plugins/mesh/impexp/dxf/*.cpp)
OBJ.DXFIE = $(addprefix $(OUT),$(notdir $(SRC.DXFIE:.cpp=$O)))
DEP.DXFIE = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += DXFIE
DSP.DXFIE.NAME = dxfie
DSP.DXFIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: dxfie dxfieclean
dxfie: $(OUTDIRS) $(DXFIE)

$(DXFIE): $(OBJ.DXFIE) $(LIB.DXFIE)
	$(DO.PLUGIN)

clean: dxfieclean
dxfieclean:
	-$(RM) $(DXFIE) $(OBJ.DXFIE)

ifdef DO_DEPEND
dep: $(OUTOS)dxfie.dep
$(OUTOS)dxfie.dep: $(SRC.DXFIE)
	$(DO.DEP)
else
-include $(OUTOS)dxfie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
