DESCRIPTION.bezierldr = Bezier mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make bezierldr    Make the $(DESCRIPTION.bezierldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bezierldr bezierldrclean
plugins meshes all: bezierldr

bezierldrclean:
	$(MAKE_CLEAN)
bezierldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/bezier/persist/standard

ifeq ($(USE_PLUGINS),yes)
  BEZIERLDR = $(OUTDLL)/bezierldr$(DLL)
  LIB.BEZIERLDR = $(foreach d,$(DEP.BEZIERLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BEZIERLDR)
else
  BEZIERLDR = $(OUT)/$(LIB_PREFIX)bezierldr$(LIB)
  DEP.EXE += $(BEZIERLDR)
  SCF.STATIC += bezierldr
  TO_INSTALL.STATIC_LIBS += $(BEZIERLDR)
endif

INF.BEZIERLDR = $(SRCDIR)/plugins/mesh/bezier/persist/standard/bezierldr.csplugin
INC.BEZIERLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/bezier/persist/standard/*.h))
SRC.BEZIERLDR = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/bezier/persist/standard/*.cpp))
OBJ.BEZIERLDR = $(addprefix $(OUT)/,$(notdir $(SRC.BEZIERLDR:.cpp=$O)))
DEP.BEZIERLDR = CSGEOM CSUTIL CSUTIL

MSVC.DSP += BEZIERLDR
DSP.BEZIERLDR.NAME = bezierldr
DSP.BEZIERLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bezierldr bezierldrclean
bezierldr: $(OUTDIRS) $(BEZIERLDR)

$(BEZIERLDR): $(OBJ.BEZIERLDR) $(LIB.BEZIERLDR)
	$(DO.PLUGIN)

clean: bezierldrclean
bezierldrclean:
	-$(RMDIR) $(BEZIERLDR) $(OBJ.BEZIERLDR) $(OUTDLL)/$(notdir $(INF.BEZIERLDR))

ifdef DO_DEPEND
dep: $(OUTOS)/bezierldr.dep
$(OUTOS)/bezierldr.dep: $(SRC.BEZIERLDR)
	$(DO.DEP)
else
-include $(OUTOS)/bezierldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
