DESCRIPTION.exploldr = Explosion mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make exploldr     Make the $(DESCRIPTION.exploldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: exploldr exploldrclean
plugins meshes all: exploldr

exploldrclean:
	$(MAKE_CLEAN)
exploldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/loader/explo

ifeq ($(USE_PLUGINS),yes)
  EXPLOLDR = $(OUTDLL)exploldr$(DLL)
  LIB.EXPLOLDR = $(foreach d,$(DEP.EXPLOLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(EXPLOLDR)
else
  EXPLOLDR = $(OUT)$(LIB_PREFIX)exploldr$(LIB)
  DEP.EXE += $(EXPLOLDR)
  SCF.STATIC += exploldr
  TO_INSTALL.STATIC_LIBS += $(EXPLOLDR)
endif

INC.EXPLOLDR = $(wildcard plugins/mesh/loader/explo/*.h)
SRC.EXPLOLDR = $(wildcard plugins/mesh/loader/explo/*.cpp)
OBJ.EXPLOLDR = $(addprefix $(OUT),$(notdir $(SRC.EXPLOLDR:.cpp=$O)))
DEP.EXPLOLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += EXPLOLDR
DSP.EXPLOLDR.NAME = exploldr
DSP.EXPLOLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: exploldr exploldrclean
exploldr: $(OUTDIRS) $(EXPLOLDR)

$(EXPLOLDR): $(OBJ.EXPLOLDR) $(LIB.EXPLOLDR)
	$(DO.PLUGIN)

clean: exploldrclean
exploldrclean:
	-$(RM) $(EXPLOLDR) $(OBJ.EXPLOLDR)

ifdef DO_DEPEND
dep: $(OUTOS)exploldr.dep
$(OUTOS)exploldr.dep: $(SRC.EXPLOLDR)
	$(DO.DEP)
else
-include $(OUTOS)exploldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
