DESCRIPTION.cubeldr = Cube mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make cubeldr      Make the $(DESCRIPTION.cubeldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cubeldr cubeldrclean
plugins meshes all: cubeldr

cubeldrclean:
	$(MAKE_CLEAN)
cubeldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/cube/persist/classic

ifeq ($(USE_PLUGINS),yes)
  CUBELDR = $(OUTDLL)cubeldr$(DLL)
  LIB.CUBELDR = $(foreach d,$(DEP.CUBELDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(CUBELDR)
else
  CUBELDR = $(OUT)$(LIB_PREFIX)cubeldr$(LIB)
  DEP.EXE += $(CUBELDR)
  SCF.STATIC += cubeldr
  TO_INSTALL.STATIC_LIBS += $(CUBELDR)
endif

INC.CUBELDR = $(wildcard plugins/mesh/cube/persist/classic/*.h)
SRC.CUBELDR = $(wildcard plugins/mesh/cube/persist/classic/*.cpp)
OBJ.CUBELDR = $(addprefix $(OUT),$(notdir $(SRC.CUBELDR:.cpp=$O)))
DEP.CUBELDR = CSGEOM CSUTIL CSSYS CSUTIL

MSVC.DSP += CUBELDR
DSP.CUBELDR.NAME = cubeldr
DSP.CUBELDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cubeldr cubeldrclean
cubeldr: $(OUTDIRS) $(CUBELDR)

$(CUBELDR): $(OBJ.CUBELDR) $(LIB.CUBELDR)
	$(DO.PLUGIN)

clean: cubeldrclean
cubeldrclean:
	-$(RM) $(CUBELDR) $(OBJ.CUBELDR)

ifdef DO_DEPEND
dep: $(OUTOS)cubeldr.dep
$(OUTOS)cubeldr.dep: $(SRC.CUBELDR)
	$(DO.DEP)
else
-include $(OUTOS)cubeldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
