DESCRIPTION.emitldr = Emit mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make emitldr      Make the $(DESCRIPTION.emitldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: emitldr emitldrclean
plugins meshes all: emitldr

emitldrclean:
	$(MAKE_CLEAN)
emitldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/emit/persist/classic

ifeq ($(USE_PLUGINS),yes)
  EMITLDR = $(OUTDLL)emitldr$(DLL)
  LIB.EMITLDR = $(foreach d,$(DEP.EMITLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(EMITLDR)
else
  EMITLDR = $(OUT)$(LIB_PREFIX)emitldr$(LIB)
  DEP.EXE += $(EMITLDR)
  SCF.STATIC += emitldr
  TO_INSTALL.STATIC_LIBS += $(EMITLDR)
endif

INC.EMITLDR = $(wildcard plugins/mesh/emit/persist/classic/*.h)
SRC.EMITLDR = $(wildcard plugins/mesh/emit/persist/classic/*.cpp)
OBJ.EMITLDR = $(addprefix $(OUT),$(notdir $(SRC.EMITLDR:.cpp=$O)))
DEP.EMITLDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += EMITLDR
DSP.EMITLDR.NAME = emitldr
DSP.EMITLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: emitldr emitldrclean
emitldr: $(OUTDIRS) $(EMITLDR)

$(EMITLDR): $(OBJ.EMITLDR) $(LIB.EMITLDR)
	$(DO.PLUGIN)

clean: emitldrclean
emitldrclean:
	-$(RM) $(EMITLDR) $(OBJ.EMITLDR)

ifdef DO_DEPEND
dep: $(OUTOS)emitldr.dep
$(OUTOS)emitldr.dep: $(SRC.EMITLDR)
	$(DO.DEP)
else
-include $(OUTOS)emitldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
