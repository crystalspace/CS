DESCRIPTION.thingldr = Thing mesh object loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)echo $"  make thingldr     Make the $(DESCRIPTION.thingldr)$"

endif # ifeq ($(MAKESECTION),rootdefines)
#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: thingldr thingldrclean
plugins meshes all: thingldr

thingldrclean:
	$(MAKE_CLEAN)
thingldr:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)
#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/mesh/loader/thing

ifeq ($(USE_PLUGINS),yes)
  THINGLDR = $(OUTDLL)thingldr$(DLL)
  LIB.THINGLDR = $(foreach d,$(DEP.THINGLDR),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(THINGLDR)
else
  THINGLDR = $(OUT)$(LIB_PREFIX)thingldr$(LIB)
  DEP.EXE += $(THINGLDR)
  SCF.STATIC += thingldr
  TO_INSTALL.STATIC_LIBS += $(THINGLDR)
endif

INC.THINGLDR = $(wildcard plugins/mesh/loader/thing/*.h)
SRC.THINGLDR = $(wildcard plugins/mesh/loader/thing/*.cpp)
OBJ.THINGLDR = $(addprefix $(OUT),$(notdir $(SRC.THINGLDR:.cpp=$O)))
DEP.THINGLDR = CSGEOM CSUTIL CSSYS

MSVC.DSP += THINGLDR
DSP.THINGLDR.NAME = thingldr
DSP.THINGLDR.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)
#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: thingldr thingldrclean
thingldr: $(OUTDIRS) $(THINGLDR)

$(THINGLDR): $(OBJ.THINGLDR) $(LIB.THINGLDR)
	$(DO.PLUGIN)

clean: thingldrclean
thingldrclean:
	-$(RM) $(THINGLDR) $(OBJ.THINGLDR)

ifdef DO_DEPEND
dep: $(OUTOS)thingldr.dep
$(OUTOS)thingldr.dep: $(SRC.THINGLDR)
	$(DO.DEP)
else
-include $(OUTOS)thingldr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
