DESCRIPTION.terrfldr = TerrFunc Terrain object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make terrfldr     Make the $(DESCRIPTION.terrfldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: terrfldr terrfldrclean
plugins all: terrfldr

terrfldrclean:
	$(MAKE_CLEAN)
terrfldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/terrain/loader/terrfunc

ifeq ($(USE_PLUGINS),yes)
  TERRFLDR = $(OUTDLL)terrfldr$(DLL)
  LIB.TERRFLDR = $(foreach d,$(DEP.TERRFLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(TERRFLDR)
else
  TERRFLDR = $(OUT)$(LIB_PREFIX)terrfldr$(LIB)
  DEP.EXE += $(TERRFLDR)
  SCF.STATIC += terrfldr
  TO_INSTALL.STATIC_LIBS += $(TERRFLDR)
endif

INC.TERRFLDR = $(wildcard plugins/terrain/loader/terrfunc/*.h)
SRC.TERRFLDR = $(wildcard plugins/terrain/loader/terrfunc/*.cpp)
OBJ.TERRFLDR = $(addprefix $(OUT),$(notdir $(SRC.TERRFLDR:.cpp=$O)))
DEP.TERRFLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += TERRFLDR
DSP.TERRFLDR.NAME = terrfldr
DSP.TERRFLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: terrfldr terrfldrclean
terrfldr: $(OUTDIRS) $(TERRFLDR)

$(TERRFLDR): $(OBJ.TERRFLDR) $(LIB.TERRFLDR)
	$(DO.PLUGIN)

clean: terrfldrclean
terrfldrclean:
	-$(RM) $(TERRFLDR) $(OBJ.TERRFLDR)

ifdef DO_DEPEND
dep: $(OUTOS)terrfldr.dep
$(OUTOS)terrfldr.dep: $(SRC.TERRFLDR)
	$(DO.DEP)
else
-include $(OUTOS)terrfldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
