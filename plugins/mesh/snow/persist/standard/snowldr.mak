DESCRIPTION.snowldr = Snow mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make snowldr      Make the $(DESCRIPTION.snowldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: snowldr snowldrclean
plugins meshes all: snowldr

snowldrclean:
	$(MAKE_CLEAN)
snowldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/snow/persist/standard

ifeq ($(USE_PLUGINS),yes)
  SNOWLDR = $(OUTDLL)/snowldr$(DLL)
  LIB.SNOWLDR = $(foreach d,$(DEP.SNOWLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(SNOWLDR)
else
  SNOWLDR = $(OUT)/$(LIB_PREFIX)snowldr$(LIB)
  DEP.EXE += $(SNOWLDR)
  SCF.STATIC += snowldr
  TO_INSTALL.STATIC_LIBS += $(SNOWLDR)
endif

INF.SNOWLDR = $(SRCDIR)/plugins/mesh/snow/persist/standard/snowldr.csplugin
INC.SNOWLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/snow/persist/standard/*.h))
SRC.SNOWLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/snow/persist/standard/*.cpp))
OBJ.SNOWLDR = $(addprefix $(OUT)/,$(notdir $(SRC.SNOWLDR:.cpp=$O)))
DEP.SNOWLDR = CSGEOM CSUTIL CSUTIL

MSVC.DSP += SNOWLDR
DSP.SNOWLDR.NAME = snowldr
DSP.SNOWLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: snowldr snowldrclean
snowldr: $(OUTDIRS) $(SNOWLDR)

$(SNOWLDR): $(OBJ.SNOWLDR) $(LIB.SNOWLDR)
	$(DO.PLUGIN)

clean: snowldrclean
snowldrclean:
	-$(RMDIR) $(SNOWLDR) $(OBJ.SNOWLDR) $(OUTDLL)/$(notdir $(INF.SNOWLDR))

ifdef DO_DEPEND
dep: $(OUTOS)/snowldr.dep
$(OUTOS)/snowldr.dep: $(SRC.SNOWLDR)
	$(DO.DEP)
else
-include $(OUTOS)/snowldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
