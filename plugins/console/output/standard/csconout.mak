DESCRIPTION.csconout = Crystal Space standard output console

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make csconout     Make the $(DESCRIPTION.csconout)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconout csconoutclean
plugins all: csconout

csconout:
	$(MAKE_TARGET) MAKE_DLL=yes
csconoutclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/plugins/console/output/standard

ifeq ($(USE_PLUGINS),yes)
  CSCONOUT = $(OUTDLL)/csconout$(DLL)
  LIB.CSCONOUT = $(foreach d,$(DEP.CSCONOUT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CSCONOUT)
else
  CSCONOUT = $(OUT)/$(LIB_PREFIX)csconout$(LIB)
  DEP.EXE += $(CSCONOUT)
  SCF.STATIC += csconout
  TO_INSTALL.STATIC_LIBS += $(CSCONOUT)
endif

INF.CSCONOUT = $(SRCDIR)/plugins/console/output/standard/csconout.csplugin
INC.CSCONOUT = $(wildcard $(addprefix $(SRCDIR)/,plugins/console/output/standard/*.h))
SRC.CSCONOUT = $(wildcard $(addprefix $(SRCDIR)/,plugins/console/output/standard/*.cpp))
OBJ.CSCONOUT = $(addprefix $(OUT)/,$(notdir $(SRC.CSCONOUT:.cpp=$O)))
DEP.CSCONOUT = CSUTIL CSGEOM CSUTIL

MSVC.DSP += CSCONOUT
DSP.CSCONOUT.NAME = csconout
DSP.CSCONOUT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconout csconoutclean
csconout: $(OUTDIRS) $(CSCONOUT)

$(CSCONOUT): $(OBJ.CSCONOUT) $(LIB.CSCONOUT)
	$(DO.PLUGIN)

clean: csconoutclean
csconoutclean:
	-$(RMDIR) $(CSCONOUT) $(OBJ.CSCONOUT) $(OUTDLL)/$(notdir $(INF.CSCONOUT))

ifdef DO_DEPEND
dep: $(OUTOS)/csconout.dep
$(OUTOS)/csconout.dep: $(SRC.CSCONOUT)
	$(DO.DEP)
else
-include $(OUTOS)/csconout.dep
endif

endif # ifeq ($(MAKESECTION),targets)
