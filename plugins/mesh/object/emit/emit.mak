DESCRIPTION.emit = Emit mesh object plug-in

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make emit         Make the $(DESCRIPTION.emit)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: emit emitclean
plugins meshes all: emit

emitclean:
	$(MAKE_CLEAN)
emit:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/object/emit plugins/mesh/object/partgen

ifeq ($(USE_PLUGINS),yes)
  EMIT = $(OUTDLL)emit$(DLL)
  LIB.EMIT = $(foreach d,$(DEP.EMIT),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(EMIT)
else
  EMIT = $(OUT)$(LIB_PREFIX)emit$(LIB)
  DEP.EXE += $(EMIT)
  SCF.STATIC += emit
  TO_INSTALL.STATIC_LIBS += $(EMIT)
endif

INC.EMIT = $(wildcard plugins/mesh/object/emit/*.h plugins/mesh/object/partgen/*.h)
SRC.EMIT = $(wildcard plugins/mesh/object/emit/*.cpp plugins/mesh/object/partgen/*.cpp)
OBJ.EMIT = $(addprefix $(OUT),$(notdir $(SRC.EMIT:.cpp=$O)))
DEP.EMIT = CSGEOM CSUTIL CSSYS

MSVC.DSP += EMIT
DSP.EMIT.NAME = emit
DSP.EMIT.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: emit emitclean
emit: $(OUTDIRS) $(EMIT)

$(EMIT): $(OBJ.EMIT) $(LIB.EMIT)
	$(DO.PLUGIN)

clean: emitclean
emitclean:
	-$(RM) $(EMIT) $(OBJ.EMIT)

ifdef DO_DEPEND
dep: $(OUTOS)emit.dep
$(OUTOS)emit.dep: $(SRC.EMIT)
	$(DO.DEP)
else
-include $(OUTOS)emit.dep
endif

endif # ifeq ($(MAKESECTION),targets)
