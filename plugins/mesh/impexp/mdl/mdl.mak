DESCRIPTION.mdlie = MDL Import/Export plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make mdlie        Make the $(DESCRIPTION.mdlie)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mdlie mdlieclean
plugins meshes all: mdlie

mdlieclean:
	$(MAKE_CLEAN)
mdlie:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/impexp/mdl

ifeq ($(USE_PLUGINS),yes)
  MDLIE = $(OUTDLL)/mdlie$(DLL)
  LIB.MDLIE = $(foreach d,$(DEP.MDLIE),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MDLIE)
else
  MDLIE = $(OUT)/$(LIB_PREFIX)mdlie$(LIB)
  DEP.EXE += $(MDLIE)
  SCF.STATIC += mdlie
  TO_INSTALL.STATIC_LIBS += $(MDLIE)
endif

INF.MDLIE = $(SRCDIR)/plugins/mesh/impexp/mdl/mdlie.csplugin
INC.MDLIE = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/impexp/mdl/*.h))
SRC.MDLIE = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/impexp/mdl/*.cpp))
OBJ.MDLIE = $(addprefix $(OUT)/,$(notdir $(SRC.MDLIE:.cpp=$O)))
DEP.MDLIE = CSGEOM CSUTIL CSUTIL CSTOOL CSUTIL CSGEOM

MSVC.DSP += MDLIE
DSP.MDLIE.NAME = mdlie
DSP.MDLIE.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mdlie mdlieclean
mdlie: $(OUTDIRS) $(MDLIE)

$(MDLIE): $(OBJ.MDLIE) $(LIB.MDLIE)
	$(DO.PLUGIN)

clean: mdlieclean
mdlieclean:
	-$(RMDIR) $(MDLIE) $(OBJ.MDLIE) $(OUTDLL)/$(notdir $(INF.MDLIE))

ifdef DO_DEPEND
dep: $(OUTOS)/mdlie.dep
$(OUTOS)/mdlie.dep: $(SRC.MDLIE)
	$(DO.DEP)
else
-include $(OUTOS)/mdlie.dep
endif

endif # ifeq ($(MAKESECTION),targets)
