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

vpath %.cpp plugins/mesh/thing/object

ifeq ($(USE_PLUGINS),yes)
  THING = $(OUTDLL)thing$(DLL)
  LIB.THING = $(foreach d,$(DEP.THING),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(THING)
else
  THING = $(OUT)$(LIB_PREFIX)thing$(LIB)
  DEP.EXE += $(THING)
  SCF.STATIC += thing
  TO_INSTALL.STATIC_LIBS += $(THING)
endif

INC.THING = $(wildcard plugins/mesh/thing/object/*.h)
SRC.THING = $(wildcard plugins/mesh/thing/object/*.cpp)
OBJ.THING = $(addprefix $(OUT),$(notdir $(SRC.THING:.cpp=$O)))
DEP.THING = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += THING
DSP.THING.NAME = thing
DSP.THING.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: thing thingclean
thing: $(OUTDIRS) $(THING)

$(THING): $(OBJ.THING) $(LIB.THING)
	$(DO.PLUGIN)

clean: thingclean
thingclean:
	-$(RM) $(THING) $(OBJ.THING)

ifdef DO_DEPEND
dep: $(OUTOS)thing.dep
$(OUTOS)thing.dep: $(SRC.THING)
	$(DO.DEP)
else
-include $(OUTOS)thing.dep
endif

endif # ifeq ($(MAKESECTION),targets)
