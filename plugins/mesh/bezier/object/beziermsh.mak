DESCRIPTION.bezier = Bezier mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make bezier       Make the $(DESCRIPTION.bezier)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: bezier bezierclean
plugins meshes all: bezier

bezierclean:
	$(MAKE_CLEAN)
bezier:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/mesh/bezier/object

ifeq ($(USE_PLUGINS),yes)
  BEZIER = $(OUTDLL)/bezier$(DLL)
  LIB.BEZIER = $(foreach d,$(DEP.BEZIER),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(BEZIER)
else
  BEZIER = $(OUT)/$(LIB_PREFIX)bezier$(LIB)
  DEP.EXE += $(BEZIER)
  SCF.STATIC += bezier
  TO_INSTALL.STATIC_LIBS += $(BEZIER)
endif

INF.BEZIER = $(SRCDIR)/plugins/mesh/bezier/object/bezier.csplugin
INC.BEZIER = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/bezier/object/*.h))
SRC.BEZIER = $(wildcard $(addprefix $(SRCDIR)/,plugins/mesh/bezier/object/*.cpp))
OBJ.BEZIER = $(addprefix $(OUT)/,$(notdir $(SRC.BEZIER:.cpp=$O)))
DEP.BEZIER = CSGEOM CSUTIL CSUTIL

MSVC.DSP += BEZIER
DSP.BEZIER.NAME = bezier
DSP.BEZIER.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: bezier bezierclean
bezier: $(OUTDIRS) $(BEZIER)

$(BEZIER): $(OBJ.BEZIER) $(LIB.BEZIER)
	$(DO.PLUGIN)

clean: bezierclean
bezierclean:
	-$(RMDIR) $(BEZIER) $(OBJ.BEZIER) $(OUTDLL)/$(notdir $(INF.BEZIER))

ifdef DO_DEPEND
dep: $(OUTOS)/bezier.dep
$(OUTOS)/bezier.dep: $(SRC.BEZIER)
	$(DO.DEP)
else
-include $(OUTOS)/bezier.dep
endif

endif # ifeq ($(MAKESECTION),targets)
