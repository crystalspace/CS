DESCRIPTION.thing = Thing mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make thing        Make the $(DESCRIPTION.thing)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: thing thingclean
plugins meshes all: thing

thingclean:
	$(MAKE_CLEAN)
thing:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

ifeq ($(USE_PLUGINS),yes)
  THING = $(OUTDLL)/thing$(DLL)
  LIB.THING = $(foreach d,$(DEP.THING),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(THING)
else
  THING = $(OUT)/$(LIB_PREFIX)thing$(LIB)
  DEP.EXE += $(THING)
  SCF.STATIC += thing
  TO_INSTALL.STATIC_LIBS += $(THING)
endif

DIR.THING = plugins/mesh/thing/object
OUT.THING = $(OUT)/$(DIR.THING)
INF.THING = $(SRCDIR)/$(DIR.THING)/thing.csplugin
INC.THING = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.THING)/*.h))
SRC.THING = $(wildcard $(addprefix $(SRCDIR)/,$(DIR.THING)/*.cpp))
OBJ.THING = $(addprefix $(OUT.THING)/,$(notdir $(SRC.THING:.cpp=$O)))
DEP.THING = CSGEOM CSUTIL CSUTIL
CFG.THING = $(SRCDIR)/data/config/thing.cfg

TO_INSTALL.CONFIG += $(CFG.THING)

OUTDIRS += $(OUT.THING)

MSVC.DSP += THING
DSP.THING.NAME = thing
DSP.THING.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: thing thingclean thingcleandep

thing: $(OUTDIRS) $(THING)

$(OUT.THING)/%$O: $(SRCDIR)/$(DIR.THING)/%.cpp
	$(DO.COMPILE.CPP)

$(THING): $(OBJ.THING) $(LIB.THING)
	$(DO.PLUGIN)

clean: thingclean
thingclean:
	-$(RMDIR) $(THING) $(OBJ.THING) $(OUTDLL)/$(notdir $(INF.THING))

cleandep: thingcleandep
thingcleandep:
	-$(RM) $(OUT.THING)/thing.dep

ifdef DO_DEPEND
dep: $(OUT.THING) $(OUT.THING)/thing.dep
$(OUT.THING)/thing.dep: $(SRC.THING)
	$(DO.DEPEND)
else
-include $(OUT.THING)/thing.dep
endif

endif # ifeq ($(MAKESECTION),targets)
