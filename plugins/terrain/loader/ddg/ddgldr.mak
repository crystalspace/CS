DESCRIPTION.ddgldr = DDG Terrain object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make ddgldr       Make the $(DESCRIPTION.ddgldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: ddgldr ddgldrclean
plugins all: ddgldr

ddgldrclean:
	$(MAKE_CLEAN)
ddgldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/terrain/loader/ddg

ifeq ($(USE_PLUGINS),yes)
  DDGLDR = $(OUTDLL)ddgldr$(DLL)
  LIB.DDGLDR = $(foreach d,$(DEP.DDGLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(DDGLDR)
else
  DDGLDR = $(OUT)$(LIB_PREFIX)ddgldr$(LIB)
  DEP.EXE += $(DDGLDR)
  SCF.STATIC += ddgldr
  TO_INSTALL.STATIC_LIBS += $(DDGLDR)
endif

INC.DDGLDR = $(wildcard plugins/terrain/loader/ddg/*.h)
SRC.DDGLDR = $(wildcard plugins/terrain/loader/ddg/*.cpp)
OBJ.DDGLDR = $(addprefix $(OUT),$(notdir $(SRC.DDGLDR:.cpp=$O)))
DEP.DDGLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += DDGLDR
DSP.DDGLDR.NAME = ddgldr
DSP.DDGLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: ddgldr ddgldrclean
ddgldr: $(OUTDIRS) $(DDGLDR)

$(DDGLDR): $(OBJ.DDGLDR) $(LIB.DDGLDR)
	$(DO.PLUGIN)

clean: ddgldrclean
ddgldrclean:
	-$(RM) $(DDGLDR) $(OBJ.DDGLDR)

ifdef DO_DEPEND
dep: $(OUTOS)ddgldr.dep
$(OUTOS)ddgldr.dep: $(SRC.DDGLDR)
	$(DO.DEP)
else
-include $(OUTOS)ddgldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
